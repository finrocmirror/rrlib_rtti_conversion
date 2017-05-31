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
/*!\file    rrlib/rtti_conversion/definition/tVoidFunctionConversionOperation.h
 *
 * \author  Max Reichardt
 *
 * \date    2016-08-13
 *
 * \brief   Contains tVoidFunctionConversionOperation
 *
 * \b tVoidFunctionConversionOperation
 *
 *  ASD Asd ASD Asd ASDAsd ASD Asd ASD Asd asd AS DAs dAS
 *
 */
//----------------------------------------------------------------------
#ifndef __rrlib__rtti_conversion__definition__tVoidFunctionConversionOperation_h__
#define __rrlib__rtti_conversion__definition__tVoidFunctionConversionOperation_h__

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "rrlib/rtti_conversion/tRegisteredConversionOperation.h"
#include "rrlib/rtti_conversion/tCompiledConversionOperation.h"

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
//! Wraps Function
/*!
 * Cast operation using specified function for cast.
 * The function must be callable with the const source type as first argument and the destination object reference as second - e.g.
 *
 * void ConversionFunction(const TSource& source, TDestination& destination)
 *
 * Note: As class does not have additional member variables, it can safely be assigned to a tRegisteredConversionOperation (e.g. in an array)
 *
 * Implementation note: providing function as template argument hopefully allows inlining.
 *
 * \tparam TSource Source type of conversion operation
 * \tparam TDestination Destination type of conversion operation
 * \tparam TFunction Type of conversion function
 * \tparam Tconversion_function Conversion function to use
 */
template<typename TSource, typename TDestination, typename TFunction, TFunction Tconversion_function>
class tVoidFunctionConversionOperation : public tRegisteredConversionOperation
{

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  /*!
   * \param name Name of conversion operation
   */
  tVoidFunctionConversionOperation(util::tManagedConstCharPointer name) :
    tRegisteredConversionOperation(std::move(name), tDataType<TSource>(), tDataType<TDestination>(), &cCONVERSION_OPTION)
  {}
  template <size_t Tchars>
  tVoidFunctionConversionOperation(const char(&name)[Tchars]) :
    tRegisteredConversionOperation(util::tManagedConstCharPointer(name, false), tDataType<TSource>(), tDataType<TDestination>(), &cCONVERSION_OPTION)
  {
  }

//----------------------------------------------------------------------
// Private fields and methods
//----------------------------------------------------------------------
private:

  static void FirstConversionFunction(const tTypedConstPointer& source_object, const tTypedPointer& destination_object, const tCurrentConversionOperation& operation)
  {
    TDestination intermediate(serialization::DefaultInstantiation<TDestination>::Create());
    tTypedPointer intermediate_pointer(&intermediate);
    (*Tconversion_function)(*source_object.Get<TSource>(), *intermediate_pointer.Get<TDestination>());
    operation.Continue(intermediate_pointer, destination_object);
  }

  static void FinalConversionFunction(const tTypedConstPointer& source_object, const tTypedPointer& destination_object, const tCurrentConversionOperation& operation)
  {
    (*Tconversion_function)(*source_object.Get<TSource>(), *destination_object.Get<TDestination>());
  }

  static constexpr tConversionOption cCONVERSION_OPTION = tConversionOption(tDataType<TSource>(), tDataType<TDestination>(), false, &FirstConversionFunction, &FinalConversionFunction);

};

template<typename TSource, typename TDestination, typename TFunction, TFunction Tconversion_function>
constexpr tConversionOption tVoidFunctionConversionOperation<TSource, TDestination, TFunction, Tconversion_function>::cCONVERSION_OPTION;

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}


#endif
