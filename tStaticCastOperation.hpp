//
// You received this file as part of RRLib
// Robotics Research Library
//
// Copyright (C) Finroc GbR (finroc.org)
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
//----------------------------------------------------------------------
/*!\file    rrlib/rtti_conversion/tStaticCastOperation.hpp
 *
 * \author  Max Reichardt
 *
 * \date    2016-08-12
 *
 */
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Debugging
//----------------------------------------------------------------------
#include <cassert>

//----------------------------------------------------------------------
// Namespace declaration
//----------------------------------------------------------------------
namespace rrlib
{
namespace rtti
{
namespace conversion
{

//----------------------------------------------------------------------
// Forward declarations / typedefs / enums
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Const values
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Implementation
//----------------------------------------------------------------------

template <typename TSource, typename TDestination>
constexpr tStaticCastOperation::tStaticCast tStaticCastOperation::tInstanceStandard<TSource, TDestination>::value;

template <typename TSource, typename TDestination>
constexpr tStaticCastOperation::tStaticCast tStaticCastOperation::tInstanceDeepCopy<TSource, TDestination>::value;

template <typename TSource, typename TDestination>
const tStaticCastOperation::tStaticCast tStaticCastOperation::tInstanceReferenceStaticOffset<TSource, TDestination>::value({ tConversionOption(tDataType<TSource>(), tDataType<TDestination>(), StaticCastReferencesSourceWithVariableOffset<TSource, TDestination>::value, ComputePointerSourceOffset()) }, IsImplicitlyConvertible<TDestination, TSource>::value);

template <typename TSource, typename TDestination>
constexpr tStaticCastOperation::tStaticCast tStaticCastOperation::tInstanceReferenceVariableOffset<TSource, TDestination>::value;

template <typename TSource, typename TDestination>
constexpr tStaticCastOperation::tStaticCast tStaticCastOperation::tInstanceVectorStandard<TSource, TDestination>::value;

template <typename TSource, typename TDestination>
constexpr tStaticCastOperation::tStaticCast tStaticCastOperation::tInstanceVectorDeepCopy<TSource, TDestination>::value;

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}
