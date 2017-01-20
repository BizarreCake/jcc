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

#include "catch.hpp"
#include <jtac/parse/lexer.hpp>
#include <string>


using namespace jcc;
using namespace jcc::jtac;


TEST_CASE( "Tokenizing various inputs using the JTAC lexer",
           "[jtac_lexer]" ) {



  SECTION( "Snippet #1" ) {

    std::istringstream ss (
        ";\n"
            "; comment\n"
            ";\n"
            "proc foo(x, y, z):\n"
            "        a = x ; another comment\n"
            "        b = a - y\n"
            "        c = a * b + z\n"
            "        ret a");
    lexer lx (ss);

    auto toks = lx.tokenize ();

    REQUIRE( toks.next ().type == JTAC_TOK_PROC );
    REQUIRE( toks.peek_next ().type == JTAC_TOK_NAME );
    REQUIRE( std::string (toks.next ().val.str) == "foo" );
    REQUIRE( toks.next ().type == JTAC_TOK_LPAREN );
    REQUIRE( toks.peek_next ().type == JTAC_TOK_NAME );
    REQUIRE( std::string (toks.next ().val.str) == "x" );
    REQUIRE( toks.next ().type == JTAC_TOK_COMMA );
    REQUIRE( toks.peek_next ().type == JTAC_TOK_NAME );
    REQUIRE( std::string (toks.next ().val.str) == "y" );
    REQUIRE( toks.next ().type == JTAC_TOK_COMMA );
    REQUIRE( toks.peek_next ().type == JTAC_TOK_NAME );
    REQUIRE( std::string (toks.next ().val.str) == "z" );
    REQUIRE( toks.next ().type == JTAC_TOK_RPAREN );
    REQUIRE( toks.next ().type == JTAC_TOK_COL );
    REQUIRE( toks.peek_next ().type == JTAC_TOK_NAME );
    REQUIRE( std::string (toks.next ().val.str) == "a" );
    REQUIRE( toks.next ().type == JTAC_TOK_ASSIGN );
    REQUIRE( toks.peek_next ().type == JTAC_TOK_NAME );
    REQUIRE( std::string (toks.next ().val.str) == "x" );
    REQUIRE( toks.peek_next ().type == JTAC_TOK_NAME );
    REQUIRE( std::string (toks.next ().val.str) == "b" );
    REQUIRE( toks.next ().type == JTAC_TOK_ASSIGN );
    REQUIRE( toks.peek_next ().type == JTAC_TOK_NAME );
    REQUIRE( std::string (toks.next ().val.str) == "a" );
    REQUIRE( toks.next ().type == JTAC_TOK_SUB );
    REQUIRE( toks.peek_next ().type == JTAC_TOK_NAME );
    REQUIRE( std::string (toks.next ().val.str) == "y" );
    REQUIRE( toks.peek_next ().type == JTAC_TOK_NAME );
    REQUIRE( std::string (toks.next ().val.str) == "c" );
    REQUIRE( toks.next ().type == JTAC_TOK_ASSIGN );
    REQUIRE( toks.peek_next ().type == JTAC_TOK_NAME );
    REQUIRE( std::string (toks.next ().val.str) == "a" );
    REQUIRE( toks.next ().type == JTAC_TOK_MUL );
    REQUIRE( toks.peek_next ().type == JTAC_TOK_NAME );
    REQUIRE( std::string (toks.next ().val.str) == "b" );
    REQUIRE( toks.next ().type == JTAC_TOK_ADD );
    REQUIRE( toks.peek_next ().type == JTAC_TOK_NAME );
    REQUIRE( std::string (toks.next ().val.str) == "z" );
    REQUIRE( toks.next ().type == JTAC_TOK_RET );
    REQUIRE( toks.peek_next ().type == JTAC_TOK_NAME );
    REQUIRE( std::string (toks.next ().val.str) == "a" );

    REQUIRE( !toks.has_next () );
  }

  SECTION( "Snippet #2" ) {

      std::istringstream ss (
          "a = 53423\n"
          "b = x + 62136498498498\n"
          "cmp a, b\n"
          "jmp .test1\n"
          "je .test2\n"
          "jne .test3\n"
          "jl .test4\n"
          "jle .test5\n"
          "jg .test6\n"
          "jge .test7\n"
          "call foobar(12, 57, x, y)\n"
          "c = d / y\n"
          "c = d % y\n");
      lexer lx (ss);

      auto toks = lx.tokenize ();

      REQUIRE( toks.peek_next ().type == JTAC_TOK_NAME );
      REQUIRE( std::string (toks.next ().val.str) == "a" );
      REQUIRE( toks.next ().type == JTAC_TOK_ASSIGN );
      REQUIRE( toks.peek_next ().type == JTAC_TOK_INTEGER );
      REQUIRE( toks.next ().val.i64 == 53423 );
      REQUIRE( toks.peek_next ().type == JTAC_TOK_NAME );
      REQUIRE( std::string (toks.next ().val.str) == "b" );
      REQUIRE( toks.next ().type == JTAC_TOK_ASSIGN );
      REQUIRE( toks.peek_next ().type == JTAC_TOK_NAME );
      REQUIRE( std::string (toks.next ().val.str) == "x" );
      REQUIRE( toks.next ().type == JTAC_TOK_ADD );
      REQUIRE( toks.peek_next ().type == JTAC_TOK_INTEGER );
      REQUIRE( toks.next ().val.i64 == 62136498498498LL );
      REQUIRE( toks.next ().type == JTAC_TOK_CMP );
      REQUIRE( toks.peek_next ().type == JTAC_TOK_NAME );
      REQUIRE( std::string (toks.next ().val.str) == "a" );
      REQUIRE( toks.next ().type == JTAC_TOK_COMMA );
      REQUIRE( toks.peek_next ().type == JTAC_TOK_NAME );
      REQUIRE( std::string (toks.next ().val.str) == "b" );
      REQUIRE( toks.next ().type == JTAC_TOK_JMP );
      REQUIRE( toks.peek_next ().type == JTAC_TOK_NAME );
      REQUIRE( std::string (toks.next ().val.str) == ".test1" );
      REQUIRE( toks.next ().type == JTAC_TOK_JE );
      REQUIRE( toks.peek_next ().type == JTAC_TOK_NAME );
      REQUIRE( std::string (toks.next ().val.str) == ".test2" );
      REQUIRE( toks.next ().type == JTAC_TOK_JNE );
      REQUIRE( toks.peek_next ().type == JTAC_TOK_NAME );
      REQUIRE( std::string (toks.next ().val.str) == ".test3" );
      REQUIRE( toks.next ().type == JTAC_TOK_JL );
      REQUIRE( toks.peek_next ().type == JTAC_TOK_NAME );
      REQUIRE( std::string (toks.next ().val.str) == ".test4" );
      REQUIRE( toks.next ().type == JTAC_TOK_JLE );
      REQUIRE( toks.peek_next ().type == JTAC_TOK_NAME );
      REQUIRE( std::string (toks.next ().val.str) == ".test5" );
      REQUIRE( toks.next ().type == JTAC_TOK_JG );
      REQUIRE( toks.peek_next ().type == JTAC_TOK_NAME );
      REQUIRE( std::string (toks.next ().val.str) == ".test6" );
      REQUIRE( toks.next ().type == JTAC_TOK_JGE );
      REQUIRE( toks.peek_next ().type == JTAC_TOK_NAME );
      REQUIRE( std::string (toks.next ().val.str) == ".test7" );
      REQUIRE( toks.next ().type == JTAC_TOK_CALL );
      REQUIRE( toks.peek_next ().type == JTAC_TOK_NAME );
      REQUIRE( std::string (toks.next ().val.str) == "foobar" );
      REQUIRE( toks.next ().type == JTAC_TOK_LPAREN );
      REQUIRE( toks.peek_next ().type == JTAC_TOK_INTEGER );
      REQUIRE( toks.next ().val.i64 == 12 );
      REQUIRE( toks.next ().type == JTAC_TOK_COMMA );
      REQUIRE( toks.peek_next ().type == JTAC_TOK_INTEGER );
      REQUIRE( toks.next ().val.i64 == 57 );
      REQUIRE( toks.next ().type == JTAC_TOK_COMMA );
      REQUIRE( toks.peek_next ().type == JTAC_TOK_NAME );
      REQUIRE( std::string (toks.next ().val.str) == "x" );
      REQUIRE( toks.next ().type == JTAC_TOK_COMMA );
      REQUIRE( toks.peek_next ().type == JTAC_TOK_NAME );
      REQUIRE( std::string (toks.next ().val.str) == "y" );
      REQUIRE( toks.next ().type == JTAC_TOK_RPAREN );
      REQUIRE( toks.peek_next ().type == JTAC_TOK_NAME );
      REQUIRE( std::string (toks.next ().val.str) == "c" );
      REQUIRE( toks.next ().type == JTAC_TOK_ASSIGN );
      REQUIRE( toks.peek_next ().type == JTAC_TOK_NAME );
      REQUIRE( std::string (toks.next ().val.str) == "d" );
      REQUIRE( toks.next ().type == JTAC_TOK_DIV );
      REQUIRE( toks.peek_next ().type == JTAC_TOK_NAME );
      REQUIRE( std::string (toks.next ().val.str) == "y" );
      REQUIRE( toks.peek_next ().type == JTAC_TOK_NAME );
      REQUIRE( std::string (toks.next ().val.str) == "c" );
      REQUIRE( toks.next ().type == JTAC_TOK_ASSIGN );
      REQUIRE( toks.peek_next ().type == JTAC_TOK_NAME );
      REQUIRE( std::string (toks.next ().val.str) == "d" );
      REQUIRE( toks.next ().type == JTAC_TOK_MOD );
      REQUIRE( toks.peek_next ().type == JTAC_TOK_NAME );
      REQUIRE( std::string (toks.next ().val.str) == "y" );


      REQUIRE( !toks.has_next () );
    }
}
