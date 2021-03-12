/**
@copyright Evgeny Sidorov 2020

Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)

*/

/****************************************************************************/

/** @file validator/utils/adjust_storable_type.hpp
*
*  Defines helpers for adjusting types used as storable elements in tuples.
*
*/

/****************************************************************************/

#ifndef DRACOSHA_VALIDATOR_ADJUST_STORABLE_TYPE_HPP
#define DRACOSHA_VALIDATOR_ADJUST_STORABLE_TYPE_HPP

#include <string>

#include <dracosha/validator/config.hpp>
#include <dracosha/validator/utils/object_wrapper.hpp>

DRACOSHA_VALIDATOR_NAMESPACE_BEGIN

struct element_aggregation_tag;
struct wrap_iterator_tag;
struct operator_tag;
struct property_tag;
struct wrap_index_tag;

//-------------------------------------------------------------

template <typename T, typename=hana::when<true>>
struct adjust_storable_type
{
};

/**
 * @brief Convert const char* type to std::string.
 *
 */
template <typename T>
struct adjust_storable_type<T,
                        hana::when<std::is_constructible<const char*,T>::value>
                    >
{
    using type=std::string;
};

/**
 * @brief Adjust special types that must be stored as copies.
 */
template <typename T>
struct adjust_storable_type<T,
                        hana::when<
                            hana::is_a<property_tag,T>
                            ||
                            hana::is_a<element_aggregation_tag,T>
                            ||
                            hana::is_a<object_wrapper_tag,T>
                            ||
                            hana::is_a<wrap_iterator_tag,T>
                            ||
                            hana::is_a<wrap_index_tag,T>
                            ||
                            hana::is_a<operator_tag,T>
                        >
                    >
{
    using type=std::decay_t<T>;
};

/**
 * @brief Adjust all the rest types that must be stored within object_wrapper.
 */
template <typename T>
struct adjust_storable_type<T,
                        hana::when<
                            !std::is_constructible<const char*,T>::value
                            &&
                            !hana::is_a<property_tag,T>
                            &&
                            !hana::is_a<element_aggregation_tag,T>
                            &&
                            !hana::is_a<object_wrapper_tag,T>
                            &&
                            !hana::is_a<wrap_iterator_tag,T>
                            &&
                            !hana::is_a<wrap_index_tag,T>
                            &&
                            !hana::is_a<operator_tag,T>
                        >
                    >
{
    using type=object_wrapper<T>;
};

/**
 * @brief Make storable type from a variable.
 */
struct adjust_storable_t
{
    template <typename T>
    auto operator () (T&& v) const
    {
        return typename adjust_storable_type<T>::type{std::forward<T>(v)};
    }
};
constexpr adjust_storable_t adjust_storable{};

DRACOSHA_VALIDATOR_NAMESPACE_END

#endif // DRACOSHA_VALIDATOR_ADJUST_STORABLE_TYPE_HPP
