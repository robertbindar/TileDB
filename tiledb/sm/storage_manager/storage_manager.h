/**
 * @file   storage_manager.h
 *
 * @section LICENSE
 *
 * The MIT License
 *
 * @copyright Copyright (c) 2017-2021 TileDB, Inc.
 * @copyright Copyright (c) 2016 MIT and Intel Corporation
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
 * This file defines class StorageManager.
 */

#ifndef TILEDB_STORAGE_MANAGER_H
#define TILEDB_STORAGE_MANAGER_H

#include <atomic>
#include <condition_variable>
#include <functional>
#include <list>
#include <map>
#include <mutex>
#include <queue>
#include <set>
#include <string>
#include <thread>

#include "tiledb/common/heap_memory.h"
#include "tiledb/common/logger_public.h"
#include "tiledb/common/status.h"
#include "tiledb/common/thread_pool.h"
#include "tiledb/sm/config/config.h"
#include "tiledb/sm/enums/walk_order.h"
#include "tiledb/sm/filesystem/uri.h"
#include "tiledb/sm/misc/cancelable_tasks.h"
#include "tiledb/sm/misc/types.h"
#include "tiledb/sm/stats/global_stats.h"
#include "tiledb/sm/tile/filtered_buffer.h"

using namespace tiledb::common;

namespace tiledb {
namespace sm {

class Array;
class ArraySchema;
class ArraySchemaEvolution;
class Buffer;
class BufferLRUCache;
class Consolidator;
class EncryptionKey;
class FragmentMetadata;
class FragmentInfo;
class Metadata;
class OpenArray;
class MemoryTracker;
class Query;
class RestClient;
class VFS;

enum class EncryptionType : uint8_t;
enum class ObjectType : uint8_t;

/** The storage manager that manages pretty much everything in TileDB. */
class StorageManager {
 public:
  /* ********************************* */
  /*          TYPE DEFINITIONS         */
  /* ********************************* */

  /** Enables iteration over TileDB objects in a path. */
  class ObjectIter {
   public:
    /**
     * There is a one-to-one correspondence between `expanded_` and `objs_`.
     * An `expanded_` value is `true` if the corresponding `objs_` path
     * has been expanded to the paths it contains in a post ored traversal.
     * This is not used in a preorder traversal.
     */
    std::list<bool> expanded_;
    /** The next URI in string format. */
    std::string next_;
    /** The next objects to be visited. */
    std::list<URI> objs_;
    /** The traversal order of the iterator. */
    WalkOrder order_;
    /** `True` if the iterator will recursively visit the directory tree. */
    bool recursive_;
  };

  /* ********************************* */
  /*     CONSTRUCTORS & DESTRUCTORS    */
  /* ********************************* */

  /** Constructor. */
  StorageManager(
      ThreadPool* compute_tp,
      ThreadPool* io_tp,
      stats::Stats* parent_stats,
      tdb_shared_ptr<Logger> logger);

  /** Destructor. */
  ~StorageManager();

  DISABLE_COPY_AND_COPY_ASSIGN(StorageManager);
  DISABLE_MOVE_AND_MOVE_ASSIGN(StorageManager);

  /* ********************************* */
  /*                API                */
  /* ********************************* */

  /**
   * Closes an array opened for reads.
   *
   * @param array The array to be closed.
   * @return Status
   */
  Status array_close_for_reads(Array* array);

  /**
   * Closes an array opened for writes.
   *
   * @param array The array to be closed.
   * @return Status
   */
  Status array_close_for_writes(Array* array);

  /**
   * Returns the array schemas and fragment metadata for the given array.
   * The function will focus only on relevant schemas and metadata in
   * range [timestamp_start, timestamp_end].
   *
   * @param array_uri The array URI.
   * @param memory_tracker The memory tracker of the array
   *     for which the fragment metadata is loaded.
   * @param enc_key The encryption key to use.
   * @param timestamp_start The starting timestamp.
   * @param timestamp_end The end timestamp.
   *     In TileDB, timestamps are in ms elapsed since
   *     1970-01-01 00:00:00 +0000 (UTC).
   * @return tuple of Status, latest ArraySchema, map of all array schemas and
   * vector of FragmentMetadata
   *        Status Ok on success, else error
   *        ArraySchema The array schema to be retrieved after the
   *           array is opened.
   *        ArraySchemaMap Map of all array schemas found keyed by name
   *        fragment_metadata The fragment metadata to be retrieved
   *           after the array is opened.
   */
  std::tuple<
      Status,
      std::optional<ArraySchema*>,
      std::optional<
          std::unordered_map<std::string, tdb_shared_ptr<ArraySchema>>>,
      std::optional<std::vector<tdb_shared_ptr<FragmentMetadata>>>>
  load_array_schemas_and_fragment_metadata(
      const URI& array_uri,
      MemoryTracker* memory_tracker,
      const EncryptionKey& enc_key,
      uint64_t timestamp_start,
      uint64_t timestamp_end);

