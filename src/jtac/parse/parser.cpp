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

#include "jtac/parse/parser.hpp"


namespace jcc {
namespace jtac {

  parser::parser (token_stream& toks)
      : toks (toks)
  {
    this->curr_proc = nullptr;
  }


  //! \brief Returns a token of the specified type or raises an exception.
  token
  parser::expect (token_type type)
  {
    if (!this->toks.has_next ())
      throw parse_error ("unexpected eof, expected " + token_str ({ .type = type }),
                         this->toks.peek_prev ().pos);
    auto tok = this->toks.next ();
    if (tok.type != type)
      throw parse_error ("expected '" + token_str ({ .type = type })
                         + "', got '" + token_str (tok) + "'", tok.pos);
    return tok;
  }

  //! \brief Raises an exception in case of EOF.
  void
  parser::check_eof ()
  {
    if (!this->toks.has_next ())
      throw parse_error ("unexpected eof", this->toks.peek_prev ().pos);
  }



  jtac_tagged_operand
  parser::parse_operand ()
  {
    jtac_tagged_operand opr;

    this->check_eof ();
    auto tok = this->toks.next ();

    switch (tok.type)
      {
      case JTAC_TOK_NAME:
        {
          std::string name = tok.val.str;
          if (name[0] == '.')
            {
              // label
              auto itr = this->label_map.find (name);
              if (itr == this->label_map.end ())
                {
                  auto lbl = this->asem.make_label ();
                  this->label_map[name] = lbl;
                  opr = jtac_label (lbl);
                }
              else
                opr = jtac_label (itr->second);

              return opr;
            }
          else
            {
              // variable
              if (this->curr_proc->has_var_name (name))
                opr = jtac_var (this->curr_proc->get_var_name_id (name));
              else
                {
                  this->curr_proc->map_var_name (name, this->next_var_id);
                  opr = jtac_var (this->next_var_id++);
                }

              return opr;
            }
        }

      case JTAC_TOK_INTEGER:
        opr = jtac_const (tok.val.i64);
        return opr;

      default:
        throw parse_error ("expected operand", tok.pos);
      }
  }

  jtac_tagged_operand
  parser::parse_name_operand ()
  {
    this->check_eof ();
    auto tok = this->toks.next ();
    if (tok.type != JTAC_TOK_NAME)
      throw parse_error ("expected name", tok.pos);

    jtac_tagged_operand opr;

    auto name = tok.val.str;
    if (this->prog.has_name (name))
      opr = jtac_name (this->prog.get_name_id (name));
    else
      {
        this->prog.map_name (name, this->next_name_id);
        opr = jtac_name (this->next_name_id ++);
      }

    return opr;
  }

  void
  parser::parse_assign_instruction ()
  {
    auto ftok = this->toks.peek_next ();
    auto dest = this->parse_operand ();

    this->expect (JTAC_TOK_ASSIGN);
    if (dest.type != JTAC_OPR_VAR)
      throw parse_error ("expected variable in left-hand side of assignment", ftok.pos);

    this->check_eof ();
    auto tok = this->toks.peek_next ();
    if (tok.type == JTAC_TOK_CALL)
      {
        this->toks.next ();

        auto name = this->parse_name_operand ();
        auto& inst = this->asem.emit_assign_call (
            tagged_operand_to_operand (dest),
            tagged_operand_to_operand (name));

        this->expect (JTAC_TOK_LPAREN);
        while (this->toks.has_next () && this->toks.peek_next ().type != JTAC_TOK_RPAREN)
          {
            auto arg = this->parse_operand ();
            inst.push_extra (tagged_operand_to_operand (arg));

            this->check_eof ();
            tok = this->toks.peek_next ();
            if (tok.type == JTAC_TOK_COMMA)
              this->toks.next ();
            else if (tok.type != JTAC_TOK_RPAREN)
              throw parse_error ("expected ',' or ')' in argument list", tok.pos);
          }

        this->expect (JTAC_TOK_RPAREN);
        return;
      }

    auto lhs = this->parse_operand ();
    this->check_eof ();

#define ASSIGN_2OPR(NAME, TOK)            \
  case TOK:                               \
  {                                       \
    this->toks.next ();                   \
    auto rhs = this->parse_operand ();    \
    this->asem.emit_assign_##NAME (       \
        tagged_operand_to_operand (dest), \
        tagged_operand_to_operand (lhs),  \
        tagged_operand_to_operand (rhs)); \
  }                                       \
  break;

    auto op_tok = this->toks.peek_next ();
    switch (op_tok.type)
      {
      ASSIGN_2OPR(add, JTAC_TOK_ADD)
      ASSIGN_2OPR(sub, JTAC_TOK_SUB)
      ASSIGN_2OPR(mul, JTAC_TOK_MUL)
      ASSIGN_2OPR(div, JTAC_TOK_DIV)
      ASSIGN_2OPR(mod, JTAC_TOK_MOD)

      default: // a = b
        {
          jtac_tagged_operand opr1a;
          jtac_tagged_operand opr2a;

          opr1a = tagged_operand_to_operand (dest);
          opr2a = tagged_operand_to_operand (lhs);
        }
        this->asem.emit_assign (tagged_operand_to_operand (dest),
                                tagged_operand_to_operand (lhs));
        break;
    }
  }

