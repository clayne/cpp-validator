/**
@copyright Evgeny Sidorov 2020

Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)

*/

/****************************************************************************/

/** \file validator/dispatcher.hpp
*
*  Defines validation dispatcher
*
*/

/****************************************************************************/

#ifndef DRACOSHA_VALIDATOR_DISPATCHER_IMPL_HPP
#define DRACOSHA_VALIDATOR_DISPATCHER_IMPL_HPP

#include <type_traits>

#include <dracosha/validator/config.hpp>
#include <dracosha/validator/property.hpp>
#include <dracosha/validator/extract.hpp>
#include <dracosha/validator/check_member.hpp>
#include <dracosha/validator/get_member.hpp>
#include <dracosha/validator/operators/exists.hpp>

DRACOSHA_VALIDATOR_NAMESPACE_BEGIN

//-------------------------------------------------------------

struct adapter_tag;
struct member_tag;
struct master_reference_tag;

//-------------------------------------------------------------
namespace detail
{

template <typename T1, typename =hana::when<true>>
struct dispatcher_impl_t
{
};

/**
 * @brief Dispatcher that first wraps object into default adapter and then forward it to actual dispatcher
 */
template <typename T1>
struct dispatcher_impl_t<T1,hana::when<!hana::is_a<adapter_tag,T1>>>
{
    /**
     *  @brief Perform validation of object at one level without member nesting
     *  @param a Object to validate
     *  @param args Validation arguments
     *  @return Validation status
     */
    template <typename ...Args>
    constexpr bool operator() (T1&& obj, Args&&... args) const;

    /**
     *  @brief Perform validation of object's property at one level without member nesting
     *  @param prop Property to validate
     *  @param a Object to validate
     *  @param args Validation arguments
     *  @return Validation status
     */
    template <typename ...Args>
    constexpr static bool invoke(T1&& obj, Args&&... args);

    //! \todo validate_and
    //! \todo validate_or
};

/**
 * @brief Dispatcher that dispatches validating requests to corresponding methods of supplied adapter
 */
template <typename T1>
struct dispatcher_impl_t<T1,hana::when<hana::is_a<adapter_tag,T1>>>
{
    /**
     *  @brief Perform validation of object at one level without member nesting
     *  @param a Adapter with object to validate
     *  @param op Operator for validation
     *  @param b Reference argument for validation
     *  @return Validation status
     */
    template <typename T2, typename OpT>
    constexpr bool operator() (T1&& a, OpT&& op, T2&& b) const
    {
        return a.validate_operator(std::forward<OpT>(op),std::forward<T2>(b));
    }

    /**
     *  @brief Perform validation of object's property at one level without member nesting
     *  @param prop Property to validate
     *  @param a Adapter with object to validate
     *  @param op Operator for validation
     *  @param b Reference argument for validation
     *  @return Validation status
     */
    template <typename T2, typename OpT, typename PropT>
    constexpr bool operator() (PropT&& prop, T1&& a, OpT&& op, T2&& b) const
    {
        return invoke(std::forward<T1>(a),std::forward<PropT>(prop),std::forward<OpT>(op),std::forward<T2>(b));
    }

    /**
     *  @brief Perform validation of object's property at one level without member nesting
     *  @param prop Property to validate
     *  @param a Adapter with object to validate
     *  @param op Operator for validation
     *  @param b Reference argument for validation
     *  @return Validation status
     */
    template <typename T2, typename OpT, typename PropT>
    constexpr static bool invoke(T1&& a, PropT&& prop, OpT&& op, T2&& b,
                                     std::enable_if_t<
                                       !hana::is_a<member_tag,T2>,
                                       void*
                                     > =nullptr
                                 )
    {
        return a.validate_property(std::forward<PropT>(prop),std::forward<OpT>(op),std::forward<T2>(b));
    }