  /**
   * Opens an array for reads at a timestamp. All the metadata of the
   * fragments created before or at the input timestamp are retrieved.
   *
   * If a timestamp_start is provided, this API will open the array between
   * `timestamp_start` and `timestamp_end`.
   *
   * @param array The array to be opened.
   * @return tuple of Status, latest ArraySchema, map of all array schemas and
   * vector of FragmentMetadata
   *        Status Ok on success, else error
   *        ArraySchema The array schema to be retrieved after the
   *           array is opened.
   *        ArraySchemaMap Map of all array schemas found keyed by name
   *        fragment_metadata The fragment metadata to be retrieved
   *           after the array is opened.
   */
  std::tuple<
      Status,
      std::optional<ArraySchema*>,
      std::optional<
          std::unordered_map<std::string, tdb_shared_ptr<ArraySchema>>>,
      std::optional<std::vector<tdb_shared_ptr<FragmentMetadata>>>>
  array_open_for_reads(Array* array);

  /**
   * Opens an array for reads without fragments.
   *
   * @param array The array to be opened.
   * @return tuple of Status, latest ArraySchema and map of all array schemas
   *        Status Ok on success, else error
   *        ArraySchema The array schema to be retrieved after the
   *          array is opened.
   *        ArraySchemaMap Map of all array schemas found keyed by name
   */
  std::tuple<
      Status,
      std::optional<ArraySchema*>,
      std::optional<
          std::unordered_map<std::string, tdb_shared_ptr<ArraySchema>>>>
  array_open_for_reads_without_fragments(Array* array);

  /** Opens an array for writes.
   *
   * @param array The array to open.
   * @return tuple of Status, latest ArraySchema and map of all array schemas
   *        Status Ok on success, else error
   *        ArraySchema The array schema to be retrieved after the
   *          array is opened.
   *        ArraySchemaMap Map of all array schemas found keyed by name
   */
  std::tuple<
      Status,
      std::optional<ArraySchema*>,
      std::optional<
          std::unordered_map<std::string, tdb_shared_ptr<ArraySchema>>>>
  array_open_for_writes(Array* array);

  /**
   * Load fragments for an already open array.
   *
   * @param array The open array.
   * @param fragment_info The list of fragment info.
   * @return Status, the fragment metadata to be loaded.
   */
  std::tuple<
      Status,
      std::optional<std::vector<tdb_shared_ptr<FragmentMetadata>>>>
  array_load_fragments(
      Array* array, const std::vector<TimestampedURI>& fragment_info);

  /**
   * Reopen an array for reads.
   *
   * @param array The array to reopen.
   * @return tuple of Status, latest ArraySchema, map of all array schemas and
   * vector of FragmentMetadata
   *        Status Ok on success, else error
   *        ArraySchema The array schema to be retrieved after the
   *          array is opened.
   *        ArraySchemaMap Map of all array schemas found keyed by name
   *        FragmentMetadata The fragment metadata to be retrieved
   *          after the array is opened.
   */
  std::tuple<
      Status,
      std::optional<ArraySchema*>,
      std::optional<
          std::unordered_map<std::string, tdb_shared_ptr<ArraySchema>>>,
      std::optional<std::vector<tdb_shared_ptr<FragmentMetadata>>>>
  array_reopen(Array* array);

  /**
   * Consolidates the fragments of an array into a single one.
   *
   * @param array_name The name of the array to be consolidated.
   * @param encryption_type The encryption type of the array
   * @param encryption_key If the array is encrypted, the private encryption
   *    key. For unencrypted arrays, pass `nullptr`.
   * @param key_length The length in bytes of the encryption key.
   * @param config Configuration parameters for the consolidation
   *     (`nullptr` means default, which will use the config associated with
   *      this instance).
   * @return Status
   */
  Status array_consolidate(
      const char* array_name,
      EncryptionType encryption_type,
      const void* encryption_key,
      uint32_t key_length,
      const Config* config);

  /**
   * Cleans up the array, such as its consolidated fragments and array
   * metadata. Note that this will coarsen the granularity of time traveling
   * (see docs for more information).
   *
   * @param array_name The name of the array to be vacuumed.
   * @param config Configuration parameters for vacuuming.
   * @return Status
   */
  Status array_vacuum(const char* array_name, const Config* config);

  /**
   * Cleans up fragments that took part in consolidation. Note that this
   * will coarsen the granularity of time traveling (see docs for more
   * information).
   *
   * @param array_name The name of the array to be vacuumed.
   * @param timestamp_start The timestamp to start vacuuming at.
   * @param timestamp_end The timestamp to end vacuuming at.
   * @return Status
   */
  Status array_vacuum_fragments(
      const char* array_name, uint64_t timestamp_start, uint64_t timestamp_end);

