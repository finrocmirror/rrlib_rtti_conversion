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
/*!\file    rrlib/rtti_conversion/tConversionOperationSequence.h
 *
 * \author  Max Reichardt
 *
 * \date    2015-12-17
 *
 * \brief   Contains tConversionOperationSequence
 *
 * \b tConversionOperationSequence
 *
 * Sequence of conversion operations with a maximum of 2 elements. May be empty.
 *
 */
//----------------------------------------------------------------------
#ifndef __rrlib__rtti_conversion__tConversionOperationSequence_h__
#define __rrlib__rtti_conversion__tConversionOperationSequence_h__

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
class tCompiledConversionOperation;

//----------------------------------------------------------------------
// Class declaration
//----------------------------------------------------------------------
//! Sequence of conversion operations
/*!
 * Sequence of conversion operations with a maximum of 2 elements. May be empty.
 *
 * Ratio / implementation note:
 *  Advantages compared to using a std::vector: More memory and computationally efficient.
 *  Advantages compared to using a std::array: Custom constructors and size() operator.
 */
class tConversionOperationSequence
{

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  /*! Constant for sequence with no conversion operations (may be handy if methods return sequences as const reference) */
  static const tConversionOperationSequence cNONE;

  /*!
   * \param first First conversion operation in chain
   * \param second Second conversion operation in chain (must be nullptr if first is nullptr)
   * \param intermediate_type Needs to be specified if there are two operations (including implicit casts) and first operation has multiple destination types and second has multiple source types
   */
  tConversionOperationSequence(const tRegisteredConversionOperation& first, const tRegisteredConversionOperation& second, const tType& intermediate_type = tType()) :
    operations {&first, &second},
             ambiguous_operation_lookup {false, false},
             intermediate_type(intermediate_type)
  {
    assert(first.SupportedDestinationTypes().single_type || second.SupportedSourceTypes().single_type || intermediate_type);
  }
  tConversionOperationSequence(const tRegisteredConversionOperation& first, const tType& intermediate_type = tType()) :
    operations {&first, nullptr},
             ambiguous_operation_lookup {false, false},
             intermediate_type(intermediate_type)
  {}
  tConversionOperationSequence() :
    operations {nullptr, nullptr},
             ambiguous_operation_lookup {false, false},
             intermediate_type()
  {}

  /*!
   * \param Name of first conversion operation
   * \param Name of second conversion operation (optional)
   * \param intermediate_type Needs to be specified if there are two operations (including implicit casts)
   * \throws Throws std::runtime_error if conversion operations with specified names could not be found
   */
  tConversionOperationSequence(const std::string& first, const std::string& second = "", const tType& intermediate_type = tType());

  tConversionOperationSequence(const tConversionOperationSequence& other) :
    operations {other.operations[0].operation, other.operations[1].operation},
             ambiguous_operation_lookup {other.ambiguous_operation_lookup[0], other.ambiguous_operation_lookup[1]},
             intermediate_type(other.intermediate_type)
  {
    if (other.operations[0].parameter)
    {
      CopyParameter(*other.operations[0].parameter, operations[0].parameter);
    }
    if (other.operations[1].parameter)
    {
      CopyParameter(*other.operations[1].parameter, operations[1].parameter);
    }
  }
  tConversionOperationSequence& operator=(const tConversionOperationSequence & other);

  tConversionOperationSequence(tConversionOperationSequence && other) = default;
  tConversionOperationSequence& operator=(tConversionOperationSequence && other) = default;

  /*!
   * Compiles conversion operation chain to a single optimized operation.
   *
   * \param allow_reference_to_source May the destination object reference the source? (if not, tConversionResultType is always INDEPENDENT; an additional deep copy operation is possibly inserted)
   * \param source_type Source Type (can be omitted if first operation has fixed source type)
   * \param destination_type Destination Type (can be omitted if last operation has fixed destination type)
   * \throw Throws exception if conversion operation sequence erroneous, ambiguous, or cannot be used to convert specified types
   */
  tCompiledConversionOperation Compile(bool allow_reference_to_source, const tType& source_type = tType(), const tType& destination_type = tType()) const;

  /*!
   * Get conversion parameter value
   *
   * \param operation_index Index of conversion operation in sequence. 0 or 1 are valid indices.
   * \return Pointer to buffer with parameter if it has been specified (otherwise nullptr -> the conversion operation should use a default value)
   */
  const tTypedConstPointer& GetParameterValue(size_t operation_index) const
  {
    return operations[operation_index].parameter ? static_cast<const tTypedConstPointer&>(*operations[operation_index].parameter) : cNO_PARAMETER_VALUE;
  }

