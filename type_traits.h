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
/*!\file    rrlib/rtti_conversion/type_traits.h
 *
 * \author  Max Reichardt
 *
 * \date    2016-08-10
 *
 * Type traits related to static casts
 *
 */
//----------------------------------------------------------------------
#ifndef __rrlib__rtti_conversion__type_traits_h__
#define __rrlib__rtti_conversion__type_traits_h__

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------

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
// Function declarations
//----------------------------------------------------------------------

/*!
 * This type trait provides information on whether the destination object references the source with a non-constant offset.
 * This is rather unlikely.
 *
 * It needs to be specialized (to true) for type combinations where one of the following conditions holds:
 * - TDestination is a reference type, and static_cast<TDestination>(source) returns a reference with a _variable_ offset from 'source'.
 * - TDestination is a wrapper type around TSource (more precisely: TDestination is no reference type and the result of static_cast<TDestination>(source) references 'source')
 */
template <typename TSource, typename TDestination>
struct StaticCastReferencesSourceWithVariableOffset
{
  enum { value = 0 };
};

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}


#endif
