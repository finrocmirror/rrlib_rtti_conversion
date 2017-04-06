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
/*!\file    rrlib/rtti_conversion/tCurrentConversionOperation.h
 *
 * \author  Max Reichardt
 *
 * \date    2016-08-03
 *
 * \brief   Contains tCurrentConversionOperation
 *
 * \b tCurrentConversionOperation
 *
 * Contains data about currently running compiled conversion operation.
 * It is passed to single conversion operation functions in order to obtain parameters and possibly continue with the next
 * operation in the conversion operation sequence.
 *
 */
//----------------------------------------------------------------------
#ifndef __rrlib__rtti_conversion__tCurrentConversionOperation_h__
#define __rrlib__rtti_conversion__tCurrentConversionOperation_h__

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
// Class declaration
//----------------------------------------------------------------------
//! Data on current conversion operations
/*!
 * Contains data about currently running compiled conversion operation.
 * It is passed to single conversion operation functions in order to obtain parameters and possibly continue with the next
 * operation in the conversion operation sequence.
 */
struct tCurrentConversionOperation
{

  /*! Reference to compiled conversion operation */
  const tCompiledConversionOperation& compiled_operation;

  /* Index of function in conversion operation sequence (relevant for accessing parameters and determining next operation) */
  unsigned int operation_index;


  /*!
   * Continue conversion operation with result of the current one.
   * (note: implemented in tCompiledConversionOperation.h to handle cyclic dependency)
   *
   * \param intermediate_object Typed pointer containing intermediate (temporary) data to convert or copy. Must have intermediate or destination type of conversion sequence.
   * \param destination_object Typed pointer containing buffer to write converted data to. Its type must be equal to destination_type.
   */
  inline void Continue(const tTypedConstPointer& intermediate_object, const tTypedPointer& destination_object) const;

  /*!
   * Get conversion parameter
   * (note: implemented in tCompiledConversionOperation.h to handle cyclic dependency)
   *
   * \return Pointer to buffer with parameter if it has been specified (otherwise nullptr -> the conversion operation should use a default value)
   */
  inline tTypedConstPointer GetParameterValue() const;
};

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}


#endif