  /*!
   * \return If sequence contains two operations: type after first operation
   */
  tType IntermediateType() const
  {
    return intermediate_type;
  }

  /*!
   * Set conversion parameter value
   *
   * \param operation_index Index of conversion operation in sequence. 0 or 1 are valid indices.
   * \param new_value Pointer to buffer with new parameter value. An empty pointer is also valid in order to reset value to default.
   * \throw Throws std::exception on invalid arguments
   */
  void SetParameterValue(size_t operation_index, const tTypedConstPointer& new_value);

  /*!
   * Set conversion parameter value
   *
   * \param operation_index Index of conversion operation in sequence. 0 or 1 are valid indices.
   * \param new_value Parameter as string. Will be deserialized when operation is compiled.
   * \throw Throws std::exception on invalid arguments
   */
  void SetParameterValue(size_t operation_index, const std::string& new_value)
  {
    SetParameterValue(operation_index, tTypedConstPointer(&new_value));
  }

  /*!
   * \return Number of conversion operations in chain
   */
  unsigned int Size() const
  {
    return operations[0].operation == nullptr ? 0 : (operations[1].operation == nullptr ? 1 : 2);
  }

  /*!
   * \param Index of conversion operation in sequence. 0 or 1 are valid indices.
   * \return Conversion operation at index. First is name of conversion operation (nullptr if there is no conversion operations at specified index). Second is pointer to conversion operation. May be null, if lookup by name was ambiguous.
   */
  const std::pair<const char*, const tRegisteredConversionOperation*> operator[](size_t index) const
  {
    return std::pair<const char*, const tRegisteredConversionOperation*>(operations[index].operation ? operations[index].operation->Name() : nullptr, operations[index].operation && (!ambiguous_operation_lookup[index]) ? operations[index].operation : nullptr);
  }

  friend bool operator==(const tConversionOperationSequence& lhs, const tConversionOperationSequence& rhs)
  {
    if (lhs.Size() != rhs.Size())
    {
      return false;
    }
    for (size_t i = 0; i < lhs.Size(); i++)
    {
      if (lhs.ambiguous_operation_lookup[i] != rhs.ambiguous_operation_lookup[i] || (!(lhs.operations[i] == rhs.operations[i])))
      {
        return false;
      }
    }
    return lhs.Size() < 2 || lhs.intermediate_type == rhs.intermediate_type;
  }

//----------------------------------------------------------------------
// Private fields and methods
//----------------------------------------------------------------------
private:

  friend serialization::tInputStream& operator >> (serialization::tInputStream& stream, tConversionOperationSequence& sequence);

  /*! Data on Single operation in sequence */
  struct tSingleOperation
  {
    /*! Stores operations */
    const tRegisteredConversionOperation* operation;

    /*! Any parameter of operation */
    std::unique_ptr<tGenericObject> parameter;

    tSingleOperation(const tRegisteredConversionOperation* operation) : operation(operation), parameter() {}

    bool operator==(const tSingleOperation& other) const
    {
      return operation == other.operation && (((!parameter) && (!other.parameter)) || (parameter && other.parameter && parameter->Equals(*other.parameter)));
    }
  };

  /*! Operations in sequence */
  tSingleOperation operations[2];

  /*! Whether name lookup of operation was ambiguous (=> only name of operation is valid; => when compiling, this ambiguity needs to be resolved) */
  bool ambiguous_operation_lookup[2];  // extra array to reduce memory overhead

  /*! If sequence contains two operations: type after first operation (may be ambiguous otherwise) */
  tType intermediate_type;

  /*! Empty pointer - returned for parameter values that have not been set */
  static const tTypedConstPointer cNO_PARAMETER_VALUE;

  /*!
   * Copies parameter
   *
   * \param source Source parameter (may be nullptr, then destination will be reset)
   * \param destination Destination parameter
   */
  static void CopyParameter(const tTypedConstPointer& source, std::unique_ptr<tGenericObject>& destination);
};


serialization::tOutputStream& operator << (serialization::tOutputStream& stream, const tConversionOperationSequence& sequence);
serialization::tInputStream& operator >> (serialization::tInputStream& stream, tConversionOperationSequence& sequence);

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}


#endif
