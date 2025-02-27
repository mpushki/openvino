# Copyright (C) 2018-2023 Intel Corporation
# SPDX-License-Identifier: Apache-2.0

import pytest
import numpy as np

from pytorch_layer_test_class import PytorchLayerTest


class TestIndex(PytorchLayerTest):
    def _prepare_input(self, input_shape, idx):
        import numpy as np
        return (np.random.randn(*input_shape).astype(np.float32), idx)

    def create_model(self, model="list"):
        import torch

        class aten_index_list(torch.nn.Module):

            def forward(self, x, idx):
                return x[idx]

        class aten_index_getitem(torch.nn.Module):

            def forward(self, x, idx):
                return x.__getitem__(idx)
        

        class aten_index_list_bool(torch.nn.Module):

            def forward(self, x, idx):
                return x[idx.to(torch.bool)]

        class aten_index_getitem_bool(torch.nn.Module):

            def forward(self, x, idx):
                return x.__getitem__(idx.to(torch.bool))
        cases = {
            "list": aten_index_list,
            "getitem": aten_index_getitem,
            "list_with_bool": aten_index_list_bool,
            "getitem_with_bool": aten_index_getitem_bool
        }

        aten_index = cases[model]

        ref_net = None

        return aten_index(), ref_net, "aten::index"

    @pytest.mark.nightly
    @pytest.mark.precommit
    @pytest.mark.parametrize("case", ["list", "getitem"])
    @pytest.mark.parametrize(("input_shape", "idx"), [
        ((1,), np.array(0).astype(int)), 
        ([2, 3], np.array(-1).astype(int)), 
        ([4, 5, 6], np.array((1, 2)).astype(int)), 
        ([7, 8, 9], np.array((-1,  2, -3)).astype(int)), 
        ([2, 2, 3, 4], np.array((1,)).astype(int))])
    def test_index(self, input_shape, idx, case, ie_device, precision, ir_version):
        self._test(*self.create_model(case), ie_device, precision, ir_version, kwargs_to_prepare_input={"input_shape": input_shape, "idx": idx})

    @pytest.mark.nightly
    @pytest.mark.precommit
    @pytest.mark.parametrize("case", ["getitem_with_bool", "list_with_bool"])
    @pytest.mark.parametrize(("input_shape", "idx"), [
        ((1, 2), np.array([[1, 0]]).astype(bool)),
        ((2, 2, 5), np.zeros([2, 2, 5]).astype(bool)),
        ((2, 2, 5), np.ones([2, 2, 5]).astype(bool)),
        ((2, 2, 5), np.random.rand(2, 2, 5) > 0)
        ])
    def test_index_bool(self, input_shape, idx, case, ie_device, precision, ir_version):
        self._test(*self.create_model(case), ie_device, precision, ir_version, kwargs_to_prepare_input={"input_shape": input_shape, "idx": idx})