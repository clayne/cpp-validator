/**
@copyright Evgeny Sidorov 2020

Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)

*/

/****************************************************************************/

/** \file validator/reporting/operand_formatter.hpp
*
* Defines wrapper for concrete phrases.
*
*/

/****************************************************************************/

#ifndef DRACOSHA_VALIDATOR_CONCRETE_PHRASE_HPP
#define DRACOSHA_VALIDATOR_CONCRETE_PHRASE_HPP

#include <string>

#include <dracosha/validator/config.hpp>
#include <dracosha/validator/reporting/word_attributtes.hpp>

DRACOSHA_VALIDATOR_NAMESPACE_BEGIN

//-------------------------------------------------------------

struct concrete_phrase_tag;

/**
 * @brief Final result of text processing that must not be altered any more.
 *
 * Concrete phrase contains text of the phrase and bitmask of lexical attributes of the phrase actual for current locale.
 */
class concrete_phrase
{
    public:

        using hana_tag=concrete_phrase_tag;

        /**
         * @brief Conctructor
         * @param text Text of the phrase
         */
        concrete_phrase(
                std::string text
            ) : _text(std::move(text)),
                _attributes(0)
        {}

        /**
         * @brief Conctructor
         * @param text Text of the phrase
         * @param attributes Bitmask of lexical attributes of the phrase
         */
        concrete_phrase(
                std::string text,
                word_attributes attributes
            ) : _text(std::move(text)),
                _attributes(attributes)
        {}

        /**
         * @brief Conctructor
         * @param phrase Other phrase to construct from
         * @param attributes Bitmask of lexical attributes of the phrase
         */
        concrete_phrase(
                concrete_phrase&& phrase,
                word_attributes attributes
            ) : _text(std::move(phrase._text)),
                _attributes(attributes)
        {}

        /**
         * @brief Conctructor
         * @param text Text of the phrase
         * @param attributes Lexical attribute of the phrase
         */
        concrete_phrase(
                std::string text,
                word attributes
            ) : _text(std::move(text)),
                _attributes(word_bitmask.bit(attributes))
        {}

        /**
         * @brief Conctructor
         * @param text Other phrase to construct from
         * @param attributes Lexical attributes of the phrase
         */
        concrete_phrase(
                std::string text,
                const std::initializer_list<word>& attributes
            ) : _text(std::move(text)),
                _attributes(word_bitmask.bits(attributes))
        {}

        /**
         * @brief Conctructor
         * @param text Other phrase to construct from
         * @param attributes Lexical attributes of the phrase
         */
        template <typename ... Attributes>
        concrete_phrase(
                std::string text,
                Attributes&&... attributes
            ) : _text(std::move(text)),
                _attributes(word_bitmask.bits(std::forward<Attributes>(attributes)...))
        {}

        /**
         * @brief Get lexical attributes of the phrase
         * @return Bitmask of lexical attributes
         */
        word_attributes attributes() const noexcept
        {
            return _attributes;
        }

        /**
         * @brief Get text of the phrase
         * @return Text
         */
        std::string text() const
        {
            return _text;
        }

        /**
         * @brief Convert the phrase to string
         * @return Text
         */
        operator std::string() const
        {
            return _text;
        }

        /**
         * @brief Set text of the phrase
         * @param Text
         */
        void set_text(std::string text)
        {
            _text=std::move(text);
        }

        /**
         * @brief Set lexical attributes of the phrase
         * @param attributes Bitmask of lexical attributes
         */
        void set_attributes(word_attributes attributes) noexcept
        {
            _attributes=attributes;
        }

        /**
         * @brief Friend operator << for phrase formatting to ostream
         * @param os Output stream
         * @param ph Phrase
         * @return Output stream
         */
        friend std::ostream& operator<<(std::ostream& os, const concrete_phrase& ph)
        {
            return os << ph.text();
        }

    private:

        std::string _text;
        word_attributes _attributes;
};

//-------------------------------------------------------------

DRACOSHA_VALIDATOR_NAMESPACE_END

#endif // DRACOSHA_VALIDATOR_CONCRETE_PHRASE_HPP