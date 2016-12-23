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
#include "assembler/x86_64/assembler.hpp"
#include <utility>


namespace jcc {
  
  namespace x86_64 {

    static inline bool
    _is_ax_register (int code)
    {
      switch (code)
        {
        case REG_AL:
        case REG_AX:
        case REG_EAX:
        case REG_RAX:
          return true;

        default:
          return false;
        }
    }



    assembler::assembler ()
    {
      this->pos = 0;
      this->sz = 0;
      this->next_lbl_id = 1;
    }



    //! \brief Creates and returns a new label.
    label_id
    assembler::make_label ()
    {
      return this->next_lbl_id ++;
    }

    //! \brief Sets the position of the specified label to the current position.
    void
    assembler::mark_label (label_id id)
    {
      this->lbl_fixes[id] = this->pos;
    }

    //! \brief Calls make_label() and mark_label() in succession.
    label_id
    assembler::make_and_mark_label ()
    {
      label_id id = this->make_label ();
      this->mark_label (id);
      return id;
    }

    //! \brief Fixes label references in the generated code.
    void
    assembler::fix_labels ()
    {
      auto ppos = this->pos;

      for (auto itr = this->lbl_uses.begin (); itr != this->lbl_uses.end (); )
        {
          auto& use = *itr;
          auto itr_fix = this->lbl_fixes.find (use.lbl.id);
          if (itr_fix == this->lbl_fixes.end ())
            {
              ++ itr;
              continue;
            }

          size_t fix = itr_fix->second;
          long long disp = (long long)fix - (long long)use.pos + use.add;

          this->pos = use.pos;
          switch (use.lbl.ss)
            {
            case SS_BYTE: this->put_u8 ((unsigned char)(char)disp); break;
            case SS_WORD: this->put_u16 ((unsigned short)(short)disp); break;
            case SS_DWORD: this->put_u32 ((unsigned int)(int)disp); break;
            case SS_QWORD: this->put_u64 ((unsigned long long)disp); break;
            }

          itr = this->lbl_uses.erase (itr);
        }

      this->pos = ppos;
    }

    
    
    void
    assembler::put_u8 (uint8_t v)
    {
      if (this->pos == this->data.size ())
        {
          this->data.push_back (v);
          ++ this->sz;
          ++ this->pos;
        }
      else
        this->data[this->pos ++] = v;
    }
    
    void
    assembler::put_u16 (uint16_t v)
    {
      this->put_u8 (v & 0xFF);
      this->put_u8 (v >> 8);
    }
    
    void
    assembler::put_u32 (uint32_t v)
    {
      this->put_u16 (v & 0xFFFF);
      this->put_u16 (v >> 16);
    }
    
    void
    assembler::put_u64 (uint64_t v)
    {
      this->put_u32 (v & 0xFFFFFFFF);
      this->put_u32 (v >> 32);
    }
    
    
    
    /*!
       Emits the specified instruction onto the underlying buffer.
     */
    void
    assembler::emit (const instruction& ins)
    {
      this->emit_prefixes (ins);
      this->emit_opcode (ins);
      this->check_operands (ins);
      this->emit_operands (ins);
    }
    
    
    
    void
    assembler::emit_prefixes (const instruction& ins)
    {
      // operand-size override prefix
      switch (ins.enc)
        {
        case OP_EN_NP:
        case OP_EN_X:
        case OP_EN_L:
          break;
        
        case OP_EN_RR:
        case OP_EN_RM:
        case OP_EN_OI:
        case OP_EN_RI:
          if (ins.opr1.reg.register_size () == 16)
            this->put_u8 (0x66);
          break;
        
        case OP_EN_MR:
          if (ins.opr2.reg.register_size () == 16)
            this->put_u8 (0x66);
          break;
        
        case OP_EN_MI:
        case OP_EN_M:
          if (ins.opr1.mem.ss == SS_WORD)
            this->put_u8 (0x66);
          break;

        case OP_EN_I:
          if (ins.opr1.imm.ss == SS_WORD)
            this->put_u8 (0x66);
          break;
        }
      
      // address-size override prefix
      switch (ins.enc)
        {
        case OP_EN_RR:
        case OP_EN_OI:
        case OP_EN_RI:
        case OP_EN_NP:
        case OP_EN_X:
        case OP_EN_L:
        case OP_EN_I:
          break;
        
        case OP_EN_MR:
        case OP_EN_MI:
        case OP_EN_M:
          if (ins.opr1.mem.base.register_size () == 32 ||
            ins.opr1.mem.index.register_size () == 32)
            this->put_u8 (0x67);
          break;
          
        case OP_EN_RM:
          if (ins.opr2.mem.base.register_size () == 32 ||
            ins.opr2.mem.index.register_size () == 32)
            this->put_u8 (0x67);
          break;
        }
      
      this->emit_rex_prefix (ins);
    }
    
