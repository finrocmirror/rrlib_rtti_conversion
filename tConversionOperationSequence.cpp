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
/*!\file    rrlib/rtti_conversion/tConversionOperationSequence.cpp
 *
 * \author  Max Reichardt
 *
 * \date    2016-07-31
 *
 */
//----------------------------------------------------------------------
#include "rrlib/rtti_conversion/tConversionOperationSequence.h"

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "rrlib/rtti_conversion/tCompiledConversionOperation.h"
#include "rrlib/rtti_conversion/tStaticCastOperation.h"

//----------------------------------------------------------------------
// Debugging
//----------------------------------------------------------------------
#include <cassert>

//----------------------------------------------------------------------
// Namespace usage
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
// Const values
//----------------------------------------------------------------------

const tConversionOperationSequence tConversionOperationSequence::cNONE;
const tTypedConstPointer tConversionOperationSequence::cNO_PARAMETER_VALUE;

/*! Enum for binary serialization */
enum tOperationSerializationFlags : uint8_t { cFULL_OPERATION = 1, cPARAMETER = 2 };

//----------------------------------------------------------------------
// Implementation
//----------------------------------------------------------------------

tConversionOperationSequence::tConversionOperationSequence(const std::string& first, const std::string& second, const tType& intermediate_type) :
  operations {nullptr, nullptr},
           ambiguous_operation_lookup {false, false},
           intermediate_type(intermediate_type)
{
  auto find_result = tRegisteredConversionOperation::Find(first);
  operations[0].operation = find_result.first;
  ambiguous_operation_lookup[0] = find_result.second;
  if (operations[0].operation == nullptr)
  {
    throw std::runtime_error("Could not find registered conversion operation with name: " + first);
  }
  if (second.length())
  {
    find_result = tRegisteredConversionOperation::Find(second);
    operations[1].operation = find_result.first;
    ambiguous_operation_lookup[1] = find_result.second;
    if (operations[1].operation == nullptr)
    {
      throw std::runtime_error("Could not find registered conversion operation with name: " + second);
    }
  }
}

tConversionOperationSequence& tConversionOperationSequence::operator=(const tConversionOperationSequence & other)
{
  operations[0].operation = other.operations[0].operation;
  operations[1].operation = other.operations[1].operation;
  CopyParameter(other.operations[0].parameter ? *other.operations[0].parameter : tTypedConstPointer(), operations[0].parameter);
  CopyParameter(other.operations[1].parameter ? *other.operations[1].parameter : tTypedConstPointer(), operations[1].parameter);
  memcpy(ambiguous_operation_lookup, other.ambiguous_operation_lookup, sizeof(ambiguous_operation_lookup));
  intermediate_type = other.intermediate_type;
  return *this;
}

