#
# tiledb/sm/compressors/CMakeLists.txt
#
#
# The MIT License
#
# Copyright (c) 2021 TileDB, Inc.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#
include(common NO_POLICY_SCOPE)

#
# `compressors` object library
#
add_library(compressors OBJECT bzip_compressor.cc dd_compressor.cc gzip_compressor.cc lz4_compressor.cc rle_compressor.cc zstd_compressor.cc)
target_link_libraries(compressors PUBLIC baseline $<TARGET_OBJECTS:baseline>)
target_link_libraries(compressors PUBLIC buffer $<TARGET_OBJECTS:buffer>)
find_package(Bzip2_EP REQUIRED)
find_package(LZ4_EP REQUIRED)
find_package(Zlib_EP REQUIRED)
find_package(Zstd_EP REQUIRED)
target_link_libraries(compressors PUBLIC Bzip2::Bzip2 LZ4::LZ4 Zlib::Zlib Zstd::Zstd)

#
# Test-compile of object library ensures link-completeness
#
add_executable(compile_compressors EXCLUDE_FROM_ALL)
target_link_libraries(compile_compressors PRIVATE compressors)
target_sources(compile_compressors PRIVATE
        test/compile_compressors_main.cc $<TARGET_OBJECTS:compressors>
)