    void
    assembler::emit_rex_prefix (const instruction& ins)
    {
      unsigned char rex = 0x40;
      
      switch (ins.enc)
        {
        case OP_EN_NP:
        case OP_EN_X:
        case OP_EN_L:
        case OP_EN_I:
        case OP_EN_M:
          break;
        
        case OP_EN_RR:
        case OP_EN_RM:
        case OP_EN_OI:
        case OP_EN_RI:
          if (ins.opr1.reg.register_size () == 64)
            rex |= 8;
          break;
        
        case OP_EN_MR:
          if (ins.opr2.reg.register_size () == 64)
            rex |= 8;
          break;
        
        case OP_EN_MI:
          if (ins.opr1.mem.ss == SS_QWORD)
            rex |= 8;
          break;
        }
      
      if (rex != 0x40)
        this->put_u8 (rex);
    }
    
    
    
    void
    assembler::emit_opcode (const instruction& ins)
    {
      int opc = ins.opcode;
      if (ins.flags & INS_FLAG_MODRM_REG_EXTEND)
        opc = opc & 0xFF;
      
      bool dest_opr8 = false;
      bool src_opr8 = false;
      switch (ins.enc)
        {
        case OP_EN_NP:
        case OP_EN_X:
          break;

        case OP_EN_I:
          if (ins.opr1.imm.ss == SS_BYTE)
            dest_opr8 = true;
          break;

        case OP_EN_L:
          if (ins.opr1.lbl.ss == SS_BYTE)
            dest_opr8 = true;
          break;
        
        case OP_EN_RR:
          if (ins.opr1.reg.register_size () == 8)
            dest_opr8 = true;
          if (ins.opr2.reg.register_size () == 8)
            src_opr8 = true;
            
          if (ins.flags & INS_FLAG_ADD_REG_TO_OPCODE)
            opc += ins.opr1.reg.code & 7;
          break;
        
        case OP_EN_RM:
          if (ins.opr1.reg.register_size () == 8)
            dest_opr8 = true;
          if (ins.opr2.mem.ss == SS_BYTE)
            src_opr8 = true;
            
          if (ins.flags & INS_FLAG_ADD_REG_TO_OPCODE)
            opc += ins.opr1.reg.code & 7;
          break;
        
        case OP_EN_RI:
          if (ins.flags & INS_FLAG_USE_OPCODE2_FOR_AX)
            {
              if (_is_ax_register (ins.opr1.reg.code))
                opc = ins.opcode2;
            }
          if (ins.flags & INS_FLAG_USE_OPCODE3_FOR_IMM8)
            {
              if (ins.opr2.imm.size == 1)
                opc = ins.opcode3;
            }
          
          // fall through
        
        case OP_EN_OI:
          if (ins.opr1.reg.register_size () == 8)
            dest_opr8 = true;
          if (ins.opr2.imm.size == 1)
            src_opr8 = true;
            
          if (ins.flags & INS_FLAG_ADD_REG_TO_OPCODE)
            opc += ins.opr1.reg.code & 7;
          break;
        
        case OP_EN_MR:
          if (ins.opr1.mem.ss == SS_BYTE)
            dest_opr8 = true;
          if (ins.opr2.reg.register_size () == 8)
            src_opr8 = true;
          break;
        
        case OP_EN_MI:
          if (ins.flags & INS_FLAG_USE_OPCODE3_FOR_IMM8)
            if (ins.opr2.imm.size == 1)
              opc = ins.opcode3;
        
          if (ins.opr1.mem.ss == SS_BYTE)
            dest_opr8 = true;
          if (ins.opr2.imm.size == 1)
            src_opr8 = true;
          break;

        case OP_EN_M:
          if (ins.opr1.mem.ss == SS_BYTE)
            dest_opr8 = true;
          break;
        }
      
      if (dest_opr8)
        {
          if (ins.flags & INS_FLAG_DEST_8_USE_OPCODE2)
            opc = ins.opcode2;
          else if (ins.flags & INS_FLAG_DEST_8_MINUS_ONE)
            -- opc;
          else if (ins.flags & INS_FLAG_DEST_8_MINUS_EIGHT)
            opc -= 8;
        }
      else if (src_opr8)
        {
          if (ins.flags & INS_FLAG_SRC_8_MINUS_ONE)
            -- opc;
          else if (ins.flags & INS_FLAG_SRC_8_MINUS_EIGHT)
            opc -= 8;
        }
      
      if (opc >> 8)
        this->put_u8 (opc >> 8);
      this->put_u8 (opc);
    }



