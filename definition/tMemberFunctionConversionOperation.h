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
/*!\file    rrlib/rtti_conversion/definition/tMemberFunctionConversionOperation.h
 *
 * \author  Max Reichardt
 *
 * \date    2016-08-13
 *
 * \brief   Contains tMemberFunctionConversionOperation
 *
 * \b tMemberFunctionConversionOperation
 *
 * Cast operation using specified member function for cast.
 *
 */
//----------------------------------------------------------------------
#ifndef __rrlib__rtti_conversion__definition__tMemberFunctionConversionOperation_h__
#define __rrlib__rtti_conversion__definition__tMemberFunctionConversionOperation_h__

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

/*!
 * Utility function declaration.
 * If a class has a const and a non-const overload of a method, this function can be used to obtain the type of the const version:
 * decltype(ConstMemberFunction(&A::F))
 */
template <class TReturn, class TClass>
auto ConstMemberFunction(TReturn(TClass::*function)() const) -> decltype(function);

//----------------------------------------------------------------------
// Class declaration
//----------------------------------------------------------------------
//! Wraps member Function
/*!
 * Cast operation using specified member function for cast.
 * Note: As class does not have additional member variables, it can safely be assigned to a tRegisteredConversionOperation (e.g. in an array)
 *
 * Implementation note: providing function as template argument hopefully allows inlining.
 *
 * \tparam TSource Source type of conversion operation
 * \tparam TDestination Destination type of conversion operation. Append '&' to wrap returned reference directly (instead of copying it). This is not recommended with cheaply-copied types.
 * \tparam TFunction Type of conversion function
 * \tparam Tconversion_function Conversion function to use
 * \tparam Tdestination_reference_source_with_variable_offset True if
 *                                                            - TDestination is a reference type, and Tconversion_function returns a reference with a _variable_ offset from 'source'  (see tConversionOptionType::VARIABLE_OFFSET_REFERENCE_TO_SOURCE_OBJECT)
 *                                                            - TDestination is a wrapper type around TSource (TDestination is no reference type)  (see tConversionOptionType::RESULT_REFERENCES_SOURCE_OBJECT)
 */
template<typename TSource, typename TDestination, typename TFunction, TFunction Tconversion_function, bool Tdestination_reference_source_with_variable_offset = false>
class tMemberFunctionConversionOperation : public tRegisteredConversionOperation
{

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  /*!
   * \param name Name of conversion operation
   */
  tMemberFunctionConversionOperation(util::tManagedConstCharPointer name) :
    tRegisteredConversionOperation(std::move(name), tDataType<TSource>(), tDataType<TDestination>(), &cCONVERSION_OPTION)
  {
  }
  template <size_t Tchars>
  tMemberFunctionConversionOperation(const char(&name)[Tchars]) :
    tRegisteredConversionOperation(util::tManagedConstCharPointer(name, false), tDataType<TSource>(), tDataType<TDestination>(), &cCONVERSION_OPTION)
  {
  }

//----------------------------------------------------------------------
// Private fields and methods
//----------------------------------------------------------------------
private:

  static void FirstConversionFunction(const tTypedConstPointer& source_object, const tTypedPointer& destination_object, const tCurrentConversionOperation& operation)
  {
    TDestination intermediate = ((*source_object.Get<TSource>()).*Tconversion_function)();
    operation.Continue(tTypedConstPointer(&intermediate), destination_object);
  }

  static void FinalConversionFunction(const tTypedConstPointer& source_object, const tTypedPointer& destination_object, const tCurrentConversionOperation& operation)
  {
    *destination_object.Get<TDestination>() = ((*source_object.Get<TSource>()).*Tconversion_function)();
  }

  static constexpr tConversionOption cCONVERSION_OPTION = tConversionOption(tDataType<TSource>(), tDataType<TDestination>(), Tdestination_reference_source_with_variable_offset, &FirstConversionFunction, &FinalConversionFunction);

};

/*! Specialization for member function that returns const offset reference */
template<typename TSource, typename TDestination, typename TFunction, TFunction Tconversion_function>
class tMemberFunctionConversionOperation<TSource, TDestination&, TFunction, Tconversion_function, false> : public tRegisteredConversionOperation
{

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  /*!
   * \param name Name of conversion operation
   */
  tMemberFunctionConversionOperation(util::tManagedConstCharPointer name) :
    tRegisteredConversionOperation(std::move(name), tDataType<TSource>(), tDataType<TDestination>(), &cCONVERSION_OPTION)
  {
  }
  template <size_t Tchars>
  tMemberFunctionConversionOperation(const char(&name)[Tchars]) :
    tRegisteredConversionOperation(util::tManagedConstCharPointer(name, false), tDataType<TSource>(), tDataType<TDestination>(), &cCONVERSION_OPTION)
  {
  }

//----------------------------------------------------------------------
// Private fields and methods
//----------------------------------------------------------------------
private:

  static size_t ComputePointerSourceOffset()
  {
    const TSource source_object;
    const TDestination& returned_object = (source_object.*Tconversion_function)();
    size_t difference = reinterpret_cast<const char*>(&returned_object) - reinterpret_cast<const char*>(&source_object);
    assert(difference + sizeof(TDestination) <= sizeof(TSource));
    return difference;
  }

  static const tConversionOption cCONVERSION_OPTION;

};

/*! Specialization for member function that returns variable offset reference */
template<typename TSource, typename TDestination, typename TFunction, TFunction Tconversion_function>
class tMemberFunctionConversionOperation<TSource, TDestination&, TFunction, Tconversion_function, true> : public tRegisteredConversionOperation
{

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  /*!
   * \param name Name of conversion operation
   */
  tMemberFunctionConversionOperation(util::tManagedConstCharPointer name) :
    tRegisteredConversionOperation(std::move(name), tDataType<TSource>(), tDataType<TDestination>(), &cCONVERSION_OPTION)
  {
  }
  template <size_t Tchars>
  tMemberFunctionConversionOperation(const char(&name)[Tchars]) :
    tRegisteredConversionOperation(util::tManagedConstCharPointer(name, false), tDataType<TSource>(), tDataType<TDestination>(), &cCONVERSION_OPTION)
  {
  }

//----------------------------------------------------------------------
// Private fields and methods
//----------------------------------------------------------------------
private:

  static void FirstConversionFunction(const tTypedConstPointer& source_object, const tTypedPointer& destination_object, const tCurrentConversionOperation& operation)
  {
    const TDestination& intermediate = ((*source_object.Get<TSource>()).*Tconversion_function)();
    operation.Continue(tTypedConstPointer(&intermediate), destination_object);
  }

  static tTypedConstPointer GetDestinationReference(const tTypedConstPointer& source_object, const tCurrentConversionOperation& operation)
  {
    const TDestination& destination = ((*source_object.Get<TSource>()).*Tconversion_function)();
    return tTypedConstPointer(&destination);
  }

  static constexpr tConversionOption cCONVERSION_OPTION = tConversionOption(tDataType<TSource>(), tDataType<TDestination>(), &FirstConversionFunction, &GetDestinationReference);

};

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}

#include "rrlib/rtti_conversion/definition/tMemberFunctionConversionOperation.hpp"

#endif
