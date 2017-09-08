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
/*!\file    rrlib/rtti_conversion/defined_conversions.h
 *
 * \author  Max Reichardt
 *
 * \date    2016-08-13
 *
 * Conversion operations already provided/defined by rrlib_rtti_conversion
 *
 */
//----------------------------------------------------------------------
#ifndef __rrlib__rtti_conversion__defined_conversions_h__
#define __rrlib__rtti_conversion__defined_conversions_h__

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
// Function declarations
//----------------------------------------------------------------------

/*! Flags for TO_STRING operation (activating the respective stream manipulators defined in std) */
enum ToStringFlags
{
  eTSF_BOOL_ALPHA = 1 << 0,
  eTSF_SHOW_BASE = 1 << 1,
  eTSF_SHOW_POINT = 1 << 2,
  eTSF_SHOW_POS = 1 << 3,
  eTSF_UPPER_CASE = 1 << 4,
  eTSF_DEC = 1 << 5,
  eTSF_HEX = 1 << 6,
  eTSF_OCT = 1 << 7,
  eTSF_FIXED = 1 << 8,
  eTSF_SCIENTIFIC = 1 << 9,
};

extern const tRegisteredConversionOperation& cTO_STRING_OPERATION;              //!< Converts any string serializable type to std::string (has flags parameter)
extern const tRegisteredConversionOperation& cSTRING_DESERIALIZATION_OPERATION; //!< Deserializes string serializable type (possibly throws exception)
extern const tRegisteredConversionOperation& cBINARY_SERIALIZATION_OPERATION;   //!< Converts any binary serializable type to serialization::tMemoryBuffer
extern const tRegisteredConversionOperation& cBINARY_DESERIALIZATION_OPERATION; //!< Deserializes binary serializable type from serialization::tMemoryBuffer

extern const tRegisteredConversionOperation& cGET_LIST_ELEMENT_OPERATION;       //!< Get Element with specified index (parameter) from list type (std::vector)
extern const tRegisteredConversionOperation& cFOR_EACH_OPERATION;               //!< Special conversion operation for std::vectors that applies second conversion operation on all elements

extern const tRegisteredConversionOperation& cGET_ARRAY_ELEMENT_OPERATION;      //!< Get Element with specified index (parameter) from array type (std::array)
extern const tRegisteredConversionOperation& cFOR_EACH_OPERATION_ARRAY;         //!< Special conversion operation for std::arrays that applies second conversion operation on all elements
extern const tRegisteredConversionOperation& cARRAY_TO_VECTOR_OPERATION;        //!< Converts std::array<T> to std::vector<T>

extern const tRegisteredConversionOperation& cWRAP_BYTE_VECTOR_OPERATION;       //!< Converts std::vector<uint> to a MemoryBuffer. This is particulary efficient (zero-copy) when first operation in a sequence.
extern const tRegisteredConversionOperation& cLIST_SIZE_OPERATION;              //!< Returns std::vector<T> size for any type T
extern const tRegisteredConversionOperation& cSTRING_TO_VECTOR_OPERATION;       //!< Converts std::string to char vector
extern const tRegisteredConversionOperation& cMAKE_STRING_OPERATION;            //!< Creates std::string from std::vector<char>

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}

#endif
