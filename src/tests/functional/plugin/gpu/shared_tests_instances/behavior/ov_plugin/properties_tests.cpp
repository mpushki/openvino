// Copyright (C) 2018-2023 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include "behavior/ov_plugin/properties_tests.hpp"
#include <openvino/runtime/auto/properties.hpp>

using namespace ov::test::behavior;
using namespace InferenceEngine::PluginConfigParams;

namespace {

const std::vector<ov::AnyMap> gpu_properties = {
        {ov::hint::performance_mode(ov::hint::PerformanceMode::LATENCY)},
        {ov::hint::performance_mode(ov::hint::PerformanceMode::THROUGHPUT)},
};

INSTANTIATE_TEST_SUITE_P(smoke_BehaviorTests, OVPropertiesTests,
        ::testing::Combine(
                ::testing::Values(CommonTestUtils::DEVICE_GPU),
                ::testing::ValuesIn(gpu_properties)),
        OVPropertiesTests::getTestCaseName);

auto auto_multi_properties = []() {
    return std::vector<ov::AnyMap>{
        {ov::device::priorities(CommonTestUtils::DEVICE_GPU),
         ov::hint::performance_mode(ov::hint::PerformanceMode::THROUGHPUT)},
        {ov::device::priorities(CommonTestUtils::DEVICE_GPU),
         ov::hint::performance_mode(ov::hint::PerformanceMode::LATENCY)},
        {ov::device::priorities(CommonTestUtils::DEVICE_GPU),
         ov::hint::performance_mode(ov::hint::PerformanceMode::CUMULATIVE_THROUGHPUT)},
        {ov::device::priorities(CommonTestUtils::DEVICE_GPU), ov::intel_auto::device_bind_buffer("YES")},
        {ov::device::priorities(CommonTestUtils::DEVICE_GPU), ov::intel_auto::device_bind_buffer("NO")},
        {ov::device::priorities(CommonTestUtils::DEVICE_GPU), ov::intel_auto::enable_startup_fallback("YES")},
        {ov::device::priorities(CommonTestUtils::DEVICE_GPU), ov::intel_auto::enable_startup_fallback("NO")}};
};

const std::vector<ov::AnyMap> multi_properties = {
        {ov::device::priorities("CPU", "GPU")},
        {ov::device::priorities("CPU(1)", "GPU")},
        {ov::device::priorities("CPU(1)", "GPU(2)")}
};

const std::vector<ov::AnyMap> auto_properties = {
        {ov::device::priorities("CPU", "GPU")},
        {ov::device::priorities("-CPU", "GPU")},
        {ov::device::priorities("CPU(1)", "GPU")},
        {ov::device::priorities("CPU(1)", "GPU(2)")},
        {ov::device::priorities("CPU", "-GPU")}
};


const std::vector<ov::AnyMap> compiled_empty_properties = {
        {}
};

const std::vector<ov::AnyMap> incorrect_device_priorities_properties = {
        {ov::device::priorities("NONE")},
        {ov::device::priorities("NONE", "GPU")},
        {ov::device::priorities("-", "GPU")},
        {ov::device::priorities("-NONE", "CPU")},
        {ov::device::priorities("-CPU", "-NONE")},
        {ov::device::priorities("-NONE", "-NONE")}
};

INSTANTIATE_TEST_SUITE_P(smoke_AutoMultiBehaviorTests, OVPropertiesTests,
        ::testing::Combine(
                ::testing::Values(CommonTestUtils::DEVICE_AUTO, CommonTestUtils::DEVICE_MULTI),
                ::testing::ValuesIn(auto_multi_properties())),
        OVPropertiesTests::getTestCaseName);

INSTANTIATE_TEST_SUITE_P(smoke_AutoBehaviorTests, OVPropertiesTests,
        ::testing::Combine(
                ::testing::Values(CommonTestUtils::DEVICE_AUTO),
                ::testing::ValuesIn(auto_properties)),
        OVPropertiesTests::getTestCaseName);

INSTANTIATE_TEST_SUITE_P(smoke_MultiBehaviorTests, OVPropertiesTests,
        ::testing::Combine(
                ::testing::Values(CommonTestUtils::DEVICE_MULTI),
                ::testing::ValuesIn(multi_properties)),
        OVPropertiesTests::getTestCaseName);

INSTANTIATE_TEST_SUITE_P(smoke_BehaviorIncorrectPropertiesTests, OVSetPropCompileModelWithIncorrectPropTests,
        ::testing::Combine(
                ::testing::Values(CommonTestUtils::DEVICE_AUTO,
                                  CommonTestUtils::DEVICE_MULTI,
                                  CommonTestUtils::DEVICE_HETERO),
                ::testing::ValuesIn(incorrect_device_priorities_properties),
                ::testing::ValuesIn(compiled_empty_properties)),
        OVSetPropCompileModelWithIncorrectPropTests::getTestCaseName);

const std::vector<ov::AnyMap> gpu_setcore_properties = {
    {ov::hint::performance_mode(ov::hint::PerformanceMode::THROUGHPUT),
     ov::hint::num_requests(2),
     ov::enable_profiling(false)}};
const std::vector<ov::AnyMap> gpu_compileModel_properties = {
    {ov::hint::performance_mode(ov::hint::PerformanceMode::LATENCY),
     ov::hint::num_requests(10),
     ov::enable_profiling(true)}};

INSTANTIATE_TEST_SUITE_P(smoke_gpuCompileModelBehaviorTests,
                         OVSetPropComplieModleGetPropTests,
                         ::testing::Combine(::testing::Values(CommonTestUtils::DEVICE_GPU),
                                            ::testing::ValuesIn(gpu_setcore_properties),
                                            ::testing::ValuesIn(gpu_compileModel_properties)),
                         OVSetPropComplieModleGetPropTests::getTestCaseName);

const std::vector<ov::AnyMap> multi_setcore_properties = {
    {ov::device::priorities(CommonTestUtils::DEVICE_GPU),
     ov::hint::performance_mode(ov::hint::PerformanceMode::THROUGHPUT),
     ov::hint::model_priority(ov::hint::Priority::HIGH)}};
const std::vector<ov::AnyMap> multi_compileModel_properties = {
    {ov::device::priorities(CommonTestUtils::DEVICE_GPU),
     ov::hint::performance_mode(ov::hint::PerformanceMode::THROUGHPUT),
     ov::hint::model_priority(ov::hint::Priority::MEDIUM)}};

INSTANTIATE_TEST_SUITE_P(smoke_MultiCompileModelBehaviorTests,
                         OVSetPropComplieModleGetPropTests,
                         ::testing::Combine(::testing::Values(CommonTestUtils::DEVICE_MULTI),
                                            ::testing::ValuesIn(multi_setcore_properties),
                                            ::testing::ValuesIn(multi_compileModel_properties)),
                         OVSetPropComplieModleGetPropTests::getTestCaseName);

const std::vector<ov::AnyMap> auto_setcore_properties = {
    {ov::device::priorities(CommonTestUtils::DEVICE_GPU),
     ov::hint::performance_mode(ov::hint::PerformanceMode::THROUGHPUT),
     ov::hint::model_priority(ov::hint::Priority::HIGH)},
    {ov::device::priorities(CommonTestUtils::DEVICE_GPU),
     ov::hint::performance_mode(ov::hint::PerformanceMode::LATENCY),
     ov::hint::model_priority(ov::hint::Priority::HIGH)},
    {ov::device::priorities(CommonTestUtils::DEVICE_GPU),
     ov::hint::performance_mode(ov::hint::PerformanceMode::CUMULATIVE_THROUGHPUT),
     ov::hint::model_priority(ov::hint::Priority::HIGH)},
};
const std::vector<ov::AnyMap> auto_compileModel_properties = {
    {ov::device::priorities(CommonTestUtils::DEVICE_GPU),
     ov::hint::performance_mode(ov::hint::PerformanceMode::LATENCY),
     ov::hint::model_priority(ov::hint::Priority::MEDIUM)},
    {ov::device::priorities(CommonTestUtils::DEVICE_GPU),
     ov::hint::performance_mode(ov::hint::PerformanceMode::CUMULATIVE_THROUGHPUT),
     ov::hint::model_priority(ov::hint::Priority::MEDIUM)},
    {ov::device::priorities(CommonTestUtils::DEVICE_GPU),
     ov::hint::performance_mode(ov::hint::PerformanceMode::THROUGHPUT),
     ov::hint::model_priority(ov::hint::Priority::MEDIUM)}};
INSTANTIATE_TEST_SUITE_P(smoke_AutoCompileModelBehaviorTests,
                         OVSetPropComplieModleGetPropTests,
                         ::testing::Combine(::testing::Values(CommonTestUtils::DEVICE_AUTO),
                                            ::testing::ValuesIn(auto_setcore_properties),
                                            ::testing::ValuesIn(auto_compileModel_properties)),
                         OVSetPropComplieModleGetPropTests::getTestCaseName);

const std::vector<std::pair<ov::AnyMap, std::string>> autoExeDeviceConfigs = {
            std::make_pair(ov::AnyMap{{ov::device::priorities("GPU.0")}}, "GPU.0"),
            #ifdef ENABLE_INTEL_CPU
            std::make_pair(ov::AnyMap{{ov::device::priorities(CommonTestUtils::DEVICE_GPU, CommonTestUtils::DEVICE_CPU)}}, "undefined"),
            std::make_pair(ov::AnyMap{{ov::device::priorities(CommonTestUtils::DEVICE_CPU, CommonTestUtils::DEVICE_GPU)}}, "CPU"),
            std::make_pair(ov::AnyMap{{ov::device::priorities(CommonTestUtils::DEVICE_CPU, CommonTestUtils::DEVICE_GPU),
                                        ov::hint::performance_mode(ov::hint::PerformanceMode::CUMULATIVE_THROUGHPUT)}}, "CPU,GPU"),
            std::make_pair(ov::AnyMap{{ov::device::priorities(CommonTestUtils::DEVICE_GPU, CommonTestUtils::DEVICE_CPU),
                                        ov::hint::performance_mode(ov::hint::PerformanceMode::CUMULATIVE_THROUGHPUT)}}, "GPU,CPU"),
            std::make_pair(ov::AnyMap{{ov::device::priorities(CommonTestUtils::DEVICE_GPU, CommonTestUtils::DEVICE_CPU),
                                        ov::hint::performance_mode(ov::hint::PerformanceMode::CUMULATIVE_THROUGHPUT),
                                        ov::hint::allow_auto_batching(true)}}, "GPU,CPU"),
            #endif
    };

const std::vector<std::pair<ov::AnyMap, std::string>> multiExeDeviceConfigs = {
            std::make_pair(ov::AnyMap{{ov::device::priorities("GPU.0")}}, "GPU.0"),
            #ifdef ENABLE_INTEL_CPU
            std::make_pair(ov::AnyMap{{ov::device::priorities(CommonTestUtils::DEVICE_GPU, CommonTestUtils::DEVICE_CPU)}}, "GPU,CPU"),
            std::make_pair(ov::AnyMap{{ov::device::priorities(CommonTestUtils::DEVICE_CPU, CommonTestUtils::DEVICE_GPU)}}, "CPU,GPU"),
            std::make_pair(ov::AnyMap{{ov::device::priorities(CommonTestUtils::DEVICE_CPU, CommonTestUtils::DEVICE_GPU),
                                        ov::hint::performance_mode(ov::hint::PerformanceMode::CUMULATIVE_THROUGHPUT)}}, "CPU,GPU"),
            std::make_pair(ov::AnyMap{{ov::device::priorities(CommonTestUtils::DEVICE_GPU, CommonTestUtils::DEVICE_CPU),
                                        ov::hint::performance_mode(ov::hint::PerformanceMode::CUMULATIVE_THROUGHPUT)}}, "GPU,CPU"),
            std::make_pair(ov::AnyMap{{ov::device::priorities(CommonTestUtils::DEVICE_GPU, CommonTestUtils::DEVICE_CPU),
                                        ov::hint::performance_mode(ov::hint::PerformanceMode::CUMULATIVE_THROUGHPUT),
                                        ov::hint::allow_auto_batching(true)}}, "GPU,CPU"),
            #endif
    };

INSTANTIATE_TEST_SUITE_P(smoke_AutoMultiCompileModelBehaviorTests,
                         OVCompileModelGetExecutionDeviceTests,
                         ::testing::Combine(::testing::Values(CommonTestUtils::DEVICE_AUTO),
                                            ::testing::ValuesIn(autoExeDeviceConfigs)),
                         OVCompileModelGetExecutionDeviceTests::getTestCaseName);

INSTANTIATE_TEST_SUITE_P(smoke_MultiCompileModelBehaviorTests,
                         OVCompileModelGetExecutionDeviceTests,
                         ::testing::Combine(::testing::Values(CommonTestUtils::DEVICE_MULTI),
                                            ::testing::ValuesIn(multiExeDeviceConfigs)),
                         OVCompileModelGetExecutionDeviceTests::getTestCaseName);
} // namespace
