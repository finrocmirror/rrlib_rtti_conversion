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
/*!\file    rrlib/rtti_conversion/defined_conversions.cpp
 *
 * \author  Max Reichardt
 *
 * \date    2016-08-13
 *
 */
//----------------------------------------------------------------------
#include "rrlib/rtti_conversion/defined_conversions.h"

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------
#include <iomanip>

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "rrlib/rtti_conversion/tStaticCastOperation.h"
#include "rrlib/rtti_conversion/definition/tVoidFunctionConversionOperation.h"

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

// Register static casts between builtin types
auto& cBUILTIN_TYPE_CASTS = tStaticCastOperation::
                            Register<int8_t, int16_t, true, true>()
                            .Register<int8_t, int32_t, true, true>()
                            .Register<int8_t, int64_t, true, true>()
                            .Register<int8_t, uint8_t, true, true>()
                            .Register<int8_t, uint16_t, true, true>()
                            .Register<int8_t, uint32_t, true, true>()
                            .Register<int8_t, uint64_t, true, true>()
                            .Register<int8_t, float, true, true>()
                            .Register<int8_t, double, true, true>()
                            .Register<int8_t, bool, true, true>()

                            .Register<int16_t, int32_t, true, true>()
                            .Register<int16_t, int64_t, true, true>()
                            .Register<int16_t, uint8_t, true, true>()
                            .Register<int16_t, uint16_t, true, true>()
                            .Register<int16_t, uint32_t, true, true>()
                            .Register<int16_t, uint64_t, true, true>()
                            .Register<int16_t, float, true, true>()
                            .Register<int16_t, double, true, true>()
                            .Register<int16_t, bool, true, true>()

                            .Register<int32_t, int64_t, true, true>()
                            .Register<int32_t, uint8_t, true, true>()
                            .Register<int32_t, uint16_t, true, true>()
                            .Register<int32_t, uint32_t, true, true>()
                            .Register<int32_t, uint64_t, true, true>()
                            .Register<int32_t, float, true, true>()
                            .Register<int32_t, double, true, true>()
                            .Register<int32_t, bool, true, true>()

                            .Register<int64_t, uint8_t, true, true>()
                            .Register<int64_t, uint16_t, true, true>()
                            .Register<int64_t, uint32_t, true, true>()
                            .Register<int64_t, uint64_t, true, true>()
                            .Register<int64_t, float, true, true>()
                            .Register<int64_t, double, true, true>()
                            .Register<int64_t, bool, true, true>()

                            .Register<uint8_t, uint16_t, true, true>()
                            .Register<uint8_t, uint32_t, true, true>()
                            .Register<uint8_t, uint64_t, true, true>()
                            .Register<uint8_t, float, true, true>()
                            .Register<uint8_t, double, true, true>()
                            .Register<uint8_t, bool, true, true>()

                            .Register<uint16_t, uint32_t, true, true>()
                            .Register<uint16_t, uint64_t, true, true>()
                            .Register<uint16_t, float, true, true>()
                            .Register<uint16_t, double, true, true>()
                            .Register<uint16_t, bool, true, true>()

                            .Register<uint32_t, uint64_t, true, true>()
                            .Register<uint32_t, float, true, true>()
                            .Register<uint32_t, double, true, true>()
                            .Register<uint32_t, bool, true, true>()

                            .Register<uint64_t, float, true, true>()
                            .Register<uint64_t, double, true, true>()
                            .Register<uint64_t, bool, true, true>()

                            .Register<float, double, true, true>()
                            .Register<float, bool, true, true>()

                            .Register<double, bool, true, true>()

                            .Register<rrlib::serialization::tMemoryBuffer, std::vector<uint8_t>>();

//----------------------------------------------------------------------
// Implementation
//----------------------------------------------------------------------

namespace
{

class tToStringOperation : public tRegisteredConversionOperation
{
public:
  tToStringOperation() : tRegisteredConversionOperation(util::tManagedConstCharPointer("ToString", false), tSupportedTypeFilter::STRING_SERIALIZABLE, tDataType<std::string>(), nullptr, tParameterDefinition("Flags", tDataType<unsigned int>(), true))
  {}

