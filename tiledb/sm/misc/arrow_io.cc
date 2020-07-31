/**
 * @file   arrow_io.cc
 *
 * @section LICENSE
 *
 * The MIT License
 *
 * @copyright Copyright (c) 2017-2020 TileDB, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * @section DESCRIPTION
 *
 * This file defines TileDB interoperation functionality with Apache Arrow.
 */

#include <memory>

#include "arrow_cdefs.h"
#include "arrow_io.h"

/*
 * Error helper: raises exception with line number context
 */
using _TileDBError = tiledb::TileDBError;
#ifndef NDEBUG
#define TDB_LERROR(m)                                                     \
  _TileDBError(                                                           \
      std::string(m) + " (" + __FILE__ + ":" + std::to_string(__LINE__) + \
      ")");
#else
#define TDB_LERROR tiledb::TileDBError
#endif

namespace tiledb {
namespace arrow {

using Status = tiledb::sm::Status;

/* ****************************** */
/*       Helper types             */
/* ****************************** */

// Arrow format and representation
struct ArrowInfo {
  std::string fmt_;
  std::string rep_;

  ArrowInfo(std::string fmt, std::string rep = std::string())
      : fmt_(fmt)
      , rep_(rep){};
};

// TileDB type information
struct TypeInfo {
  tiledb_datatype_t type;
  uint64_t elem_size;
  uint32_t cell_val_num;

