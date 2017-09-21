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
/*!\file    rrlib/rtti_conversion/tStaticCastOperation.h
 *
 * \author  Max Reichardt
 *
 * \date    2016-07-17
 *
 * \brief   Contains tStaticCastOperation
 *
 * \b tStaticCastOperation
 *
 * Due to their special/universal nature, casts are handled separately from other conversion operations.
 * For instance, they may be implicit and new casts can be registered at any time.
 *
 */
//----------------------------------------------------------------------
#ifndef __rrlib__rtti_conversion__tStaticCastOperation_h__
#define __rrlib__rtti_conversion__tStaticCastOperation_h__

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------
#include "rrlib/thread/tLock.h"

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "rrlib/rtti_conversion/tRegisteredConversionOperation.h"
#include "rrlib/rtti_conversion/type_traits.h"
#include "rrlib/rtti_conversion/tCompiledConversionOperation.h"

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
//! Static cast operations
/*!
 * Due to their special/universal nature, casts are handled separately from other conversion operations.
 * For instance, they may be implicit and new casts can be registered at any time.
 */
class tStaticCastOperation : public tRegisteredConversionOperation
{
  typedef tConversionOptionStaticCast tStaticCast;

  /*! Standard static cast operation */
  template <typename TSource, typename TDestination>
  struct tInstanceStandard
  {
    static void ConvertFinal(const tTypedConstPointer& source_object, const tTypedPointer& destination_object, const tCurrentConversionOperation& operation)
    {
      (*destination_object.Get<TDestination>()) = static_cast<TDestination>(*source_object.Get<TSource>());
    }

    static void ConvertFirst(const tTypedConstPointer& source_object, const tTypedPointer& destination_object, const tCurrentConversionOperation& operation)
    {
      TDestination intermediate = static_cast<TDestination>(*source_object.Get<TSource>());
      operation.Continue(tTypedConstPointer(&intermediate), destination_object);
    }

    static constexpr tStaticCast value = { { tConversionOption(tDataType<TSource>(), tDataType<TDestination>(), StaticCastReferencesSourceWithVariableOffset<TSource, TDestination>::value, &ConvertFirst, &ConvertFinal) }, IsImplicitlyConvertible<TDestination, TSource>::value };
    enum { cREGISTER_OPERATION = 1 };
  };

  /*! If source and destination types have the same underlying type, this is used */
  template <typename TSource, typename TDestination>
  struct tInstanceDeepCopy
  {
    static constexpr tStaticCast value = { { tConversionOption(tDataType<TSource>(), tDataType<TDestination>(), 0) }, IsImplicitlyConvertible<TDestination, TSource>::value };
    enum { cREGISTER_OPERATION = 0 };
  };

  struct tInstanceNone
  {
    static const tStaticCast value;
    enum { cREGISTER_OPERATION = 0 };
  };


  /*! Any static cast operation */
  template <typename TSource, typename TDestination>
  struct tInstance : std::conditional<std::is_same<TSource, void>::value, tInstanceNone, typename std::conditional<std::is_same<typename UnderlyingType<TSource>::type, typename UnderlyingType<TDestination>::type>::value, tInstanceDeepCopy<TSource, TDestination>, tInstanceStandard<TSource, TDestination>>::type>::type
  {
  };

  /*! Static cast operation with fixed offset reference to source */
  template <typename TSource, typename TDestination>
  struct tInstanceReferenceStaticOffset
  {
    static_assert(sizeof(TSource) >= sizeof(TDestination), "Static up-casts by-reference are not supported (potentially unsafe without further checks)");

    static const tStaticCast value;

    static size_t ComputePointerSourceOffset()
    {
      const TSource source_object = TSource();
      const TDestination& returned_object = static_cast<const TDestination&>(source_object);
      size_t difference = reinterpret_cast<const char*>(&returned_object) - reinterpret_cast<const char*>(&source_object);
      assert(difference + sizeof(TDestination) <= sizeof(TSource));
      return difference;
    }
  };

  /*! Static cast operation with variable offset reference to source */
  template <typename TSource, typename TDestination>
  struct tInstanceReferenceVariableOffset
  {
    static tTypedConstPointer GetDestinationReference(const tTypedConstPointer& source_object, const tCurrentConversionOperation& operation)
    {
      return tTypedConstPointer(&static_cast<const TDestination&>(*source_object.Get<TSource>()));
    }

