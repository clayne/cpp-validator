/**
@copyright Evgeny Sidorov 2020

Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)

*/

/****************************************************************************/

/** \file validator/status.hpp
*
*  Defines status of validation operations
*
*/

/****************************************************************************/

#ifndef DRACOSHA_VALIDATOR_STATUS_HPP
#define DRACOSHA_VALIDATOR_STATUS_HPP

#include <dracosha/validator/config.hpp>

DRACOSHA_VALIDATOR_NAMESPACE_BEGIN

//-------------------------------------------------------------

/**
 * @brief Status to be used as result of validation operation
 */
class status
{
    public:

        enum class code : int
        {
            ok,
            fail,
            ignore
        };

        status() noexcept : _code(code::ok)
        {}
        ~status()=default;
        status(const status&)=default;
        status(status&&)=default;
        status& operator= (const status&)=default;
        status& operator= (status&&)=default;

        /**
         * @brief Constructor from code
         * @param c Status code
         */
        status(code c) noexcept : _code(c)
        {}

        /**
         * @brief Constructor from boolean
         * @param ok Boolean status
         */
        status(bool ok) noexcept : _code(ok?code::ok:code::fail)
        {}

        /**
         * @brief Convert to boolean
         */
        operator bool () const noexcept
        {
            return _code!=code::fail;
        }

        /**
         * @brief Get value
         * @return Status code
         */
        code value() const noexcept
        {
            return _code;
        }

    private:

        code _code;
};

//-------------------------------------------------------------

DRACOSHA_VALIDATOR_NAMESPACE_END

#endif // DRACOSHA_VALIDATOR_STATUS_HPP