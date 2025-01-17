/**
 * \file imperative/src/impl/op_trait.h
 * MegEngine is Licensed under the Apache License, Version 2.0 (the "License")
 *
 * Copyright (c) 2014-2021 Megvii Inc. All rights reserved.
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT ARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 */

#pragma once

#include "megbrain/imperative/op_def.h"

namespace mgb {
namespace imperative {

namespace detail {
template <typename Signature>
struct OpMeth;
template <typename RType, typename... Args>
struct OpMeth<RType(Args...)> : public thin_function<RType(Args...)> {
    using Base = thin_function<RType(Args...)>;
    using Base::Base;
    RType operator()(Args... args) const {
        if (!this->Base::operator bool()) {
            mgb_throw(MegBrainError, "Not Implemented");
        }
        return this->Base::operator()(std::forward<Args>(args)...);
    }
};
template<typename T>
struct ToVarNodeArray: std::false_type {};
template<>
struct ToVarNodeArray<SymbolVar>: std::true_type {
    VarNodeArray operator()(const SymbolVar& inp) {
        return {inp.node()};
    }
};
template<>
struct ToVarNodeArray<SymbolVarArray>: std::true_type {
    VarNodeArray operator()(const SymbolVarArray& inputs) {
        return cg::to_var_node_array(inputs);
    }
};
template<size_t N>
struct ToVarNodeArray<std::array<SymbolVar, N>>: std::true_type {
    VarNodeArray operator()(const std::array<SymbolVar, N>& inp) {
        return cg::to_var_node_array({inp.begin(), inp.end()});
    }
};
template<>
struct ToVarNodeArray<cg::OperatorNodeBase*>: std::true_type {
    VarNodeArray operator()(const cg::OperatorNodeBase* opr) {
        return opr->usable_output();
    }
};
}  // namespace detail

using OpDefMaker = detail::OpMeth<
        decltype(OpDef::make_from_op_node)>;
using DecideDispatchMode = detail::OpMeth<
        decltype(OpDef::decide_dispatch_mode)>;
using ApplyOnPhysicalTensor = detail::OpMeth<
        decltype(OpDef::apply_on_physical_tensor)>;
using InferOutputMemDesc = detail::OpMeth<
        decltype(OpDef::infer_output_mem_desc)>;
using Execute = detail::OpMeth<
        decltype(OpDef::execute)>;
using ApplyOnDeviceTensorND = detail::OpMeth<
        decltype(OpDef::apply_on_device_tensornd)>;
using ApplyOnVarNode = detail::OpMeth<
        decltype(OpDef::apply_on_var_node)>;
using InferOutputAttrsFallible = detail::OpMeth<
        decltype(OpDef::infer_output_attrs_fallible)>;
using GradMaker = detail::OpMeth<
        decltype(OpDef::make_backward_graph)>;
using Props = detail::OpMeth<decltype(OpDef::props)>;
using HashFunc = detail::OpMeth<size_t(const OpDef&)>;
using IsSame = detail::OpMeth<bool(const OpDef&, const OpDef&)>;
using MakeNameFunc = detail::OpMeth<std::string(const OpDef&)>;

struct OpTrait {
    const char* name;
    OpDefMaker make_from_op_node;
    DecideDispatchMode decide_dispatch_mode;
    ApplyOnPhysicalTensor apply_on_physical_tensor;
    InferOutputMemDesc infer_output_mem_desc;
    Execute execute;
    ApplyOnDeviceTensorND apply_on_device_tensornd;
    ApplyOnVarNode apply_on_var_node;
    InferOutputAttrsFallible infer_output_attrs_fallible;
    GradMaker make_backward_graph;
    Props props;
    HashFunc hash;
    IsSame is_same_st;
    MakeNameFunc make_name;
    OpTrait(const char* name);
    static OpTrait* find_by_name(const char* name);
    static OpTrait* find_by_typeinfo(Typeinfo* type);
    static void for_each_trait(thin_function<void(OpTrait&)> visitor);
};

#define FOR_EACH_OP_METH(cb) \
    cb(make_from_op_node) \
    cb(decide_dispatch_mode) \
    cb(apply_on_physical_tensor) \
    cb(infer_output_mem_desc) \
    cb(execute) \
    cb(apply_on_device_tensornd) \
    cb(apply_on_var_node) \
    cb(infer_output_attrs_fallible) \
    cb(make_backward_graph) \
    cb(props) \
    cb(hash) \
    cb(is_same_st) \
    cb(make_name)

struct OpTraitRegistry {
    OpTrait* trait;
#define DECL(meth) \
    OpTraitRegistry& meth(decltype(OpTrait::meth) f) { \
        mgb_assert(!trait->meth, "op %s has duplicate method %s", trait->name, #meth); \
        trait->meth = f; \
        return *this; \
    }
    FOR_EACH_OP_METH(DECL)
#undef DECL

    OpTraitRegistry& fallback();

    template<typename T>
    void insert() {
        do_insert(T::typeinfo());
    }

    template<typename T0, typename T1, typename ...Ts>
    void insert() {
        insert<T0>();
        insert<T1, Ts...>();
    }

    template<typename ...Args>
    static OpTraitRegistry insert(const char* name) {
        auto&& ret = do_insert(name);
        ret.insert<Args...>();
        return ret;
    }

    void do_insert(Typeinfo* type);

    static OpTraitRegistry do_insert(const char* name);

    template<typename T,
        typename To = detail::ToVarNodeArray<T>,
        typename = std::enable_if_t<To::value>>
    OpTraitRegistry& apply_on_var_node(T (*f)(const OpDef&, const VarNodeArray&)) {
        return apply_on_var_node([=](const OpDef& opdef, const VarNodeArray& inputs) {
            return To()(f(opdef, inputs));
        });
    }
};

} // namespace imperative
} // namespace mgb

#define OP_TRAIT_REG(name, ...) \
    static OpTraitRegistry __##name##_global_registry__ = \
        OpTraitRegistry::insert<__VA_ARGS__>(#name)

// vim: syntax=cpp.doxygen foldmethod=marker foldmarker=f{{{,f}}}
