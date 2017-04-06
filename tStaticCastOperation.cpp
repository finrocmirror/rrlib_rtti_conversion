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
/*!\file    rrlib/rtti_conversion/tStaticCastOperation.cpp
 *
 * \author  Max Reichardt
 *
 * \date    2016-07-17
 *
 */
//----------------------------------------------------------------------
#include "rrlib/rtti_conversion/tStaticCastOperation.h"

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------
#include "rrlib/thread/tLock.h"

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------

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

//----------------------------------------------------------------------
// Implementation
//----------------------------------------------------------------------

tStaticCastOperation tStaticCastOperation::instance;

const tStaticCastOperation::tStaticCast tStaticCastOperation::tInstanceNone::value = { { tConversionOption() }, false };


tStaticCastOperation::tStaticCastOperation() : tRegisteredConversionOperation()
{}

tConversionOption tStaticCastOperation::GetConversionOption(const tType& source_type, const tType& destination_type) const
{
  if (source_type == destination_type)
  {
    return tConversionOption(source_type, destination_type, 0);
  }
  if ((source_type.GetUnderlyingType() == destination_type) ||
      (source_type.GetUnderlyingType() == destination_type.GetUnderlyingType() && (destination_type.GetTypeTraits() & trait_flags::cIS_REINTERPRET_CAST_FROM_UNDERLYING_TYPE_VALID)))
  {
    return tConversionOption(source_type, destination_type, 0);
  }
  const tRegisteredConversionOperation::tRegisteredOperations& registered_operations = tRegisteredConversionOperation::RegisteredOperations();
  for (auto & option : registered_operations.static_casts)
  {
    if (source_type == option->conversion_option.source_type && destination_type == option->conversion_option.destination_type)
    {
      return option->conversion_option;
    }
  }

  return tConversionOption();
}

tConversionOption tStaticCastOperation::GetImplicitConversionOption(const rrlib::rtti::tType& source_type, const rrlib::rtti::tType& destination_type)
{
  const tRegisteredConversionOperation::tRegisteredOperations& registered_operations = tRegisteredConversionOperation::RegisteredOperations();
  return GetImplicitConversionOption(source_type, destination_type, registered_operations);
}

tConversionOption tStaticCastOperation::GetImplicitConversionOption(const rrlib::rtti::tType& source_type, const rrlib::rtti::tType& destination_type, const tRegisteredConversionOperation::tRegisteredOperations& registered_operations)
{
  if (source_type == destination_type)
  {
    return tConversionOption(source_type, destination_type, 0);
  }
  if ((source_type.GetUnderlyingType() == destination_type && (source_type.GetTypeTraits() & trait_flags::cIS_CAST_TO_UNDERLYING_TYPE_IMPLICIT)) ||
      (source_type == destination_type.GetUnderlyingType() && (destination_type.GetTypeTraits() & trait_flags::cIS_CAST_FROM_UNDERLYING_TYPE_IMPLICIT)) ||
      (source_type.GetUnderlyingType() == destination_type.GetUnderlyingType() && (source_type.GetTypeTraits() & trait_flags::cIS_CAST_TO_UNDERLYING_TYPE_IMPLICIT) && (destination_type.GetTypeTraits() & trait_flags::cIS_CAST_FROM_UNDERLYING_TYPE_IMPLICIT)))
  {
    return tConversionOption(source_type, destination_type, 0);
  }
  for (auto & option : registered_operations.static_casts)
  {
    if (option->implicit && source_type == option->conversion_option.source_type && destination_type == option->conversion_option.destination_type)
    {
      return option->conversion_option;
    }
  }
  return tConversionOption();
}


std::pair<tConversionOption, tConversionOption> tStaticCastOperation::GetImplicitConversionOptions(const rrlib::rtti::tType& source_type, const rrlib::rtti::tType& destination_type)
{
  const tRegisteredConversionOperation::tRegisteredOperations& registered_operations = tRegisteredConversionOperation::RegisteredOperations();
  tConversionOption single_result = GetImplicitConversionOption(source_type, destination_type, registered_operations);
  if (single_result.type != tConversionOptionType::NONE)
  {
    return std::pair<tConversionOption, tConversionOption>(single_result, tConversionOption());
  }

  // Try all registered operations
  for (auto & option : registered_operations.static_casts)
  {
    if (option->implicit)
    {
      if (source_type == option->conversion_option.source_type)
      {
        tConversionOption second_option = GetImplicitConversionOption(option->conversion_option.destination_type, destination_type, registered_operations);
        if (second_option.type != tConversionOptionType::NONE)
        {
          return std::pair<tConversionOption, tConversionOption>(option->conversion_option, second_option);
        }
      }
      if (destination_type == option->conversion_option.destination_type)
      {
        tConversionOption first_option = GetImplicitConversionOption(source_type, option->conversion_option.source_type, registered_operations);
        if (first_option.type != tConversionOptionType::NONE)
        {
          return std::pair<tConversionOption, tConversionOption>(first_option, option->conversion_option);
        }
      }
    }
  }
  return std::pair<tConversionOption, tConversionOption>(tConversionOption(), tConversionOption());
}

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}