tCompiledConversionOperation tConversionOperationSequence::Compile(bool allow_reference_to_source, const tType& source_type, const tType& destination_type) const
{
  // ############
  // Resolve any ambiguous conversion operations
  // ############
  const tRegisteredConversionOperation* first_operation = operations[0].operation;
  if (first_operation && ambiguous_operation_lookup[0])
  {
    first_operation = &tRegisteredConversionOperation::Find(first_operation->Name(), source_type, Size() == 2 ? intermediate_type : destination_type);
  }
  const tRegisteredConversionOperation* second_operation = operations[1].operation;
  if (second_operation && ambiguous_operation_lookup[1])
  {
    second_operation = &tRegisteredConversionOperation::Find(first_operation->Name(), intermediate_type, destination_type);
  }

  // ############
  // Infer conversion options
  // This includes adding implicit conversions if less than two registered conversions are in chain
  // ############

  // Infer any undefined types
  tType type_source = source_type;
  tType type_destination = destination_type;
  tType type_intermediate = this->intermediate_type;
  if (!type_source)
  {
    type_source = first_operation ? first_operation->SupportedSourceTypes().single_type : tType();
  }
  if (!type_source)
  {
    throw std::runtime_error("Source type must be specified");
  }
  const tRegisteredConversionOperation* last_operation = second_operation ? second_operation : first_operation;
  if (!type_destination)
  {
    type_destination = last_operation ? last_operation->SupportedDestinationTypes().single_type : tType();
  }
  if (!type_destination)
  {
    throw std::runtime_error("Destination type must be specified");
  }
  if ((!type_intermediate) && second_operation)
  {
    type_intermediate = first_operation ? first_operation->SupportedDestinationTypes().single_type : tType();
    if (!type_intermediate)
    {
      type_intermediate = second_operation ? second_operation->SupportedSourceTypes().single_type : tType();
    }
    if (!type_intermediate)
    {
      throw std::runtime_error("Intermediate type must be specified");
    }
  }

  // Variables for result
  tConversionOption temp_conversion_option_1, temp_conversion_option_2;
  const tConversionOption* conversion1 = nullptr;
  const tConversionOption* conversion2 = nullptr;

  // No conversion operation specified: Look for implicit cast
  if ((!first_operation))
  {
    if (type_source == type_destination)
    {
      temp_conversion_option_1 = tConversionOption(type_source, type_destination, 0);
      conversion1 = &temp_conversion_option_1;
    }
    else
    {
      auto implicit_conversion = tStaticCastOperation::GetImplicitConversionOptions(type_source, type_destination);
      if (implicit_conversion.first.type == tConversionOptionType::NONE)
      {
        throw std::runtime_error("Type " + source_type.GetName() + " cannot be implicitly casted to " + destination_type.GetName());
      }
      temp_conversion_option_1 = implicit_conversion.first;
      conversion1 = &temp_conversion_option_1;
      if (implicit_conversion.second.type != tConversionOptionType::NONE)
      {
        temp_conversion_option_2 = implicit_conversion.second;
        conversion2 = &temp_conversion_option_2;
      }
    }
  }

  // For each operation
  else if (first_operation == &cFOR_EACH_OPERATION)
  {
    if (!(type_source.IsListType() && type_destination.IsListType()))
    {
      throw std::runtime_error("ForEach operation only applicable on list types");
    }
    if (!second_operation)
    {
      temp_conversion_option_2 = tStaticCastOperation::GetImplicitConversionOption(type_source.GetElementType(), type_destination.GetElementType());
      if (temp_conversion_option_2.type == tConversionOptionType::NONE)
      {
        throw std::runtime_error("Type " + source_type.GetElementType().GetName() + " cannot be implicitly casted to " + destination_type.GetElementType().GetName() + ". Second operation for ForEach must be specified.");
      }
    }
    else
    {
      temp_conversion_option_2 = second_operation->GetConversionOption(type_source.GetElementType(), type_destination.GetElementType());
      if (temp_conversion_option_2.type == tConversionOptionType::NONE)
      {
        throw std::runtime_error("Type " + source_type.GetElementType().GetName() + " cannot be converted to " + destination_type.GetElementType().GetName() + " with the selected operations.");
      }
    }
    conversion2 = &temp_conversion_option_2;
    temp_conversion_option_1 = cFOR_EACH_OPERATION.GetConversionOption(type_source, type_destination);
    conversion1 = &temp_conversion_option_1;
  }

  // Two conversion operations specified: Check types
  else if (second_operation)
  {
    temp_conversion_option_1 = first_operation->GetConversionOption(type_source, type_intermediate);
    temp_conversion_option_2 = second_operation->GetConversionOption(type_intermediate, type_destination);
    if (temp_conversion_option_1.type != tConversionOptionType::NONE && temp_conversion_option_2.type != tConversionOptionType::NONE)
    {
      conversion1 = &temp_conversion_option_1;
      conversion2 = &temp_conversion_option_2;
    }
  }

  // One conversion option specified: Is it enough - or do we need additional implicit cast?
  else
  {
    temp_conversion_option_1 = first_operation->GetConversionOption(type_source, type_destination);
    if (temp_conversion_option_1.type != tConversionOptionType::NONE)
    {
      conversion1 = &temp_conversion_option_1;
    }
    else
    {
      // Try implicit cast as second
      if (first_operation->SupportedSourceTypes().single_type == source_type && (first_operation->SupportedDestinationTypes().single_type || type_intermediate))
      {
        type_intermediate = type_intermediate ? type_intermediate : first_operation->SupportedDestinationTypes().single_type;
        temp_conversion_option_1 = first_operation->GetConversionOption(type_source, type_intermediate);
        temp_conversion_option_2 = tStaticCastOperation::GetImplicitConversionOption(type_intermediate, type_destination);
      }
      else if ((first_operation->SupportedSourceTypes().single_type || type_intermediate) && first_operation->SupportedDestinationTypes().single_type == destination_type)
      {
        type_intermediate = type_intermediate ? type_intermediate : first_operation->SupportedSourceTypes().single_type;
        temp_conversion_option_1 = tStaticCastOperation::GetImplicitConversionOption(type_source, type_intermediate);
        temp_conversion_option_2 = first_operation->GetConversionOption(type_intermediate, type_destination);
      }
      if (temp_conversion_option_1.type != tConversionOptionType::NONE && temp_conversion_option_2.type != tConversionOptionType::NONE)
      {
        conversion1 = &temp_conversion_option_1;
        conversion2 = &temp_conversion_option_2;
      }
      else
      {
        throw std::runtime_error("Intermediate type must be specified");
      }
    }
  }
  if (!conversion1)
  {
    throw std::runtime_error("Type " + source_type.GetName() + " cannot be casted to " + destination_type.GetName() + " with the selected operations");
  }


  // ############
  // Compile conversion operation from conversion options
  // ############
  typedef tCompiledConversionOperation::tFlag tFlag;
  assert(conversion1);
  const tConversionOption* last_conversion = conversion2 ? conversion2 : conversion1;

  // Do some sanity checks
  if ((conversion1->type == tConversionOptionType::CONST_OFFSET_REFERENCE_TO_SOURCE_OBJECT && conversion1->const_offset_reference_to_source_object > std::numeric_limits<unsigned int>::max() / 2) ||
      (last_conversion->type == tConversionOptionType::CONST_OFFSET_REFERENCE_TO_SOURCE_OBJECT && last_conversion->const_offset_reference_to_source_object > std::numeric_limits<unsigned int>::max() / 2))
  {
    throw std::runtime_error("Invalid fixed offset in conversion option");
  }

  // Prepare result
  tCompiledConversionOperation result;
  result.operations[0].operation = first_operation;
  result.operations[1].operation = second_operation;
  result.destination_type = last_conversion->destination_type;

  // Handle special case: only const offsets
  if (conversion1->type == tConversionOptionType::CONST_OFFSET_REFERENCE_TO_SOURCE_OBJECT && last_conversion->type == tConversionOptionType::CONST_OFFSET_REFERENCE_TO_SOURCE_OBJECT)
  {
    result.type_after_first_fixed_offset = result.destination_type;
    result.intermediate_type = result.destination_type;
    result.fixed_offset_first = static_cast<unsigned int>(conversion1->const_offset_reference_to_source_object + (conversion2 ? conversion2->const_offset_reference_to_source_object : 0));
    result.flags = tFlag::cRESULT_INDEPENDENT | tFlag::cRESULT_REFERENCES_SOURCE_DIRECTLY | tFlag::cDEEPCOPY_ONLY;
    return result;
  }

  // Handle cases where first operation is const offset
  bool first_op_is_const_offset = conversion1->type == tConversionOptionType::CONST_OFFSET_REFERENCE_TO_SOURCE_OBJECT;
  result.type_after_first_fixed_offset = first_op_is_const_offset ? conversion1->destination_type : conversion1->source_type;
  if (first_op_is_const_offset)
  {
    result.fixed_offset_first = static_cast<unsigned int>(conversion1->const_offset_reference_to_source_object);

    // first operation is done, so move second to first
    conversion1 = nullptr;
    std::swap(conversion1, conversion2);
    result.flags |= tFlag::cFIRST_OPERATION_OPTIMIZED_AWAY;
  }
  result.intermediate_type = conversion1->destination_type;

  // Single operation REFERENCES_SOURCE
  if (conversion1->type == tConversionOptionType::RESULT_REFERENCES_SOURCE_OBJECT && (!conversion2))
  {
    result.conversion_function_first = allow_reference_to_source ? conversion1->final_conversion_function : conversion1->first_conversion_function;
    result.flags = allow_reference_to_source ? tFlag::cRESULT_REFERENCES_SOURCE_INTERNALLY : (tFlag::cRESULT_INDEPENDENT | tFlag::cDO_FINAL_DEEPCOPY_AFTER_FIRST_FUNCTION);
  }
  // First operation is standard or REFERENCES_SOURCE
  else if (conversion1->type == tConversionOptionType::STANDARD_CONVERSION_FUNCTION || conversion1->type == tConversionOptionType::RESULT_REFERENCES_SOURCE_OBJECT)
  {
    result.conversion_function_first = conversion2 ? conversion1->first_conversion_function : conversion1->final_conversion_function;
    result.flags = tFlag::cRESULT_INDEPENDENT;
    if (conversion2 && conversion2->type == tConversionOptionType::STANDARD_CONVERSION_FUNCTION)
    {
      result.conversion_function_final = conversion2->final_conversion_function;
    }
    else if (conversion2 && conversion2->type == tConversionOptionType::CONST_OFFSET_REFERENCE_TO_SOURCE_OBJECT)
    {
      if (conversion2->const_offset_reference_to_source_object == 0 && conversion2->source_type == conversion2->destination_type && (conversion1->type == tConversionOptionType::STANDARD_CONVERSION_FUNCTION || allow_reference_to_source))
      {
        result.conversion_function_first =  conversion1->final_conversion_function; // second operation can be optimized away
        result.intermediate_type = result.destination_type;
        if (conversion1->type == tConversionOptionType::RESULT_REFERENCES_SOURCE_OBJECT)
        {
          assert(allow_reference_to_source);
          result.flags = tFlag::cRESULT_REFERENCES_SOURCE_INTERNALLY;
        }
      }
      else
      {
        result.flags |= tFlag::cDO_FINAL_DEEPCOPY_AFTER_FIRST_FUNCTION;
        result.fixed_offset_final = conversion2->const_offset_reference_to_source_object;
      }
    }
    else if (conversion2 && (conversion2->type == tConversionOptionType::VARIABLE_OFFSET_REFERENCE_TO_SOURCE_OBJECT || conversion2->type == tConversionOptionType::RESULT_REFERENCES_SOURCE_OBJECT))
    {
      result.conversion_function_final = conversion2->first_conversion_function;
      result.flags |= tFlag::cDO_FINAL_DEEPCOPY_AFTER_SECOND_FUNCTION;
    }
  }
  // First operation is variable offset
  else if (conversion1->type == tConversionOptionType::VARIABLE_OFFSET_REFERENCE_TO_SOURCE_OBJECT)
  {
    bool reference_result = allow_reference_to_source && ((!conversion2) || conversion2->type != tConversionOptionType::STANDARD_CONVERSION_FUNCTION);
    if (reference_result)
    {
      if (conversion2 && conversion2->type == tConversionOptionType::RESULT_REFERENCES_SOURCE_OBJECT)
      {
        result.flags |= tFlag::cRESULT_REFERENCES_SOURCE_INTERNALLY;
        result.conversion_function_first = conversion1->first_conversion_function;
        result.conversion_function_final = conversion2->final_conversion_function;
      }
      else
      {
        result.flags |= tFlag::cRESULT_REFERENCES_SOURCE_DIRECTLY;
        result.get_destination_reference_function_first = conversion2->destination_reference_function;
        if (conversion2 && conversion2->type == tConversionOptionType::CONST_OFFSET_REFERENCE_TO_SOURCE_OBJECT)
        {
          result.fixed_offset_final = conversion2->const_offset_reference_to_source_object;
        }
        else if (conversion2 && conversion2->type == tConversionOptionType::VARIABLE_OFFSET_REFERENCE_TO_SOURCE_OBJECT)
        {
          result.get_destination_reference_function_final = conversion2->destination_reference_function;
        }
      }
    }
    else
    {
      result.conversion_function_first = conversion1->first_conversion_function;
      result.flags |= tFlag::cRESULT_INDEPENDENT;
      if (!conversion2)
      {
        result.flags |= tFlag::cDO_FINAL_DEEPCOPY_AFTER_FIRST_FUNCTION;
      }
      else if (conversion2->type == tConversionOptionType::STANDARD_CONVERSION_FUNCTION)
      {
        result.conversion_function_final = conversion2->final_conversion_function;
      }
      else if (conversion2->type == tConversionOptionType::CONST_OFFSET_REFERENCE_TO_SOURCE_OBJECT)
      {
        result.flags |= tFlag::cDO_FINAL_DEEPCOPY_AFTER_FIRST_FUNCTION;
        result.fixed_offset_final = conversion2->const_offset_reference_to_source_object;
      }
      else if (conversion2 && (conversion2->type == tConversionOptionType::VARIABLE_OFFSET_REFERENCE_TO_SOURCE_OBJECT || conversion2->type == tConversionOptionType::RESULT_REFERENCES_SOURCE_OBJECT))
      {
        result.conversion_function_final = conversion2->first_conversion_function;
        result.flags |= tFlag::cDO_FINAL_DEEPCOPY_AFTER_SECOND_FUNCTION;
      }
    }
  }

  // ############
  // Convert any parameters provided as strings to their required types
  // ############
  for (size_t i = 0; i < 2; i++)
  {
    const tRegisteredConversionOperation* operation = i == 0 ? first_operation : second_operation;
    if (operation && operation->Parameter() && GetParameterValue(i))
    {
      const tTypedConstPointer& value = GetParameterValue(i);
      if (value.GetType() == operation->Parameter().GetType())
      {
        CopyParameter(value, result.operations[i].parameter);
      }
      else if (value.GetType() == tDataType<std::string>())
      {
        serialization::tStringInputStream stream(*value.Get<std::string>());
        result.operations[i].parameter.reset(operation->Parameter().GetType().CreateGenericObject());
        result.operations[i].parameter->Deserialize(stream);
      }
      else
      {
        throw std::runtime_error(std::string("Parameter ") + operation->Parameter().GetName() + " has invalid type");
      }
    }
  }

  return result;
}

