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

#ifndef _JCC__JTAC__JTAC__H_
#define _JCC__JTAC__JTAC__H_

#include <cstdint>
#include <bits/move.h>


namespace jcc {
namespace jtac {

  /*!
     \enum jtac_opcode
     \brief Enumeration of JTAC instruction opcodes.
   */
  enum jtac_opcode
  {
    JTAC_OP_UNDEF = 0xFFFF,

    JTAC_OP_ASSIGN = 0,       // t1 = t2
    JTAC_OP_ASSIGN_ADD,       // t1 = t2 + t3
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

    // special instructions:
    JTAC_SOP_ASSIGN_PHI,      // t1 = phi(t2, t3, ...)
  };

  //! \brief Returns true if the opcode described an instruction of the form: X = Y.
  bool is_opcode_assign (jtac_opcode op);


  enum jtac_opcode_class
  {
    JTAC_OPC_NONE,
    JTAC_OPC_ASSIGN2,           // x = y
    JTAC_OPC_ASSIGN3,           // x = y op z
    JTAC_OPC_USE1,              // op x
    JTAC_OPC_USE2,              // op x, y
    JTAC_OPC_ASSIGN_CALL,       // x = y(oprs...)
    JTAC_OPC_ASSIGN_FIXED_CALL, // x = FIXED(oprs...)
  };

  //! \brief Returns the class of the specified opcode.
  jtac_opcode_class get_opcode_class (jtac_opcode op);

  //! \brief Returns the number of operands used by the specified opcode.
  int get_operand_count (jtac_opcode op);

  //! \brief Checks whether the specified opcode requires extra operands.
  bool has_extra_operands (jtac_opcode op);



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

  inline jtac_var_id
  make_var_id (int base, int subscript = 0)
  { return base | (subscript << 16); }

  inline int var_base (jtac_var_id id) { return id & 0xFFFF; }
  inline int var_subscript (jtac_var_id id) { return id >> 16; }

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
    { *this = other; }

    jtac_tagged_operand (jtac_tagged_operand&& other)
    { *this = std::move (other); }

   public:
    jtac_tagged_operand& operator= (const jtac_tagged_operand& other);
    jtac_tagged_operand& operator= (jtac_tagged_operand&& other);
    jtac_tagged_operand& operator= (const jtac_operand& opr);
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
      unsigned char cap;
      jtac_tagged_operand *oprs;
    } extra;

   public:
    jtac_instruction ();
    jtac_instruction (const jtac_instruction& other);
    jtac_instruction (jtac_instruction&& other);

   public:
    //! \brief Inserts the specified operand into the instruction's "extra" list.
    jtac_instruction& push_extra (const jtac_operand& opr);

   public:
    jtac_instruction& operator= (const jtac_instruction& other);
    jtac_instruction& operator= (jtac_instruction&& other);
  };
}
}

#endif //_JCC__JTAC__JTAC__H_