  virtual tConversionOption GetConversionOption(const tType& source_type, const tType& destination_type, const tGenericObject* parameter) const override
  {
    if ((source_type.GetTypeTraits() & trait_flags::cIS_STRING_SERIALIZABLE) && destination_type == tDataType<std::string>())
    {
      return tConversionOption(source_type, destination_type, false, &FirstConversionFunction, &FinalConversionFunction);
    }
    return tConversionOption();
  }

  static void MainConversionFunction(const tTypedConstPointer& source_object, std::string& destination, const tCurrentConversionOperation& operation)
  {
    rrlib::serialization::tStringOutputStream stream;

    // init stream
    /*auto precision = operation.GetParameterValue(0);
    if (precision)
    {
      stream.GetWrappedStringStream() << std::setprecision(*precision.Get<int>());
    }*/
    auto flags = operation.GetParameterValue();
    if (flags)
    {
      unsigned int f = *flags.Get<unsigned int>();
      if (f)
      {
        if (f & eTSF_BOOL_ALPHA)
        {
          stream.GetWrappedStringStream() << std::boolalpha;
        }
        if (f & eTSF_SHOW_BASE)
        {
          stream.GetWrappedStringStream() << std::showbase;
        }
        if (f & eTSF_SHOW_POINT)
        {
          stream.GetWrappedStringStream() << std::showpoint;
        }
        if (f & eTSF_SHOW_POS)
        {
          stream.GetWrappedStringStream() << std::showpos;
        }
        if (f & eTSF_UPPER_CASE)
        {
          stream.GetWrappedStringStream() << std::uppercase;
        }
        if (f & eTSF_DEC)
        {
          stream.GetWrappedStringStream() << std::dec;
        }
        if (f & eTSF_HEX)
        {
          stream.GetWrappedStringStream() << std::hex;
        }
        if (f & eTSF_OCT)
        {
          stream.GetWrappedStringStream() << std::oct;
        }
        if (f & eTSF_FIXED)
        {
          stream.GetWrappedStringStream() << std::fixed;
        }
        if (f & eTSF_SCIENTIFIC)
        {
          stream.GetWrappedStringStream() << std::scientific;
        }
      }
    }

    source_object.Serialize(stream);
    destination = stream.ToString();
  }

  static void FirstConversionFunction(const tTypedConstPointer& source_object, const tTypedPointer& destination_object, const tCurrentConversionOperation& operation)
  {
    std::string intermediate;
    MainConversionFunction(source_object, intermediate, operation);
    operation.Continue(tTypedConstPointer(&intermediate), destination_object);
  }

  static void FinalConversionFunction(const tTypedConstPointer& source_object, const tTypedPointer& destination_object, const tCurrentConversionOperation& operation)
  {
    MainConversionFunction(source_object, *destination_object.Get<std::string>(), operation);
  }
};

class tStringDeserializationOperation : public tRegisteredConversionOperation
{
public:
  tStringDeserializationOperation() : tRegisteredConversionOperation(util::tManagedConstCharPointer("String Deserialization", false), tDataType<std::string>(), tSupportedTypeFilter::STRING_SERIALIZABLE)
  {}

  virtual tConversionOption GetConversionOption(const tType& source_type, const tType& destination_type, const tGenericObject* parameter) const override
  {
    if ((destination_type.GetTypeTraits() & trait_flags::cIS_STRING_SERIALIZABLE) && source_type == tDataType<std::string>())
    {
      return tConversionOption(source_type, destination_type, false, &FirstConversionFunction, &FinalConversionFunction);
    }
    return tConversionOption();
  }

  static void FirstConversionFunction(const tTypedConstPointer& source_object, const tTypedPointer& destination_object, const tCurrentConversionOperation& operation)
  {
    tType inter_type = operation.compiled_operation.IntermediateType();
    char intermediate_memory[inter_type.GetSize(true)];
    auto intermediate_object = inter_type.EmplaceGenericObject(intermediate_memory);
    serialization::tStringInputStream stream(*source_object.Get<std::string>());
    intermediate_object->Deserialize(stream);
    operation.Continue(*intermediate_object, destination_object);
  }

