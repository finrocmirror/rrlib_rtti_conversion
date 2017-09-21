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
/*!\file    rrlib/rtti_conversion/tRegisteredConversionOperation.h
 *
 * \author  Max Reichardt
 *
 * \date    2016-07-15
 *
 * \brief   Contains tRegisteredConversionOperation
 *
 * \b tRegisteredConversionOperation
 *
 * Conversion operation registered for runtime lookup.
 * Registered operations may be applicable to multiple combinations of data types.
 * They can always be compiled - and may not be usable directly.
 *
 * They must exist until the program shuts down.
 * If they are allocated via new, they can be flagged to be deleted at shutdown.
 *
 */
//----------------------------------------------------------------------
#ifndef __rrlib__rtti_conversion__tRegisteredConversionOperation_h__
#define __rrlib__rtti_conversion__tRegisteredConversionOperation_h__

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------
#include "rrlib/rtti/rtti.h"
#include "rrlib/rtti/tParameterDefinition.h"
#include <atomic>

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "rrlib/thread/tMutex.h"

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
struct tConversionOption;
struct tConversionOptionStaticCast;

/*!
 * Used to encode supported types of tRegisteredConversionOperations for external tools
 * This enum is to be extended if further filters are needed.
 */
enum class tSupportedTypeFilter : uint8_t
{
  SINGLE,              //!< Only a single type is supported
  BINARY_SERIALIZABLE, //!< All binary-serializable types are supported
  STRING_SERIALIZABLE, //!< All string-serializable types are supported
  LISTS,               //!< All list types are supported
  ALL,                 //!< All types are supported

  // Special operations defined in rrlib_rtti_conversion (known in Java tooling)
  STATIC_CAST,         //!< Types supported by static casts (only used for tStaticCastOperation)
  FOR_EACH,            //!< Types supported by for-each operation
  GET_LIST_ELEMENT,    //!< Types supported by get list element
  ARRAY_TO_VECTOR,     //!< Types supported by array to vector operation
  GET_TUPLE_ELEMENT    //!< Types supported by get tuple element operation
};

//----------------------------------------------------------------------
// Class declaration
//----------------------------------------------------------------------
//! Registered conversion operation
/*!
 * Conversion operation registered for runtime lookup.
 * Registered operations may be applicable to multiple combinations of data types.
 * They can always be compiled - and may not be usable directly.
 *
 * They must exist until the program shuts down.
 * If they are allocated via new, they can be flagged to be deleted at shutdown.
 */
class tRegisteredConversionOperation : public rrlib::util::tNoncopyable
{

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  /*! Name of static cast operation */
  static const char* cSTATIC_CAST_NAME;

  /*! Supported types for one end of cast operation */
  struct tSupportedTypes
  {
    /*! Type filter */
    tSupportedTypeFilter filter;

    /*! In case 'filter' is SINGLE, contains the single supported type; otherwise it is ignored */
    rrlib::rtti::tType single_type;

    /*! Constructor for multiple type support */
    tSupportedTypes(tSupportedTypeFilter filter) : filter(filter), single_type() {}

    /*! Custructor for single type support */
    tSupportedTypes(const rrlib::rtti::tType& single_type) : filter(tSupportedTypeFilter::SINGLE), single_type(single_type) {}


//    /*!
//     * Is type supported?
//     *
//     * \param type Type to check
//     * \return Whether type is supported
//     */
//    bool IsTypeSupported(const rrlib::rtti::tType& type)
//    {
//      switch (filter)
//      {
//      case tSupportedTypeFilter::SINGLE:
//        return type == single_type;
//        break;
//      case tSupportedTypeFilter::BINARY_SERIALIZABLE:
//        return type.GetTypeTraits() & trait_flags::cIS_BINARY_SERIALIZABLE;
//        break;
//      case tSupportedTypeFilter::STRING_SERIALIZABLE:
//        return type.GetTypeTraits() & trait_flags::cIS_STRING_SERIALIZABLE;
//        break;
//      case tSupportedTypeFilter::STATIC_CAST_SUPPORT:
//        return // TODO? Do we actually need this method (since type support ;
//        break;
//      }
//    }
  };

  /*! Data structure for managing registered operations */
  struct tRegisteredOperations
  {
    /*! The list of registered operations */
    typedef rrlib::serialization::tRegister<const tRegisteredConversionOperation*, 64, 64, uint16_t> tOperationsRegister;
    tOperationsRegister operations;

    /*! List of operations to auto-delete */
    typedef rrlib::concurrent_containers::tRegister<std::unique_ptr<const tRegisteredConversionOperation>, 32, 32> tAutoDeleteRegister;
    tAutoDeleteRegister auto_delete;

    /*! Registered static cast operations */
    typedef rrlib::serialization::tRegister<const tConversionOptionStaticCast*, 64, 64, uint16_t> tStaticCastRegister;
    tStaticCastRegister static_casts;
  };


  virtual ~tRegisteredConversionOperation();