void tConversionOperationSequence::CopyParameter(const tTypedConstPointer& source, std::unique_ptr<tGenericObject>& destination)
{
  if (source)
  {
    if ((!destination) || destination->GetType() != source.GetType())
    {
      destination.reset(source.GetType().CreateGenericObject());
      destination->DeepCopyFrom(source);
    }
  }
  else
  {
    destination.reset();
  }
}

void tConversionOperationSequence::SetParameterValue(size_t operation_index, const tTypedConstPointer& new_value)
{
  assert(operation_index < 2);
  CopyParameter(new_value, operations[operation_index].parameter);
}

serialization::tOutputStream& operator << (serialization::tOutputStream& stream, const tConversionOperationSequence& sequence)
{
  stream.WriteByte(static_cast<uint8_t>(sequence.Size()));
  for (size_t i = 0; i < sequence.Size(); i++)
  {
    auto operation = sequence[i];
    auto parameter_value = sequence.GetParameterValue(i);
    uint8_t flags = (operation.second ? cFULL_OPERATION : 0) | (parameter_value ? cPARAMETER : 0);
    stream.WriteByte(flags);
    if (operation.second)
    {
      stream << *operation.second;
    }
    else
    {
      assert(operation.first);
      stream << operation.first;
    }
    if (parameter_value)
    {
      parameter_value.Serialize(stream);
    }
  }
  if (sequence.Size() > 1)
  {
    stream << sequence.IntermediateType();
  }
  return stream;
}