  static void FinalConversionFunction(const tTypedConstPointer& source_object, const tTypedPointer& destination_object, const tCurrentConversionOperation& operation)
  {
    serialization::tStringInputStream stream(*source_object.Get<std::string>());
    destination_object.Deserialize(stream);
  }
};

class tBinarySerializationOperation : public tRegisteredConversionOperation
{
public:
  tBinarySerializationOperation() : tRegisteredConversionOperation(util::tManagedConstCharPointer("Binary Serialization", false), tSupportedTypeFilter::BINARY_SERIALIZABLE, tDataType<serialization::tMemoryBuffer>())
  {}

  virtual tConversionOption GetConversionOption(const tType& source_type, const tType& destination_type, const tGenericObject* parameter) const override
  {
    if ((source_type.GetTypeTraits() & trait_flags::cIS_BINARY_SERIALIZABLE) && destination_type == tDataType<serialization::tMemoryBuffer>())
    {
      return tConversionOption(source_type, destination_type, false, &FirstConversionFunction, &FinalConversionFunction);
    }
    return tConversionOption();
  }

  static void FirstConversionFunction(const tTypedConstPointer& source_object, const tTypedPointer& destination_object, const tCurrentConversionOperation& operation)
  {
    serialization::tStackMemoryBuffer<serialization::cSTACK_BUFFERS_SIZE> buffer;
    rrlib::serialization::tOutputStream stream(buffer);
    source_object.Serialize(stream);
    stream.Close();
    operation.Continue(tTypedConstPointer(&static_cast<serialization::tMemoryBuffer&>(buffer)), destination_object);
  }

  static void FinalConversionFunction(const tTypedConstPointer& source_object, const tTypedPointer& destination_object, const tCurrentConversionOperation& operation)
  {
    serialization::tOutputStream stream(*destination_object.Get<serialization::tMemoryBuffer>());
    destination_object.Serialize(stream);
  }
};

class tBinaryDeserializationOperation : public tRegisteredConversionOperation
{
public:
  tBinaryDeserializationOperation() : tRegisteredConversionOperation(util::tManagedConstCharPointer("Binary Deserialization", false), tDataType<serialization::tMemoryBuffer>(), tSupportedTypeFilter::STRING_SERIALIZABLE)
  {}

  virtual tConversionOption GetConversionOption(const tType& source_type, const tType& destination_type, const tGenericObject* parameter) const override
  {
    if ((destination_type.GetTypeTraits() & trait_flags::cIS_BINARY_SERIALIZABLE) && source_type == tDataType<serialization::tMemoryBuffer>())
    {
      return tConversionOption(source_type, destination_type, false, &FirstConversionFunction, &FinalConversionFunction);
    }
    return tConversionOption();
  }

  static void FirstConversionFunction(const tTypedConstPointer& source_object, const tTypedPointer& destination_object, const tCurrentConversionOperation& operation)
  {
    tType inter_type = operation.compiled_operation.IntermediateType();
    char intermediate_memory[inter_type.GetSize(true)];
    auto intermediate_object = inter_type.EmplaceGenericObject(intermediate_memory);
    serialization::tInputStream stream(*source_object.Get<serialization::tMemoryBuffer>());
    intermediate_object->Deserialize(stream);
    operation.Continue(*intermediate_object, destination_object);
  }

  static void FinalConversionFunction(const tTypedConstPointer& source_object, const tTypedPointer& destination_object, const tCurrentConversionOperation& operation)
  {
    serialization::tInputStream stream(*source_object.Get<serialization::tMemoryBuffer>());
    destination_object.Deserialize(stream);
  }
};

class tGetListElement : public tRegisteredConversionOperation
{
public:
  tGetListElement() : tRegisteredConversionOperation(util::tManagedConstCharPointer("[]", false), tSupportedTypeFilter::GET_LIST_ELEMENT, tSupportedTypeFilter::GET_LIST_ELEMENT, nullptr, tParameterDefinition("Index", tDataType<unsigned int>(), true))
  {}

