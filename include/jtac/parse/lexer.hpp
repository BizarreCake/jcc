/*
 * jcc - A compiler framework.
 * Copyright (C) 2016-2017 Jacob Zhitomirsky
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _JCC__JTAC__PARSE__LEXER__H_
#define _JCC__JTAC__PARSE__LEXER__H_

#include "jtac/parse/token.hpp"
#include <iosfwd>
#include <stdexcept>


namespace jcc {
namespace jtac {

#ifndef EOF
  #define EOF ((int)-1)
#endif

  /*!
     \class lexer_stream
     \brief Wraps a stream and provides line and column number tracking.
   */
  class lexer_stream
  {
    std::istream& strm;
    int ln, col, pcol;

   public:
    inline int get_line () const { return this->ln; }
    inline int get_column () const { return this->col; }

   public:
    lexer_stream (std::istream& strm);

   public:
    //! \brief Returns the next character in the stream before advancing it.
    int get ();

    //! \brief Returns the next character in the stream.
    int peek () const;

    //! \brief Rolls the stream backwards by one character.
    void unget ();
  };


  /*!
     \class lexer_error
     \brief Thrown by the lexer in case of failure.
   */
  class lexer_error: public std::runtime_error
  {
    token_pos pos;

   public:
    inline token_pos get_pos () const { return this->pos; }

   public:
    lexer_error (const std::string& str, token_pos pos)
        : std::runtime_error (str), pos (pos)
    { }
  };


  /*!
     \class lexer
     \brief JTAC tokenizer.
   */
  class lexer
  {
    lexer_stream strm;

   public:
    lexer (std::istream& strm);

   public:
    /*!
       \brief Tokenizes the underlying stream and returns a token stream.
       \throws lexer_error In case an unrecognized token is encountered.
     */
    token_stream tokenize ();

   private:
    token read_token ();

    bool try_read_punctuation (token& tok);

    bool try_read_number (token& tok);

    bool try_read_name_or_keyword (token& tok);

    //! \brief Skips whitespace characters (including comments).
    void skip_whitespace ();
  };
}
}

#endif //_JCC__JTAC__PARSE__LEXER__H_
