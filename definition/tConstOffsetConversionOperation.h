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
/*!\file    rrlib/rtti_conversion/definition/tConstOffsetConversionOperation.h
 *
 * \author  Max Reichardt
 *
 * \date    2016-08-13
 *
 * \brief   Contains tConstOffsetConversionOperation
 *
 * \b tConstOffsetConversionOperation
 *
 * Conversion that is done by adding a constant pointer offset a source object and doing a reinterpret_cast.
 *
 */
//----------------------------------------------------------------------
#ifndef __rrlib__rtti_conversion__definition__tConstOffsetConversionOperation_h__
#define __rrlib__rtti_conversion__definition__tConstOffsetConversionOperation_h__

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "rrlib/rtti_conversion/tRegisteredConversionOperation.h"

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
// Class declaration
//----------------------------------------------------------------------
//! Constant pointer offset conversion
/*!
 * Conversion that is done by adding a constant pointer offset a source object and doing a reinterpret_cast:
 *
 *  reinterpret_cast<TDestination&>(*(reinterpret_cast<const char*>(source_object) + Toffset));
 *
 * This is very efficient, but also somewhat dangerous due to minimal checks that compiler can do w.r.t. correctness.
 * (maintenance is critical, when memory layout of source or destination types changes)
 *
 * Note: As class does not have additional member variables, it can safely be assigned to a tRegisteredConversionOperation (e.g. in an array)
 *
 * \tparam TSource Source type of conversion operation
 * \tparam TDestination Destination type of conversion operation. Append '&' to wrap returned reference directly (instead of copying it). This is not recommended with cheaply-copied types.
 * \tparam Toffset constant offset
 */
template<typename TSource, typename TDestination, size_t Toffset>
class tConstOffsetConversionOperation : public tRegisteredConversionOperation
{

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  /*!
   * \param name Name of conversion operation
   */
  tConstOffsetConversionOperation(util::tManagedConstCharPointer name) :
    tRegisteredConversionOperation(std::move(name), cCONVERSION_OPTION)
  {}
  template <size_t Tchars>
  tConstOffsetConversionOperation(const char(&name)[Tchars]) :
    tRegisteredConversionOperation(util::tManagedConstCharPointer(name, false), cCONVERSION_OPTION)
  {
  }

//----------------------------------------------------------------------
// Private fields and methods
//----------------------------------------------------------------------
private:

  static constexpr tConversionOption cCONVERSION_OPTION = tConversionOption(tDataType<TSource>(), tDataType<TDestination>(), Toffset);

};

template<typename TSource, typename TDestination, size_t Toffset>
constexpr tConversionOption tConstOffsetConversionOperation<TSource, TDestination, Toffset>::cCONVERSION_OPTION;

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}

#endif