  void
  parser::parse_instruction ()
  {
    this->check_eof ();

    // check for label definitions
    auto tok = this->toks.peek_next ();
    if (tok.type == JTAC_TOK_NAME && tok.val.str[0] == '.')
      {
        this->toks.next ();
        this->expect (JTAC_TOK_COL);

        std::string name = tok.val.str;
        auto itr = this->label_map.find (name);
        if (itr == this->label_map.end ())
          this->label_map[name] = this->asem.make_and_mark_label ();
        else
          this->asem.mark_label (itr->second);

        return;
      }

#define INST_1OPR(NAME, TOK)                                  \
  case TOK:                                                   \
  {                                                           \
    this->toks.next ();                                       \
    auto opr = this->parse_operand ();                        \
    this->asem.emit_##NAME (tagged_operand_to_operand (opr)); \
  }                                                           \
  break;

#define INST_2OPR(NAME, TOK)                                    \
  case TOK:                                                     \
  {                                                             \
    this->toks.next ();                                         \
    auto opr1 = this->parse_operand ();                         \
    this->expect (JTAC_TOK_COMMA);                              \
    auto opr2 = this->parse_operand ();                         \
    this->asem.emit_##NAME (tagged_operand_to_operand (opr1),   \
                            tagged_operand_to_operand (opr2));  \
  }                                                             \
  break;

#define INST_1LBL(NAME, TOK)                                  \
  case TOK:                                                   \
  {                                                           \
    this->toks.next ();                                       \
    tok = this->toks.peek_next ();                            \
                                                              \
    auto opr = this->parse_operand ();                        \
    if (opr.type != JTAC_OPR_LABEL)                           \
      throw parse_error ("expected label operand", tok.pos);  \
                                                              \
    this->asem.emit_##NAME (opr.val.lbl);                     \
  }                                                           \
  break;

    tok = this->toks.peek_next ();
    switch (tok.type)
      {
      INST_1LBL(jmp, JTAC_TOK_JMP)
      INST_1LBL(je, JTAC_TOK_JE)
      INST_1LBL(jne, JTAC_TOK_JNE)
      INST_1LBL(jl, JTAC_TOK_JL)
      INST_1LBL(jle, JTAC_TOK_JLE)
      INST_1LBL(jg, JTAC_TOK_JG)
      INST_1LBL(jge, JTAC_TOK_JGE)
      INST_1OPR(ret, JTAC_TOK_RET)
      INST_2OPR(cmp, JTAC_TOK_CMP)

      case JTAC_TOK_CALL:
        {
          this->toks.next ();

          auto name = this->parse_name_operand ();
          auto& inst = this->asem.emit_call (
              tagged_operand_to_operand (name));

          this->expect (JTAC_TOK_LPAREN);
          while (this->toks.has_next () && this->toks.peek_next ().type != JTAC_TOK_RPAREN)
            {
              auto arg = this->parse_operand ();
              inst.push_extra (tagged_operand_to_operand (arg));

              this->check_eof ();
              tok = this->toks.peek_next ();
              if (tok.type == JTAC_TOK_COMMA)
                this->toks.next ();
              else if (tok.type != JTAC_TOK_RPAREN)
                throw parse_error ("expected ',' or ')' in argument list", tok.pos);
            }

          this->expect (JTAC_TOK_RPAREN);
        }
        break;

      default:
        return this->parse_assign_instruction ();
      }

#undef INST_1LBL
#undef INST_1OPR
  }

  void
  parser::parse_proc ()
  {
    this->toks.next (); // skip proc

    // procedure name
    if (!this->toks.has_next ())
      throw parse_error ("unexpected eof, expected procedure name",
                         this->toks.peek_prev ().pos);
    auto tok = this->toks.next ();
    if (tok.type != JTAC_TOK_NAME)
      throw parse_error ("expected name after 'proc'", tok.pos);
    std::string name = tok.val.str;

    // procedure parameter list
    std::vector<token> params;
    this->expect (JTAC_TOK_LPAREN);
    while (this->toks.has_next () && this->toks.peek_next ().type != JTAC_TOK_RPAREN)
      {
        tok = this->toks.next ();
        if (tok.type != JTAC_TOK_NAME)
          throw parse_error ("expected name", tok.pos);

        params.push_back (tok);

        tok = this->toks.peek_next ();
        if (tok.type == JTAC_TOK_COMMA)
          this->toks.next ();
        else if (tok.type != JTAC_TOK_RPAREN)
          throw parse_error ("expected ',' or ')' in procedure parameter list", tok.pos);
      }

    this->expect (JTAC_TOK_RPAREN);
    this->expect (JTAC_TOK_COL);

    this->asem.clear ();
    auto& proc = this->prog.emplace_procedure (name);
    this->curr_proc = &proc;
    this->next_var_id = 1;
    this->next_name_id = 1;
    this->label_map.clear ();

    // map parameters into variables
    for (auto param : params)
      {
        if (proc.has_var_name (param.val.str))
          throw parse_error ("procedure parameter specified twice", tok.pos);
        proc.map_var_name (param.val.str, this->next_var_id++);
      }

    // procedure body
    while (this->toks.has_next () && this->toks.peek_next ().type != JTAC_TOK_ENDPROC)
      {
        this->parse_instruction ();
      }

    this->expect (JTAC_TOK_ENDPROC);
    this->curr_proc = nullptr;

    this->asem.fix_labels ();
    auto& insts = this->asem.get_instructions ();
    proc.insert_instructions (insts.begin (), insts.end ());
  }

  void
  parser::parse_top_level ()
  {
    while (this->toks.has_next ())
      {
        auto tok = this->toks.peek_next ();
        switch (tok.type)
          {
          case JTAC_TOK_PROC:
            this->parse_proc ();
            break;

          default:
            throw parse_error ("unexpected top-level token: " + token_str (tok), tok.pos);
          }
      }
  }

  /*!
   \brief Parses the underlying token stream and returns a JTAC program.
  */
  program
  parser::parse ()
  {
    this->parse_top_level ();
    return std::move (this->prog);
  }
}
}