    static void ConvertFirst(const tTypedConstPointer& source_object, const tTypedPointer& destination_object, const tCurrentConversionOperation& operation)
    {
      const TDestination& intermediate = static_cast<const TDestination&>(*source_object.Get<TSource>());
      operation.Continue(tTypedConstPointer(&intermediate), destination_object);
    }

    static constexpr tStaticCast value = { { tConversionOption(tDataType<TSource>(), tDataType<TDestination>(), &ConvertFirst, &GetDestinationReference) }, IsImplicitlyConvertible<TDestination&, TSource>::value };
  };

  /* Specialization for reference type destination */
  template <typename TSource, typename TDestination>
  struct tInstance<TSource, TDestination&> : std::conditional<StaticCastReferencesSourceWithVariableOffset<TSource, TDestination&>::value, tInstanceReferenceVariableOffset<TSource, TDestination>, tInstanceReferenceStaticOffset<TSource, TDestination>>::type
  {
    enum { cREGISTER_OPERATION = 1 };
  };

  /*! Standard dedicated static cast operation for vectors*/
  template <typename TSource, typename TDestination>
  struct tInstanceVectorStandard
  {
    static void ConvertFinal(const tTypedConstPointer& source_object, const tTypedPointer& destination_object, const tCurrentConversionOperation& operation)
    {
      const std::vector<TSource>& source = *source_object.Get<std::vector<TSource>>();
      std::vector<TDestination>& destination = *destination_object.Get<std::vector<TDestination>>();
      destination.resize(source.size());
      auto it_dest = destination.begin();
      for (auto it = source.begin(); it != source.end(); ++it, ++it_dest)
      {
        *it_dest = static_cast<TDestination>(*it);
      }
    }

    static void ConvertFirst(const tTypedConstPointer& source_object, const tTypedPointer& destination_object, const tCurrentConversionOperation& operation)
    {
      const std::vector<TSource>& source = *source_object.Get<std::vector<TSource>>();
      std::vector<TDestination> intermediate;
      intermediate.resize(source.size());
      auto it_dest = intermediate.begin();
      for (auto it = source.begin(); it != source.end(); ++it, ++it_dest)
      {
        *it_dest = static_cast<TDestination>(*it);
      }
      operation.Continue(tTypedConstPointer(&intermediate), destination_object);
    }

    static constexpr tStaticCast value = { { tConversionOption(tDataType<std::vector<TSource>>(), tDataType<std::vector<TDestination>>(), StaticCastReferencesSourceWithVariableOffset<TSource, TDestination>::value, &ConvertFirst, &ConvertFinal) }, false };
    enum { cREGISTER_OPERATION = 1 };
  };

  /*! If source and destination types have the same underlying type, this is used */
  template <typename TSource, typename TDestination>
  struct tInstanceVectorDeepCopy
  {
    static constexpr tStaticCast value = { { tConversionOption(tDataType<std::vector<TSource>>(), tDataType<std::vector<TDestination>>(), 0) }, false };
    enum { cREGISTER_OPERATION = 0 };
  };

  /*! Any static cast operation */
  template <typename TSource, typename TDestination>
  struct tInstanceVector : std::conditional<std::is_same<TSource, void>::value, tInstanceNone, typename std::conditional<std::is_same<typename UnderlyingType<TSource>::type, typename UnderlyingType<TDestination>::type>::value, tInstanceVectorDeepCopy<TSource, TDestination>, tInstanceVectorStandard<TSource, TDestination>>::type>::type
  {
  };

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  /*!
   * Get single implicit conversion option from source to destination type (if any).
   *
   * \param source_type Source type of implicit conversion operation
   * \param destination_type Destination type of implicit conversion operation
   * \return Implicit conversion option. If there is no implicit conversion option, the type is NONE. If source_type == destination_type, returns CONST_OFFSET_REFERENCE_TO_SOURCE_OBJECT option (with offset 0).
   */
  static tConversionOption GetImplicitConversionOption(const rrlib::rtti::tType& source_type, const rrlib::rtti::tType& destination_type);

  /*!
   * Get implicit conversion options from source to destination type (if any).
   *
   * \param source_type Source type of implicit conversion operations
   * \param destination_type Destination type of implicit conversion operations
   * \return Implicit conversion options. If there is no implicit conversion option, the type is NONE. If one conversion option is sufficient, the second's type is NONE. If source_type == destination_type, returns CONST_OFFSET_REFERENCE_TO_SOURCE_OBJECT option (with offset 0).
   */
  static std::pair<tConversionOption, tConversionOption> GetImplicitConversionOptions(const rrlib::rtti::tType& source_type, const rrlib::rtti::tType& destination_type);

