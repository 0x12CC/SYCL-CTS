/*******************************************************************************
//
//  SYCL 2020 Conformance Test Suite
//
//  Copyright (c) 2023 The Khronos Group Inc.
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
//
//  Provides common functions for the sycl::atomic_ref tests.
//
*******************************************************************************/

#ifndef SYCL_CTS_ATOMIC_REF_COMMON_H
#define SYCL_CTS_ATOMIC_REF_COMMON_H

#include "../common/common.h"
#include "../common/section_name_builder.h"
#include "../common/type_coverage.h"

// FIXME: legal address spaces are not yet defined for atomic_ref used on the
// host, it's possible that will be decided that atomic_ref isn't allowed in
// host code at all. It can be tracked in this issue
// https://gitlab.khronos.org/sycl/Specification/-/issues/637. When the decision
// about atomic_ref usage have been done re-enable test running on host side
// by setting this define to 1 or remove it and the corresponding code
// conserning of testing sycl::atomic_ref on host side
#define SYCL_CTS_ATOMIC_REF_ON_HOST 0

namespace atomic_ref::tests::common {
using namespace sycl_cts;

constexpr int expected_val = 42;
constexpr int changed_val = 1;

/**
 * @brief Function helps to get string section name that will contain template
 * parameters and function arguments
 *
 * @tparam Dimension Integer representing dimension
 * @param type_name String with name of the testing type
 * @param memory_order_name String with name of the testing memory_order
 * @param memory_scope_name String with name of the testing memory_scope
 * @param address_space String with name of the address_space
 * @param section_description String with human-readable description of the test
 * @return std::string String with name for section
 */
inline std::string get_section_name(const std::string& type_name,
                                    const std::string& memory_order_name,
                                    const std::string& memory_scope_name,
                                    const std::string& address_space_name,
                                    const std::string& section_description) {
  return section_name(section_description)
      .with("T", type_name)
      .with("memory_order", memory_order_name)
      .with("memory_scope", memory_scope_name)
      .with("address_space", address_space_name)
      .create();
}

/**
 * @brief Function helps to get string section name that will contain template
 * parameters and function arguments
 *
 * @tparam Dimension Integer representing dimension
 * @param type_name String with name of the testing type
 * @param memory_order_name String with name of the testing memory_order
 * @param memory_scope_name String with name of the testing memory_scope
 * @param address_space String with name of the address_space
 * @param memory_order sycl::memory_order which will be used as parameter of
 * atomic_ref method
 * @param momory_scope sycl::memory_scope which will be used as parameter of
 * atomic_ref method
 * @param section_description String with human-readable description of the test
 * @return std::string String with name for section
 */
inline std::string get_section_name(const std::string& type_name,
                                    const std::string& memory_order_name,
                                    const std::string& memory_scope_name,
                                    const std::string& address_space_name,
                                    const sycl::memory_order& memory_order,
                                    const sycl::memory_scope& memory_scope,
                                    const std::string& section_description) {
  return section_name(section_description)
      .with("T", type_name)
      .with("memory_order", memory_order_name)
      .with("memory_scope", memory_scope_name)
      .with("address_space", address_space_name)
      .with("memory_order arg", memory_order)
      .with("memory_scope arg", memory_scope)
      .create();
}

/**
 * @brief Factory function for getting type_pack with fp64 type
 */
inline auto get_atomic64_types() {
  static const auto types =
#if SYCL_CTS_ENABLE_FULL_CONFORMANCE
      named_type_pack<long long, unsigned long long, double>::generate(
          "long long", "unsigned long long", "double");
#else
      named_type_pack<long long, double>::generate("long long", "double");
#endif
  return types;
}

/**
 * @brief Factory function for getting type_pack with all generic types
 */
inline auto get_full_conformance_type_pack() {
  static const auto types =
      named_type_pack<int, unsigned int, long int, unsigned long int,
                      float>::generate("int", "unsigned int", "long int",
                                       "unsigned long int", "float");
  return types;
}

/**
 * @brief Factory function for getting type_pack with generic types
 */
inline auto get_lightweight_type_pack() {
  static const auto types =
      named_type_pack<int, float>::generate("int", "float");
  return types;
}

/**
 * @brief Factory function for getting type_pack with types that depends on full
 *        conformance mode enabling status
 * @return lightweight or full named_type_pack
 */
inline auto get_conformance_type_pack() {
#if SYCL_CTS_ENABLE_FULL_CONFORMANCE
  return get_full_conformance_type_pack();
#else
  return get_lightweight_type_pack();
#endif  // SYCL_CTS_ENABLE_FULL_CONFORMANCE
}

/**
 * @brief Factory function for getting type_pack with memory_order values
 */
inline auto get_memory_orders() {
  static const auto memory_orders =
      value_pack<sycl::memory_order, sycl::memory_order::relaxed,
                 sycl::memory_order::acq_rel,
                 sycl::memory_order::seq_cst>::generate_named();
  return memory_orders;
}

/**
 * @brief Factory function for getting type_pack with memory_scope values
 */
inline auto get_memory_scopes() {
  static const auto memory_scopes =
      value_pack<sycl::memory_scope, sycl::memory_scope::work_item,
                 sycl::memory_scope::sub_group, sycl::memory_scope::work_group,
                 sycl::memory_scope::device,
                 sycl::memory_scope::system>::generate_named();
  return memory_scopes;
}

/**
 * @brief Factory function for getting type_pack with address_space values
 */
inline auto get_address_spaces() {
  static const auto address_spaces =
      value_pack<sycl::access::address_space,
                 sycl::access::address_space::global_space,
                 sycl::access::address_space::local_space,
                 sycl::access::address_space::generic_space>::generate_named();
  return address_spaces;
}

inline bool memory_order_is_supported(sycl::queue& q,
                                      sycl::memory_order order) {
  std::vector<sycl::memory_order> memory_orders_supported =
      q.get_device()
          .get_info<sycl::info::device::atomic_memory_order_capabilities>();
  auto it = std::find(memory_orders_supported.begin(),
                      memory_orders_supported.end(), order);
  return it != memory_orders_supported.end();
}

inline bool memory_scope_is_suppoted(sycl::queue& q, sycl::memory_scope scope) {
  std::vector<sycl::memory_scope> memory_scopes_supported =
      q.get_device()
          .get_info<sycl::info::device::atomic_memory_scope_capabilities>();
  auto it = std::find(memory_scopes_supported.begin(),
                      memory_scopes_supported.end(), scope);
  return it != memory_scopes_supported.end();
}

inline bool memory_order_and_scope_are_supported(sycl::queue& q,
                                                 sycl::memory_order order,
                                                 sycl::memory_scope scope) {
  return memory_order_is_supported(q, order) &&
         memory_scope_is_suppoted(q, scope);
}

inline bool memory_order_and_scope_are_not_supported(sycl::queue& q,
                                                     sycl::memory_order order,
                                                     sycl::memory_scope scope) {
  return !memory_order_and_scope_are_supported(q, order, scope);
}

/**
 * @brief Function to compare two floatinig point values
 */
template <typename T>
bool compare_act_and_expd_with_epsilon(T actual, T expected, T eps) {
  return ((expected - eps) <= actual) && (actual <= (expected - eps));
}

}  // namespace atomic_ref::tests::common

#endif  // SYCL_CTS_ATOMIC_REF_COMMON_H