  /*!
   * Deserializes registered conversion operation from input stream and returns any registered operation matching the deserialized data.
   *
   * \param stream Stream to deserialize from
   * \param throw_exception_if_not_found Throw std::runtime_error if no matching registered operation could be found?
   * \return Registered operation matching the deserialized data. nullptr if no such operation exists.
   */
  static const tRegisteredConversionOperation* Deserialize(rrlib::serialization::tInputStream& stream, bool throw_exception_if_not_found = false);

  /*!
   * Find registered conversion operation with specified name that support specified source and destination types
   *
   * \param name Name of conversion operation
   * \return Registered operation with the specified name. Second indicates whether there are more registered conversion operations with this name.
   */
  static std::pair<const tRegisteredConversionOperation*, bool> Find(const std::string& name);

  /*!
   * Find registered conversion operation with specified name that support specified source and destination types
   *
   * \param name Name of conversion operation
   * \param source_type Source type of operation (optional)
   * \param destination_type Destination type of operation (optional)
   * \return Registered operation with the specified name and types.
   * \throw Throws std::runtime_error if conversion was not found or is ambiguous.
   */
  static const tRegisteredConversionOperation& Find(const std::string& name, const tType& source_type, const tType& destination_type);

  /*!
   * Gets conversion option for converting the specified types.
   * Needs to be overridden unless single_conversion_option is set.
   *
   * \param source_type Source Type
   * \param destination_type Destination Type
   * \param parameter Conversion parameter (nullptr means default)
   * \return Conversion option for the specified types (result's type is tConversionOptionType::NONE if no option for specified types can be provided)
   */
  virtual tConversionOption GetConversionOption(const tType& source_type, const tType& destination_type, const tGenericObject* parameter) const;

  /*!
   * \return Local handle of operation
   */
  uint16_t GetHandle() const
  {
    return handle;
  }

  /*!
   * \return Handle of conversion operation that this one is not usually combined with (0xFFFF if there is no such operation)
   */
  uint16_t GetNotUsuallyCombinedWithHandle() const
  {
    return not_usually_combined_with_handle;
  }

  /*!
   * \return Registered type conversion operations.
   */
  static const tRegisteredOperations& GetRegisteredOperations()
  {
    return RegisteredOperations();
  }

  /*!
   * \return Name of conversion operation
   */
  const char* Name() const
  {
    return name.Get();
  }

  /*!
   * \return Parameter of this conversion operation (possibly empty if operation has no parameter)
   */
  const tParameterDefinition& Parameter() const
  {
    return parameter;
  }

  /*!
   * \return Supported source types of cast operation
   */
  const tSupportedTypes& SupportedDestinationTypes() const
  {
    return supported_destination_types;
  }

  /*!
   * \return Supported source types of cast operation
   */
  const tSupportedTypes& SupportedSourceTypes() const
  {
    return supported_source_types;
  }

//----------------------------------------------------------------------
// Protected types
//----------------------------------------------------------------------
protected:

  /*!
   * \param name Name of conversion operation (must be unique for every supported combination of source and destination types)
   * \param supported_source_types Supported source types
   * \param supported_destination_types Supported destination types
   * \param single_conversion_option If operation provides only a single conversion option - points to this option (otherwise override GetConversionOptions())
   * \param parameter Any parameter of conversion operation (optional)
   * \param not_usually_combined_with Conversion operation that this one is not usually combined with (a deprecated combination if you want)
   */
  tRegisteredConversionOperation(util::tManagedConstCharPointer name, const tSupportedTypes& supported_source_types, const tSupportedTypes& supported_destination_types,
                                 const tConversionOption* single_conversion_option = nullptr, const tParameterDefinition& parameter = tParameterDefinition(),
                                 const tRegisteredConversionOperation* not_usually_combined_with = nullptr);

  /*!
   * Adds this operation to list of operations that are automatically deleted on shutdown.
   * This should only be done with operations allocated via new() and not deleted elsewhere.
   */
  void AutoDelete();

//----------------------------------------------------------------------
// Private fields and methods
//----------------------------------------------------------------------
private:

  friend class tStaticCastOperation;

  /*! Name of conversion operation (must be unique for every supported combination of source and destination types) */
  util::tManagedConstCharPointer name;

  /*! Supported source and destination types */
  const tSupportedTypes supported_source_types, supported_destination_types;

  /*! Parameter for this conversion operation (may be empty) */
  const tParameterDefinition parameter;

  /*! If operation provides only a single conversion option - points to this option */
  const tConversionOption* single_conversion_option;

  /*! Local handle of operation */
  uint16_t handle;

  /*! Handle of conversion operation that this one is not usually combined with (0xFFFF if there is no such operation) */
  uint16_t not_usually_combined_with_handle;


  /*! constructor for tStaticCastOperation */
  tRegisteredConversionOperation();

  /*!
   * \return Registered type conversion operations.
   */
  static tRegisteredOperations& RegisteredOperations();
};


serialization::tOutputStream& operator << (serialization::tOutputStream& stream, const tRegisteredConversionOperation& operation);

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}


#endif