  /*!
   * \return Single instance of static cast operation
   */
  static const tStaticCastOperation& GetInstance()
  {
    return instance;
  }

  /*!
   * Is source type implicitly convertible to destination type (possibly using a sequence of two implicit cast operations)?
   *
   * \param source_type Source type of implicit conversion
   * \param destination_type Destination type of implicit conversion
   * \return Whether implicit conversion is possible (also true if source_type == destination_type)
   */
  static bool IsImplicitlyConvertibleTo(const rrlib::rtti::tType& source_type, const rrlib::rtti::tType& destination_type)
  {
    return GetImplicitConversionOptions(source_type, destination_type).first.type != tConversionOptionType::NONE;
  }

  /*!
   * Registers static cast operation.
   * (Not necessary for casts specified via WrapsType type trait)
   *
   * \tparam TSource Source type of conversion operation
   * \tparam TDestination Destination type of conversion operation. Append '&' to cast source reference to destination reference.
   * \tparam Tregister_reverse_operation Also create/register reverse operation?
   * \tparam Tregister_dedicated_vector_cast Create dedicated cast-operation for std::vector<TSource> to std::vector<TDestination>, too? (otherwise a less efficient generic one will be used; a dedicated one makes sense for vectors with typically many elements such as std::vector<float>)
   * \return Reference to static_cast operation for convenience (to make additional Register calls)
   */
  template <typename TSource, typename TDestination, bool Tregister_reverse_operation = false, bool Tregister_dedicated_vector_cast = false>
  static tStaticCastOperation& Register()
  {
    static_assert(!std::is_reference<TSource>::value, "Source type must not be reference");
    static_assert((!std::is_reference<TDestination>::value) || (!Tregister_reverse_operation), "Cannot register reverse operation for reference type destination");
    static_assert(((!IsStdVector<TSource>::value) && (!IsStdVector<TDestination>::value)) || (!Tregister_dedicated_vector_cast), "Cannot register dedicated vector cast for std::vector types");

    typedef tInstance<TSource, TDestination> tOperation;
    typedef tInstance<typename std::conditional<Tregister_reverse_operation, TDestination, void>::type, typename std::conditional<Tregister_reverse_operation, TSource, void>::type> tOperationReverse;
    typedef tInstanceVector < typename std::conditional<Tregister_dedicated_vector_cast, TSource, void>::type,
            typename std::conditional<Tregister_dedicated_vector_cast, TDestination, void>::type > tVectorOperation;
    typedef tInstanceVector < typename std::conditional < Tregister_dedicated_vector_cast && Tregister_reverse_operation, TDestination, void >::type,
            typename std::conditional < Tregister_dedicated_vector_cast && Tregister_reverse_operation, TSource, void >::type > tVectorOperationReverse;

    tRegisteredOperations& registered_ops = tRegisteredConversionOperation::RegisteredOperations();
    if (tOperation::cREGISTER_OPERATION)
    {
      registered_ops.static_casts.Add(&tOperation::value);
    }
    if (tOperationReverse::cREGISTER_OPERATION)
    {
      registered_ops.static_casts.Add(&tOperationReverse::value);
    }
    if (tVectorOperation::cREGISTER_OPERATION)
    {
      registered_ops.static_casts.Add(&tVectorOperation::value);
    }
    if (tVectorOperationReverse::cREGISTER_OPERATION)
    {
      registered_ops.static_casts.Add(&tVectorOperationReverse::value);
    }

    return instance;
  }

//----------------------------------------------------------------------
// Private fields and methods
//----------------------------------------------------------------------
private:

  /*! Single instance of static cast operation */
  static tStaticCastOperation instance;

  tStaticCastOperation();

  /*!
   * Internal version of GetImplicitConversionOption (must only be called with lock in registered_operations)
   */
  static tConversionOption GetImplicitConversionOption(const rrlib::rtti::tType& source_type, const rrlib::rtti::tType& destination_type, const tRegisteredConversionOperation::tRegisteredOperations& registered_operations);

  virtual tConversionOption GetConversionOption(const tType& source_type, const tType& destination_type, const tGenericObject* parameter) const override;
};

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}

#include "rrlib/rtti_conversion/tStaticCastOperation.hpp"

#endif