  /**
   * Cleans up consolidated fragment metadata (all except the last one).
   *
   * @param array_name The name of the array to be consolidated.
   * @return Status
   */
  Status array_vacuum_fragment_meta(const char* array_name);

  /**
   * Cleans up consolidated array metadata.
   *
   * @param array_name The name of the array to be consolidated.
   * @param timestamp_start The timestamp to start vacuuming at.
   * @param timestamp_end The timestamp to end vacuuming at.
   * @return Status
   */
  Status array_vacuum_array_meta(
      const char* array_name, uint64_t timestamp_start, uint64_t timestamp_end);

  /**
   * Consolidates the metadata of an array into a single file.
   *
   * @param array_name The name of the array whose metadata will be
   *     consolidated.
   * @param encryption_type The encryption type of the array
   * @param encryption_key If the array is encrypted, the private encryption
   *    key. For unencrypted arrays, pass `nullptr`.
   * @param key_length The length in bytes of the encryption key.
   * @param config Configuration parameters for the consolidation
   *     (`nullptr` means default, which will use the config associated with
   *      this instance).
   * @return Status
   */
  Status array_metadata_consolidate(
      const char* array_name,
      EncryptionType encryption_type,
      const void* encryption_key,
      uint32_t key_length,
      const Config* config);

  /**
   * Creates a TileDB array storing its schema.
   *
   * @param array_uri The URI of the array to be created.
   * @param array_schema The array schema.
   * @param encryption_key The encryption key to use.
   * @return Status
   */
  Status array_create(
      const URI& array_uri,
      ArraySchema* array_schema,
      const EncryptionKey& encryption_key);

  /**
   * Evolve a TileDB array schema and store its new schema.
   *
   * @param array_uri The URI of the array to be evolved.
   * @param schema_evolution The schema evolution.
   * @param encryption_key The encryption key to use.
   * @return Status
   */
  Status array_evolve_schema(
      const URI& array_uri,
      ArraySchemaEvolution* array_schema,
      const EncryptionKey& encryption_key);

  /**
   * Upgrade a TileDB array to latest format version.
   *
   * @param array_uri The URI of the array to be upgraded.
   * @param config Configuration parameters for the upgrade
   *     (`nullptr` means default, which will use the config associated with
   *      this instance).
   * @return Status
   */
  Status array_upgrade_version(const URI& array_uri, const Config* config);

  /**
   * Retrieves the non-empty domain from an array. This is the union of the
   * non-empty domains of the array fragments.
   *
   * @param array An open array object (must be already open).
   * @param domain The domain to be retrieved.
   * @param is_empty `true` if the non-empty domain is empty (the array
   *     is empty).
   * @return Status
   */
  Status array_get_non_empty_domain(
      Array* array, NDRange* domain, bool* is_empty);

  /**
   * Retrieves the non-empty domain from an array. This is the union of the
   * non-empty domains of the array fragments.
   *
   * @param array An open array object (must be already open).
   * @param domain The domain to be retrieved.
   * @param is_empty `ture` if the non-empty domain is empty (the array
   *     is empty).
   * @return Status
   */
  Status array_get_non_empty_domain(Array* array, void* domain, bool* is_empty);

  /**
   * Retrieves the non-empty domain from an array on the given dimension.
   * This is the union of the non-empty domains of the array fragments.
   *
   * @param array An open array object (must be already open).
   * @param idx The dimension index.
   * @param domain The domain to be retrieved.
   * @param is_empty `ture` if the non-empty domain is empty (the array
   *     is empty).
   * @return Status
   */
  Status array_get_non_empty_domain_from_index(
      Array* array, unsigned idx, void* domain, bool* is_empty);

  /**
   * Retrieves the non-empty domain from an array on the given dimension.
   * This is the union of the non-empty domains of the array fragments.
   *
   * @param array An open array object (must be already open).
   * @param name The dimension name.
   * @param domain The domain to be retrieved.
   * @param is_empty `ture` if the non-empty domain is empty (the array
   *     is empty).
   * @return Status
   */
  Status array_get_non_empty_domain_from_name(
      Array* array, const char* name, void* domain, bool* is_empty);

  /**
   * Retrieves the non-empty domain size from an array on the given dimension.
   * This is the union of the non-empty domains of the array fragments.
   * Applicable only to var-sized dimensions.
   *
   * @param array An open array object (must be already open).
   * @param idx The dimension index.
   * @param start_size The size in bytes of the range start.
   * @param end_size The size in bytes of the range end.
   * @param is_empty `ture` if the non-empty domain is empty (the array
   *     is empty).
   * @return Status
   */
  Status array_get_non_empty_domain_var_size_from_index(
      Array* array,
      unsigned idx,
      uint64_t* start_size,
      uint64_t* end_size,
      bool* is_empty);

