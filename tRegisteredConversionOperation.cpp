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
/*!\file    rrlib/rtti_conversion/tRegisteredConversionOperation.cpp
 *
 * \author  Max Reichardt
 *
 * \date    2016-07-15
 *
 */
//----------------------------------------------------------------------
#include "rrlib/rtti_conversion/tRegisteredConversionOperation.h"

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------
#include "rrlib/thread/tLock.h"

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "rrlib/rtti_conversion/tConversionOption.h"
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
const char* tRegisteredConversionOperation::cSTATIC_CAST_NAME = "static_cast";

//----------------------------------------------------------------------
// Implementation
//----------------------------------------------------------------------

tRegisteredConversionOperation::tRegisteredConversionOperation(util::tManagedConstCharPointer name, const tSupportedTypes& supported_source_types, const tSupportedTypes& supported_destination_types,
    const tConversionOption* single_conversion_option, const tParameterDefinition& parameter) :
  name(std::move(name)),
  supported_source_types(supported_source_types),
  supported_destination_types(supported_destination_types),
  parameter(parameter),
  single_conversion_option(single_conversion_option)
{
  if (parameter && (!(parameter.GetType().GetTypeTraits() & trait_flags::cIS_STRING_SERIALIZABLE)))
  {
    throw std::runtime_error(std::string("Conversion operation: '") + this->name.Get() + "'. Parameters have to be string serializable.");
  }
  handle = static_cast<decltype(handle)>(tRegisteredConversionOperation::RegisteredOperations().operations.Add(this));
}

tRegisteredConversionOperation::tRegisteredConversionOperation(util::tManagedConstCharPointer name, const tConversionOption& single_conversion_option, const tParameterDefinition& parameter) :
  tRegisteredConversionOperation(std::move(name), single_conversion_option.source_type, single_conversion_option.destination_type, &single_conversion_option, parameter)
{
}

tRegisteredConversionOperation::tRegisteredConversionOperation() :
  name(cSTATIC_CAST_NAME, false),
  supported_source_types(tSupportedTypeFilter::STATIC_CAST),
  supported_destination_types(tSupportedTypeFilter::STATIC_CAST),
  parameter(),
  single_conversion_option(nullptr),
  handle(-1)
{
  handle = static_cast<decltype(handle)>(tRegisteredConversionOperation::RegisteredOperations().operations.Add(this));
}

tRegisteredConversionOperation::~tRegisteredConversionOperation()
{
}

void tRegisteredConversionOperation::AutoDelete()
{
  tRegisteredConversionOperation::tRegisteredOperations& registered_operations = tRegisteredConversionOperation::RegisteredOperations();
  registered_operations.auto_delete.Emplace(this);
}

const tRegisteredConversionOperation* tRegisteredConversionOperation::Deserialize(rrlib::serialization::tInputStream& stream, bool throw_exception_if_not_found)
{
  const tRegisteredConversionOperation* result = nullptr;
  if (tRegisteredConversionOperation::GetRegisteredOperations().operations.ReadEntry(stream, result))
  {
    std::string name = stream.ReadString();
    tRegisteredConversionOperation::tSupportedTypes source_types = tType();
    tRegisteredConversionOperation::tSupportedTypes destination_types = tType();
    stream >> source_types.filter;
    if (source_types.filter == tSupportedTypeFilter::SINGLE)
    {
      source_types.single_type = tType::GetType(stream.ReadShort());
    }
    stream >> destination_types.filter;
    if (destination_types.filter == tSupportedTypeFilter::SINGLE)
    {
      destination_types.single_type = tType::GetType(stream.ReadShort());
    }

    // Find matching conversion operation
    if (name == cSTATIC_CAST_NAME)
    {
      return &tStaticCastOperation::GetInstance();
    }

    tRegisteredConversionOperation::tRegisteredOperations& registered_operations = tRegisteredConversionOperation::RegisteredOperations();
    for (auto & operation : registered_operations.operations)
    {
      if (operation->supported_source_types.filter == source_types.filter && operation->supported_source_types.single_type == source_types.single_type && operation->supported_destination_types.filter == destination_types.filter && operation->supported_destination_types.single_type == destination_types.single_type && name == operation->Name())
      {
        return operation;
      }
    }
    if (throw_exception_if_not_found)
    {
      throw std::runtime_error("Encoded registered conversion operation not found");
    }
  }
  return result;
}

std::pair<const tRegisteredConversionOperation*, bool> tRegisteredConversionOperation::Find(const std::string& name)
{
  std::pair<const tRegisteredConversionOperation*, bool> result(nullptr, false);
  if (name == cSTATIC_CAST_NAME)
  {
    result.first = &tStaticCastOperation::GetInstance();
    return result;
  }

  tRegisteredConversionOperation::tRegisteredOperations& registered_operations = tRegisteredConversionOperation::RegisteredOperations();
  for (auto & operation : registered_operations.operations)
  {
    if (name == operation->Name())
    {
      if (!result.first)
      {
        result.first = operation;
      }
      else
      {
        result.second = true;
        return result;
      }
    }
  }
  return result;
}

const tRegisteredConversionOperation& tRegisteredConversionOperation::Find(const std::string& name, const tType& source_type, const tType& destination_type)
{
  const tRegisteredConversionOperation* result = nullptr;
  if (name == cSTATIC_CAST_NAME)
  {
    return tStaticCastOperation::GetInstance();
  }

  tRegisteredConversionOperation::tRegisteredOperations& registered_operations = tRegisteredConversionOperation::RegisteredOperations();
  for (auto & operation : registered_operations.operations)
  {
    if (name == operation->Name())
    {
      auto option = operation->GetConversionOption(source_type, destination_type);
      if (option.type != tConversionOptionType::NONE)
      {
        if (result)
        {
          throw std::runtime_error("Lookup of registered conversion operation " + name + " is ambiguous");
        }
        result = operation;
      }
    }
  }
  if (!result)
  {
    throw std::runtime_error("Lookup of registered conversion operation " + name + " with specified types failed");
  }

  return *result;
}

tConversionOption tRegisteredConversionOperation::GetConversionOption(const tType& source_type, const tType& destination_type) const
{
  if ((!source_type) || (!destination_type))
  {
    throw std::invalid_argument("Source type and destination type must be specified");
  }
  if (!single_conversion_option)
  {
    throw std::logic_error("Method must be overridden if no single conversion option is specified");
  }
  if (single_conversion_option->source_type == source_type && single_conversion_option->destination_type == destination_type)
  {
    return *single_conversion_option;
  }
  return tConversionOption();
}

tRegisteredConversionOperation::tRegisteredOperations& tRegisteredConversionOperation::RegisteredOperations()
{
  static tRegisteredOperations operations;
  return operations;
}

serialization::tOutputStream& operator << (serialization::tOutputStream& stream, const tRegisteredConversionOperation& operation)
{
  if (tRegisteredConversionOperation::GetRegisteredOperations().operations.WriteEntry(stream, operation.GetHandle()))
  {
    stream << operation.Name() << operation.SupportedSourceTypes().filter;
    if (operation.SupportedSourceTypes().filter == tSupportedTypeFilter::SINGLE)
    {
      stream << operation.SupportedSourceTypes().single_type;
    }
    stream << operation.SupportedDestinationTypes().filter;
    if (operation.SupportedDestinationTypes().filter == tSupportedTypeFilter::SINGLE)
    {
      stream << operation.SupportedDestinationTypes().single_type;
    }
  }

  return stream;
}

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}
