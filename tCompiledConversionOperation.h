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
/*!\file    rrlib/rtti_conversion/tCompiledConversionOperation.h
 *
 * \author  Max Reichardt
 *
 * \date    2016-07-15
 *
 * \brief   Contains tCompiledConversionOperation
 *
 * \b tCompiledConversionOperation
 *
 * Conversion operation compiled from possibly multiple elementary and/or possibly generic ones.
 * Compiled conversion operations are furthermore optimized for runtime performance.
 *
 */
//----------------------------------------------------------------------
#ifndef __rrlib__rtti_conversion__tCompiledConversionOperation_h__
#define __rrlib__rtti_conversion__tCompiledConversionOperation_h__

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "rrlib/rtti_conversion/defined_conversions.h"
#include "rrlib/rtti_conversion/tConversionOperationSequence.h"
#include "rrlib/rtti_conversion/tConversionOption.h"
#include "rrlib/rtti_conversion/tCurrentConversionOperation.h"

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
//! Compiled conversion operation
/*!
 * Conversion operation compiled from possibly multiple elementary and/or possibly generic ones.
 * Compiled conversion operations are furthermore optimized for runtime performance.
 */
class tCompiledConversionOperation : public tConversionOperationSequence
{

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  /*! Flags for conversion operation */
  enum tFlag
  {
    /*! Info required for execution */
    cDO_FINAL_DEEPCOPY_AFTER_FIRST_FUNCTION = 1 << 0,  //!< Do final DeepCopy after first conversion function?
    cDO_FINAL_DEEPCOPY_AFTER_SECOND_FUNCTION = 1 << 1, //!< Do final DeepCopy after second conversion function?
    cDEEPCOPY_ONLY = 1 << 2,                           //!< True if conversion operation can be performed with a simple memcpy
    cFIRST_OPERATION_OPTIMIZED_AWAY = 1 << 3,          //!< True if first operation was optimized away (relevant for parameter lookup)

    /*! Result of conversion operation */
    cRESULT_INDEPENDENT = 1 << 29,                   //!< Conversion can be performed with Convert(source_object, destination_object). Destination does not reference source object.
    cRESULT_REFERENCES_SOURCE_INTERNALLY = 1 << 30,  //!< Conversion can be performed with Convert(source_object, destination_object). Destination references source object.
    cRESULT_REFERENCES_SOURCE_DIRECTLY = 1 << 31,    //!< Conversion can be performed with Convert(source_object).
  };

  tCompiledConversionOperation() : tConversionOperationSequence(), conversion_function_first(nullptr), conversion_function_final(nullptr), fixed_offset_first(0), fixed_offset_final(0), flags(0)
  {}

  /*!
   * Perform actual conversion operation.
   * Available for any conversion result type. Fills provided destination objects with result.
   *
   * \param source_object Typed pointer containing data to convert. Must have source type of this operation.
   * \param destination_object Typed pointer containing buffer to write converted data to. Its type must be equal to destination_type.
   *
   * (note: For performance reasons, in general no type checks are performed.
   *        The caller is responsible for ensuring that typed pointer point to objects of correct types.)
   */
  inline void Convert(const tTypedConstPointer& source_object, const tTypedPointer& destination_object) const
  {
    assert(flags & (tFlag::cRESULT_INDEPENDENT | tFlag::cRESULT_REFERENCES_SOURCE_INTERNALLY));
    tTypedConstPointer intermediate_object(static_cast<const char*>(source_object.GetRawDataPointer()) + fixed_offset_first, type_after_first_fixed_offset); // in case we have a fixed offset and conversion function is a deep copy operation
    if (flags & tFlag::cDEEPCOPY_ONLY)
    {
      destination_object.DeepCopyFrom(intermediate_object);
    }
    else
    {
      tCurrentConversionOperation current_operation = { *this, 0 };
      (*conversion_function_first)(intermediate_object, destination_object, current_operation);
    }
  }