    void
    assembler::check_operands (const instruction& ins)
    {
      switch (ins.enc)
        {
        case OP_EN_I:
          if (ins.opr1.imm.ss == SS_QWORD)
            throw invalid_instruction_error ("invalid immediate operand size specifier");
          break;

        case OP_EN_L:
          if (ins.opr1.lbl.ss == SS_WORD || ins.opr1.lbl.ss == SS_QWORD)
            throw invalid_instruction_error ("invalid label size specifier");
          break;

        case OP_EN_M:
          if (ins.opr1.mem.ss == SS_DWORD)
            throw invalid_instruction_error ("memory operand size cannot be a DWORD");
          break;

        default:
          break;
        }
    }

    void
    assembler::emit_operands (const instruction& ins)
    {
      this->emit_modrm_and_sib (ins);
      
      switch (ins.enc)
        {
        case OP_EN_RR:
        case OP_EN_NP:
          break;
        
        case OP_EN_MR:
        case OP_EN_RM:
        case OP_EN_M:
          {
            auto& mem = (ins.enc == OP_EN_RM) ? ins.opr2.mem : ins.opr1.mem;
            if (mem.disp_size > 0)
            {
              if (mem.base.code == REG_NONE && mem.index.code == REG_NONE)
                {
                  this->put_u32 (mem.disp);
                  break;
                }
              
              switch (ins.opr1.mem.disp_size)
                {
                case 1: this->put_u8 (mem.disp); break;
                
                default:
                  this->put_u32 (mem.disp);
                  break;
                }
            }
          }
          break;
        
        case OP_EN_OI:
        case OP_EN_RI:
          {
            int sz = ins.opr2.imm.size;
            if (sz == -1)
              {
                switch (ins.opr1.reg.register_size ())
                  {
                  case 8: sz = 1; break;
                  case 16: sz = 2; break;
                  case 32: sz = 4; break;
                  case 64: sz = 8; break;
                  }
              }
            
            switch (sz)
              {
              case 1: this->put_u8 (ins.opr2.imm.val); break;
              case 2: this->put_u16 (ins.opr2.imm.val); break;
              case 4: this->put_u32 (ins.opr2.imm.val); break;
              case 8:
                if (ins.flags & INS_FLAG_IMM_MAX_32)
                  this->put_u32 (ins.opr2.imm.val);
                else
                  this->put_u64 (ins.opr2.imm.val);
                break;
              }
          }
          break;
        
        case OP_EN_MI:
          switch (ins.opr1.mem.ss)
            {
            case SS_BYTE: this->put_u8 (ins.opr2.imm.val); break;
            case SS_WORD: this->put_u16 (ins.opr2.imm.val); break;
            case SS_DWORD: this->put_u32 (ins.opr2.imm.val); break;
            case SS_QWORD:
              if (ins.flags & INS_FLAG_IMM_MAX_32)
                this->put_u32 (ins.opr2.imm.val);
              else
                this->put_u64 (ins.opr2.imm.val);
              break;
            }
          break;

        case OP_EN_X:
          {
            relocation reloc;
            reloc.type = R_PC32;
            reloc.sym = ins.opr1.rel.sym;
            reloc.offset = this->pos;
            reloc.size = 4;
            reloc.add = -4;
            this->relocs.push_back (reloc);
            this->put_u32 (0);
          }
          break;

        case OP_EN_L:
          {
            if (ins.opr1.lbl.fixed)
              {
                switch (ins.opr1.lbl.ss)
                  {
                  case SS_BYTE: this->put_u8 ((unsigned char)(char)ins.opr1.lbl.val); break;
                  case SS_WORD: this->put_u16 ((unsigned short)(short)ins.opr1.lbl.val); break;
                  case SS_DWORD: this->put_u32 ((unsigned int)(int)ins.opr1.lbl.val); break;
                  case SS_QWORD: this->put_u64 ((unsigned long long)ins.opr1.lbl.val); break;
                  }
              }
            else
              {
                label_use use;
                use.lbl = ins.opr1.lbl;
                use.pos = this->pos;

                switch (use.lbl.ss)
                  {
                    case SS_BYTE: use.add = -1; this->put_u8 (0); break;
                    case SS_WORD: use.add = -2; this->put_u16 (0); break;
                    case SS_DWORD: use.add = -4; this->put_u32 (0); break;
                    case SS_QWORD: use.add = -8; this->put_u64 (0); break;
                  }

                this->lbl_uses.push_back (use);
              }
          }
          break;

        case OP_EN_I:
          switch (ins.opr1.imm.ss)
            {
              case SS_BYTE: this->put_u8 (ins.opr1.imm.val); break;
              case SS_WORD: this->put_u16 (ins.opr1.imm.val); break;
              case SS_DWORD: this->put_u32 (ins.opr1.imm.val); break;
              case SS_QWORD:
                if (ins.flags & INS_FLAG_IMM_MAX_32)
                  this->put_u32 (ins.opr1.imm.val);
                else
                  this->put_u64 (ins.opr1.imm.val);
              break;
            }
          break;
        }
    }
    
    
    