  /**
   * Retrieves the non-empty domain size from an array on the given dimension.
   * This is the union of the non-empty domains of the array fragments.
   * Applicable only to var-sized dimensions.
   *
   * @param array An open array object (must be already open).
   * @param name The dimension name.
   * @param start_size The size in bytes of the range start.
   * @param end_size The size in bytes of the range end.
   * @param is_empty `ture` if the non-empty domain is empty (the array
   *     is empty).
   * @return Status
   */
  Status array_get_non_empty_domain_var_size_from_name(
      Array* array,
      const char* name,
      uint64_t* start_size,
      uint64_t* end_size,
      bool* is_empty);

  /**
   * Retrieves the non-empty domain from an array on the given dimension.
   * This is the union of the non-empty domains of the array fragments.
   * Applicable only to var-sized dimensions.
   *
   * @param array An open array object (must be already open).
   * @param idx The dimension index.
   * @param start The domain range start to set.
   * @param end The domain range end to set.
   * @param is_empty `ture` if the non-empty domain is empty (the array
   *     is empty).
   * @return Status
   */
  Status array_get_non_empty_domain_var_from_index(
      Array* array, unsigned idx, void* start, void* end, bool* is_empty);

  /**
   * Retrieves the non-empty domain from an array on the given dimension.
   * This is the union of the non-empty domains of the array fragments.
   * Applicable only to var-sized dimensions.
   *
   * @param array An open array object (must be already open).
   * @param name The dimension name.
   * @param start The domain range start to set.
   * @param end The domain range end to set.
   * @param is_empty `ture` if the non-empty domain is empty (the array
   *     is empty).
   * @return Status
   */
  Status array_get_non_empty_domain_var_from_name(
      Array* array, const char* name, void* start, void* end, bool* is_empty);

  /**
   * Retrieves the encryption type from an array.
   *
   * @param array_uri URI of the array
   * @param encryption_type Set to the encryption type of the array.
   * @return Status
   */
  Status array_get_encryption(
      const std::string& array_uri, EncryptionType* encryption_type);

  /**
   * Pushes an async query to the queue.
   *
   * @param query The async query.
   * @return Status
   */
  Status async_push_query(Query* query);

  /** Cancels all background tasks. */
  Status cancel_all_tasks();

  /** Returns true while all tasks are being cancelled. */
  bool cancellation_in_progress();

  /** Returns the configuration parameters. */
  const Config& config() const;

  /** Creates a directory with the input URI. */
  Status create_dir(const URI& uri);

  /** Creates an empty file with the input URI. */
  Status touch(const URI& uri);

  /**
   * Retrieves all the fragment URIs of an array, along with the latest
   * consolidated fragment metadata URI `meta_uri`.
   */
  Status get_fragment_uris(
      const URI& array_uri,
      std::vector<URI>* fragment_uris,
      URI* meta_uri) const;

  /**
   * It computes the URIs `to_vacuum` from the input `uris`, considering
   * only the URIs whose first timestamp is greater than or equal to
   * `timestamp_start` and second timestamp is smaller than or equal to
   * `timestamp_end`. The function also retrieves the `vac_uris` (files with
   * `.vac` suffix) that were used to compute `to_vacuum`.
   *
   * @param uris The input fragment URIs.
   * @param timestamp_start The function considers only URIs whose
   *     timestamps fall into [`timestamp_start`, `timestamp_end`].
   * @param timestamp_end The function considers only URIs whose
   *     timestamps fall into [`timestamp_start`, `timestamp_end`].
   * @param to_vacuum The URIs determined that should be vacuumed.
   * @param vac_uris The `.vac` files used to determine `to_vacuum`.
   * @param allow_partial If `true` the function will load URIs
   *    whose timestamps partial overlap [`timestamp_start`, `timestamp_end`].
   * @return Status
   */
  Status get_uris_to_vacuum(
      const std::vector<URI>& uris,
      uint64_t timestamp_start,
      uint64_t timestamp_end,
      std::vector<URI>* to_vacuum,
      std::vector<URI>* vac_uris,
      bool allow_partial = true) const;

  /** Returns the current map of any set tags. */
  const std::unordered_map<std::string, std::string>& tags() const;

  /**
   * Creates a TileDB group.
   *
   * @param group The URI of the group to be created.
   * @return Status
   */
  Status group_create(const std::string& group);

  /**
   * Initializes the storage manager.
   *
   * @param config The configuration parameters.
   * @return Status
   */
  Status init(const Config* config);

  /** Returns the thread pool for compute-bound tasks. */
  ThreadPool* compute_tp();

  /** Returns the thread pool for io-bound tasks. */
  ThreadPool* io_tp();

