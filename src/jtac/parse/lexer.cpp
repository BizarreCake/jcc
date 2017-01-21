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

#include "jtac/parse/lexer.hpp"
#include <istream>
#include <cctype>
#include <unordered_map>
#include <cstring>


namespace jcc {
namespace jtac {

  lexer_stream::lexer_stream (std::istream& strm)
    : strm (strm)
  {
    this->ln = 1;
    this->col = 1;
    this->pcol = 1;
  }



  //! \brief Returns the next character in the stream before advancing it.
  int
  lexer_stream::get ()
  {
    int c = this->strm.get ();
    if (c == std::istream::traits_type::eof ())
      return EOF;

    if (c == '\n')
      {
        ++ this->ln;
        this->col = 1;
      }
    else
      ++ this->col;

    return c;
  }

  //! \brief Returns the next character in the stream.
  int
  lexer_stream::peek () const
  {
    int c = this->strm.peek ();
    return (c == std::istream::traits_type::eof ()) ? EOF : c;
  }

  //! \brief Rolls the stream backwards by one character.
  void
  lexer_stream::unget ()
  {
    this->strm.unget ();
    if (this->peek () == '\n')
      {
        this->col = this->pcol;
        this->pcol = 1;
        -- this->ln;
      }
    else
      -- this->col;
  }



//------------------------------------------------------------------------------

  lexer::lexer (std::istream& strm)
      : strm (strm)
  { }



  /*!
     \brief Tokenizes the underlying stream and returns a token stream.
     \throws lexer_error In case an unrecognized token is encountered.
   */
  token_stream
  lexer::tokenize ()
  {
    token_stream toks;

    for (;;)
      {
        token tok = this->read_token ();
        if (tok.type == JTAC_TOK_EOF)
          break;
        else if (tok.type == JTAC_TOK_UNDEF)
          throw lexer_error ("unrecognized token", tok.pos);

        toks.push_token (tok);
      }

    return toks;
  }



  //! \brief Skips whitespace characters (including comments).
  void
  lexer::skip_whitespace ()
  {
    int c;
    while ((c = this->strm.peek ()) != EOF && (std::isspace (c) || c == ';'))
      {
        this->strm.get ();
        if (c == ';')
          while ((c = this->strm.get ()) != EOF && c != '\n')
            ;
      }
  }


  bool
  lexer::try_read_punctuation (token& tok)
  {
    int c = this->strm.peek ();
    switch (c)
      {
      case '(': this->strm.get (); tok.type = JTAC_TOK_LPAREN; return true;
      case ')': this->strm.get (); tok.type = JTAC_TOK_RPAREN; return true;
      case '=': this->strm.get (); tok.type = JTAC_TOK_ASSIGN; return true;
      case ':': this->strm.get (); tok.type = JTAC_TOK_COL; return true;
      case ',': this->strm.get (); tok.type = JTAC_TOK_COMMA; return true;
      case '+': this->strm.get (); tok.type = JTAC_TOK_ADD; return true;
      case '-': this->strm.get (); tok.type = JTAC_TOK_SUB; return true;
      case '*': this->strm.get (); tok.type = JTAC_TOK_MUL; return true;
      case '/': this->strm.get (); tok.type = JTAC_TOK_DIV; return true;
      case '%': this->strm.get (); tok.type = JTAC_TOK_MOD; return true;

      default: ;
      }

    return false;
  }


  bool
  lexer::try_read_number (token& tok)
  {
    if (!std::isdigit (this->strm.peek ()))
      return false;

    std::string str;
    while (std::isdigit (this->strm.peek ()))
      str.push_back ((char)this->strm.get ());

    tok.type = JTAC_TOK_INTEGER;
    tok.val.i64 = std::strtol (str.c_str (), nullptr, 10);
    return true;
  }


  static bool
  _is_name_char (int c)
  {
    if (std::isalnum (c))
      return true;

    switch (c)
      {
        case '.': return true;
        case '_': return true;
        case '!': return true;
        case '@': return true;
        case '#': return true;
        case '$': return true;

        default: return false;
      }
  }

  static bool
  _is_first_name_char (int c)
  { return _is_name_char (c) && !std::isdigit (c); }

  bool
  lexer::try_read_name_or_keyword (token& tok)
  {
    if (!_is_first_name_char (this->strm.peek ()))
      return false;

    int c;
    std::string name (1, (char)this->strm.get ());
    while ((c = this->strm.peek ()) != EOF && _is_name_char (c))
      name.push_back ((char)this->strm.get ());

    static std::unordered_map<std::string, token_type> _keywords {
        { "proc", JTAC_TOK_PROC },
        { "endproc", JTAC_TOK_ENDPROC },
        { "cmp", JTAC_TOK_CMP },
        { "jmp", JTAC_TOK_JMP },
        { "je", JTAC_TOK_JE },
        { "jne", JTAC_TOK_JNE },
        { "jl", JTAC_TOK_JL },
        { "jle", JTAC_TOK_JLE },
        { "jg", JTAC_TOK_JG },
        { "jge", JTAC_TOK_JGE },
        { "call", JTAC_TOK_CALL },
        { "ret", JTAC_TOK_RET }
    };

    auto itr = _keywords.find (name);
    if (itr != _keywords.end ())
      { tok.type = itr->second; return true; }

    tok.type = JTAC_TOK_NAME;
    tok.val.str = new char [name.length () + 1];
    std::strcpy (tok.val.str, name.c_str ());
    return true;
  }


  token
  lexer::read_token ()
  {
    this->skip_whitespace ();

    token tok;
    tok.type = JTAC_TOK_UNDEF;
    tok.pos.ln = this->strm.get_line ();
    tok.pos.col = this->strm.get_column ();

    if (this->strm.peek () == EOF)
      { tok.type = JTAC_TOK_EOF; return tok; }

    if (this->try_read_punctuation (tok)) return tok;
    if (this->try_read_number (tok)) return tok;
    if (this->try_read_name_or_keyword (tok)) return tok;

    return tok;
  }
}
}