  /*!
   * Perform actual conversion operation.
   * This method is only available if conversion result type is REFERENCES_SOURCE_DIRECTLY.
   * It returns a direct result to the source object - and therefore does not require any copying.
   *
   * \param source_object Source object
   * \return Destination object (references source object)
   *
   * (note: For performance reasons, in general no type checks are performed.
   *        The caller is responsible for ensuring that typed pointer point to objects of correct types.)
   */
  inline tTypedConstPointer Convert(const tTypedConstPointer& source_object) const
  {
    assert(flags & tFlag::cRESULT_REFERENCES_SOURCE_DIRECTLY);
    tTypedConstPointer result(static_cast<const char*>(source_object.GetRawDataPointer()) + fixed_offset_first, type_after_first_fixed_offset);
    if (get_destination_reference_function_first != nullptr)
    {
      tCurrentConversionOperation current_operation = { *this, 0 };
      result = (*get_destination_reference_function_first)(result, current_operation);
      if (get_destination_reference_function_final != nullptr)
      {
        tCurrentConversionOperation current_operation = { *this, 1 };
        result = (*get_destination_reference_function_final)(result, current_operation);
      }
    }
    result = tTypedConstPointer(static_cast<const char*>(result.GetRawDataPointer()) + fixed_offset_final, destination_type);
    return result;
  }

  /*!
   * \return Flags for conversion operation
   */
  unsigned int Flags() const
  {
    return flags;
  }

  /*!
   * \return Data type after first conversion function (possibly == destination_type)
   */
  const tType& IntermediateType() const
  {
    return intermediate_type;
  }

//----------------------------------------------------------------------
// Private fields and methods
//----------------------------------------------------------------------
private:

  friend class tConversionOperationSequence;
  friend class tCurrentConversionOperation;

  /*! Data type after applying first fixed offset */
  tType type_after_first_fixed_offset;

  /*! Data type after first conversion function (possibly == destination_type) */
  tType intermediate_type;

  /*! Final data type */
  tType destination_type;

  /*!
   * Function pointers to conversion operations. May be null if conversion is performed in another way.
   * This is preferred to a std::function object here, as it always has minimal memory and computation overhead.
   * A std::function might be added as a third option if binding of parameters is required.
   */
  union
  {
    tConversionOption::tConversionFunction conversion_function_first;
    tConversionOption::tGetDestinationReferenceFunction get_destination_reference_function_first;
  };
  union
  {
    tConversionOption::tConversionFunction conversion_function_final;
    tConversionOption::tGetDestinationReferenceFunction get_destination_reference_function_final;
  };

  /*!
   * Fixed offsets. In case a memcpy is possible, the second one is the size.
   */
  unsigned int fixed_offset_first, fixed_offset_final;

  /*! Flags for conversion operation */
  unsigned int flags;
};


// defined here to handle cyclic dependency (also very closely coupled to tCompiledConversionOperation)
inline void tCurrentConversionOperation::Continue(const tTypedConstPointer& intermediate_object, const tTypedPointer& destination_object) const
{
  unsigned int next_operation_index = operation_index + 1;
  if (compiled_operation.flags & next_operation_index)
  {
    // Do final DeepCopy
    tTypedConstPointer intermediate_2(static_cast<const char*>(intermediate_object.GetRawDataPointer()) + compiled_operation.fixed_offset_final, destination_object.GetType());  // in case second operation is a const_offset
    destination_object.DeepCopyFrom(intermediate_2);
  }
  else
  {
    // Call second conversion function
    tCurrentConversionOperation current_operation = { compiled_operation, next_operation_index };
    (*compiled_operation.conversion_function_final)(intermediate_object, destination_object, current_operation);
  }
}

inline tTypedConstPointer tCurrentConversionOperation::GetParameterValue() const
{
  return compiled_operation.GetParameterValue((compiled_operation.flags & tCompiledConversionOperation::tFlag::cFIRST_OPERATION_OPTIMIZED_AWAY) ? 1 : operation_index);
}

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}


#endif