  /**
   * If the storage manager was configured with a REST server, return the
   * client instance. Else, return nullptr.
   */
  RestClient* rest_client() const;

  /**
   * Checks if the input URI represents an array.
   *
   * @param The URI to be checked.
   * @param is_array Set to `true` if the URI is an array and `false` otherwise.
   * @return Status
   */
  Status is_array(const URI& uri, bool* is_array) const;

  /**
   * Checks if the input URI represents a directory.
   *
   * @param The URI to be checked.
   * @param is_dir Set to `true` if the URI is a directory and `false`
   *     otherwise.
   * @return Status
   */
  Status is_dir(const URI& uri, bool* is_dir) const;

  /**
   * Checks if the input URI represents a fragment. The functions takes into
   * account the fragment version, which is retrived directly from the URI.
   * For versions >= 5, the function checks whether the URI is included
   * in `ok_uris`. For versions < 5, `ok_uris` is empty so the function
   * checks for the existence of the fragment metadata file in the fragment
   * URI directory. Therefore, the function is more expensive for earlier
   * fragment versions.
   *
   * @param The URI to be checked.
   * @param ok_uris For checking URI existence of versions >= 5.
   * @param is_fragment Set to `1` if the URI is a fragment and `0`
   *     otherwise.
   * @return Status
   */
  Status is_fragment(
      const URI& uri, const std::set<URI>& ok_uris, int* is_fragment) const;

  /**
   * Checks if the input URI represents a group.
   *
   * @param The URI to be checked.
   * @param is_group Set to `true` if the URI is a group and `false`
   *     otherwise.
   * @return Status
   */
  Status is_group(const URI& uri, bool* is_group) const;

  /**
   * Checks if the input URI represents a file.
   *
   * @param The URI to be checked.
   * @param is_file Set to `true` if the URI is a file and `false`
   *     otherwise.
   * @return Status
   */
  Status is_file(const URI& uri, bool* is_file) const;

  /**
   * Check if a URI is a vacuum file or not based on the file suffix
   * @param uri
   * @return true is vacuum file, false otherwise
   */
  bool is_vacuum_file(const URI& uri) const;

  /**
   * Retrieve all array schemas for an array uri under its __schema directory.
   *
   * @param array_uri The URI path of the array.
   * @param uris The vector of array schema URIS sorted from earliest to the
   * latest.
   * @return Status
   */
  Status get_array_schema_uris(
      const URI& array_uri, std::vector<URI>* schema_uris) const;

  /**
   * Get latest array schema for an array uri.
   *
   * @param array_uri The URI path of the array.
   * @param uri The latest array schema URI.
   * @return Status
   */
  Status get_latest_array_schema_uri(
      const URI& array_uri, URI* schema_uri) const;

  /**
   * Loads the schema of a schema uri from persistent storage into memory.
   *
   * @param array_schema_uri The URI path of the array schema.
   * @param array_uri The URI path of the array.
   * @param encryption_key The encryption key to use.
   * @param array_schema The array schema to be retrieved.
   * @return Status
   */
  Status load_array_schema_from_uri(
      const URI& array_schema_uri,
      const EncryptionKey& encryption_key,
      ArraySchema** array_schema);

  /**
   * Loads the latest schema of an array from persistent storage into memory.
   *
   * @param array_uri The URI path of the array.
   * @param encryption_key The encryption key to use.
   * @param array_schema The array schema to be retrieved.
   * @return Status
   */
  Status load_array_schema_latest(
      const URI& array_uri,
      const EncryptionKey& encryption_key,
      ArraySchema** array_schema);

  /**
   * It loads and returns the latest schema and all the array schemas
   * (in the presence of schema evolution).
   *
   * @param array_uri The URI path of the array.
   * @param encryption_key The encryption key to use.
   * @return tuple of Status, latest array schema and all array schemas.
   *   Status Ok on success, else error
   *   ArraySchema The latest array schema.
   *   ArraySchemaMap Map of all array schemas loaded, keyed by name
   */
  std::tuple<
      Status,
      std::optional<ArraySchema*>,
      std::optional<
          std::unordered_map<std::string, tdb_shared_ptr<ArraySchema>>>>
  load_array_schemas(const URI& array_uri, const EncryptionKey& encryption_key);

  /**
   * Loads all schemas of an array from persistent storage into memory.
   *
   * @param array_uri The URI path of the array.
   * @param encryption_key The encryption key to use.
   * @return tuple of Status and optional unordered map. If Status is an error
   * the unordered_map will be nullopt
   *        Status Ok on success, else error
   *        ArraySchemaMap Map of all array schemas found keyed by name
   */
  std::tuple<
      Status,
      std::optional<
          std::unordered_map<std::string, tdb_shared_ptr<ArraySchema>>>>
  load_all_array_schemas(
      const URI& array_uri, const EncryptionKey& encryption_key);

