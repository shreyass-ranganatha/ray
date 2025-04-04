// Copyright 2017 The Ray Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//  http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "io_ray_runtime_gcs_GlobalStateAccessor.h"

#include <jni.h>

#include <string>

#include "boost/algorithm/string.hpp"
#include "jni_utils.h"  // NOLINT(build/include_subdir)
#include "ray/common/ray_config.h"
#include "ray/core_worker/common.h"
#include "ray/gcs/gcs_client/global_state_accessor.h"

#ifdef __cplusplus
extern "C" {
#endif
JNIEXPORT jlong JNICALL
Java_io_ray_runtime_gcs_GlobalStateAccessor_nativeCreateGlobalStateAccessor(
    JNIEnv *env,
    jobject o,
    jstring j_bootstrap_address,
    jstring j_redis_username,
    jstring j_redis_password) {
  std::string bootstrap_address = JavaStringToNativeString(env, j_bootstrap_address);
  std::string redis_username = JavaStringToNativeString(env, j_redis_username);
  std::string redis_password = JavaStringToNativeString(env, j_redis_password);
  gcs::GlobalStateAccessor *gcs_accessor = nullptr;
  ray::gcs::GcsClientOptions client_options(bootstrap_address,
                                            ray::ClusterID::Nil(),
                                            /*allow_cluster_id_nil=*/true,
                                            /*fetch_cluster_id_if_nil=*/false);
  gcs_accessor = new gcs::GlobalStateAccessor(client_options);
  return reinterpret_cast<jlong>(gcs_accessor);
}

JNIEXPORT void JNICALL
Java_io_ray_runtime_gcs_GlobalStateAccessor_nativeDestroyGlobalStateAccessor(
    JNIEnv *env, jobject o, jlong gcs_accessor_ptr) {
  auto *gcs_accessor = reinterpret_cast<gcs::GlobalStateAccessor *>(gcs_accessor_ptr);
  delete gcs_accessor;
}

JNIEXPORT jboolean JNICALL Java_io_ray_runtime_gcs_GlobalStateAccessor_nativeConnect(
    JNIEnv *env, jobject o, jlong gcs_accessor_ptr) {
  auto *gcs_accessor = reinterpret_cast<gcs::GlobalStateAccessor *>(gcs_accessor_ptr);
  return gcs_accessor->Connect();
}

JNIEXPORT jobject JNICALL Java_io_ray_runtime_gcs_GlobalStateAccessor_nativeGetAllJobInfo(
    JNIEnv *env, jobject o, jlong gcs_accessor_ptr) {
  auto *gcs_accessor = reinterpret_cast<gcs::GlobalStateAccessor *>(gcs_accessor_ptr);
  auto job_info_list = gcs_accessor->GetAllJobInfo();
  return NativeVectorToJavaList<std::string>(
      env, job_info_list, [](JNIEnv *env, const std::string &str) {
        return NativeStringToJavaByteArray(env, str);
      });
}

JNIEXPORT jbyteArray JNICALL
Java_io_ray_runtime_gcs_GlobalStateAccessor_nativeGetNextJobID(JNIEnv *env,
                                                               jobject o,
                                                               jlong gcs_accessor_ptr) {
  auto *gcs_accessor = reinterpret_cast<gcs::GlobalStateAccessor *>(gcs_accessor_ptr);
  const auto &job_id = gcs_accessor->GetNextJobID();
  return IdToJavaByteArray<JobID>(env, job_id);
}

JNIEXPORT jobject JNICALL
Java_io_ray_runtime_gcs_GlobalStateAccessor_nativeGetAllNodeInfo(JNIEnv *env,
                                                                 jobject o,
                                                                 jlong gcs_accessor_ptr) {
  auto *gcs_accessor = reinterpret_cast<gcs::GlobalStateAccessor *>(gcs_accessor_ptr);
  auto node_info_list = gcs_accessor->GetAllNodeInfo();
  return NativeVectorToJavaList<std::string>(
      env, node_info_list, [](JNIEnv *env, const std::string &str) {
        return NativeStringToJavaByteArray(env, str);
      });
}

JNIEXPORT jobject JNICALL
Java_io_ray_runtime_gcs_GlobalStateAccessor_nativeGetAllActorInfo(
    JNIEnv *env,
    jobject o,
    jlong gcs_accessor_ptr,
    jbyteArray j_job_id,
    jstring j_actor_state_name) {
  std::optional<JobID> job_id = std::nullopt;
  std::optional<std::string> actor_state_name = std::nullopt;
  if (j_job_id != NULL) {
    job_id = std::make_optional<JobID>(JavaByteArrayToId<JobID>(env, j_job_id));
  }
  if (j_actor_state_name != NULL) {
    actor_state_name = std::make_optional<std::string>(
        JavaStringToNativeString(env, j_actor_state_name));
  }
  auto *gcs_accessor = reinterpret_cast<gcs::GlobalStateAccessor *>(gcs_accessor_ptr);
  auto actor_info_list =
      gcs_accessor->GetAllActorInfo(std::nullopt, job_id, actor_state_name);
  return NativeVectorToJavaList<std::string>(
      env, actor_info_list, [](JNIEnv *env, const std::string &str) {
        return NativeStringToJavaByteArray(env, str);
      });
}

JNIEXPORT jbyteArray JNICALL
Java_io_ray_runtime_gcs_GlobalStateAccessor_nativeGetActorInfo(JNIEnv *env,
                                                               jobject o,
                                                               jlong gcs_accessor_ptr,
                                                               jbyteArray j_actor_id) {
  const auto actor_id = JavaByteArrayToId<ActorID>(env, j_actor_id);
  auto *gcs_accessor = reinterpret_cast<gcs::GlobalStateAccessor *>(gcs_accessor_ptr);
  auto actor_info = gcs_accessor->GetActorInfo(actor_id);
  if (actor_info) {
    return NativeStringToJavaByteArray(env, *actor_info);
  }
  return nullptr;
}

JNIEXPORT jbyteArray JNICALL
Java_io_ray_runtime_gcs_GlobalStateAccessor_nativeGetPlacementGroupInfo(
    JNIEnv *env, jobject o, jlong gcs_accessor_ptr, jbyteArray placement_group_id_bytes) {
  const auto placement_group_id =
      JavaByteArrayToId<PlacementGroupID>(env, placement_group_id_bytes);
  auto *gcs_accessor = reinterpret_cast<gcs::GlobalStateAccessor *>(gcs_accessor_ptr);
  auto placement_group = gcs_accessor->GetPlacementGroupInfo(placement_group_id);
  if (placement_group) {
    return NativeStringToJavaByteArray(env, *placement_group);
  }
  return nullptr;
}

JNIEXPORT jbyteArray JNICALL
Java_io_ray_runtime_gcs_GlobalStateAccessor_nativeGetPlacementGroupInfoByName(
    JNIEnv *env, jobject o, jlong gcs_accessor_ptr, jstring name, jstring ray_namespace) {
  std::string placement_group_name = JavaStringToNativeString(env, name);
  auto *gcs_accessor = reinterpret_cast<gcs::GlobalStateAccessor *>(gcs_accessor_ptr);
  auto placement_group = gcs_accessor->GetPlacementGroupByName(
      placement_group_name, JavaStringToNativeString(env, ray_namespace));
  if (placement_group) {
    return NativeStringToJavaByteArray(env, *placement_group);
  }
  return nullptr;
}

JNIEXPORT jobject JNICALL
Java_io_ray_runtime_gcs_GlobalStateAccessor_nativeGetAllPlacementGroupInfo(
    JNIEnv *env, jobject o, jlong gcs_accessor_ptr) {
  auto *gcs_accessor = reinterpret_cast<gcs::GlobalStateAccessor *>(gcs_accessor_ptr);
  auto placement_group_info_list = gcs_accessor->GetAllPlacementGroupInfo();
  return NativeVectorToJavaList<std::string>(
      env, placement_group_info_list, [](JNIEnv *env, const std::string &str) {
        return NativeStringToJavaByteArray(env, str);
      });
}

JNIEXPORT jbyteArray JNICALL
Java_io_ray_runtime_gcs_GlobalStateAccessor_nativeGetInternalKV(
    JNIEnv *env, jobject o, jlong gcs_accessor_ptr, jstring n, jstring k) {
  std::string key = JavaStringToNativeString(env, k);
  std::string ns = JavaStringToNativeString(env, n);
  auto *gcs_accessor = reinterpret_cast<gcs::GlobalStateAccessor *>(gcs_accessor_ptr);
  auto value = gcs_accessor->GetInternalKV(ns, key);
  if (value) {
    return NativeStringToJavaByteArray(env, *value);
  }
  return nullptr;
}

JNIEXPORT jbyteArray JNICALL
Java_io_ray_runtime_gcs_GlobalStateAccessor_nativeGetNodeToConnectForDriver(
    JNIEnv *env, jobject o, jlong gcs_accessor_ptr, jstring nodeIpAddress) {
  std::string node_ip_address = JavaStringToNativeString(env, nodeIpAddress);
  auto *gcs_accessor = reinterpret_cast<gcs::GlobalStateAccessor *>(gcs_accessor_ptr);
  std::string node_to_connect;
  auto status =
      gcs_accessor->GetNodeToConnectForDriver(node_ip_address, &node_to_connect);
  THROW_EXCEPTION_AND_RETURN_IF_NOT_OK(env, status, nullptr);
  return NativeStringToJavaByteArray(env, node_to_connect);
}

#ifdef __cplusplus
}
#endif