  virtual tConversionOption GetConversionOption(const tType& source_type, const tType& destination_type, const tGenericObject* parameter) const override
  {
    if (source_type.IsListType() && source_type.GetElementType() == destination_type)
    {
      return tConversionOption(source_type, destination_type, &FirstConversionFunction, &GetDestinationReference);
    }
    return tConversionOption();
  }

  static tTypedConstPointer GetDestinationReference(const tTypedConstPointer& source_object, const tCurrentConversionOperation& operation)
  {
    auto index_parameter = operation.GetParameterValue();
    unsigned int index = index_parameter ? (*index_parameter.Get<unsigned int>()) : 0;
    auto result = source_object.GetVectorElement(index);
    if (!result)
    {
      throw std::invalid_argument("Index out of bounds");
    }
    return result;
  }

  static void FirstConversionFunction(const tTypedConstPointer& source_object, const tTypedPointer& destination_object, const tCurrentConversionOperation& operation)
  {
    auto index_parameter = operation.GetParameterValue();
    unsigned int index = index_parameter ? (*index_parameter.Get<unsigned int>()) : 0;
    auto intermediate = source_object.GetVectorElement(index);
    if (!intermediate)
    {
      throw std::invalid_argument("Index out of bounds");
    }
    operation.Continue(intermediate, destination_object);
  }
};

class tForEach : public tRegisteredConversionOperation
{
public:
  tForEach() : tRegisteredConversionOperation(util::tManagedConstCharPointer("For Each", false), tSupportedTypeFilter::GENERIC_VECTOR_CAST, tSupportedTypeFilter::GENERIC_VECTOR_CAST)
  {}

  virtual tConversionOption GetConversionOption(const tType& source_type, const tType& destination_type, const tGenericObject* parameter) const override
  {
    if ((source_type.GetTypeTraits() & trait_flags::cIS_LIST_TYPE) && destination_type)
    {
      return tConversionOption(source_type, destination_type, false, &FirstConversionFunction, &FinalConversionFunction);
    }
    return tConversionOption();
  }

  static void FirstConversionFunction(const tTypedConstPointer& source_object, const tTypedPointer& destination_object, const tCurrentConversionOperation& operation)
  {
    size_t size = source_object.GetVectorSize();
    destination_object.ResizeVector(size);
    if (size)
    {
      tTypedConstPointer source_first = source_object.GetVectorElement(0);
      tTypedPointer destination_first = destination_object.GetVectorElement(0);
      operation.Continue(source_first, destination_object);
      if (size > 1)
      {
        tTypedConstPointer source_next = source_object.GetVectorElement(1);
        tTypedPointer destination_next = destination_object.GetVectorElement(1);
        operation.Continue(source_next, destination_next);
        size_t offset_source = static_cast<const char*>(source_next.GetRawDataPointer()) - static_cast<const char*>(source_first.GetRawDataPointer());
        size_t offset_destination = static_cast<const char*>(destination_next.GetRawDataPointer()) - static_cast<const char*>(destination_first.GetRawDataPointer());
        for (size_t i = 2; i < size; i++)
        {
          source_next = tTypedConstPointer(static_cast<const char*>(source_next.GetRawDataPointer()) + offset_source, source_next.GetType());
          destination_next = tTypedPointer(static_cast<char*>(destination_next.GetRawDataPointer()) + offset_destination, destination_next.GetType());
          operation.Continue(source_next, destination_next);
        }
      }
    }
  }

  static void FinalConversionFunction(const tTypedConstPointer& source_object, const tTypedPointer& destination_object, const tCurrentConversionOperation& operation)
  {
    throw std::logic_error("Not supported as single or second operation");
  }
};

class tGetArrayElement : public tRegisteredConversionOperation
{
public:
  tGetArrayElement() : tRegisteredConversionOperation(util::tManagedConstCharPointer("[]", false), tSupportedTypeFilter::GET_ARRAY_ELEMENT, tSupportedTypeFilter::GET_ARRAY_ELEMENT, nullptr, tParameterDefinition("Index", tDataType<unsigned int>(), true))
  {}

