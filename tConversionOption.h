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
 * \brief   Contains tConversionOption
 *
 * \b tConversionOption
 *
 * One concrete option of conversion provided by a registered conversion operation (that may provide multiple).
 * Used for selection of possibly a sequence of conversions and for compiling them.
 *
 */
//----------------------------------------------------------------------
#ifndef __rrlib__rtti_conversion__tConversionOption_h__
#define __rrlib__rtti_conversion__tConversionOption_h__

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

struct tCurrentConversionOperation;

/*!
 * Type of conversion option
 */
enum class tConversionOptionType
{
  /*!
   * Empty/invalid conversion option
   */
  NONE,

  /*!
   * Conversion is performed with a standard tConversionFunction with no reference on the source object.
   *
   * Computational overhead:
   * - Single/final operation: a function pointer call + copying to the destination object.
   * - First operation in sequence: a function pointer call + creating and copying to intermediate object on the stack (expensive for destination types which allocate memory internally)
   */
  STANDARD_CONVERSION_FUNCTION,

  // The options below reference the source object -  so it must either be const and available as long as destination object is used - or the data (finally) needs to be copied

  /*!
   * Destination data is available as reference to source object - with a fixed offset.
   *
   * Computational overhead (preferred option):
   * - Single/final operation: Simple memcpy for destination for types that support bitwise copy (may even be optimized away).
   *                           Otherwise, this will result in a virtual function call for copying to the destination object.
   * - First operation in sequence: optimized away
   */
  CONST_OFFSET_REFERENCE_TO_SOURCE_OBJECT,

  /*!
   * Destination data is available as reference to source object - with a variable offset (e.g. a std::vector element).
   * Offset of destination type in source objects is determined via tGetDestinationReferenceFunction.
   *
   * Computational overhead (good option for expensively copied objects):
   * - Single/final operation: a standard conversion function (overhead see above)
   * - First operation in sequence: a function pointer call (without copying)
   */
  VARIABLE_OFFSET_REFERENCE_TO_SOURCE_OBJECT,

  /*!
   * Conversion is performed with a standard tConversionFunction, however, destination object references/wraps the source object.
   *
   * Computational overhead (in some cases good option for expensively copied objects in a sequence):
   * - Single operation: a standard conversion function (overhead see above)
   * - Chaining: a function pointer call (creating wrapper; to make sense this should require less allocation/copying than STANDARD_CONVERSION_FUNCTION)
   */
  RESULT_REFERENCES_SOURCE_OBJECT
};

//----------------------------------------------------------------------
// Class declaration
//----------------------------------------------------------------------
//! Conversion option
/*!
 * One concrete option of conversion provided by a registered conversion operation (that may provide multiple).
 * Used for selection of possibly a sequence of conversions and for compiling them.
 */
struct tConversionOption
{
  /*!
   * Function pointer for conversion operation (used by any tConversionOptionType except of CONST_OFFSET_REFERENCE_TO_SOURCE_OBJECT)
   *
   * \param source_object Source object to convert
   * \param destination_object Destination object to copy result to.
   * \param operation Provides access to current conversion operation (e.g. flags and parameters). Any operation that does not write to destination_object must call Continue() on this object.
   */
  typedef void (*tConversionFunction)(const tTypedConstPointer& source_object, const tTypedPointer& destination_object, const tCurrentConversionOperation& operation);

  /*!
   * Function pointer for obtaining variable offset to source object (VARIABLE_OFFSET_REFERENCE_TO_SOURCE_OBJECT)
   *
   * \param source_object Source object
   * \param operation Provides access to current conversion operation (e.g. flags and parameters). (Continue() must not be called)
   * \return Destination object (references source object)
   */
  typedef tTypedConstPointer(*tGetDestinationReferenceFunction)(const tTypedConstPointer& source_object, const tCurrentConversionOperation& operation);

  /*! Source and destination types */
  rrlib::rtti::tType source_type, destination_type;

  /*! Type of conversion option - determines how variables below are filled */
  enum tConversionOptionType type;

  // Conversion function if first operation in sequence
  union
  {
    /*! Contains offset of destination type data in source type data when type is CONST_OFFSET_REFERENCE_TO_SOURCE_OBJECT */
    size_t const_offset_reference_to_source_object;

    /*! Function pointer for first conversion operation when type is not CONST_OFFSET_REFERENCE_TO_SOURCE_OBJECT */
    tConversionFunction first_conversion_function;
  };

  // Additional operation
  union
  {
    /*! Contains final conversion function when type is STANDARD_CONVERSION_FUNCTION and RESULT_REFERENCES_SOURCE_OBJECT */
    tConversionFunction final_conversion_function;

    /*! Function pointer for obtaining variable offset to source object when type is VARIABLE_OFFSET_REFERENCE_TO_SOURCE_OBJECT */
    tGetDestinationReferenceFunction destination_reference_function;
  };

  /*!
   * Constructor for STANDARD_CONVERSION_FUNCTION and RESULT_REFERENCES_SOURCE_OBJECT
   */
  constexpr tConversionOption(const rrlib::rtti::tType& source_type, const rrlib::rtti::tType& destination_type, bool destination_references_source, tConversionFunction first_conversion_function, tConversionFunction final_conversion_function) :
    source_type(source_type),
    destination_type(destination_type),
    type(destination_references_source ? tConversionOptionType::RESULT_REFERENCES_SOURCE_OBJECT : tConversionOptionType::STANDARD_CONVERSION_FUNCTION),
    first_conversion_function(first_conversion_function),
    final_conversion_function(final_conversion_function)
  {}

  /*!
   * Constructor for CONST_OFFSET_REFERENCE_TO_SOURCE_OBJECT
   */
  constexpr tConversionOption(const rrlib::rtti::tType& source_type, const rrlib::rtti::tType& destination_type, size_t const_offset_reference_to_source_object) :
    source_type(source_type),
    destination_type(destination_type),
    type(tConversionOptionType::CONST_OFFSET_REFERENCE_TO_SOURCE_OBJECT),
    const_offset_reference_to_source_object(const_offset_reference_to_source_object),
    final_conversion_function(nullptr)
  {}

  /*!
   * Constructor for VARIABLE_OFFSET_REFERENCE_TO_SOURCE_OBJECT
   */
  constexpr tConversionOption(const rrlib::rtti::tType& source_type, const rrlib::rtti::tType& destination_type, tConversionFunction first_conversion_function, tGetDestinationReferenceFunction destination_reference_function) :
    source_type(source_type),
    destination_type(destination_type),
    type(tConversionOptionType::VARIABLE_OFFSET_REFERENCE_TO_SOURCE_OBJECT),
    first_conversion_function(first_conversion_function),
    destination_reference_function(destination_reference_function)
  {}

  /*!
   * Constructor for NONE
   */
  constexpr tConversionOption() :
    source_type(),
    destination_type(),
    type(tConversionOptionType::NONE),
    const_offset_reference_to_source_object(0),
    final_conversion_function(nullptr)
  {}
};

/*!
 * Conversion option for static_cast (includes information whether cast is implicit)
 */
struct tConversionOptionStaticCast
{
  /*! Conversion option with main data on cast operation */
  tConversionOption conversion_option;

  /*! True if this is a implicit cast */
  bool implicit;
};

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}


#endif