  // is this represented as "Arrow large"
  bool arrow_large;
};

struct BufferInfo {
 public:
  TypeInfo tdbtype;
  bool is_var;
  uint64_t elem_num;    // element count
  void* data;           // data pointer
  uint64_t offset_num;  // # offsets, always uint64_t
  uint64_t* offsets;
  uint64_t elem_size;
};

/* ****************************** */
/*        Type conversions        */
/* ****************************** */

// Get Arrow format from TileDB BufferInfo
ArrowInfo tiledb_buffer_arrow_fmt(BufferInfo bufferinfo, bool use_list = true) {
  auto typeinfo = bufferinfo.tdbtype;
  auto cell_val_num = typeinfo.cell_val_num;

  // TODO support List<T> for simple scalar T
  (void)use_list;
  /*
  if (use_list && cell_val_num == TILEDB_VAR_NUM) {
    switch(typeinfo.type) {
      case TILEDB_STRING_UTF8:
      case TILEDB_STRING_ASCII:
        break;
      case TILEDB_INT8:
      case TILEDB_INT16:
      case TILEDB_INT32:
      case TILEDB_INT64:
      case TILEDB_UINT8:
      case TILEDB_UINT16:
      case TILEDB_UINT32:
      case TILEDB_UINT64:
      case TILEDB_FLOAT32:
      case TILEDB_FLOAT64:
        return ArrowInfo("+l");
      default:
        throw TDB_LERROR(
          "TileDB-Arrow: List<T> translation not yet supported for var-length
  TileDB type ('"
          + tiledb::impl::type_to_str(typeinfo.type) + "')");
    }
  }
  */

  switch (typeinfo.type) {
    ////////////////////////////////////////////////////////////////////////
    // NOTE: should export as arrow's "large utf8" because TileDB offsets
    //       are natively uint64_t. However: our offset buffers are 1 elem
    //       short of the expected length for an arrow offset buffer.
    //       For now, we transform to int32_it in place and export "u"
    //       TODO: check that the length does not exceed INT32_MAX
    case TILEDB_STRING_ASCII:
    case TILEDB_STRING_UTF8:
      return ArrowInfo("u");
    case TILEDB_CHAR:
      return ArrowInfo("z");
      ////////////////////////////////////////////////////////////////////////

    case TILEDB_INT32:
      return ArrowInfo("i");
    case TILEDB_INT64:
      return ArrowInfo("l");
    case TILEDB_FLOAT32:
      return ArrowInfo("f");
    case TILEDB_FLOAT64:
      return ArrowInfo("g");
    case TILEDB_INT8:
      return ArrowInfo("c");
    case TILEDB_UINT8:
      return ArrowInfo("C");
    case TILEDB_INT16:
      return ArrowInfo("s");
    case TILEDB_UINT16:
      return ArrowInfo("S");
    case TILEDB_UINT32:
      return ArrowInfo("I");
    case TILEDB_UINT64:
      return ArrowInfo("L");

    case TILEDB_DATETIME_NS:
      return ArrowInfo("ttn");
    case TILEDB_DATETIME_MS:
      return ArrowInfo("tdm");

    // TODO: these can be represented w/ additional metadata but it is
    //       language API-specific.
    case TILEDB_DATETIME_YEAR:
    case TILEDB_DATETIME_MONTH:
    case TILEDB_DATETIME_WEEK:
    case TILEDB_DATETIME_DAY:
    case TILEDB_DATETIME_HR:
    case TILEDB_DATETIME_MIN:
    case TILEDB_DATETIME_SEC:
    case TILEDB_DATETIME_US:
    case TILEDB_DATETIME_PS:
    case TILEDB_DATETIME_FS:
    case TILEDB_DATETIME_AS:
    case TILEDB_STRING_UTF16:
    case TILEDB_STRING_UTF32:
    case TILEDB_STRING_UCS2:
    case TILEDB_STRING_UCS4:
    case TILEDB_ANY:
    default:
      break;
  }
  throw TDB_LERROR(
      "TileDB-Arrow: tiledb datatype not understood ('" +
      tiledb::impl::type_to_str(typeinfo.type) +
      "', cell_val_num: " + std::to_string(cell_val_num) + ")");
}

TypeInfo tiledb_dt_info(const tiledb::ArraySchema& schema, std::string name) {
  if (schema.has_attribute(name)) {
    auto attr = schema.attribute(name);

    return (TypeInfo){.type = attr.type(),
                      .elem_size = tiledb::impl::type_size(attr.type()),
                      .cell_val_num = attr.cell_val_num()};

  } else if (schema.domain().has_dimension(name)) {
    auto dom = schema.domain();
    auto dim = dom.dimension(name);

    return (TypeInfo){.type = dim.type(),
                      .elem_size = tiledb::impl::type_size(dim.type()),
                      .cell_val_num = dim.cell_val_num()};

  } else {
    throw TDB_LERROR("Schema does not have attribute named '" + name + "'");
  }
}

TypeInfo arrow_type_to_tiledb(ArrowSchema* arw_schema) {
  auto fmt = std::string(arw_schema->format);
  bool large = false;
  if (fmt == "+l") {
    large = false;
    assert(arw_schema->n_children == 1);
    arw_schema = arw_schema->children[0];
  } else if(fmt == "+L") {
    large = true;
    assert(arw_schema->n_children == 1);
    arw_schema = arw_schema->children[0];
  }

  if (fmt == "i")
    return {TILEDB_INT32, 4, 1, large};
  else if (fmt == "l")
    return {TILEDB_INT64, 8, 1, large};
  else if (fmt == "f")
    return {TILEDB_FLOAT32, 4, 1, large};
  else if (fmt == "g")
    return {TILEDB_FLOAT64, 8, 1, large};
  else if (fmt == "c")
    return {TILEDB_INT8, 1, 1, large};
  else if (fmt == "C")
    return {TILEDB_UINT8, 1, 1, large};
  else if (fmt == "s")
    return {TILEDB_INT16, 2, 1, large};
  else if (fmt == "S")
    return {TILEDB_UINT16, 2, 1, large};
  else if (fmt == "I")
    return {TILEDB_UINT32, 4, 1, large};
  else if (fmt == "L")
    return {TILEDB_UINT64, 8, 1, large};
  else if (fmt == "z" || fmt == "Z")
    return {TILEDB_CHAR, 1, TILEDB_VAR_NUM, fmt == "Z"};
  else if (fmt == "u" || fmt == "U")
    return {TILEDB_STRING_UTF8, 1, TILEDB_VAR_NUM, fmt == "U"};
  else
    throw tiledb::TileDBError("[TileDB-Arrow]: Unknown or unsupported Arrow format string '" + fmt + "'");
}

/* ****************************** */
/*        Helper functions        */
/* ****************************** */

// Reorder offsets into Arrow-compatible format
// NOTE: this currently hard-codes an inplace conversion from
//       uint64 (TileDB) to int32 ("arrow small") only
void offsets_to_arrow(BufferInfo binfo) {
  size_t elem_size = binfo.elem_size;

  uint64_t* offsets = binfo.offsets;
  int32_t* offsets_i32 = (int32_t*)binfo.offsets;

  size_t idx = 1;  // important: don't divide by 0
  while (idx < binfo.offset_num) {
    // we must check for zeros here to handle a run of empty
    // values (zero-length) at the start of the buffer
    // note that given the i32 conversion, we can simply
    // skip the assignment because
    //   offsets[idx] == 0 => offsets_i32[i:i+1] == 0
    if (offsets[idx] != 0)
      offsets_i32[idx] = offsets[idx] / elem_size;
    ++idx;
  }
  offsets_i32[idx] = binfo.elem_num;
}

void check_arrow_schema(ArrowSchema* arw_schema) {
  if (arw_schema == nullptr)
    TDB_LERROR("[ArrowIO]: Invalid ArrowSchema object!");

  // sanity check the arrow schema
  if (arw_schema->release == nullptr)
    TDB_LERROR(
        "[ArrowIO]: Invalid ArrowSchema: cannot import released schema.");
  if (arw_schema->format != std::string("+s"))
    TDB_LERROR("[ArrowIO]: Unsupported ArrowSchema: must be struct (+s).");
  if (arw_schema->n_children < 1)
    TDB_LERROR("[ArrowIO]: Unsupported ArrowSchema with 0 children.");
  if (arw_schema->children == nullptr)
    TDB_LERROR(
        "[ArrowIO]: Invalid ArrowSchema with n_children>0 and children==NULL");
}

/* ****************************** */
/*  Arrow C API Struct wrappers   */
/* ****************************** */

// NOTE: These structs manage the lifetime of the contained C structs.
// CAUTION: they do *not* manage the lifetime of the underlying buffers.

struct CPPArrowSchema {
  CPPArrowSchema(
      std::string name,
      std::string format,
      std::string metadata,
      int64_t flags,
      std::vector<ArrowSchema*> children,
      std::shared_ptr<CPPArrowSchema> dictionary)
      : schema_((ArrowSchema*)malloc(sizeof(ArrowSchema)))
      , format_(format)
      , name_(name)
      , metadata_(metadata)
      , children_(children)
      , dictionary_(dictionary) {
    flags_ = flags;
    n_children_ = children.size();

    // Initialize ArrowSchema with data *owned by this object*
    *schema_ =
        (struct ArrowSchema){// Type description
                             .format = format_.c_str(),
                             .name = name_.c_str(),
                             .metadata = metadata.c_str(),
                             .flags = flags,
                             .n_children = n_children_,

                             // Cross-refs
                             .children = nullptr,
                             .dictionary = nullptr,
                             .private_data = this,

                             // Release callback
                             .release = ([](ArrowSchema* this_p) {
                               delete (CPPArrowSchema*)(this_p->private_data);
                             })

        };

    if (n_children_ > 0) {
      schema_->children = (ArrowSchema**)children.data();
    }

    if (dictionary) {
      schema_->dictionary = dictionary.get()->ptr();
    }
  }