  virtual tConversionOption GetConversionOption(const tType& source_type, const tType& destination_type, const tGenericObject* parameter) const override
  {
    unsigned int index = parameter ? parameter->GetData<unsigned int>() : 0;
    if (source_type.IsArray() && source_type.GetElementType() == destination_type && index < source_type.GetArraySize())
    {
      return tConversionOption(source_type, destination_type, index * source_type.GetElementType().GetSize());
    }
    return tConversionOption();
  }
};

class tForEachArray : public tRegisteredConversionOperation
{
public:
  tForEachArray() : tRegisteredConversionOperation(util::tManagedConstCharPointer("For Each", false), tSupportedTypeFilter::GENERIC_ARRAY_CAST, tSupportedTypeFilter::GENERIC_ARRAY_CAST)
  {}

  virtual tConversionOption GetConversionOption(const tType& source_type, const tType& destination_type, const tGenericObject* parameter) const override
  {
    if ((source_type.GetTypeTraits() & trait_flags::cIS_ARRAY) && destination_type)
    {
      return tConversionOption(source_type, destination_type, false, &FirstConversionFunction, &FinalConversionFunction);
    }
    return tConversionOption();
  }

  static void FirstConversionFunction(const tTypedConstPointer& source_object, const tTypedPointer& destination_object, const tCurrentConversionOperation& operation)
  {
    tType source_type = source_object.GetType();
    tType destination_type = destination_object.GetType();
    size_t size = source_type.GetArraySize();
    if (size != destination_type.GetArraySize())
    {
      throw std::runtime_error("Arrays must have the same size");
    }

    size_t source_element_offset = source_type.GetSize() / size;
    size_t destination_element_offset = destination_type.GetSize() / size;
    for (size_t i = 0; i < size; i++)
    {
      tTypedConstPointer source(static_cast<const char*>(source_object.GetRawDataPointer()) + i * source_element_offset, source_type);
      tTypedPointer destination(static_cast<char*>(destination_object.GetRawDataPointer()) + i * destination_element_offset, destination_type);
      operation.Continue(source, destination);
    }
  }

  static void FinalConversionFunction(const tTypedConstPointer& source_object, const tTypedPointer& destination_object, const tCurrentConversionOperation& operation)
  {
    throw std::logic_error("Not supported as single or second operation");
  }
};

class tWrapByteVectorOperation : public tRegisteredConversionOperation
{
public:
  tWrapByteVectorOperation() : tRegisteredConversionOperation(util::tManagedConstCharPointer("Wrap", false), tDataType<std::vector<uint8_t>>(), tDataType<rrlib::serialization::tMemoryBuffer>(), &cCONVERSION_OPTION)
  {}

  static void FirstConversionFunction(const tTypedConstPointer& source_object, const tTypedPointer& destination_object, const tCurrentConversionOperation& operation)
  {
    const std::vector<uint8_t>& vector = *source_object.Get<std::vector<uint8_t>>();
    if (vector.size())
    {
      const rrlib::serialization::tMemoryBuffer buffer(const_cast<uint8_t*>(&vector[0]), vector.size());
      operation.Continue(tTypedConstPointer(&buffer), destination_object);
    }
    else
    {
      const rrlib::serialization::tMemoryBuffer buffer(0);
      operation.Continue(tTypedConstPointer(&buffer), destination_object);
    }
  }

  static void FinalConversionFunction(const tTypedConstPointer& source_object, const tTypedPointer& destination_object, const tCurrentConversionOperation& operation)
  {
    rrlib::serialization::tMemoryBuffer& buffer = *destination_object.Get<rrlib::serialization::tMemoryBuffer>();
    const std::vector<uint8_t>& vector = *source_object.Get<std::vector<uint8_t>>();
    const rrlib::serialization::tMemoryBuffer temp_buffer(const_cast<uint8_t*>(&vector[0]), vector.size());
    buffer.CopyFrom(temp_buffer);
  }

