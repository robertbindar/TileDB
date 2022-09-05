#include <tiledb/tiledb_experimental.h>
#include <fstream>
#include <iostream>
#include <tiledb/tiledb>

using namespace tiledb;

std::string array_name("tiledb://demo/test_gow_rest");
std::string s3_array("s3://tiledb-robert/test_gow_rest");
uint64_t tile_extent = 32;
uint64_t dim_limit = 11534336;  // 88MB
uint64_t ncells = 2621440;      // 20MB
// Needs to be >5mb and tile aligned
uint64_t chunk_size = 655360;
std::vector<uint64_t> a1;

int main(int argc, char** argv) {
  Config cfg;
  cfg["rest.username"] = "demo";
  cfg["rest.password"] = "demodemo";
  cfg["rest.server_address"] = "http://localhost:80";
  cfg["vfs.s3.aws_access_key_id"] = "";
  cfg["vfs.s3.aws_secret_access_key"] = "";

  Context ctx(cfg);
  auto create_array = [&]() {
    ArraySchema schema(ctx, TILEDB_DENSE);
    Domain domain(ctx);
    domain.add_dimension(
        Dimension::create<uint64_t>(ctx, "d1", {0, dim_limit}, tile_extent));
    schema.set_domain(domain);
    schema.add_attribute(Attribute::create<uint64_t>(ctx, "a1"));
    Array::create(s3_array, schema);
  };

  auto global_write = [&]() {
    Array array(ctx, array_name, TILEDB_WRITE);

    for (uint64_t i = 0; i < ncells; i++) {
      a1.push_back(i);
    }

    uint64_t last_space_tile =
        (ncells / chunk_size +
         static_cast<uint64_t>(ncells % chunk_size != 0)) *
            chunk_size -
        1;

    Query query(ctx, array);
    query.set_layout(TILEDB_GLOBAL_ORDER);
    Subarray subarray(ctx, array);
    subarray.add_range(0, (uint64_t)0, last_space_tile);
    query.set_subarray(subarray);

    uint64_t begin = 0;
    while (begin < ncells - chunk_size) {
      query.set_data_buffer("a1", a1.data() + begin, chunk_size);
      query.submit();

      begin += chunk_size;
    }

    query.set_data_buffer("a1", a1.data() + begin, last_space_tile - begin + 1);
    query.submit_and_finalize();
    if (query.query_status() != Query::Status::COMPLETE) {
      throw std::runtime_error("Query incomplete");
    }
  };

  auto read_and_validate = [&]() {
    Array array(ctx, array_name, TILEDB_READ);

    Query query(ctx, array);
    query.set_layout(TILEDB_ROW_MAJOR);
    Subarray subarray(ctx, array);
    subarray.add_range(0, (uint64_t)0, ncells - 1);
    query.set_subarray(subarray);
    std::vector<uint64_t> data(ncells);
    query.set_data_buffer("a1", data.data(), data.size());
    query.submit();

    if (query.query_status() != Query::Status::COMPLETE) {
      throw std::runtime_error("Query incomplete during read");
    }

    for (uint64_t i = 0; i < ncells; ++i) {
      if (a1[i] != data[i]) {
        throw std::runtime_error("Incorrect data read");
      }
    }
  };

  try {
    create_array();
  } catch (...) {
    tiledb::VFS vfs(ctx);
    vfs.remove_dir(s3_array);
    std::cout << "Removed existing array" << std::endl;
    create_array();
  }
  global_write();
  read_and_validate();

  return 0;
}
