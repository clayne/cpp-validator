/**
@copyright Evgeny Sidorov 2020

Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)

*/

/****************************************************************************/

/** @file validator/variadic_property.hpp
*
* Defines helpers to work with properties with any number of arguments.
*
*/

/****************************************************************************/

#ifndef DRACOSHA_VALIDATOR_VARIADIC_PROPERTY_HPP
#define DRACOSHA_VALIDATOR_VARIADIC_PROPERTY_HPP

#include <type_traits>

#include <dracosha/validator/config.hpp>
#include <dracosha/validator/utils/class_method_args.hpp>
#include <dracosha/validator/utils/object_wrapper.hpp>
#include <dracosha/validator/utils/unwrap_object.hpp>
#include <dracosha/validator/property.hpp>
#include <dracosha/validator/utils/to_string.hpp>
#include <dracosha/validator/reporting/backend_formatter.hpp>
#include <dracosha/validator/reporting/format_join_grammar_cats.hpp>
#include <dracosha/validator/variadic_arg.hpp>
#include <dracosha/validator/aggregation/wrap_index.hpp>

DRACOSHA_VALIDATOR_NAMESPACE_BEGIN

//-------------------------------------------------------------

struct always_has_property_t
{
    template <typename ...Args>
    constexpr static bool apply(Args&&...) noexcept
    {
        return true;
    }
};
constexpr always_has_property_t always_has_property{};

template <typename ObjT, typename FnT, typename AccumulatedArgsT, typename FnArgTypes, typename FnHasT>
struct variadic_property_closure
{
    template <typename T>
    constexpr static bool has_c()
    {
        auto current_type=hana::at(
                        FnArgTypes{},
                        hana_tuple_size<AccumulatedArgsT>{}
                    );
        return std::is_convertible<T,typename decltype(current_type)::type>::value;
    }

    template <typename T>
    auto at(T&& arg,
            std::enable_if_t<variadic_property_closure::has_c<T>(), void*> =nullptr
        ) const -> decltype(auto)
    {
        return hana::if_(
            hana::equal(hana::plus(hana::size(_args),hana::size_c<1>),hana::size(_arg_types)),
            [](auto&& self, auto&& arg) -> decltype(auto)
            {
                auto apply=[&](auto&&... args) -> decltype(auto)
                {
                    return self->_fn(self->_obj,std::forward<decltype(args)>(args)...);
                };
                return hana::unpack(self->_args,hana::reverse_partial(apply,std::forward<decltype(arg)>(arg)));
            },
            [](auto&& self, auto&& arg)
            {
                auto next_args=hana::append(std::move(self->_args),std::forward<decltype(arg)>(arg));
                return variadic_property_closure<ObjT,FnT,decltype(next_args),FnArgTypes,FnHasT>{
                    std::move(self->_obj),
                    std::move(self->_fn),
                    std::move(next_args),
                    std::move(self->_arg_types),
                    std::move(self->_fn_has)
                };
            }
        )(this,std::forward<T>(arg));
    }

    template <typename T>
    bool has(
            T&& arg,
            std::enable_if_t<
                variadic_property_closure::has_c<T>()
            , void*> =nullptr
        ) const
    {
        return hana::if_(
            hana::equal(hana::plus(hana::size(_args),hana::size_c<1>),hana::size(_arg_types)),
            [](auto&& self, auto&& arg)
            {
                auto apply=[&](auto&&... args)
                {
                    return self->_fn_has.apply(self->_obj,std::forward<decltype(args)>(args)...);
                };
                return hana::unpack(self->_args,hana::reverse_partial(apply,std::forward<decltype(arg)>(arg)));
            },
            [](auto&&, auto&&)
            {
                return true;
            }
        )(this,std::forward<T>(arg));
    }

    ObjT _obj;
    FnT _fn;
    AccumulatedArgsT _args;
    FnArgTypes _arg_types;
    FnHasT _fn_has;
};

//! @todo Consider taking into account preceeding gramma categories and use
//! two formatters: "member_names" for property name and "strings" for aggregation descriptions.
template <typename PropT, typename FormatterT>
std::string format_variadic_property(const PropT& prop, const FormatterT& formatter)
{
    const auto& arg=unwrap_object(hana::front(prop.args()));
    return hana::eval_if(
        hana::and_(
            hana::equal(hana::size(prop.args()),hana::size_c<1>),
            is_wrap_index(arg)
        ),
        [&](auto&& _)
        {
            std::string dst;
            format_join_grammar_cats(dst,
                                     hana::make_tuple(_(formatter),_(formatter)),
                                     _(arg).aggregation().base_phrase_str(),
                                     _(prop).name());
            return dst;
        },
        [&](auto&& _)
        {
            std::string dst{formatter(_(prop).name(),true)};
            backend_formatter.append(dst,"(");
            backend_formatter.append_join(dst,",",hana::transform(_(prop).args(),_(formatter)));
            backend_formatter.append(dst,")");
            return _(formatter)(dst,false,true);
        }
    );
}