  static constexpr tConversionOption cCONVERSION_OPTION = tConversionOption(tDataType<std::vector<uint8_t>>(), tDataType<rrlib::serialization::tMemoryBuffer>(), false, &FirstConversionFunction, &FinalConversionFunction);
};

constexpr tConversionOption tWrapByteVectorOperation::cCONVERSION_OPTION;

class tListSize : public tRegisteredConversionOperation
{
public:
  tListSize() : tRegisteredConversionOperation(util::tManagedConstCharPointer("size()", false), tSupportedTypeFilter::LISTS, tDataType<size_t>())
  {}

  virtual tConversionOption GetConversionOption(const tType& source_type, const tType& destination_type, const tGenericObject* parameter) const override
  {
    if (source_type.IsListType() && destination_type == tDataType<size_t>())
    {
      return tConversionOption(source_type, destination_type, false, &FirstConversionFunction, &FinalConversionFunction);
    }
    return tConversionOption();
  }

  static void FirstConversionFunction(const tTypedConstPointer& source_object, const tTypedPointer& destination_object, const tCurrentConversionOperation& operation)
  {
    size_t size = source_object.GetVectorSize();
    operation.Continue(tTypedConstPointer(&size), destination_object);
  }

  static void FinalConversionFunction(const tTypedConstPointer& source_object, const tTypedPointer& destination_object, const tCurrentConversionOperation& operation)
  {
    (*destination_object.Get<size_t>()) = source_object.GetVectorSize();
  }
};

void StringToVectorConversionFunction(const std::string& source, std::vector<char>& destination)
{
  destination = std::vector<char>(source.begin(), source.end());
}

void VectorToStringConversionFunction(const std::vector<char>& source, std::string& destination)
{
  destination = std::string(source.begin(), source.end());
}


const tToStringOperation cTO_STRING;
const tStringDeserializationOperation cSTRING_DESERIALIZATION;
const tBinarySerializationOperation cBINARY_SERIALIZATION;
const tBinaryDeserializationOperation cBINARY_DESERIALIZATION;
const tGetListElement cGET_LIST_ELEMENT;
const tForEach cFOR_EACH;
const tGetArrayElement cGET_ARRAY_ELEMENT;
const tForEachArray cFOR_EACH_ARRAY;
const tWrapByteVectorOperation cWRAP_BYTE_VECTOR;
const tListSize cLIST_SIZE;
const tVoidFunctionConversionOperation<std::string, std::vector<char>, decltype(&StringToVectorConversionFunction), &StringToVectorConversionFunction> cSTRING_TO_VECTOR("ToVector");
const tVoidFunctionConversionOperation<std::vector<char>, std::string, decltype(&VectorToStringConversionFunction), &VectorToStringConversionFunction> cVECTOR_TO_STRING("MakeString");

}

const tRegisteredConversionOperation& cTO_STRING_OPERATION = cTO_STRING;
const tRegisteredConversionOperation& cSTRING_DESERIALIZATION_OPERATION = cSTRING_DESERIALIZATION;
const tRegisteredConversionOperation& cBINARY_SERIALIZATION_OPERATION = cBINARY_SERIALIZATION;
const tRegisteredConversionOperation& cBINARY_DESERIALIZATION_OPERATION = cBINARY_DESERIALIZATION;
const tRegisteredConversionOperation& cGET_LIST_ELEMENT_OPERATION = cGET_LIST_ELEMENT;
const tRegisteredConversionOperation& cFOR_EACH_OPERATION = cFOR_EACH;
const tRegisteredConversionOperation& cGET_ARRAY_ELEMENT_OPERATION = cGET_ARRAY_ELEMENT;
const tRegisteredConversionOperation& cFOR_EACH_OPERATION_ARRAY = cFOR_EACH_ARRAY;
const tRegisteredConversionOperation& cWRAP_BYTE_VECTOR_OPERATION = cWRAP_BYTE_VECTOR;
const tRegisteredConversionOperation& cLIST_SIZE_OPERATION = cLIST_SIZE;
const tRegisteredConversionOperation& cSTRING_TO_VECTOR_OPERATION = cSTRING_TO_VECTOR;
const tRegisteredConversionOperation& cMAKE_STRING_OPERATION = cVECTOR_TO_STRING;



//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}