  ArrowSchema* mutable_ptr() {
    return schema_;
  }

  ArrowSchema* ptr() const {
    return schema_;
  }

 private:
  ArrowSchema* schema_;
  std::string format_;
  std::string name_;
  std::string metadata_;
  int64_t flags_;
  int64_t n_children_;
  std::vector<ArrowSchema*> children_;
  std::shared_ptr<CPPArrowSchema> dictionary_;
};

struct CPPArrowArray {
  ~CPPArrowArray() {
    std::cout << "deleted" << std::endl;
  }

  CPPArrowArray(
      int64_t elem_num,
      int64_t null_num,
      int64_t offset,
      std::vector<std::shared_ptr<CPPArrowArray>> children,
      std::shared_ptr<CPPArrowArray> dictionary,
      std::vector<void*> buffers)
      : array_((ArrowArray*)malloc(sizeof(ArrowArray))) {
    (void)dictionary;

    *array_ =
        (struct ArrowArray){// Data description
                            .length = elem_num,
                            .offset = offset,
                            .null_count = null_num,
                            .n_buffers = (int64_t)buffers.size(),
                            .n_children = (int64_t)children.size(),
                            .dictionary = NULL,
                            .children = NULL,
                            // Bookkeeping
                            .private_data = this,
                            .release = ([](ArrowArray* this_p) {
                              delete (CPPArrowArray*)(this_p->private_data);
                            })};
    buffers_ = buffers;
    array_->buffers = (const void**)buffers_.data();
  }

  ArrowArray* ptr() const {
    return array_;
  }

  ArrowArray* mutable_ptr() {
    return array_;
  }

