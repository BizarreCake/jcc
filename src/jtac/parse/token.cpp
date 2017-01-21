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

#include "jtac/parse/token.hpp"
#include <cstring>
#include <sstream>
#include <stdexcept>


namespace jcc {
namespace jtac {

  //! \brief Releases memory used by the specified token.
  void
  destroy_token (token tok)
  {
    switch (tok.type)
      {
      case JTAC_TOK_NAME:
        delete[] tok.val.str;
        break;

      default: ;
      }
  }

  //! \brief Returns a copy of the specified token.
  token
  copy_token (token tok)
  {
    token cpy;
    cpy.type = tok.type;
    cpy.pos = tok.pos;
    switch (tok.type)
      {
      case JTAC_TOK_NAME:
        cpy.val.str = new char [std::strlen (tok.val.str) + 1];
        std::strcpy (cpy.val.str, tok.val.str);
        break;

      case JTAC_TOK_INTEGER:
        cpy.val.i64 = tok.val.i64;
        break;

      default: ;
      }

    return cpy;
  }

  //! \brief Returns a textual representation of the specified token.
  std::string
  token_str (token tok)
  {
    switch (tok.type)
      {
      case JTAC_TOK_UNDEF: return "<undef>";
      case JTAC_TOK_EOF: return "<eof>";

      case JTAC_TOK_NAME: return tok.val.str;
      case JTAC_TOK_INTEGER:
        {
          std::ostringstream ss;
          ss << tok.val.i64;
          return ss.str ();
        }

      case JTAC_TOK_LPAREN:  return "(";
      case JTAC_TOK_RPAREN:  return ")";
      case JTAC_TOK_COL:     return ":";
      case JTAC_TOK_COMMA:   return ",";
      case JTAC_TOK_ASSIGN:  return "=";
      case JTAC_TOK_ADD:     return "+";
      case JTAC_TOK_SUB:     return "-";
      case JTAC_TOK_MUL:     return "*";
      case JTAC_TOK_DIV:     return "/";
      case JTAC_TOK_MOD:     return "%";

      case JTAC_TOK_PROC:    return "proc";
      case JTAC_TOK_ENDPROC: return "endproc";
      case JTAC_TOK_CMP:     return "cmp";
      case JTAC_TOK_JMP:     return "jmp";
      case JTAC_TOK_JE:      return "je";
      case JTAC_TOK_JNE:     return "jne";
      case JTAC_TOK_JL:      return "jl";
      case JTAC_TOK_JLE:     return "jle";
      case JTAC_TOK_JG:      return "jg";
      case JTAC_TOK_JGE:     return "jge";
      case JTAC_TOK_CALL:    return "call";
      case JTAC_TOK_RET:     return "ret";
      }

    return "";
  }



//------------------------------------------------------------------------------

  token_stream::token_stream ()
  {
    this->pos = 0;
  }

  token_stream::token_stream (const token_stream& other)
  {
    this->pos = other.pos;
    for (auto& tok : other.toks)
      this->toks.push_back (copy_token (tok));
  }

  token_stream::token_stream (token_stream&& other)
    : toks (std::move (other.toks))
  {
    this->pos = other.pos;
  }

  token_stream::~token_stream ()
  {
    for (auto& tok : this->toks)
      destroy_token (tok);
  }



  //! \brief Returns the current token and advances the stream.
  token
  token_stream::next ()
  {
    if (this->pos >= this->toks.size ())
      throw std::runtime_error ("token_stream::next: end of stream");
    return this->toks[this->pos ++];
  }

  //! \brief Returns the current token without advancing the stream.
  token
  token_stream::peek_next () const
  {
    if (this->pos >= this->toks.size ())
      throw std::runtime_error ("token_stream::peek_next: end of stream");
    return this->toks[this->pos];
  }

  //! \brief Returns true if there are tokens left to be returned by next().
  bool
  token_stream::has_next () const
  {
    return (this->pos < this->toks.size ());
  }

  //! \brief Rolls the stream back by one token, and returns the current token.
  token
  token_stream::prev ()
  {
    if (this->pos == 0)
      throw std::runtime_error ("token_stream::prev: end of stream");
    return this->toks[-- this->pos];
  }

  //! \brief Returns the previous token.
  token
  token_stream::peek_prev () const
  {
    if (this->pos == 0)
      throw std::runtime_error ("token_stream::peek_prev: end of stream");
    return this->toks[this->pos - 1];
  }

  //! \brief Returns true if there are tokens left to be returned by prev().
  bool
  token_stream::has_prev () const
  {
    return this->pos > 0;
  }



  //! \brief Inserts the specified token to the end of the stream.
  void
  token_stream::push_token (token tok)
  {
    this->toks.push_back (tok);
  }
}
}
