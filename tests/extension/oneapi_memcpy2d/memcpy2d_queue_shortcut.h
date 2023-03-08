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
//  Provides class for tests to check queue class functions gained with
//  oneapi_memcpy2d extension
//
*******************************************************************************/

#ifndef SYCL_CTS_MEMCPY2D_MEMCPY2D_QUEUE_SHORTCUT_H
#define SYCL_CTS_MEMCPY2D_MEMCPY2D_QUEUE_SHORTCUT_H

#include <memory>

#include "catch2/catch_test_macros.hpp"

#include "../../common/section_name_builder.h"
#include "../../common/value_operations.h"

#include "memcpy2d_common.h"

namespace memcpy2d_queue_shortcut_tests {
using namespace memcpy2d_common_tests;

#ifdef SYCL_EXT_ONEAPI_MEMCPY2D

template <typename T, typename SrcPtrT, typename DestPtrT>
class run_queue_shortcut_tests {
  static constexpr pointer_type SrcPtrType = SrcPtrT::value;
  static constexpr pointer_type DestPtrType = DestPtrT::value;

 public:
  void operator()(sycl::queue& queue, const std::string& t_name,
                  const std::string& src_ptr_type_name,
                  const std::string& dest_ptr_type_name) {
    if (!check_device_aspect_allocations<SrcPtrType, DestPtrType>(queue)) {
      SKIP(
          "Device does not support USM device allocations. "
          "Skipping the test case.");
    }

    T init_v = value_operations::init<T>(init_val);
    T expected_v = value_operations::init<T>(expected_val);

    auto src = allocate_memory<T, SrcPtrType>(src_pitch * array_height, queue);
    fill_memory<T, SrcPtrType>(src.get(), expected_v, src_pitch * array_height,
                               queue);

    auto dst1 =
        allocate_memory<T, DestPtrType>(dest_pitch * array_height, queue);
    fill_memory<T, DestPtrType>(dst1.get(), init_v, dest_pitch * array_height,
                                queue);

    auto dst2 =
        allocate_memory<T, DestPtrType>(dest_pitch * array_height, queue);
    fill_memory<T, DestPtrType>(dst2.get(), init_v, dest_pitch * array_height,
                                queue);

    auto dst3 =
        allocate_memory<T, DestPtrType>(dest_pitch * array_height, queue);
    fill_memory<T, DestPtrType>(dst3.get(), init_v, dest_pitch * array_height,
                                queue);

    constexpr size_t result_size = dest_pitch * array_height;
    T result[result_size];
    T result2[result_size];
    T result3[result_size];

    if constexpr (std::is_same_v<T, unsigned char>) {
      SECTION(sycl_cts::section_name(std::string("Check memcpy2d with T = ") +
                                     t_name +
                                     " src_ptr_type = " + src_ptr_type_name +
                                     " and"
                                     " dest_ptr_type = " +
                                     dest_ptr_type_name)
                  .create()) {
        auto dest_address = get_region_address(dst1.get(), dest_pitch);
        auto src_address = get_region_address(src.get(), src_pitch);
        auto event1 =
            queue.ext_oneapi_memcpy2d(dest_address, dest_pitch, src_address,
                                      src_pitch, region_width, region_height);

        dest_address = get_region_address(dst2.get(), dest_pitch);
        src_address = get_region_address(dst1.get(), dest_pitch);
        auto event2 = queue.ext_oneapi_memcpy2d(
            dest_address, dest_pitch, src_address, dest_pitch, region_width,
            region_height, event1);

        dest_address = get_region_address(dst3.get(), dest_pitch);
        src_address = get_region_address(dst2.get(), dest_pitch);
        queue.ext_oneapi_memcpy2d(dest_address, dest_pitch, src_address,
                                  dest_pitch, region_width, region_height,
                                  {event1, event2});

        queue.wait();
        copy_destination_to_host_result<DestPtrType>(dst3.get(), result,
                                                     result_size, queue);

        for_index([&](size_t index) {
          T val = get_expected_value(index, init_v, expected_v);
          CHECK(val == result[index]);
        });
      }
    }
    SECTION(sycl_cts::section_name(std::string("Check copy2d with T = ") +
                                   t_name +
                                   " src_ptr_type = " + src_ptr_type_name +
                                   " and"
                                   " dest_ptr_type = " +
                                   dest_ptr_type_name)
                .create()) {
      auto dest_address = get_region_address(dst1.get(), dest_pitch);
      auto src_address = get_region_address(src.get(), src_pitch);
      auto event1 =
          queue.ext_oneapi_copy2d(src_address, src_pitch, dest_address,
                                  dest_pitch, region_width, region_height);

      dest_address = get_region_address(dst2.get(), dest_pitch);
      src_address = get_region_address(dst1.get(), dest_pitch);
      auto event2 = queue.ext_oneapi_copy2d(
          src_address, dest_pitch, dest_address, dest_pitch, region_width,
          region_height, event1);

      dest_address = get_region_address(dst3.get(), dest_pitch);
      src_address = get_region_address(dst2.get(), dest_pitch);
      queue.ext_oneapi_copy2d(src_address, dest_pitch, dest_address, dest_pitch,
                              region_width, region_height, {event1, event2});

      queue.wait();
      copy_destination_to_host_result<DestPtrType>(dst3.get(), result,
                                                   result_size, queue);

      for_index([&](size_t index) {
        T val = get_expected_value(index, init_v, expected_v);
        CHECK(val == result[index]);
      });
    }
    if constexpr (std::is_same_v<T, unsigned char>) {
      SECTION(sycl_cts::section_name(std::string("Check memset2d with T = ") +
                                     t_name +
                                     " src_ptr_type = " + src_ptr_type_name +
                                     " and"
                                     " dest_ptr_type = " +
                                     dest_ptr_type_name)
                  .create()) {
        int value = expected_val;

        auto dest_address = get_region_address(dst1.get(), dest_pitch);
        auto src_address = get_region_address(src.get(), src_pitch);
        auto event1 = queue.ext_oneapi_memset2d(dest_address, dest_pitch, value,
                                                region_width, region_height);

        dest_address = get_region_address(dst2.get(), dest_pitch);
        src_address = get_region_address(dst1.get(), dest_pitch);
        auto event2 =
            queue.ext_oneapi_memset2d(dest_address, dest_pitch, value,
                                      region_width, region_height, event1);

        dest_address = get_region_address(dst3.get(), dest_pitch);
        src_address = get_region_address(dst2.get(), dest_pitch);
        queue.ext_oneapi_memset2d(dest_address, dest_pitch, value, region_width,
                                  region_height, {event1, event2});

        queue.wait();
        copy_destination_to_host_result<DestPtrType>(dst1.get(), result,
                                                     result_size, queue);
        for_index([&](size_t index) {
          T val = get_expected_value(index, init_v, expected_v);
          CHECK(val == result[index]);
        });

        copy_destination_to_host_result<DestPtrType>(dst2.get(), result2,
                                                     result_size, queue);
        for_index([&](size_t index) {
          T val = get_expected_value(index, init_v, expected_v);
          CHECK(val == result2[index]);
        });

        copy_destination_to_host_result<DestPtrType>(dst3.get(), result3,
                                                     result_size, queue);
        for_index([&](size_t index) {
          T val = get_expected_value(index, init_v, expected_v);
          CHECK(val == result3[index]);
        });
      }
    }
    SECTION(sycl_cts::section_name(std::string("Check fill2d with T = ") +
                                   t_name +
                                   " src_ptr_type = " + src_ptr_type_name +
                                   " and"
                                   " dest_ptr_type = " +
                                   dest_ptr_type_name)
                .create()) {
      T value = value_operations::init<T>(expected_val);

      auto dest_address = get_region_address(dst1.get(), dest_pitch);
      auto src_address = get_region_address(src.get(), src_pitch);
      auto event1 = queue.ext_oneapi_fill2d(dest_address, dest_pitch, value,
                                            region_width, region_height);

      dest_address = get_region_address(dst2.get(), dest_pitch);
      src_address = get_region_address(dst1.get(), dest_pitch);
      auto event2 = queue.ext_oneapi_fill2d(
          dest_address, dest_pitch, value, region_width, region_height, event1);

      dest_address = get_region_address(dst3.get(), dest_pitch);
      src_address = get_region_address(dst2.get(), dest_pitch);
      queue.ext_oneapi_fill2d(dest_address, dest_pitch, value, region_width,
                              region_height, {event1, event2});

      queue.wait();
      copy_destination_to_host_result<DestPtrType>(dst1.get(), result,
                                                   result_size, queue);
      for_index([&](size_t index) {
        T val = get_expected_value(index, init_v, expected_v);
        CHECK(val == result[index]);
      });

      copy_destination_to_host_result<DestPtrType>(dst2.get(), result2,
                                                   result_size, queue);
      for_index([&](size_t index) {
        T val = get_expected_value(index, init_v, expected_v);
        CHECK(val == result2[index]);
      });

      copy_destination_to_host_result<DestPtrType>(dst3.get(), result3,
                                                   result_size, queue);
      for_index([&](size_t index) {
        T val = get_expected_value(index, init_v, expected_v);
        CHECK(val == result3[index]);
      });
    }
  }
};

#endif  // SYCL_EXT_ONEAPI_MEMCPY2D

}  // namespace memcpy2d_queue_shortcut_tests

#endif  // SYCL_CTS_MEMCPY2D_MEMCPY2D_QUEUE_SHORTCUT_H