serialization::tInputStream& operator >> (serialization::tInputStream& stream, tConversionOperationSequence& sequence)
{
  size_t size = stream.ReadByte();
  if (size > 2)
  {
    throw std::runtime_error("Invalid sequence size");
  }
  for (size_t i = 0; i < 2; i++)
  {
    if (i >= size)
    {
      sequence.operations[i].operation = nullptr;
      sequence.operations[i].parameter.reset();
      sequence.ambiguous_operation_lookup[i] = false;
    }
    else
    {
      uint8_t flags = stream.ReadByte();
      if (flags & cFULL_OPERATION)
      {
        sequence.operations[i].operation = tRegisteredConversionOperation::Deserialize(stream, true);
        sequence.ambiguous_operation_lookup[i] = false;
      }
      else
      {
        std::string name = stream.ReadString();
        auto search_result = tRegisteredConversionOperation::Find(name);
        if (!search_result.first)
        {
          throw std::runtime_error("No conversion operation named " + name + " found");
        }
        sequence.operations[i].operation = search_result.first;
        sequence.ambiguous_operation_lookup[i] = search_result.second;
      }
      if (flags & cPARAMETER)
      {
        if ((!sequence.operations[i].operation) || (!sequence.operations[i].operation->Parameter().GetType()))
        {
          throw std::runtime_error("No parameter defined in conversion operation to deserialize");
        }
        if ((!sequence.operations[i].parameter))
        {
          sequence.operations[i].parameter.reset(sequence.operations[i].operation->Parameter().GetType().CreateGenericObject());
        }
        sequence.operations[i].parameter->Deserialize(stream);
      }
      else
      {
        sequence.operations[i].parameter.reset();
      }
    }
  }
  if (size > 1)
  {
    stream >> sequence.intermediate_type;
  }
  else
  {
    sequence.intermediate_type = rrlib::rtti::tType();
  }
  return stream;
}

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}
