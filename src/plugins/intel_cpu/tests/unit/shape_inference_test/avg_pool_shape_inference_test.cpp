// Copyright (C) 2018-2023 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include <gmock/gmock.h>

#include "common_test_utils/test_assertions.hpp"
#include "openvino/opsets/opset10.hpp"
#include "utils.hpp"

using namespace ov;
using namespace ov::intel_cpu;
using namespace testing;

class AvgPoolV1StaticShapeInferenceTest : public OpStaticShapeInferenceTest<op::v1::AvgPool> {
protected:
    void SetUp() override {
        output_shapes.resize(1);
    }
};

TEST_F(AvgPoolV1StaticShapeInferenceTest, default_ctor) {
    op = make_op();
    op->set_strides({1, 1});
    op->set_pads_begin({2, 2});
    op->set_pads_end({2, 1});
    op->set_kernel({3, 2});
    op->set_rounding_type(op::RoundingType::FLOOR);
    op->set_auto_pad(op::PadType::VALID);

    input_shapes = ShapeVector{{1, 3, 10, 12}};
    auto shape_infer = make_shape_inference(op);
    output_shapes = shape_infer->infer(input_shapes, {}).shapes;

    EXPECT_EQ(output_shapes.size(), 1);
    EXPECT_EQ(output_shapes.front(), StaticShape({1, 3, 8, 11}));
    EXPECT_EQ(shape_infer->get_pads_begin(), CoordinateDiff({0, 0}));
    EXPECT_EQ(shape_infer->get_pads_end(), CoordinateDiff({0, 0}));
}

TEST_F(AvgPoolV1StaticShapeInferenceTest, no_auto_pad_round_floor) {
    const auto data = std::make_shared<op::v0::Parameter>(element::f64, PartialShape{-1, -1, -1, -1});

    const Strides strides{1, 1};
    const Shape pads_begin{2, 2};
    const Shape pads_end{2, 1};
    const Shape kernel_shape{3, 2};
    const auto rounding_mode = op::RoundingType::FLOOR;
    const auto pad_type = op::PadType::EXPLICIT;

    op = make_op(data, strides, pads_begin, pads_end, kernel_shape, false, rounding_mode, pad_type);

    input_shapes = ShapeVector{{1, 3, 10, 12}};
    auto shape_infer = make_shape_inference(op);
    output_shapes = shape_infer->infer(input_shapes, {}).shapes;

    EXPECT_EQ(output_shapes.size(), 1);
    EXPECT_EQ(output_shapes.front(), StaticShape({1, 3, 12, 14}));
    EXPECT_EQ(shape_infer->get_pads_begin(), CoordinateDiff({2, 2}));
    EXPECT_EQ(shape_infer->get_pads_end(), CoordinateDiff({2, 1}));
}

TEST_F(AvgPoolV1StaticShapeInferenceTest, auto_padding_same_lower_round_ceil) {
    const auto data = std::make_shared<op::v0::Parameter>(element::f64, PartialShape::dynamic());

    const Strides strides{1, 3, 2};
    const Shape pads_begin{2, 2, 1};
    const Shape pads_end{2, 1, 10};
    const Shape kernel_shape{5, 5, 5};
    const auto rounding_mode = op::RoundingType::CEIL;
    const auto pad_type = op::PadType::SAME_LOWER;

    op = make_op(data, strides, pads_begin, pads_end, kernel_shape, false, rounding_mode, pad_type);

    input_shapes = ShapeVector{{1, 3, 10, 12, 20}};
    auto shape_infer = make_shape_inference(op);
    output_shapes = shape_infer->infer(input_shapes, {}).shapes;

    EXPECT_EQ(output_shapes.size(), 1);
    EXPECT_EQ(output_shapes.front(), StaticShape({1, 3, 10, 4, 10}));
    EXPECT_EQ(shape_infer->get_pads_begin(), CoordinateDiff({2, 1, 2}));
    EXPECT_EQ(shape_infer->get_pads_end(), CoordinateDiff({2, 1, 1}));
}

TEST_F(AvgPoolV1StaticShapeInferenceTest, auto_padding_same_upper_round_floor_exclude_pad) {
    const auto data = std::make_shared<op::v0::Parameter>(element::f64, PartialShape::dynamic());

    const Strides strides{1, 3, 2};
    const Shape pads_begin{2, 2, 1};
    const Shape pads_end{2, 1, 10};
    const Shape kernel_shape{5, 5, 5};
    const auto rounding_mode = op::RoundingType::FLOOR;
    const auto pad_type = op::PadType::SAME_UPPER;

    op = make_op(data, strides, pads_begin, pads_end, kernel_shape, true, rounding_mode, pad_type);

    input_shapes = ShapeVector{{1, 3, 10, 12, 20}};
    auto shape_infer = make_shape_inference(op);
    output_shapes = shape_infer->infer(input_shapes, {}).shapes;

    EXPECT_EQ(output_shapes.size(), 1);
    EXPECT_EQ(output_shapes.front(), StaticShape({1, 3, 10, 4, 10}));
    EXPECT_EQ(shape_infer->get_pads_begin(), CoordinateDiff({2, 1, 1}));
    EXPECT_EQ(shape_infer->get_pads_end(), CoordinateDiff({2, 1, 2}));
}

TEST_F(AvgPoolV1StaticShapeInferenceTest, 5d_auto_padding_same_upper_round_floor) {
    const auto data = std::make_shared<op::v0::Parameter>(element::f64, PartialShape::dynamic());

    const Strides strides{1, 1, 1};
    const Shape pads_begin{0, 0, 0};
    const Shape pads_end{0, 0, 0};
    const Shape kernel_shape{2, 2, 2};
    const auto rounding_mode = op::RoundingType::FLOOR;
    const auto pad_type = op::PadType::SAME_UPPER;

    op = make_op(data, strides, pads_begin, pads_end, kernel_shape, true, rounding_mode, pad_type);

    input_shapes = ShapeVector{{32, 32, 2, 2, 4}};
    auto shape_infer = make_shape_inference(op);
    output_shapes = shape_infer->infer(input_shapes, {}).shapes;

    EXPECT_EQ(output_shapes.size(), 1);
    EXPECT_EQ(output_shapes.front(), StaticShape({32, 32, 2, 2, 4}));
    EXPECT_EQ(shape_infer->get_pads_begin(), CoordinateDiff({0, 0, 0}));
    EXPECT_EQ(shape_infer->get_pads_end(), CoordinateDiff({1, 1, 1}));
}