 private:
  ArrowArray* array_;
  std::vector<void*> buffers_;
};

/* ****************************** */
/*         Arrow Importer         */
/* ****************************** */

class ArrowImporter {
 public:
  ArrowImporter(std::shared_ptr<tiledb::Query> query);
  ~ArrowImporter();

  void import_(std::string name, ArrowSchema* schema, ArrowArray* array);

 private:
  std::shared_ptr<tiledb::Query> query_;
  std::vector<void*> offset_buffers_; //

};  // class ArrowExporter

ArrowImporter::ArrowImporter(std::shared_ptr<tiledb::Query> query) {
  query_ = query;
}

ArrowImporter::~ArrowImporter() {
  for (auto p : offset_buffers_) {
    free(p);
  }
}

void ArrowImporter::import_(
    std::string name, ArrowSchema* arw_schema, ArrowArray* arw_array) {

  auto typeinfo = arrow_type_to_tiledb(arw_schema);

  // buffer conversion

  if (typeinfo.cell_val_num == TILEDB_VAR_NUM) {
    assert(arw_array->n_buffers == 3);

    const void* p_offsets_arw = (void*)arw_array->buffers[1]; // note: cast away const
    const void* p_data = (void*)arw_array->buffers[2]; // note: cast away const
    uint64_t data_num = arw_array->length;
    uint64_t data_nbytes = 0;

    uint64_t* p_offsets = (uint64_t*)malloc(arw_array->length * sizeof(uint64_t) + 1);
    offset_buffers_.push_back((void*)p_offsets);
    if (typeinfo.arrow_large) {
      int64_t* p_offsets_int64 = (int64_t*)p_offsets_arw;
      for (size_t i = 0; i < (size_t)data_num; i++) {
        p_offsets[i] = p_offsets_int64[i] * typeinfo.elem_size;
      }
      data_nbytes = p_offsets_int64[data_num] * typeinfo.elem_size;
    } else {
      int32_t* p_offsets_int32 = (int32_t*)p_offsets_arw;
      // the Arrow offsets buffer has length+1 elements
      for (size_t i = 0; i < (size_t)data_num; i++) {
        p_offsets[i] = p_offsets_int32[i] * typeinfo.elem_size;
      }
      data_nbytes = p_offsets_int32[data_num] * typeinfo.elem_size;
    }
    query_->set_buffer(name, (uint64_t*)p_offsets, data_num,
                       (void*)p_data, data_nbytes);
  } else {
    assert(arw_array->n_buffers == 2);

    const void* p_data = (void*)arw_array->buffers[1]; // note: cast away const
    uint64_t data_num = arw_array->length;

    query_->set_buffer(name, (void*)p_data, data_num);
  }
}

/* ****************************** */
/*         Arrow Exporter         */
/* ****************************** */

class ArrowExporter {
 public:
  ArrowExporter(std::shared_ptr<tiledb::Query> query);

  void export_(std::string name, ArrowSchema** schema, ArrowArray** array);

  BufferInfo buffer_info(std::string name);

  std::unique_ptr<CPPArrowSchema> buffer_schema(std::string name);

