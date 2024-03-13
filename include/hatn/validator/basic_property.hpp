/**
@copyright Evgeny Sidorov 2020

Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)

*/

/****************************************************************************/

/** @file validator/basic_property.hpp
*
* Defines basic_property and property_tag.
*
*/

/****************************************************************************/

#ifndef HATN_VALIDATOR_BASIC_PROPERTY_HPP
#define HATN_VALIDATOR_BASIC_PROPERTY_HPP

#include <hatn/validator/config.hpp>
#include <hatn/validator/utils/adjust_storable_ignore.hpp>

HATN_VALIDATOR_NAMESPACE_BEGIN

struct property_tag;

//-------------------------------------------------------------

/**
 * @brief base struct for properties.
 */
struct basic_property : public adjust_storable_ignore
{
    using hana_tag=property_tag;
};


//-------------------------------------------------------------

HATN_VALIDATOR_NAMESPACE_END

#endif // HATN_VALIDATOR_BASIC_PROPERTY_HPP
