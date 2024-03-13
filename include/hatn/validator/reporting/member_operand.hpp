/**
@copyright Evgeny Sidorov 2020

Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)

*/

/****************************************************************************/

/** @file validator/reporting/member_operand.hpp
*
* Defines wrapper of other member used as operand.
*
*/

/****************************************************************************/

#ifndef HATN_VALIDATOR_MEMBER_OPERAND_HPP
#define HATN_VALIDATOR_MEMBER_OPERAND_HPP

#include <hatn/validator/config.hpp>
#include <hatn/validator/utils/object_wrapper.hpp>

HATN_VALIDATOR_NAMESPACE_BEGIN

//-------------------------------------------------------------
struct member_operand_tag;

/**
 * @brief Wrapper of member used as operand.
 */
template <typename T>
struct member_operand : public object_wrapper<T>
{
    using hana_tag=member_operand_tag;
    using object_wrapper<T>::object_wrapper;
};

/**
  @brief Make a member operand wrapper around member.
  @param b ID to wrap into member operand.
  @return Member operand.
  */
template <typename T>
auto make_member_operand(T&& b)
{
    return member_operand<T>{std::forward<T>(b)};
}

//-------------------------------------------------------------

HATN_VALIDATOR_NAMESPACE_END

#endif // HATN_VALIDATOR_MEMBER_OPERAND_HPP
