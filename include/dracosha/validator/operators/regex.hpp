/**
@copyright Evgeny Sidorov 2020

Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)

*/

/****************************************************************************/

/** \file validator/operators/regex.hpp
*
*  Defines regular expressions operator.
*
*/

/****************************************************************************/

#ifndef DRACOSHA_VALIDATOR_REGEX_HPP
#define DRACOSHA_VALIDATOR_REGEX_HPP

#include <boost/regex.hpp>

#include <dracosha/validator/config.hpp>

DRACOSHA_VALIDATOR_NAMESPACE_BEGIN

//-------------------------------------------------------------

/**
 * @brief Definition of operator "match regular expression".
 */
struct regex_match_t : public op<regex_match_t>
{
    constexpr static const char* description="must match expression";
    constexpr static const char* n_description="must not match expression";

    template <typename T1, typename T2>
    constexpr bool operator() (const T1& a, const T2& b,
                               std::enable_if_t<
                                !std::is_same<T2,boost::regex>::value,
                                void*
                               > =nullptr) const
    {
        return boost::regex_match(a,boost::regex(b));
    }

    template <typename T1, typename T2>
    constexpr bool operator() (const T1& a, const T2& b,
                               std::enable_if_t<
                                std::is_same<T2,boost::regex>::value,
                                void*
                               > =nullptr) const
    {
        return boost::regex_match(a,b);
    }
};

/**
    @brief Operator "match regular expression".
*/
constexpr regex_match_t regex_match{};

/**
 * @brief Definition of operator "not match regular expression".
 */
struct regex_nmatch_t : public op<regex_nmatch_t>
{
    constexpr static const char* description=regex_match_t::n_description;
    constexpr static const char* n_description=regex_match_t::description;

    template <typename T1, typename T2>
    constexpr bool operator() (const T1& a, const T2& b) const
    {
        return !regex_match(a,b);
    }
};

/**
    @brief Operator "not match regular expression".
*/
constexpr regex_nmatch_t regex_nmatch{};

/**
 * @brief Format boost::regex operand.
 */
template <typename T>
struct format_operand_t<T,hana::when<std::is_same<std::decay_t<T>,boost::regex>::value>>
{
    /**
     * @brief Format boost::regex operand.
     * @param traits Formatter traits.
     * @param val Operand value.
     * @return Formatted interval.
     */
    template <typename TraitsT, typename T1>
    auto operator () (const TraitsT& traits, T1&& val, grammar_categories cats) const
    {
        return decorate(traits,translate(traits,val.str(),cats));
    }
};

//-------------------------------------------------------------

DRACOSHA_VALIDATOR_NAMESPACE_END

#endif // DRACOSHA_VALIDATOR_REGEX_HPP
