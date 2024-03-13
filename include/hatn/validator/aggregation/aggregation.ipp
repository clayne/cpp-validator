/**
@copyright Evgeny Sidorov 2020

Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)

*/

/****************************************************************************/

/** @file validator/aggregation/aggregation.ipp
*
*  Defines aggregation used for logical operators.
*
*/

/****************************************************************************/

#ifndef HATN_VALIDATOR_AGGREGATION_IPP
#define HATN_VALIDATOR_AGGREGATION_IPP

#include <hatn/validator/config.hpp>
#include <hatn/validator/aggregation/aggregation.hpp>
#include <hatn/validator/member.hpp>
#include <hatn/validator/make_member.hpp>
#include <hatn/validator/extract.hpp>
#include <hatn/validator/get_member.hpp>
#include <hatn/validator/reporting/reporting_adapter_impl.hpp>

HATN_VALIDATOR_NAMESPACE_BEGIN

//-------------------------------------------------------------

/**
 * @brief Helper for construction of element aggregation reports.
 */
template <typename AdapterT, typename=hana::when<true>>
struct aggregate_report
{
    template <typename AdapterT1, typename AggregationT, typename MemberT>
    static void open(AdapterT1&&, AggregationT&&, MemberT&&)
    {}

    template <typename AdapterT1>
    static void close(AdapterT1&&, status)
    {}
};

template <typename AdapterT>
struct aggregate_report<AdapterT,
        hana::when<
            std::is_base_of<reporting_adapter_tag,typename std::decay_t<AdapterT>::type>::value
        >>
{
    template <typename AdapterT1, typename AggregationT, typename PathT>
    static void open(AdapterT1&& adapter, AggregationT&& str, PathT&& path)
    {
        auto& reporter=traits_of(adapter).reporter();
        hana::eval_if(
            hana::is_empty(path),
            [&](auto&&)
            {
                reporter.aggregate_open(str);
            },
            [&](auto&& _)
            {
                reporter.aggregate_open(str,make_member(_(path)));
            }
        );
    }

    template <typename AdapterT1>
    static void close(AdapterT1&& adapter, status ret)
    {
        traits_of(adapter).reporter().aggregate_close(ret);
    }
};

//-------------------------------------------------------------

HATN_VALIDATOR_NAMESPACE_END

#endif // HATN_VALIDATOR_AGGREGATION_IPP
