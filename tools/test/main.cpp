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

#include <iostream>
#include <fstream>
#include <jtac/parse/lexer.hpp>
#include <jtac/parse/parser.hpp>
#include <jtac/control_flow.hpp>
#include <jtac/printer.hpp>


int
main (int argc, char *argv[])
{
  if (argc < 2)
    { std::cerr << "usage: " << argv[0] << " <JTAC file>" << std::endl; return -1; }

  std::ifstream fs (argv[1]);
  if (!fs)
    { std::cerr << "Failed to open file." << std::endl; return -1; }

  jcc::jtac::lexer lexer (fs);
  auto toks = lexer.tokenize ();
  jcc::jtac::parser parser (toks);

  jcc::jtac::program prog;
  try
    {
      prog = parser.parse ();
    }
  catch (const jcc::jtac::parse_error& ex)
    {
      auto pos = ex.get_pos ();
      std::cout << "Parse error:" << pos.ln << ":" << pos.col << ": " << ex.what () << std::endl;
      return -1;
    }

  std::cout << "Parsed.\n" << std::endl;

  for (auto& proc : prog.get_procedures ())
    {
      std::cout << "Procedure " << proc.get_name () << std::endl;
      std::cout << std::string (10 + proc.get_name ().length (), '=') << std::endl;

      auto cfg = jcc::jtac::control_flow_analyzer::make_cfg (proc.get_body ());
      for (auto& blk : cfg.get_blocks ())
        {
          jcc::jtac::printer printer;
          std::cout << printer.print_basic_block (*blk) << std::endl << std::endl;
        }
    }

  return 0;
}