  /**
   * Loads the array metadata from persistent storage that were created
   * at or before `timestamp_end` and at or after `timestamp_start`.
   */
  Status load_array_metadata(
      const URI& array_uri,
      const EncryptionKey& encryption_key,
      uint64_t timestamp_start,
      uint64_t timestamp_end,
      Metadata* metadata);

  /** Removes a TileDB object (group, array). */
  Status object_remove(const char* path) const;

  /**
   * Renames a TileDB object (group, array). If
   * `new_path` exists, `new_path` will be overwritten.
   */
  Status object_move(const char* old_path, const char* new_path) const;

  /**
   * Creates a new object iterator for the input path. The iteration
   * in this case will be recursive in the entire directory tree rooted
   * at `path`.
   *
   * @param obj_iter The object iterator to be created (memory is allocated for
   *     it by the function).
   * @param path The path the iterator will target at.
   * @param order The traversal order of the iterator.
   * @return Status
   */
  Status object_iter_begin(
      ObjectIter** obj_iter, const char* path, WalkOrder order);

  /**
   * Creates a new object iterator for the input path. The iteration will
   * not be recursive, and only the children of `path` will be visited.
   *
   * @param obj_iter The object iterator to be created (memory is allocated for
   *     it by the function).
   * @param path The path the iterator will target at.
   * @return Status
   */
  Status object_iter_begin(ObjectIter** obj_iter, const char* path);

  /** Frees the object iterator. */
  void object_iter_free(ObjectIter* obj_iter);

  /**
   * Retrieves the next object path and type.
   *
   * @param obj_iter The object iterator.
   * @param path The object path that is retrieved.
   * @param type The object type that is retrieved.
   * @param has_next True if an object path was retrieved and false otherwise.
   * @return Status
   */
  Status object_iter_next(
      ObjectIter* obj_iter,
      const char** path,
      ObjectType* type,
      bool* has_next);

  /**
   * Retrieves the next object in the post-order traversal.
   *
   * @param obj_iter The object iterator.
   * @param path The object path that is retrieved.
   * @param type The object type that is retrieved.
   * @param has_next True if an object path was retrieved and false otherwise.
   * @return Status
   */
  Status object_iter_next_postorder(
      ObjectIter* obj_iter,
      const char** path,
      ObjectType* type,
      bool* has_next);

  /**
   * Retrieves the next object in the post-order traversal.
   *
   * @param obj_iter The object iterator.
   * @param path The object path that is retrieved.
   * @param type The object type that is retrieved.
   * @param has_next True if an object path was retrieved and false otherwise.
   * @return Status
   */
  Status object_iter_next_preorder(
      ObjectIter* obj_iter,
      const char** path,
      ObjectType* type,
      bool* has_next);

  /**
   * Returns the tiledb object type
   *
   * @param uri Path to TileDB object resource
   * @param type The ObjectType to be retrieved.
   * @return Status
   */
  Status object_type(const URI& uri, ObjectType* type) const;

  /** Submits a query for (sync) execution. */
  Status query_submit(Query* query);

  /**
   * Submits a query for async execution.
   *
   * @param query The query to submit.
   * @return Status
   */
  Status query_submit_async(Query* query);

  /**
   * Reads from the cache into the input buffer. `uri` and `offset` collectively
   * form the key of the cached object to be read. Essentially, this is used
   * to read potentially cached tiles. `uri` is the URI of the attribute the
   * tile belongs to, and `offset` is the offset in the attribute file where
   * the tile is located. Observe that the `uri`, `offset` pair is unique.
   *
   * @param uri The URI of the cached object.
   * @param offset The offset of the cached object.
   * @param buffer The buffer to write into. The function reallocates memory
   *     for the buffer, sets its size to *nbytes* and resets its offset.
   * @param nbytes Number of bytes to be read.
   * @param in_cache This is set to `true` if the object is in the cache,
   *     and `false` otherwise.
   * @return Status.
   */
  Status read_from_cache(
      const URI& uri,
      uint64_t offset,
      FilteredBuffer& buffer,
      uint64_t nbytes,
      bool* in_cache) const;

  /**
   * Reads from a file into the input buffer.
   *
   * @param uri The URI file to read from.
   * @param offset The offset in the file the read will start from.
   * @param buffer The buffer to write into. The function reallocates memory
   *     for the buffer, sets its size to *nbytes* and resets its offset.
   * @param nbytes The number of bytes to read.
   * @return Status.
   */
  Status read(
      const URI& uri, uint64_t offset, Buffer* buffer, uint64_t nbytes) const;