    static std::pair<int, int>
    _make_modrm_and_sib (const instruction& ins, const mem_t& mem, const reg_t& reg)
    {
      int rc = reg.code & 7;
      if (reg.code == REG_NONE || reg.code == REG_RIP)
        rc = 0;
      if (ins.flags & INS_FLAG_MODRM_REG_EXTEND)
        rc = (ins.opcode >> 8) & 7;

      if (mem.base.code == REG_RIP)
        {
          if (mem.scale != 1)
            throw invalid_instruction_error ("RIP register cannot be scaled");
          return std::make_pair ((rc << 3) | 5, (4 << 3) | 5);
        }

      if (mem.disp_size > 0 && mem.base.code == REG_NONE
        && mem.index.code == REG_NONE)
        {
          int sib = (4 << 3) | 5;
          return std::make_pair ((rc << 3) | 4, sib);
        }
    
      int mod = 0;
      if (mem.disp_size == 1)
        mod = 1;
      else if (mem.disp_size >= 4)
        mod = 2;
      
      if (mem.index.code != REG_NONE)
        {
          int sib = 0;
          switch (mem.scale)
            {
            case 1: break;
            case 2: sib |= (1 << 6); break;
            case 4: sib |= (2 << 6); break;
            case 8: sib |= (3 << 6); break;

            default:
              throw invalid_instruction_error ("invalid index register scale");
            }
          
          sib |= (mem.index.code & 7) << 3;
          sib |= mem.base.code & 7;
          
          return std::make_pair ((mod << 6) | (rc << 3) | 4, sib);
        }
      
      return std::make_pair (
        (mod << 6) | (rc << 3) | (mem.base.code & 7), 0);
    }
    
    void
    assembler::emit_modrm_and_sib (const instruction& ins)
    {
      switch (ins.enc)
        {
        case OP_EN_OI:
        case OP_EN_NP:
        case OP_EN_X:
        case OP_EN_L:
        case OP_EN_I:
          break;
        
        case OP_EN_RR:
          this->put_u8 ((3 << 6) | ((ins.opr2.reg.code & 7) << 3) |
            (ins.opr1.reg.code & 7));
          break;
        
        case OP_EN_MR:
          {
            auto ms = _make_modrm_and_sib (ins, ins.opr1.mem, ins.opr2.reg);
            this->put_u8 (ms.first);
            if ((ms.first & 7) == 4)
              // need sib
              this->put_u8 (ms.second);
          }
          break;
        
        case OP_EN_RM:
          {
            auto ms = _make_modrm_and_sib (ins, ins.opr2.mem, ins.opr1.reg);
            this->put_u8 (ms.first);
            if ((ms.first & 7) == 4)
              // need sib
              this->put_u8 (ms.second);
          }
          break;
        
        case OP_EN_MI:
        case OP_EN_M:
          {
            auto ms = _make_modrm_and_sib (ins, ins.opr1.mem, reg_t (REG_NONE));
            this->put_u8 (ms.first);
            if ((ms.first & 7) == 4)
              // need sib
              this->put_u8 (ms.second);
          }
          break;
        
        case OP_EN_RI:
          if (ins.flags & INS_FLAG_USE_OPCODE2_FOR_AX)
            if (_is_ax_register (ins.opr1.reg.code))
              if (!((ins.flags & INS_FLAG_USE_OPCODE3_FOR_IMM8) && (ins.opr2.imm.size == 1)))
                break;
          
          this->put_u8 ((3 << 6) | (ins.opr1.reg.code & 7));
          break;
        }
    }
  }
}

