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
#include <jtac/printer.hpp>
#include <jtac/assembler.hpp>
#include <jtac/control_flow.hpp>
#include <jtac/data_flow.hpp>
#include <jtac/ssa.hpp>
#include <iostream>


using namespace jcc;


TEST_CASE( "Transforming a CFG into maximal SSA form",
           "[printer][jtac_assembler][control_flow][ssa]" ) {

  using namespace jcc::jtac;
  assembler asem;
  printer p;

  asem.emit_assign (jtac_var (1), jtac_const (5));
  asem.emit_assign (jtac_var (2), jtac_const (7));
  asem.emit_assign_add (jtac_var (3), jtac_var(1), jtac_var(2));

  int lbl_else = asem.make_label ();
  asem.emit_cmp (jtac_var (3), jtac_const (8));
  asem.emit_jle (jtac_label (lbl_else));

  asem.emit_assign_add (jtac_var (3), jtac_var (3), jtac_const (3));
  int lbl_end = asem.make_label ();
  asem.emit_jmp (jtac_label (lbl_end));

  asem.mark_label (lbl_else);
  asem.emit_assign_mul (jtac_var (3), jtac_var (3), jtac_const (2));

  asem.mark_label (lbl_end);
  asem.emit_assign (jtac_var (4), jtac_const (1));
  asem.emit_assign_add (jtac_var (5), jtac_var (3), jtac_var (4));

  asem.fix_labels ();

  // create control flow graph
  auto cfg = control_flow_analyzer::make_cfg (asem.get_instructions ());

  ssa_builder ssab;
  ssab.transform (cfg);

  //
  // print blocks
  //

  auto str = p.print_basic_block (*cfg.find_block (1));
  REQUIRE( str == "Basic Block #1\n"
      "--------------\n"
      "0: t1_1 = 5\n"
      "1: t2_1 = 7\n"
      "2: t3_1 = t1_1 + t2_1\n"
      "3: cmp t3_1, 8\n"
      "4: jle 7\n"
      "--------------\n"
      "Prev: none\n"
      "Next: #3 #2" );

  str = p.print_basic_block (*cfg.find_block (2));
  REQUIRE( str == "Basic Block #2\n"
      "--------------\n"
      "5: t3_4 = t3_1 + 3\n"
      "6: jmp 8\n"
      "--------------\n"
      "Prev: #1\n"
      "Next: #4" );

  str = p.print_basic_block (*cfg.find_block (3));
  REQUIRE( str == "Basic Block #3\n"
      "--------------\n"
      "7: t3_3 = t3_1 * 2\n"
      "--------------\n"
      "Prev: #1\n"
      "Next: #4" );

  str = p.print_basic_block (*cfg.find_block (4));
  REQUIRE( str == "Basic Block #4\n"
      "--------------\n"
      "8: t3_2 = phi(t3_3, t3_4)\n"
      "9: t4_1 = 1\n"
      "10: t5_1 = t3_2 + t4_1\n"
      "--------------\n"
      "Prev: #3 #2\n"
      "Next: none" );
}
