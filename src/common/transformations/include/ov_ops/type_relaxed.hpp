// Copyright (C) 2018-2023 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#pragma once

#include <algorithm>
#include <memory>
#include <mutex>
#include <openvino/op/convert.hpp>
#include <string>
#include <transformations_visibility.hpp>
#include <vector>

namespace ov {
namespace op {

/// A base class for templated TypeRelaxed that maintains overridden input types and output types for an operation.
class OPENVINO_API TypeRelaxedBase {
public:
    virtual ~TypeRelaxedBase();

    explicit TypeRelaxedBase(const element::TypeVector& _input_data_types = {},
                             const element::TypeVector& _output_data_types = {})
        : m_input_data_types(_input_data_types),
          m_output_data_types(_output_data_types) {}

    /// \return Data type that will be set for output with a given index outputIndex.
    /// If output with a specified index outputIndex hasn't been set before, element::undefined will returned.
    /// Undefined means no type override happens for a given outputIndex and it will deduced as original
    /// operation defineds in its infer function.
    ///
    /// This method may look similar to Node::get_output_element_type, but it is not the same thing, because
    /// get_output_element_type returns the result of type inference, so it is completely deduced from
    /// an operation inputs and attributes, and get_overridden_output_type returns value of the attribute that
    /// is used to deduce output type. In some cases they don't match: get_overridden_output_type may return
    /// element::undefined for some index i, and get_output_element_type will return some real type for
    /// the same index i.
    const element::Type& get_overridden_output_type(size_t outputIndex = 0) const {
        if (outputIndex >= m_output_data_types.size()) {
            return element::undefined;
        }
        return m_output_data_types[outputIndex];
    }

    /// Set data type that overrides the original data type for output port with outputIndex index
    /// In case if outputIndex is out of range of known outputs (and this class cannot detect
    /// the real number of outputs for original operation), the number of overridden outputs
    /// is changed according to a given outputIndex value.
    void set_overridden_output_type(const element::Type& element_type, size_t outputIndex = 0) {
        if (outputIndex >= m_output_data_types.size()) {
            m_output_data_types.resize(outputIndex + 1, element::undefined);
        }
        m_output_data_types[outputIndex] = element_type;
    }

    /// \return Data type that will be set for input when original shape/type inference function is called.
    /// If index inputIndex hasn't been set before, element::undefined will returned. Undefined means that
    /// the type from input tensor descriptor is used for a given index.
    const element::Type& get_origin_input_type(size_t inputIndex = 0) const {
        if (inputIndex >= m_input_data_types.size()) {
            return element::undefined;
        }
        return m_input_data_types[inputIndex];
    }

    /// Set data type that overrides the original data type for input port with inputIndex index.
    /// In case if inputIndex is out of range of known inputs (and this class cannot detect
    /// the real number of inputs for original operation), the number of overridden inputs
    /// is changed according to a given inputIndex value. All new entries except one added
    /// at inputIndex position are undefined.
    void set_origin_input_type(const element::Type& element_type, size_t inputIndex = 0) {
        if (inputIndex >= m_input_data_types.size()) {
            m_input_data_types.resize(inputIndex + 1, element::undefined);
        }
        m_input_data_types[inputIndex] = element_type;
    }

protected:
    void remember_input_data_types(Node& node, element::TypeVector& old_input_types) {
        // Remember all input data types
        for (size_t i = 0; i < node.get_input_size(); ++i) {
            old_input_types.push_back(node.get_input_element_type(i));
        }

        // Reset input data types to m_output_data_type.
        for (size_t i = 0; i < node.get_input_size(); ++i) {
            auto origin_input_type = get_origin_input_type(i);
            if (origin_input_type != element::undefined) {
                OPENVINO_SUPPRESS_DEPRECATED_START
                node.get_input_tensor(i).set_tensor_type(origin_input_type, node.get_input_partial_shape(i));
                OPENVINO_SUPPRESS_DEPRECATED_END
            }
        }
    }