  /**
   * Reads from a file into the raw input buffer.
   *
   * @param uri The URI file to read from.
   * @param offset The offset in the file the read will start from.
   * @param buffer The buffer to write into.
   * @param nbytes The number of bytes to read.
   * @return Status.
   */
  Status read(
      const URI& uri, uint64_t offset, void* buffer, uint64_t nbytes) const;

  /**
   * Sets a string/string KV "tag" on the storage manager instance.
   *
   * This is currently only meant for internal TileDB Inc. usage.
   *
   * @param key Tag key
   * @param value Tag value
   * @return Status
   */
  Status set_tag(const std::string& key, const std::string& value);

  /**
   * Stores an array schema into persistent storage.
   *
   * @param array_schema The array metadata to be stored.
   * @param encryption_key The encryption key to use.
   * @return Status
   */
  Status store_array_schema(
      ArraySchema* array_schema, const EncryptionKey& encryption_key);

  /**
   * Stores the array metadata into persistent storage.
   *
   * @param array_uri The array URI.
   * @param encryption_key The encryption key to use.
   * @param array_metadata The array metadata.
   * @return Status
   */
  Status store_array_metadata(
      const URI& array_uri,
      const EncryptionKey& encryption_key,
      Metadata* array_metadata);

  /** Closes a file, flushing its contents to persistent storage. */
  Status close_file(const URI& uri);

  /** Syncs a file or directory, flushing its contents to persistent storage. */
  Status sync(const URI& uri);

  /** Returns the virtual filesystem object. */
  VFS* vfs() const;

  /**
   * Writes the contents of a buffer into the cache. `uri` and `offset`
   * collectively form the key of the object to be cached. Essentially, this is
   * used to cach tiles. `uri` is the URI of the attribute the
   * tile belongs to, and `offset` is the offset in the attribute file where
   * the tile is located. Observe that the `uri`, `offset` pair is unique.
   *
   * @param uri The URI of the cached object.
   * @param offset The offset of the cached object.
   * @param buffer The buffer whose contents will be cached.
   * @return Status.
   */
  Status write_to_cache(
      const URI& uri, uint64_t offset, const FilteredBuffer& buffer) const;

  /**
   * Writes the contents of a buffer into a URI file.
   *
   * @param uri The file to write into.
   * @param buffer The buffer to write.
   * @return Status.
   */
  Status write(const URI& uri, Buffer* buffer) const;

  /**
   * Writes the input data into a URI file.
   *
   * @param uri The file to write into.
   * @param data The data to write.
   * @param size The data size in bytes.
   * @return Status.
   */
  Status write(const URI& uri, void* data, uint64_t size) const;

  /** Returns `stats_`. */
  stats::Stats* stats();

  /** Returns the internal logger object. */
  tdb_shared_ptr<Logger> logger() const;

 private:
  /* ********************************* */
  /*        PRIVATE DATATYPES          */
  /* ********************************* */

  /**
   * Helper RAII struct that increments 'queries_in_progress' in the constructor
   * and decrements in the destructor, on the given StorageManager instance.
   *
   * This ensures that the counter is decremented even in the case of
   * exceptions.
   */
  struct QueryInProgress {
    /** The StorageManager instance. */
    StorageManager* sm;

    /** Constructor. Calls increment_in_progress() on given StorageManager. */
    QueryInProgress(StorageManager* sm)
        : sm(sm) {
      sm->increment_in_progress();
    }

    /** Destructor. Calls decrement_in_progress() on given StorageManager. */
    ~QueryInProgress() {
      sm->decrement_in_progress();
    }
  };

  /* ********************************* */
  /*        PRIVATE ATTRIBUTES         */
  /* ********************************* */

  /** The class stats. */
  stats::Stats* stats_;

  /** The class logger. */
  tdb_shared_ptr<Logger> logger_;

  /** Set to true when tasks are being cancelled. */
  bool cancellation_in_progress_;

  /** Mutex protecting cancellation_in_progress_. */
  std::mutex cancellation_in_progress_mtx_;

  /**
   * The condition variable for exlcusively locking arrays. This is used
   * to wait for an array to be closed, before being exclusively locked
   * by `array_xlock`.
   */
  std::condition_variable xlock_cv_;

  /** Mutex for providing thread-safety upon creating TileDB objects. */
  std::mutex object_create_mtx_;

  /** Stores the TileDB configuration parameters. */
  Config config_;

  /** Keeps track of which arrays are open. */
  std::set<Array*> open_arrays_;

  /** Mutex for managing open arrays. */
  std::mutex open_arrays_mtx_;

  /** Count of the number of queries currently in progress. */
  uint64_t queries_in_progress_;

  /** Guards queries_in_progress_ counter. */
  std::mutex queries_in_progress_mtx_;

  /** Guards queries_in_progress_ counter. */
  std::condition_variable queries_in_progress_cv_;

  /** The thread pool for compute-bound tasks. */
  ThreadPool* const compute_tp_;

