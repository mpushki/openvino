// Copyright (C) 2018-2023 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

/**
 * @brief a header file for ExecutableNetwork
 * @file executable_network.hpp
 */
#pragma once

#include <ie_common.h>

#include <cpp_interfaces/impl/ie_executable_network_thread_safe_default.hpp>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "async_infer_request.hpp"
#include "ie_icore.hpp"
#include "infer_request.hpp"
#include "plugin.hpp"

namespace HeteroPlugin {

/**
 * @class ExecutableNetwork
 * @brief Interface of executable network
 */
class HeteroExecutableNetwork : public InferenceEngine::ExecutableNetworkThreadSafeDefault {
public:
    typedef std::shared_ptr<HeteroExecutableNetwork> Ptr;

    HeteroExecutableNetwork(const InferenceEngine::CNNNetwork& network, const Configs& user_config, Engine* plugin);
    HeteroExecutableNetwork(std::istream& heteroModel, const Configs& user_config, Engine* plugin);

    InferenceEngine::IInferRequestInternal::Ptr CreateInferRequestImpl(
        InferenceEngine::InputsDataMap networkInputs,
        InferenceEngine::OutputsDataMap networkOutputs) override;

    InferenceEngine::IInferRequestInternal::Ptr CreateInferRequestImpl(
        const std::vector<std::shared_ptr<const ov::Node>>& inputs,
        const std::vector<std::shared_ptr<const ov::Node>>& outputs) override;

    InferenceEngine::IInferRequestInternal::Ptr CreateInferRequest() override;

    InferenceEngine::Parameter GetConfig(const std::string& name) const override;

    InferenceEngine::Parameter GetMetric(const std::string& name) const override;

    void Export(std::ostream& modelFile) override;

private:
    struct NetworkDesc {
        std::string _device;
        InferenceEngine::CNNNetwork _clonedNetwork;
        InferenceEngine::SoExecutableNetworkInternal _network;
    };

    std::vector<NetworkDesc> _networks;
    Engine* _heteroPlugin;
    std::string _name;
    Configs _hetero_config;
    Configs _device_config;
    std::unordered_map<std::string, std::string> _blobNameMap;
};

}  // namespace HeteroPlugin
