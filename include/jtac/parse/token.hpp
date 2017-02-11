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

#ifndef _JCC__JTAC__PARSE__TOKEN__H_
#define _JCC__JTAC__PARSE__TOKEN__H_

#include "jtac/jtac.hpp"
#include <vector>
#include <cstddef>
#include <string>


namespace jcc {
namespace jtac {

  enum token_type
  {
    JTAC_TOK_UNDEF,
    JTAC_TOK_EOF,

    JTAC_TOK_NAME,
    JTAC_TOK_INTEGER,

    JTAC_TOK_LPAREN, // (
    JTAC_TOK_RPAREN, // )
    JTAC_TOK_COL,    // :
    JTAC_TOK_COMMA,  // ,
    JTAC_TOK_ASSIGN, // =
    JTAC_TOK_ADD,    // +
    JTAC_TOK_SUB,    // -
    JTAC_TOK_MUL,    // *
    JTAC_TOK_DIV,    // /
    JTAC_TOK_MOD,    // %

    JTAC_TOK_PROC,    // proc
    JTAC_TOK_ENDPROC, // endproc
    JTAC_TOK_CMP,     // cmp
    JTAC_TOK_JMP,     // jmp
    JTAC_TOK_JE,      // je
    JTAC_TOK_JNE,     // jne
    JTAC_TOK_JL,      // jl
    JTAC_TOK_JLE,     // jle
    JTAC_TOK_JG,      // jg
    JTAC_TOK_JGE,     // jge
    JTAC_TOK_CALL,    // call
    JTAC_TOK_RET,     // ret
    JTAC_TOK_RETN,    // retn
  };

  struct token_pos { int ln, col; };

  struct token
  {
    token_type type;
    union {
      char *str;
      long i64;
    } val;

    token_pos pos;
  };


  //! \brief Releases memory used by the specified token.
  void destroy_token (token tok);

  //! \brief Returns a copy of the specified token.
  token copy_token (token tok);

  //! \brief Returns a textual representation of the specified token.
  std::string token_str (token tok);


  /*!
     \class token_stream
     \brief Stores an array of tokens.

     Provides a convenient set of methods to extract tokens from the stream.
   */
  class token_stream
  {
    std::vector<token> toks;
    size_t pos;

   public:
    token_stream ();
    token_stream (const token_stream& other);
    token_stream (token_stream&& other);
    ~token_stream ();

   public:
    //! \brief Returns the current token and advances the stream.
    token next ();

    //! \brief Returns the current token without advancing the stream.
    token peek_next () const;

    //! \brief Returns true if there are tokens left to be returned by next().
    bool has_next () const;

    //! \brief Rolls the stream back by one token, and returns the current token.
    token prev ();

    //! \brief Returns the previous token.
    token peek_prev () const;

    //! \brief Returns true if there are tokens left to be returned by prev().
    bool has_prev () const;

   public:
    //! \brief Inserts the specified token to the end of the stream.
    void push_token (token tok);
  };
}
}

#endif //_JCC__JTAC__PARSE__TOKEN__H_
