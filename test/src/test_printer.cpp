/*
 * jcc - A compiler framework.
 * Copyright (C) 2016 Jacob Zhitomirsky
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
#include <jtac/printer.hpp>
#include <jtac/assembler.hpp>

using namespace jcc;


TEST_CASE( "Printing JTAC instructions using a printer",
           "[printer][jtac_assembler]" ) {

  jtac::printer p;
  jtac::assembler asem;

  asem.clear ();
  asem.emit_assign_add (jtac::jtac_var (1), jtac::jtac_var(2), jtac::jtac_var (3));
  REQUIRE( p.print_instruction (asem.get_instructions ()[0]) == "t1 = t2 + t3" );

  asem.clear ();
  asem.emit_assign_sub (jtac::jtac_var (1), jtac::jtac_var(2), jtac::jtac_var (3));
  REQUIRE( p.print_instruction (asem.get_instructions ()[0]) == "t1 = t2 - t3" );

  asem.clear ();
  asem.emit_assign_mul (jtac::jtac_var (1), jtac::jtac_var(2), jtac::jtac_var (3));
  REQUIRE( p.print_instruction (asem.get_instructions ()[0]) == "t1 = t2 * t3" );

  asem.clear ();
  asem.emit_assign_div (jtac::jtac_var (1), jtac::jtac_var(2), jtac::jtac_var (3));
  REQUIRE( p.print_instruction (asem.get_instructions ()[0]) == "t1 = t2 / t3" );

  asem.clear ();
  asem.emit_assign_mod (jtac::jtac_var (1), jtac::jtac_var(2), jtac::jtac_var (3));
  REQUIRE( p.print_instruction (asem.get_instructions ()[0]) == "t1 = t2 % t3" );

  asem.clear ();
  asem.emit_cmp (jtac::jtac_var (1), jtac::jtac_var(2));
  REQUIRE( p.print_instruction (asem.get_instructions ()[0]) == "cmp t1, t2" );
}
