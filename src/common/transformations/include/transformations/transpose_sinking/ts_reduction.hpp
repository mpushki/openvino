// Copyright (C) 2022-2023 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#pragma once

#include "openvino/pass/graph_rewrite.hpp"
#include "openvino/pass/pass.hpp"
#include "transformations_visibility.hpp"

namespace ov {
namespace pass {
namespace transpose_sinking {

class TRANSFORMATIONS_API TSReductionForward;
class TRANSFORMATIONS_API TSReductionBackward;

}  // namespace transpose_sinking
}  // namespace pass
}  // namespace ov

/**
 * @ingroup ie_transformation_common_api
 * @brief TransposeReductionForward transformation sinks Transpose through Reduce operations
 * in the forward direction.
 */
class ov::pass::transpose_sinking::TSReductionForward : public ov::pass::MatcherPass {
public:
    OPENVINO_RTTI("ov::pass::TSReductionForward", "0");
    TSReductionForward();
};

/**
 * @ingroup ie_transformation_common_api
 * @brief TransposeReductionBackward transformation sinks Transpose through Reduce operations
 * in the backward direction.
 */
class ov::pass::transpose_sinking::TSReductionBackward : public ov::pass::MatcherPass {
public:
    OPENVINO_RTTI("ov::pass::TSReductionBackward", "0");
    TSReductionBackward();
};