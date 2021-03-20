/**
@copyright Evgeny Sidorov 2020

Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)

*/

/****************************************************************************/

/** @file validator/aggregation/element_aggregation.hpp
*
*  Defines helpers used for element aggregations.
*
*/

/****************************************************************************/

#ifndef DRACOSHA_VALIDATOR_ELEMENT_AGGREGATION_HPP
#define DRACOSHA_VALIDATOR_ELEMENT_AGGREGATION_HPP

#include <dracosha/validator/config.hpp>
#include <dracosha/validator/utils/adjust_storable_ignore.hpp>
#include <dracosha/validator/aggregation/aggregation.hpp>
#include <dracosha/validator/filter_member.hpp>
#include <dracosha/validator/variadic_arg_tag.hpp>

DRACOSHA_VALIDATOR_NAMESPACE_BEGIN

//-------------------------------------------------------------

struct element_aggregation_tag
{
    template <typename T>
    constexpr bool operator == (T) const noexcept
    {
        return true;
    }
    template <typename T>
    constexpr bool operator != (T) const noexcept
    {
        return false;
    }

    template <typename T>
    constexpr friend bool operator == (T, const element_aggregation_tag&) noexcept
    {
        return true;
    }
    template <typename T>
    constexpr friend bool operator != (T, const element_aggregation_tag&) noexcept
    {
        return true;
    }
};

//-------------------------------------------------------------

struct element_aggregation : public adjust_storable_ignore
{
    using hana_tag=element_aggregation_tag;

    template <typename PredicateT, typename EmptyFnT, typename AggregationT,
              typename UsedPathSizeT, typename PathT, typename AdapterT, typename HandlerT>
    static status invoke(PredicateT&& pred, EmptyFnT&& empt, AggregationT&& aggr,
                         UsedPathSizeT&& used_path_size, PathT&& path, AdapterT&& adapter, HandlerT&& handler);

    template <typename PredicateT, typename EmptyFnT, typename AggregationT,
              typename UsedPathSizeT, typename PathT, typename AdapterT, typename HandlerT>
    static status invoke_variadic(PredicateT&& pred, EmptyFnT&& empt, AggregationT&& aggr,
                         UsedPathSizeT&& used_path_size, PathT&& path, AdapterT&& adapter, HandlerT&& handler);
};

//-------------------------------------------------------------

struct tree_tag;

struct is_element_aggregation_t
{
    template <typename PrevResultT, typename T>
    constexpr auto operator () (PrevResultT prev, T) const
    {
        using type=typename T::type;
        return hana::if_(
            prev,
            prev,
            hana::bool_
            <
                hana::is_a<element_aggregation_tag,type>
                ||
                hana::is_a<tree_tag,type>
                ||
                std::is_base_of<variadic_arg_aggregation_tag,std::decay_t<type>>::value
            >{}
        );
    }
};
constexpr is_element_aggregation_t is_element_aggregation{};

//-------------------------------------------------------------

struct element_aggregation_modifier_tag;

template <typename ModifierT>
struct element_aggregation_with_modifier : public element_aggregation
{
    constexpr static const ModifierT& modifier=ModifierT::instance();
};

//-------------------------------------------------------------

template <typename DerivedT>
struct element_aggregation_modifier
{
    using hana_tag=element_aggregation_modifier_tag;
};

struct keys_t : public element_aggregation_modifier<keys_t>
{
    constexpr static const keys_t& instance();
};
constexpr keys_t keys{};
constexpr const keys_t& keys_t::instance()
{
    return keys;
}

struct values_t : public element_aggregation_modifier<values_t>
{
    constexpr static const values_t& instance();
};
constexpr values_t values{};
constexpr const values_t& values_t::instance()
{
    return values;
}

struct iterators_t : public element_aggregation_modifier<iterators_t>
{
    constexpr static const iterators_t& instance();
};
constexpr iterators_t iterators{};
constexpr const iterators_t& iterators_t::instance()
{
    return iterators;
}

//-------------------------------------------------------------

template <typename KeyT>
struct generate_paths_t<KeyT,hana::when<std::is_base_of<element_aggregation,std::decay_t<KeyT>>::value>>
{
    template <typename UsedPathSizeT, typename PathT, typename AdapterT, typename HandlerT>
    status operator () (UsedPathSizeT&& used_path_size, PathT&& path, AdapterT&& adapter, HandlerT&& handler) const
    {
        const auto& aggregation=hana::back(path);
        return element_aggregation::invoke(
            hana::partial(aggregation.predicate(),std::forward<AdapterT>(adapter)),
            aggregation.post_empty_handler(),
            aggregation.string(),
            std::forward<UsedPathSizeT>(used_path_size),
            path,
            std::forward<AdapterT>(adapter),
            std::forward<HandlerT>(handler)
        );
    }
};

//-------------------------------------------------------------

template <typename KeyT>
struct generate_paths_t<KeyT,hana::when<std::is_base_of<variadic_arg_aggregation_tag,KeyT>::value>>
{
    template <typename UsedPathSizeT, typename PathT, typename AdapterT, typename HandlerT>
    status operator () (UsedPathSizeT&& used_path_size, PathT&& path, AdapterT&& adapter, HandlerT&& handler) const
    {
        const auto& aggregation=hana::back(path).get().aggregation;
        return element_aggregation::invoke_variadic(
            hana::partial(aggregation.predicate(),std::forward<AdapterT>(adapter)),
            aggregation.post_empty_handler(),
            aggregation.string(),
            std::forward<UsedPathSizeT>(used_path_size),
            path,
            std::forward<AdapterT>(adapter),
            std::forward<HandlerT>(handler)
        );
    }
};

//-------------------------------------------------------------

DRACOSHA_VALIDATOR_NAMESPACE_END

#endif // DRACOSHA_VALIDATOR_ELEMENT_AGGREGATION_HPP
