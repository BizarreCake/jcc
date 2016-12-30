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

#ifndef _JCC__JTAC__JTAC__H_
#define _JCC__JTAC__JTAC__H_

#include <cstdint>
#include <bits/move.h>


namespace jcc {

  /*!
     \enum jtac_opcode
     \brief Enumeration of JTAC instruction opcodes.
   */
  enum jtac_opcode
  {
    JTAC_OP_UNDEF = 0xFFFF,

    JTAC_OP_ASSIGN_ADD = 0,   // t1 = t2 + t3
    JTAC_OP_ASSIGN_SUB,       // t1 = t2 - t3
    JTAC_OP_ASSIGN_MUL,       // t1 = t2 * t3
    JTAC_OP_ASSIGN_DIV,       // t1 = t2 / t3
    JTAC_OP_ASSIGN_MOD,       // t1 = t2 % t3
    JTAC_OP_ASSIGN_CALL,      // t1 = call proc (params...)
    JTAC_OP_CMP,              // cmp t1, t2
    JTAC_OP_JMP,              // jmp lbl
    JTAC_OP_JE,               // je lbl
    JTAC_OP_JNE,              // jne lbl
    JTAC_OP_JL,               // jl lbl
    JTAC_OP_JLE,              // jle lbl
    JTAC_OP_JG,               // jg lbl
    JTAC_OP_JGE,              // jge lbl
    JTAC_OP_RET,              // ret t1
  };


  /*!
     \enum jtac_operand_type
     \brief Enumeration of possible instruction operand types.
   */
  enum jtac_operand_type
  {
    JTAC_OPR_CONST,  // constant
    JTAC_OPR_VAR,    // variable
    JTAC_OPR_LABEL,  // label
    JTAC_OPR_OFFSET, // offset
  };


  /*!
     \class jtac_operand
     \brief Base class for JTAC operands.
   */
  class jtac_operand
  {
   public:
    //! \brief Returns the type of this operand.
    virtual jtac_operand_type get_type () const = 0;
  };


  /*!
     \class jtac_const
     \brief Constant operand.
   */
  class jtac_const: public jtac_operand
  {
    int64_t val;

   public:
    inline int64_t get_value () const { return this->val; }
    inline void set_value (int64_t val) { this->val = val; }

   public:
    jtac_const ()
        : val (0)
    { }

    jtac_const (int64_t val)
        : val (val)
    { }

   public:
    virtual jtac_operand_type get_type () const override { return JTAC_OPR_CONST; }
  };


  //! \brief Variable identifier.
  using jtac_var_id = int;

  /*!
     \class jtac_var
     \brief Variable operand.
   */
  class jtac_var: public jtac_operand
  {
    jtac_var_id id;

   public:
    inline jtac_var_id get_id () const { return this->id; }
    inline void set_id (jtac_var_id id) { this->id = id; }

   public:
    jtac_var ()
        : id (0)
    { }

    jtac_var (jtac_var_id id)
        : id (id)
    { }

   public:
    virtual jtac_operand_type get_type () const override { return JTAC_OPR_VAR; }
  };


  //! \brief Label identifier.
  using jtac_label_id = int;

  /*!
     \class jtac_label
     \brief Label operand (used in branch instructions).
   */
  class jtac_label: public jtac_operand
  {
    jtac_label_id id;

   public:
    inline jtac_label_id get_id () const { return this->id; }
    inline void set_id (jtac_label_id id) { this->id = id; }

   public:
    jtac_label ()
        : id (0)
    { }

    jtac_label (jtac_label_id id)
    : id (id)
    { }

   public:
    virtual jtac_operand_type get_type () const override { return JTAC_OPR_LABEL; }
  };


  /*!
     \struct jtac_offset
     \brief Constant displacement operand.
   */
  class jtac_offset: public jtac_operand
  {
    int off;

   public:
    inline int get_offset () const { return this->off; }
    inline void set_offset (int off) { this->off = off; }

   public:
    jtac_offset ()
        : off (0)
    { }

    jtac_offset (int off)
        : off (off)
    { }

   public:
    virtual jtac_operand_type get_type () const override { return JTAC_OPR_OFFSET; }
  };


  /*!
     \struct jtac_tagged_operand
     \brief Stores a union of all possible operand types along with the type
            of the actual operand.
   */
  struct jtac_tagged_operand
  {
    jtac_operand_type type;
    union jtac_tagged_operand_value
    {
      jtac_const konst;
      jtac_var var;
      jtac_label lbl;
      jtac_offset off;

      jtac_tagged_operand_value ()
          : konst (0)
      { }
    } val;

   public:
    jtac_tagged_operand ()
    { }

    jtac_tagged_operand (const jtac_tagged_operand& other)
        : type (other.type)
    {
      switch (this->type)
        {
          case JTAC_OPR_CONST: this->val.konst = other.val.konst; break;
          case JTAC_OPR_VAR: this->val.var = other.val.var; break;
          case JTAC_OPR_LABEL: this->val.lbl = other.val.lbl; break;
          case JTAC_OPR_OFFSET: this->val.off = other.val.off; break;
        }
    }

    jtac_tagged_operand (jtac_tagged_operand&& other)
        : type (other.type)
    {
      switch (this->type)
        {
        case JTAC_OPR_CONST: this->val.konst = std::move (other.val.konst); break;
        case JTAC_OPR_VAR: this->val.var = std::move (other.val.var); break;
        case JTAC_OPR_LABEL: this->val.lbl = std::move (other.val.lbl); break;
        case JTAC_OPR_OFFSET: this->val.off = std::move (other.val.off); break;
        }
    }
  };


  /*!
     \struct jtac_instruction
     \brief Stores a single JTAC instruction.
   */
  struct jtac_instruction
  {
    jtac_opcode op;
    jtac_tagged_operand oprs[3];
    struct
    {
      unsigned char count;
      jtac_tagged_operand *oprs;
    } extra;
  };
}

#endif //_JCC__JTAC__JTAC__H_
