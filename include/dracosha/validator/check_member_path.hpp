/**
@copyright Evgeny Sidorov 2020

Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)

*/

/****************************************************************************/

/** \file validator/check_member_path.hpp
*
*  Defines helpers for checking existance of member path in type of an object
*
*/

/****************************************************************************/

#ifndef DRACOSHA_VALIDATOR_CHECK_MEMBER_PATH_HPP
#define DRACOSHA_VALIDATOR_CHECK_MEMBER_PATH_HPP

#include <dracosha/validator/config.hpp>
#include <dracosha/validator/check_member.hpp>

DRACOSHA_VALIDATOR_NAMESPACE_BEGIN

//-------------------------------------------------------------

/**
  @brief Check if path of types in member descriptor can exist in the object
  @param obj Object under validation
  @param path Member path as a tuple
  @return Validation status

  This operation is expected to be static and must be performed at compile time.
*/
BOOST_HANA_CONSTEXPR_LAMBDA auto check_member_path =[](auto&& obj,auto&& path)
{
    auto path_c=hana::transform(path,hana::make_type);
    auto obj_c=hana::type_c<decltype(obj)>;

    return !hana::is_nothing(hana::monadic_fold_left<hana::optional_tag>(path_c,obj_c,hana::sfinae(check_member)));
};

/**
  * @brief Check if members have paths of the same types.
  * @param member1 First member
  * @param Second member
  * @return Result of check operation
  */
BOOST_HANA_CONSTEXPR_LAMBDA auto check_member_path_types =[](const auto& member1,const auto& member2)
{
    auto path1_c=hana::transform(member1.path,hana::make_type);
    auto path2_c=hana::transform(member2.path,hana::make_type);

    return hana::equal(path1_c,path2_c);
};

//-------------------------------------------------------------

DRACOSHA_VALIDATOR_NAMESPACE_END

#endif // DRACOSHA_VALIDATOR_CHECK_MEMBER_PATH_HPP
