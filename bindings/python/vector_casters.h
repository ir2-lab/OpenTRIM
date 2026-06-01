/*
 * vector_casters.h
 *
 * pybind11 type_caster for the OpenTRIM geometry vector types:
 *   vector3  = Eigen::AlignedVector3<float>  <->  Python list of 3 floats
 *   ivector3 = Eigen::AlignedVector3<int>    <->  Python list of 3 ints
 */

#pragma once

#include <pybind11/pybind11.h>
#include "geometry.h"

namespace pybind11 { namespace detail {

template <>
struct type_caster<vector3>
{
    PYBIND11_TYPE_CASTER(vector3, const_name("list[float]"));

    bool load(handle src, bool)
    {
        try {
            sequence seq = src.cast<sequence>();
            if (seq.size() != 3)
                return false;
            value = vector3(seq[0].cast<float>(),
                            seq[1].cast<float>(),
                            seq[2].cast<float>());
            return true;
        } catch (...) {
            return false;
        }
    }

    static handle cast(const vector3& v, return_value_policy, handle)
    {
        list out;
        out.append(pybind11::cast(v.x()));
        out.append(pybind11::cast(v.y()));
        out.append(pybind11::cast(v.z()));
        return out.release();
    }
};

template <>
struct type_caster<ivector3>
{
    PYBIND11_TYPE_CASTER(ivector3, const_name("list[int]"));

    bool load(handle src, bool)
    {
        try {
            sequence seq = src.cast<sequence>();
            if (seq.size() != 3)
                return false;
            value = ivector3(seq[0].cast<int>(),
                             seq[1].cast<int>(),
                             seq[2].cast<int>());
            return true;
        } catch (...) {
            return false;
        }
    }

    static handle cast(const ivector3& v, return_value_policy, handle)
    {
        list out;
        out.append(pybind11::cast(v.x()));
        out.append(pybind11::cast(v.y()));
        out.append(pybind11::cast(v.z()));
        return out.release();
    }
};

}} // namespace pybind11::detail
