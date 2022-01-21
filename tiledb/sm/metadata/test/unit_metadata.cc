/**
 * @file tiledb/sm/metadata/test/unit_metadata.cc
 *
 * @section LICENSE
 *
 * The MIT License
 *
 * @copyright Copyright (c) 2021 TileDB, Inc.
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
 * This file defines a test `main()`
 */

#include <catch.hpp>
#include "../metadata.h"
#include "tiledb/common/dynamic_memory/dynamic_memory.h"
#include "tiledb/sm/buffer/buffer.h"
#include "tiledb/sm/enums/datatype.h"
#include "tiledb/sm/filesystem/uri.h"
#include "tiledb/sm/misc/time.h"
#include "tiledb/sm/misc/uuid.h"

using namespace tiledb;
using namespace tiledb::common;
using namespace tiledb::sm;

template <class T, int n>
inline T& buffer_metadata(void* p) {
  return *static_cast<T*>(static_cast<void*>(static_cast<char*>(p) + n));
}

TEST_CASE(
    "Metadata: Test metadata deserialization", "[metadata][deserialization]") {
  std::vector<tdb_shared_ptr<Buffer>> metadata_buffs;

  // key1:a, value1:100,200
  std::string key1 = "key1";
  uint32_t key1_size = static_cast<uint32_t>(key1.size());
  std::vector<int> value1_vector{100, 200};

  // key2:key1, value2:1.1(double)
  std::string key2 = "key2";
  uint32_t key2_size = static_cast<uint32_t>(key2.size());
  uint32_t value2_size = 1;
  double value2 = 1.0;

  // key3:key3, value3:strmetadata
  std::string key3 = "key3";
  uint32_t key3_size = static_cast<uint32_t>(key2.size());
  std::string value3 = "strmetadata";
  uint32_t value3_size = static_cast<uint32_t>(value3.size());

  char serialized_buffer1[22];
  char* p1 = &serialized_buffer1[0];
  // set key1:value1 integer metadata
  buffer_metadata<uint32_t, 0>(p1) = static_cast<uint32_t>(key1.size());
  std::memcpy(&buffer_metadata<char, 4>(p1), key1.c_str(), key1_size);
  buffer_metadata<char, 8>(p1) = 0;
  buffer_metadata<char, 9>(p1) = static_cast<char>(Datatype::INT32);
  buffer_metadata<uint32_t, 10>(p1) = (uint32_t)value1_vector.size();
  buffer_metadata<int32_t, 14>(p1) = value1_vector[0];
  buffer_metadata<int32_t, 18>(p1) = value1_vector[1];
  metadata_buffs.push_back(make_shared<Buffer>(
      HERE(), &serialized_buffer1, sizeof(serialized_buffer1)));

  char serialized_buffer2[22];
  char* p2 = &serialized_buffer2[0];
  // set key2:value2 double metadata
  buffer_metadata<uint32_t, 0>(p2) = static_cast<uint32_t>(key2.size());
  std::memcpy(&buffer_metadata<char, 4>(p2), key2.c_str(), key2_size);
  buffer_metadata<char, 8>(p2) = 0;
  buffer_metadata<char, 9>(p2) = (char)Datatype::FLOAT64;
  buffer_metadata<uint32_t, 10>(p2) = value2_size;
  buffer_metadata<double, 14>(p2) = value2;
  metadata_buffs.push_back(make_shared<Buffer>(
      HERE(), &serialized_buffer2, sizeof(serialized_buffer2)));

  char serialized_buffer3[25];
  char* p3 = &serialized_buffer3[0];
  // set key3:value3 string metadata
  buffer_metadata<uint32_t, 0>(p3) = static_cast<uint32_t>(key3.size());
  std::memcpy(&buffer_metadata<char, 4>(p3), key3.c_str(), key3_size);
  buffer_metadata<char, 8>(p3) = 0;
  buffer_metadata<char, 9>(p3) = (char)Datatype::STRING_ASCII;
  buffer_metadata<uint32_t, 10>(p3) = value3_size;
  std::memcpy(&buffer_metadata<char, 14>(p3), value3.c_str(), value3_size);
  metadata_buffs.push_back(make_shared<Buffer>(
      HERE(), &serialized_buffer3, sizeof(serialized_buffer3)));

  auto&& [st_meta, meta]{Metadata::deserialize(metadata_buffs)};

  REQUIRE(st_meta.ok());

  Datatype type;
  uint32_t v_num;

  // Read key1 metadata
  const int32_t* v1;
  meta.value()->get("key1", &type, &v_num, &v1);
  CHECK(type == Datatype::INT32);
  CHECK(v_num == (uint32_t)(value1_vector.size()));
  CHECK(*(v1) == 100);
  CHECK(*(v1 + 1) == 200);

  // Read key2 metadata
  const double* v2;
  meta.value()->get("key2", &type, &v_num, &v2);
  CHECK(type == Datatype::FLOAT64);
  CHECK(v_num == value2_size);
  CHECK(*(v2) == value2);

  // Read key3 metadata
  const char* v3;
  meta.value()->get("key3", &type, &v_num, &v3);
  CHECK(type == Datatype::STRING_ASCII);
  CHECK(v_num == value3_size);
  CHECK(std::string(v3) == value3);
}