    void restore_input_data_types(Node& node, const element::TypeVector& old_input_types) {
        // Restore original input data types
        for (size_t i = 0; i < node.get_input_size(); ++i) {
            OPENVINO_SUPPRESS_DEPRECATED_START
            node.get_input_tensor(i).set_tensor_type(old_input_types[i], node.get_input_partial_shape(i));
            OPENVINO_SUPPRESS_DEPRECATED_END
        }

        if (m_original_output_data_types.empty()) {
            m_original_output_data_types = element::TypeVector(node.get_output_size());
        }

        // Save inferred output types
        for (size_t i = 0; i < node.get_output_size(); ++i) {
            m_original_output_data_types[i] = node.get_output_element_type(i);
        }

        // Override (some) output types
        for (size_t i = 0; i < node.get_output_size(); ++i) {
            auto overridden_output_type = get_overridden_output_type(i);
            if (overridden_output_type != element::undefined) {
                node.set_output_type(i, overridden_output_type, node.get_output_partial_shape(i));
            }
        }
    }

    void visit_attributes(AttributeVisitor& visitor) {
        bool type_relax = true;
        visitor.on_attribute("type_relax", type_relax);
        visitor.on_attribute("input_data_types", m_input_data_types);
        visitor.on_attribute("output_data_types", m_output_data_types);
    }

    typedef struct {
    } init_rt_result;

    init_rt_result init_rt_info(Node& node) const {
        node.get_rt_info()["opset"] = "type_relaxed_opset";
        return {};
    }

protected:
    // Data types that are used for parent shape/type infer function input ports
    // to infer output data types
    element::TypeVector m_input_data_types;
    element::TypeVector m_output_data_types;
    element::TypeVector m_original_output_data_types;
};

/// Set another type for a specified output for the period of time when an instance of the class exists.
/// When the execution leaves the scope where an onject of TemporaryReplaceOutputType is defined,
/// the type of the output is set to its original value. Used when initialized TypeRelaxed<BaseOp> operation
/// in case when inputs have types that are not compatible with BaseOp infer function. In this case
/// before TypeRelaxed is constructed the BaseOp contructor requires modified data types.
/// So it should be
class TemporaryReplaceOutputType {
    Output<Node> m_output;
    element::Type orig_type;

public:
    /// Replace element type for a given output port by tmp_type
    TemporaryReplaceOutputType(Output<Node> output, element::Type tmp_type) : m_output(output) {
        // save original element type in order to restore it in the destructor
        orig_type = m_output.get_element_type();
        OPENVINO_SUPPRESS_DEPRECATED_START
        m_output.get_tensor().set_element_type(tmp_type);
        OPENVINO_SUPPRESS_DEPRECATED_END
    }

    /// Return the output port that was used in the constructor
    Output<Node> get() const {
        return m_output;
    }

    /// Restores the original element type for the output
    ~TemporaryReplaceOutputType() {
        OPENVINO_SUPPRESS_DEPRECATED_START
        m_output.get_tensor().set_element_type(orig_type);
        OPENVINO_SUPPRESS_DEPRECATED_END
    }
};

// TODO: remove once FusedOp is removed
OPENVINO_SUPPRESS_DEPRECATED_START

/// Relaxes tensor element type requirements for BaseOp inputs and outputs
/// This class template should be used with Node descendant class. Defines a new operation by extending the
/// original BaseOp operation with ability to accept inputs and provide outputs with element type that is
/// unusual for BaseOp. For example, TypeRelaxed<opset1::Add> can accept mixed-precision inputs and provide
/// another type of output. New types are provided as inputs attributes for TypeRelaxed template and fixed.
/// There is no any deduction logic for types are provided as a part of this class and it should be
/// implemented outside if required.
template <typename BaseOp>
class TypeRelaxed : public BaseOp, public TypeRelaxedBase {
public:
    OPENVINO_OP(BaseOp::get_type_info_static().name, BaseOp::get_type_info_static().version_id, BaseOp);

    using BaseOp::BaseOp;

    TypeRelaxed() = default;

    TypeRelaxed(const BaseOp& base_op, element::Type overridden_type)
        : TypeRelaxed(base_op,
                      element::TypeVector(base_op.get_input_size(), overridden_type),
                      element::TypeVector(base_op.get_output_size(), overridden_type)) {}

    explicit TypeRelaxed(const BaseOp& base_op,
                         const element::TypeVector& _input_data_types = {},
                         const element::TypeVector& _output_data_types = {})
        : BaseOp(base_op),
          TypeRelaxedBase(_input_data_types, _output_data_types) {
        init();
    }

    /// Creating a new TypeRelaxed operation by calling one of the original op ctors forwarding arguments directly.
    template <typename... Args>
    TypeRelaxed(const element::TypeVector& _input_data_types,
                const element::TypeVector& _output_data_types,
                Args&&... args)
        : BaseOp(std::forward<Args>(args)...),
          TypeRelaxedBase(_input_data_types, _output_data_types) {
        init();
    }

