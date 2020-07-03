/**
@copyright Evgeny Sidorov 2020

Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)

*/

/****************************************************************************/

/** \file validator/reporting/order_and_presentation.hpp
*
*  Defines helper that adjusts order and presentation of strings.
*
*/

/****************************************************************************/

#ifndef DRACOSHA_VALIDATOR_ORDER_AND_PRESENTATION_HPP
#define DRACOSHA_VALIDATOR_ORDER_AND_PRESENTATION_HPP

#include <dracosha/validator/config.hpp>
#include <dracosha/validator/detail/reorder_and_present.hpp>

DRACOSHA_VALIDATOR_NAMESPACE_BEGIN

//-------------------------------------------------------------

struct order_and_presentation_tag;

/**
 * @brief Helper to adjust strings ordering and presentation
 */
struct order_and_presentation_t
{
    using hana_tag=order_and_presentation_tag;

    template <typename ...Args>
    constexpr auto operator () (Args&&... args) const -> decltype(auto)
    {
        return detail::reorder_and_present(std::forward<Args>(args)...);
    }
};
constexpr order_and_presentation_t default_order_and_presentation{};

//-------------------------------------------------------------

DRACOSHA_VALIDATOR_NAMESPACE_END

#endif // DRACOSHA_VALIDATOR_ORDER_AND_PRESENTATION_HPP