    /**
     *  @brief Validate existance of a member
     *  @param a Adapter with object to validate
     *  @param member Member descriptor
     *  @param b Boolean flag, when true check if member exists, when false check if member does not exist
     *  @return Validation status
     */
    template <typename T2, typename OpT, typename PropT, typename MemberT>
    constexpr static bool invoke(T1&& a, MemberT&& member, PropT&&, OpT&&, T2&& b,
                                 std::enable_if_t<std::is_same<exists_t,typename std::decay<OpT>::type>::value,
                                   void*
                                 > =nullptr
                                )
    {
        return a.validate_exists(std::forward<MemberT>(member),std::forward<T2>(b));
    }

    /**
     *  @brief Normal validation of a member
     *  @param a Adapter with object to validate
     *  @param member Member descriptor
     *  @param prop Property to validate
     *  @param op Operator for validation
     *  @param b Reference argument for validation
     *  @return Validation status
     */
    template <typename T2, typename OpT, typename PropT, typename MemberT>
    constexpr static bool invoke(T1&& a, MemberT&& member, PropT&& prop, OpT&& op, T2&& b,
                                 std::enable_if_t<
                                   (!hana::is_a<member_tag,T2> &&
                                    !hana::is_a<master_reference_tag,T2> &&
                                    !std::is_same<exists_t,typename std::decay<OpT>::type>::value),
                                   void*
                                 > =nullptr
                                )
    {
        return a.validate(std::forward<MemberT>(member),std::forward<PropT>(prop),std::forward<OpT>(op),std::forward<T2>(b));
    }

    /**
     *  @brief Validate using other member of the same object as a reference argument for validation
     *  @param a Object to validate
     *  @param member Member descriptor
     *  @param prop Property to validate
     *  @param op Operator for validation
     *  @param b Descriptor of reference member of the same object
     *  @return Validation status
     */
    template <typename T2, typename OpT, typename PropT, typename MemberT>
    constexpr static bool invoke(T1&& a, MemberT&& member, PropT&& prop, OpT&& op, T2&& b,
                                 std::enable_if_t<
                                   hana::is_a<member_tag,T2>,
                                   void*
                                 > =nullptr
                                )
    {
        return a.validate_with_other_member(std::forward<MemberT>(member),std::forward<PropT>(prop),std::forward<OpT>(op),std::forward<T2>(b));
    }

    /**
     *  @brief Validate using the same member of a reference object
     *  @param a Object to validate
     *  @param member Member descriptor
     *  @param prop Property to validate
     *  @param op Operator for validation
     *  @param b Reference object whose member to use as argument passed to validation operator
     *  @return Validation status
     */
    template <typename T2, typename OpT, typename PropT, typename MemberT>
    constexpr static bool invoke(T1&& a, MemberT&& member, PropT&& prop, OpT&& op, T2&& b,
                                 std::enable_if_t<
                                   hana::is_a<master_reference_tag,T2>,
                                   void*
                                 > =nullptr
                                )
    {
        return a.validate_with_master_reference(std::forward<MemberT>(member),std::forward<PropT>(prop),std::forward<OpT>(op),std::forward<T2>(b));
    }

    //! \todo validate_and
    //! \todo validate_or
};

template <typename T1>
constexpr dispatcher_impl_t<T1> dispatcher_impl{};

template <typename T1>
template <typename ...Args>
constexpr bool dispatcher_impl_t<T1,hana::when<!hana::is_a<adapter_tag,T1>>>::operator() (T1&& obj, Args&&... args) const
{
    return dispatcher_impl<decltype(adapter(std::forward<T1>(obj)))>(adapter(std::forward<T1>(obj)),std::forward<Args>(args)...);
}

template <typename T1>
template <typename ...Args>
constexpr bool dispatcher_impl_t<T1,hana::when<!hana::is_a<adapter_tag,T1>>>::invoke(T1&& obj, Args&&... args)
{
    return dispatcher_impl<decltype(adapter(std::forward<T1>(obj)))>.invoke(adapter(std::forward<T1>(obj)),std::forward<Args>(args)...);
}

}

//-------------------------------------------------------------

DRACOSHA_VALIDATOR_NAMESPACE_END

#endif // DRACOSHA_VALIDATOR_DISPATCHER_IMPL_HPP