  /** The thread pool for io-bound tasks. */
  ThreadPool* const io_tp_;

  /** Tracks all scheduled tasks that can be safely cancelled before execution.
   */
  CancelableTasks cancelable_tasks_;

  /** Tags for the context object. */
  std::unordered_map<std::string, std::string> tags_;

  /** A tile cache. */
  tdb_unique_ptr<BufferLRUCache> tile_cache_;

  /**
   * Virtual filesystem handler. It directs queries to the appropriate
   * filesystem backend. Note that this is stateful.
   */
  VFS* vfs_;

  /** The rest client (may be null if none was configured). */
  tdb_unique_ptr<RestClient> rest_client_;

  /* ********************************* */
  /*         PRIVATE METHODS           */
  /* ********************************* */

  /** Decrement the count of in-progress queries. */
  void decrement_in_progress();

  /** Retrieves all the array metadata URI's of an array. */
  Status get_array_metadata_uris(
      const URI& array_uri, std::vector<URI>* array_metadata_uris) const;

  /** Increment the count of in-progress queries. */
  void increment_in_progress();

  /**
   * Loads the fragment metadata of an open array given a vector of
   * fragment URIs `fragments_to_load`.
   * The function stores the fragment metadata of each fragment
   * in `fragments_to_load` into the returned vector, such
   * that there is a one-to-one correspondence between the two vectors.
   *
   * If `meta_buf` has data, then some fragment metadata may be contained
   * in there and does not need to be loaded from storage. In that
   * case, `offsets` helps identifying each fragment metadata in the
   * buffer.
   *
   * @param memory_tracker The memory tracker of the array
   *     for which the metadata is loaded. This will be passed to
   *     the constructor of each of the metadata loaded.
   * @param array_schema_latest The latest array schema.
   * @param array_schemas_all All the array schemas in a map keyed by the
   *     schema filename.
   * @param encryption_key The encryption key to use.
   * @param fragments_to_load The fragments whose metadata to load.
   * @param meta_buff A buffer that may contain the consolidated fragment
   *     metadata.
   * @param offsets A map from a fragment name to an offset in `meta_buff`
   *     where the basic fragment metadata can be found. If the offset
   *     cannot be found, then the metadata of that fragment will be loaded from
   *     storage instead.
   * @return tuple of Status and vector of FragmentMetadata
   *        Status Ok on success, else error
   *        Vector of FragmentMetadata is the fragment metadata to be retrieved.
   */
  std::tuple<
      Status,
      std::optional<std::vector<tdb_shared_ptr<FragmentMetadata>>>>
  load_fragment_metadata(
      MemoryTracker* memory_tracker,
      ArraySchema* array_schema_latest,
      const std::unordered_map<std::string, tdb_shared_ptr<ArraySchema>>&
          array_schemas_all,
      const EncryptionKey& encryption_key,
      const std::vector<TimestampedURI>& fragments_to_load,
      Buffer* meta_buff,
      const std::unordered_map<std::string, uint64_t>& offsets);

  /**
   * Loads the latest consolidated fragment metadata from storage.
   *
   * @param uri The URI of the consolidated fragment metadata.
   * @param enc_key The encryption key that may be needed to access the file.
   * @param f_buff The buffer to hold the consolidated fragment metadata.
   * @param offsets A map from the fragment name to the offset in `f_buff` where
   *     the basic fragment metadata starts.
   * @return Status
   */
  Status load_consolidated_fragment_meta(
      const URI& uri,
      const EncryptionKey& enc_key,
      Buffer* f_buff,
      std::unordered_map<std::string, uint64_t>* offsets);

  /**
   * Retrieves the URI of the latest consolidated fragment metadata,
   * among the URIs in `uris`.
   */
  Status get_consolidated_fragment_meta_uri(
      const std::vector<URI>& uris, URI* meta_uri) const;

  /**
   * Applicable to fragment and array metadata URIs.
   *
   * Gets the sorted URIs in ascending first timestamp order,
   * breaking ties with lexicographic
   * sorting of UUID. Only the URIs with timestamp between `timestamp_start`
   * and `timestamp_end` (inclusive) are considered. The sorted URIs are
   * stored in the last input, including their timestamps.
   */
  Status get_sorted_uris(
      const std::vector<URI>& uris,
      std::vector<TimestampedURI>* sorted_uris,
      uint64_t timestamp_start,
      uint64_t timestamp_end) const;

  /** Block until there are zero in-progress queries. */
  void wait_for_zero_in_progress();

  /** Initializes a REST client, if one was configured. */
  Status init_rest_client();

  /** Sets default tag values on this StorageManager. */
  Status set_default_tags();
};

}  // namespace sm
}  // namespace tiledb

#endif  // TILEDB_STORAGE_MANAGER_H