 private:
  std::shared_ptr<tiledb::Query> query_;
};

// ArrowExporter implementation
ArrowExporter::ArrowExporter(std::shared_ptr<tiledb::Query> query) {
  query_ = query;
}

BufferInfo ArrowExporter::buffer_info(std::string name) {
  void* data;
  uint64_t* data_nbytes;
  uint64_t* offsets;
  uint64_t* offsets_nelem;
  uint64_t elem_size;

  auto typeinfo = tiledb_dt_info(query_->schema(), name);

  auto result_elts = query_->result_buffer_elements();
  auto result_elt_iter = result_elts.find(name);
  if (result_elt_iter == result_elts.end()) {
    TDB_LERROR("No results found for attribute '" + name + "'");
  }

  bool is_var = (result_elt_iter->second.first != 0);

  // NOTE: result sizes are in bytes
  if (is_var) {
    query_->get_buffer(
        name, &offsets, &offsets_nelem, &data, &data_nbytes, &elem_size);
    // convert offsets bytes to elements
    *offsets_nelem = *offsets_nelem / sizeof(uint64_t);
  } else {
    query_->get_buffer(name, &data, &data_nbytes, &elem_size);
  }

  int nelem_rem = (*data_nbytes) % elem_size;
  assert(nelem_rem == 0);

  return BufferInfo({typeinfo,
                     is_var,
                     (*data_nbytes / elem_size),
                     data,
                     (is_var ? *offsets_nelem : 1),
                     offsets,
                     elem_size});
}

int64_t flags_for_buffer(BufferInfo binfo) {
  /*  TODO, use these defs from arrow_cdefs.h -- currently not applicable.
      #define ARROW_FLAG_DICTIONARY_ORDERED 1
      #define ARROW_FLAG_NULLABLE 2
      #define ARROW_FLAG_MAP_KEYS_SORTED 4
  */
  (void)binfo;
  return 0;
}

void ArrowExporter::export_(
    std::string name, ArrowSchema** schema, ArrowArray** array) {
  auto bufferinfo = this->buffer_info(name);
  // TODO add checks?:
  // -  .elem_num < INT64_MAX
  // - check that the length of a var-len element does not exceed INT32_MAX
  //   currently we export as "u" and "b" (int32 offsets) rather than
  //   "U and "B" (int64 offsets)

  if (schema == nullptr || array == nullptr) {
    throw tiledb::TileDBError(
        "ArrowExporter: received invalid pointer to output array or schema.");
  }

  std::cout << "bufferinfo.elem_num: " << bufferinfo.elem_num << std::endl
            << "bufferinfo.data: " << bufferinfo.data << std::endl;

  auto arrow_fmt = tiledb_buffer_arrow_fmt(bufferinfo);
  auto arrow_flags = flags_for_buffer(bufferinfo);

  // lifetime:
  //   - address is stored in ArrowSchema.private_data
  //   - delete is called by lambda stored in ArrowSchema.release
  CPPArrowSchema* cpp_schema =
      new CPPArrowSchema(name, arrow_fmt.fmt_, "", arrow_flags, {}, {});

  std::vector<void*> buffers;
  if (bufferinfo.is_var) {
    offsets_to_arrow(bufferinfo);  // convert in-place
    buffers = {NULL, bufferinfo.offsets, bufferinfo.data};
  } else {
    cpp_schema =
        new CPPArrowSchema(name, arrow_fmt.fmt_, "", arrow_flags, {}, {});
    buffers = {NULL, bufferinfo.data};
  }
  (*schema) = cpp_schema->ptr();

  auto cpp_arrow_array = new CPPArrowArray(
      (bufferinfo.is_var ? bufferinfo.offset_num : bufferinfo.elem_num),
      0,  // null_num
      0,
      {},
      {},
      buffers);
  (*array) = cpp_arrow_array->ptr();
}

ArrowAdapter::ArrowAdapter(std::shared_ptr<tiledb::Query> query) {
  importer_ = new ArrowImporter(query);
  if (!importer_) {
    throw tiledb::TileDBError("[TileDB-Arrow] Failed to allocate ArrowImporter!");
  }
  exporter_ = new ArrowExporter(query);
  if (!exporter_) {
    delete importer_;
    throw tiledb::TileDBError("[TileDB-Arrow] Failed to allocate ArrowImporter!");
  }
}

void ArrowAdapter::export_buffer(const char* name, void** arrow_schema, void** arrow_array) {
  exporter_->export_(name, (ArrowSchema**)arrow_schema, (ArrowArray**)arrow_array);
}

void ArrowAdapter::import_buffer(const char* name, void* arrow_schema, void* arrow_array) {
  importer_->import_(name, (ArrowSchema*)arrow_schema, (ArrowArray*)arrow_array);
}

ArrowAdapter::~ArrowAdapter() {
  delete importer_;
  delete exporter_;
}

Status query_get_buffer_arrow_array(
  std::shared_ptr<Query> query,
  std::string name,
  void** v_arw_schema,
  void** v_arw_array) {

  ArrowExporter exporter(query);

  exporter.export_(
      name, (ArrowSchema**)v_arw_schema, (ArrowArray**)v_arw_array);

  return Status::Ok();
}

Status query_set_buffer_arrow_array(
    std::shared_ptr<Query> query,
    std::string name,
    void* v_arw_schema,
    void* v_arw_array) {

  auto arw_schema = (ArrowSchema*)v_arw_schema;
  auto arw_array = (ArrowArray*)v_arw_array;
  check_arrow_schema(arw_schema);

  ArrowImporter importer(query);
  importer.import_(name, arw_schema, arw_array);

  return Status::Ok();
}


};  // end namespace arrow
};  // end namespace tiledb