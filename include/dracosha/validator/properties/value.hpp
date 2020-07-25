/**
@copyright Evgeny Sidorov 2020

Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)

*/

/****************************************************************************/

/** \file validator/properties/value.hpp
*
*  Defines special property "value" that serves as proxy wrapper to the object itself.
*
*/

/****************************************************************************/

#ifndef DRACOSHA_VALIDATOR_VALUE_HPP
#define DRACOSHA_VALIDATOR_VALUE_HPP

#include <dracosha/validator/config.hpp>
#include <dracosha/validator/property.hpp>
#include <dracosha/validator/reporting/flag_presets.hpp>
#include <dracosha/validator/property_validator.hpp>

DRACOSHA_VALIDATOR_NAMESPACE_BEGIN

/**
 * @brief Proxy wrapper that is used to get the object itself via property interface
 */
struct type_p_value
{
    using hana_tag=property_tag;
    template <typename T>
    constexpr static T get(T&& v)
    {
        return hana::id(std::forward<T>(v));
    }
    template <typename T>
    constexpr static bool has()
    {
        return true;
    }
    template <typename ... Args> \
    constexpr auto operator () (Args&&... args) const;

    constexpr static const char* name()
    {
        return "value";
    }

    constexpr static const char* flag_str(bool)
    {
        return nullptr;
    }

    constexpr static bool has_flag_str()
    {
        return false;
    }
};
constexpr type_p_value value{};
template <typename ... Args>
constexpr auto type_p_value::operator () (Args&&... args) const
{
    return make_property_validator(value,std::forward<Args>(args)...);
}

//-------------------------------------------------------------

DRACOSHA_VALIDATOR_NAMESPACE_END

#endif // DRACOSHA_VALIDATOR_VALUE_HPP