    void validate_and_infer_types() override;
    OPENVINO_SUPPRESS_DEPRECATED_START
    bool evaluate(const HostTensorVector& outputs, const HostTensorVector& inputs) const override;
    OPENVINO_SUPPRESS_DEPRECATED_END

    std::shared_ptr<Node> clone_with_new_inputs(const OutputVector& new_args) const override;

    bool visit_attributes(AttributeVisitor& visitor) override;

private:
    mutable std::mutex type_relax_mutex;
    void init() {
        validate_and_infer_types();
    }

    init_rt_result init_rt = init_rt_info(*this);
};

OPENVINO_SUPPRESS_DEPRECATED_START
template <typename BaseOp>
bool TypeRelaxed<BaseOp>::evaluate(const HostTensorVector& outputs, const HostTensorVector& inputs) const {
    std::shared_ptr<ov::op::v0::Convert> convert;
    HostTensorVector casted_inputs(BaseOp::get_input_size());
    for (size_t i = 0; i < BaseOp::get_input_size(); ++i) {
        const auto expected_input_type = get_origin_input_type(i);

        if (inputs[i]->get_element_type() == expected_input_type || expected_input_type == element::undefined) {
            casted_inputs[i] = inputs[i];
        } else {
            if (convert == nullptr) {
                convert = std::make_shared<ov::op::v0::Convert>();
            }

            convert->set_destination_type(expected_input_type);
            casted_inputs[i] = std::make_shared<HostTensor>(expected_input_type, inputs[i]->get_shape());
            if (!convert->evaluate({casted_inputs[i]}, {inputs[i]})) {
                return false;
            }
        }
    }

    HostTensorVector original_outputs(BaseOp::get_output_size());
    for (size_t i = 0; i < BaseOp::get_output_size(); ++i) {
        const auto expected_output_type = get_overridden_output_type(i);
        if (expected_output_type == element::undefined || expected_output_type == m_original_output_data_types[i]) {
            original_outputs[i] = outputs[i];
        } else {
            original_outputs[i] =
                std::make_shared<HostTensor>(m_original_output_data_types[i], BaseOp::get_output_partial_shape(i));
        }
    }

    if (!BaseOp::evaluate(original_outputs, casted_inputs)) {
        return false;
    }

    for (size_t i = 0; i < BaseOp::get_output_size(); ++i) {
        const auto expected_output_type = get_overridden_output_type(i);

        if (expected_output_type != element::undefined &&
            original_outputs[i]->get_element_type() != expected_output_type) {
            if (convert == nullptr) {
                convert = std::make_shared<ov::op::v0::Convert>();
            }

            convert->set_destination_type(expected_output_type);
            const auto casted_output =
                std::make_shared<HostTensor>(expected_output_type, original_outputs[i]->get_shape());
            if (!convert->evaluate({outputs[i]}, {original_outputs[i]})) {
                return false;
            }
        }
    }

    return true;
}
OPENVINO_SUPPRESS_DEPRECATED_END

template <typename BaseOp>
void TypeRelaxed<BaseOp>::validate_and_infer_types() {
    element::TypeVector old_input_types;

    remember_input_data_types(*this, old_input_types);

    OPENVINO_SUPPRESS_DEPRECATED_START
    BaseOp::validate_and_infer_types();
    OPENVINO_SUPPRESS_DEPRECATED_END

    restore_input_data_types(*this, old_input_types);
}

template <typename BaseOp>
std::shared_ptr<Node> TypeRelaxed<BaseOp>::clone_with_new_inputs(const OutputVector& new_args) const {
    std::lock_guard<std::mutex> lock(type_relax_mutex);
    // copy then modify inputs
    std::shared_ptr<Node> new_node =
        std::make_shared<TypeRelaxed<BaseOp>>((BaseOp&)(*this), m_input_data_types, m_output_data_types);
    for (size_t i = 0; i < new_node->get_input_size(); ++i) {
        new_node->input(i).replace_source_output(new_args[i]);
    }

    new_node->validate_and_infer_types();
    return new_node;
}

template <typename BaseOp>
bool TypeRelaxed<BaseOp>::visit_attributes(AttributeVisitor& visitor) {
    TypeRelaxedBase::visit_attributes(visitor);
    BaseOp::visit_attributes(visitor);
    return true;
}

OPENVINO_SUPPRESS_DEPRECATED_END

}  // namespace op
}  // namespace ov