struct variadic_property_tag{};
struct variadic_property_base_tag{};

#define DRACOSHA_VALIDATOR_VARIADIC_PROPERTY_HF(prop,has_prop,flag_dscr,n_flag_dscr) \
struct might_have_variadic_t_##prop \
{ \
    template <typename T> \
    constexpr auto operator() (T*) -> decltype(&T::prop) \
    { \
        return nullptr; \
    } \
}; \
constexpr might_have_variadic_t_##prop might_have_variadic_##prop{}; \
struct type_variadic_p_##prop : public variadic_property_base_tag \
{ \
    using hana_tag=DRACOSHA_VALIDATOR_NAMESPACE::property_tag; \
    \
    template <typename T, typename ... Args> \
    static auto apply(T&& v, Args&&... args) -> decltype(auto) \
    { \
        return v.prop(std::forward<Args>(args)...); \
    } \
    \
    template <typename T> \
    static auto create_closure(T&& v) \
    { \
        auto apply_prop_to_v=[](auto&& v, auto&&... args) -> decltype(auto) \
        { \
            return apply(std::forward<decltype(v)>(v),std::forward<decltype(args)>(args)...); \
        }; \
        auto init_args=::boost::hana::tuple<>{}; \
        auto arg_types=DRACOSHA_VALIDATOR_NAMESPACE::class_method_args<decltype(&std::decay_t<decltype(v)>::prop)>::types(); \
        return DRACOSHA_VALIDATOR_NAMESPACE::variadic_property_closure< \
                    T,decltype(apply_prop_to_v),decltype(init_args),decltype(arg_types),decltype(has_prop)> \
                    { \
                    std::forward<T>(v), \
                    std::move(apply_prop_to_v), \
                    std::move(init_args), \
                    std::move(arg_types), \
                    has_prop \
                }; \
    } \
    \
    template <typename T> \
    static auto get(T&& v) \
    { \
        return create_closure(std::forward<T>(v)); \
    } \
    \
    template <typename T> \
    constexpr static bool has() \
    { \
        return ::boost::hana::sfinae(might_have_variadic_##prop)(std::add_pointer_t<T>(nullptr)) \
                != ::boost::hana::nothing; \
    } \
    constexpr static const char* name() \
    {\
        return #prop; \
    }\
    constexpr static const char* flag_str(bool b) \
    {\
        if (b) \
        { \
            return flag_dscr; \
        } \
        return n_flag_dscr; \
    }\
    constexpr static bool has_flag_str() \
    { \
        return flag_dscr!=nullptr && n_flag_dscr!=nullptr; \
    } \
    template <typename T> constexpr bool operator == (T) const \
    { \
        return false; \
    } \
    template <typename T> constexpr bool operator != (T) const \
    { \
        return true; \
    } \
    constexpr bool operator == (const type_variadic_p_##prop&) const \
    { \
        return true; \
    } \
    \
    constexpr bool operator != (const type_variadic_p_##prop&) const \
    { \
        return false; \
    } \
    \
    template <typename ...Args> \
    auto operator () (Args&&... args) const;\
}; \
constexpr type_variadic_p_##prop prop{}; \
template <typename StoredArgsT> \
struct type_variadic_p_notation_##prop : public type_variadic_p_##prop, public variadic_property_tag \
{ \
    type_variadic_p_notation_##prop(StoredArgsT&& stored_args):_args(std::move(stored_args))\
    {}\
    \
    template <typename T> \
    auto get(T&& v) const -> decltype(auto) \
    { \
        auto apply=[&](auto&&... args) -> decltype(auto) \
        { \
            return type_variadic_p_##prop::apply(std::forward<T>(v),std::forward<decltype(args)>(args)...); \
        }; \
        return hana::unpack(hana::transform(_args,unwrap_object),apply); \
    } \
    \
    template <typename ...Args> \
    constexpr auto operator () (Args&&... args) const \
    {\
        return make_property_validator(*this,std::forward<Args>(args)...); \
    }\
    static const char* name()\
    {\
        return #prop; \
    }\
    auto args() const -> decltype(auto) \
    {\
        return _args; \
    }\
    template <typename FormatterT> \
    std::string name(const FormatterT& formatter) const \
    {\
        return format_variadic_property(*this,formatter); \
    }\
    template <typename FormatterT> \
    std::string flag_str(bool b, const FormatterT& formatter, bool member_of=false) const \
    {\
        auto str=flag_dscr; \
        if (!b) \
        { \
            str=n_flag_dscr; \
        } \
        if (member_of) \
        { \
            return str; \
        } \
        std::string dst{formatter.get()(*this)}; \
        backend_formatter.append(dst," ",str); \
        return dst; \
    }\
    \
    template <typename Arg> \
    auto next(Arg&& arg) const \
    { \
        auto stored_args=hana::append(_args,adjust_storable(std::forward<Arg>(arg))); \
        return type_variadic_p_notation_##prop<decltype(stored_args)>{std::move(stored_args)}; \
    } \
    template <typename T> \
    bool operator == (const type_variadic_p_notation_##prop<T>& other) const noexcept \
    { \
        return check_paths_equal(_args,other._args);  \
    } \
    \
    StoredArgsT _args; \
}; \
template <typename ...Args> \
auto type_variadic_p_##prop::operator () (Args&&... args) const \
{ \
    auto stored_args=hana::make_tuple(adjust_storable(std::forward<Args>(args))...); \
    return type_variadic_p_notation_##prop<decltype(stored_args)>{std::move(stored_args)}; \
}

#define DRACOSHA_VALIDATOR_VARIADIC_PROPERTY(prop) DRACOSHA_VALIDATOR_VARIADIC_PROPERTY_HF(prop,always_has_property,nullptr,nullptr)
#define DRACOSHA_VALIDATOR_VARIADIC_PROPERTY_HAS(prop,has) DRACOSHA_VALIDATOR_VARIADIC_PROPERTY_HF(prop,has,nullptr,nullptr)
#define DRACOSHA_VALIDATOR_VARIADIC_PROPERTY_FLAG(prop,flag_dscr,n_flag_dscr) DRACOSHA_VALIDATOR_VARIADIC_PROPERTY_HF(prop,always_has_property,flag_dscr,n_flag_dscr)

struct compact_variadic_property_t
{
    template <typename PathT>
    auto operator () (PathT&& path) const -> decltype(auto)
    {
        return hana::fold(
            path,
            hana::tuple<>{},
            [](auto&& accumulated, auto&& current_key)
            {
                // check if current key is variadic property
                using type=std::decay_t<decltype(current_key)>;
                return hana::eval_if(
                    hana::and_(
                        std::is_base_of<variadic_arg_tag,type>{},
                        hana::not_(hana::is_empty(accumulated))
                    ),
                    [&](auto&& _)
                    {
                        // modify last element in path if current key is variadic property
                        const auto& prev=hana::back(_(accumulated));
                        using prev_type=unwrap_object_t<decltype(prev)>;
                        return hana::eval_if(
                            std::is_base_of<variadic_property_base_tag,prev_type>{},
                            [&](auto&& _)
                            {
                                const auto& p1=_(prev);
                                using prev_type1=unwrap_object_t<decltype(p1)>;
                                return hana::eval_if(
                                    std::is_base_of<variadic_property_tag,prev_type1>{},
                                    [&](auto&& _)
                                    {
                                        // create next property notation appending current key
                                        auto new_prev=_(prev).next(_(current_key));
                                        return hana::append(
                                                        hana::drop_back(_(accumulated)),
                                                        new_prev
                                                    );
                                    },
                                    [&](auto&& _)
                                    {
                                        // create property notation with current key as the first argument
                                        auto new_prev=_(prev)(_(current_key));
                                        return hana::append(
                                                        hana::drop_back(_(accumulated)),
                                                        new_prev
                                                    );
                                    }
                                );
                            },
                            [&](auto&& _)
                            {
                                // varg must follow variadic property
                                // if not then leave the key as is
                                return hana::append(_(accumulated),_(current_key));
                            }
                        );
                    },
                    [&](auto&& _)
                    {
                        // leave the key as is if current key is not variadic property
                        // or varg is the first key in the path
                        return hana::append(_(accumulated),_(current_key));
                    }
                );
            }
        );
    }
};
constexpr compact_variadic_property_t compact_variadic_property{};

//-------------------------------------------------------------

DRACOSHA_VALIDATOR_NAMESPACE_END

#endif // DRACOSHA_VALIDATOR_VARIADIC_PROPERTY_HPP
