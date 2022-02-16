/*****************************************************************************
 ________          ___ ________  ________          ________  ________  ___  ___     
|\_____  \        /  /|\   __  \|\   __  \        |\   ____\|\   __  \|\  \|\  \    
 \|___/  /|      /  //\ \  \|\  \ \  \|\  \       \ \  \___|\ \  \|\  \ \  \\\  \   
     /  / /     /  //  \ \   __  \ \  \\\  \       \ \  \    \ \   ____\ \  \\\  \  
    /  /_/__   /  //    \ \  \|\  \ \  \\\  \       \ \  \____\ \  \___|\ \  \\\  \ 
   |\________\/_ //      \ \_______\ \_______\       \ \_______\ \__\    \ \_______\
    \|_______|__|/        \|_______|\|_______|        \|_______|\|__|     \|_______|
                                                                                    
MIT License

Copyright (c) 2022 Mike Sharkey

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
******************************************************************************/

#include <sim80vm_z80a.h>
#include <stdio.h>

#define inherited sim80vm_i8080

sim80vm_z80a::sim80vm_z80a(sim80mem* m,sim80io* io)
 : sim80vm_i8080(m,io)
 , index(reg_HL)
 , _f(0)
 , N(0)
{
}


sim80vm_z80a::~sim80vm_z80a()
{
}

void sim80vm_z80a::run2(void)
{
	inherited::run2();
	f = (S<<5 | Z<<4 | AC<<3 | P<<2 | N<<1 | CY);
	index = reg_HL; /** make sure we reset default index register addressing mode! */
}

void sim80vm_z80a::rstINT()
{
	/** FIXME: I don't think INT is quite exactly like RST7? */
	rst7();
}

void sim80vm_z80a::rstNMI()
{
	/** FIXME: I don't think INT is quite exactly like RST7? */
	rst7();
}

uint16_t sim80vm_z80a::getIndex(int8_t displacement)
{
	return getIndex() + displacement;
}

uint16_t sim80vm_z80a::getIndex()
{
	switch ( index ) {
		case reg_IX: return ix; break;
		case reg_IY: return iy; break;
		default:
			return inherited::getIndex();
			break;
	}
}


void sim80vm_z80a::putIndex(uint16_t i)
{
	switch ( index ) {
		case reg_IX: ix=i; break;
		case reg_IY: iy=i; break;
		case reg_HL: 
			inherited::putIndex(i);
			break;
	}
}

/** 
 * @brief Get value from an 8-bit register
 */
uint8_t sim80vm_z80a::reg8get(uint8_t i,uint8_t disp)
{ 
	uint8_t rc=0xff;
	if (i==0x06)
	{
		switch ( index ) {
			case reg_HL: rc=mem()->get((uint16_t)((h<<8)|l)); break;
			case reg_IX: rc=mem()->get(ix+disp); break;
			case reg_IY: rc=mem()->get(iy+disp); break;
		}
	}
	else
	{
		rc=inherited::reg8get(i,disp);
	}
	return rc;
}

/** 
 * @brief Set value to an 8-bit register
 */
uint8_t sim80vm_z80a::reg8put(uint8_t i,uint8_t v,uint8_t disp)
{
	if (i==0x06)
	{
		switch ( index ) {
			case reg_HL: mem()->put((uint16_t)((h<<8)|l),v); break;
			case reg_IX: mem()->put(ix+disp,v); break;
			case reg_IY: mem()->put(iy+disp,v); break;
		}
	}
	else 
	{
		inherited::reg8put(i,v,disp);
	}
	return v;
}

void sim80vm_z80a::op_flow()
{
	switch(opcode())
	{
		/** special opcode()s */
		
		case 0x10:					/** djnz #disp */
			if (--b) pc += (int8_t)(imm8get());
			++pc;
			break;
			
		case 0x18:					/** jr #disp */
			pc += (int8_t)(imm8get());
			++pc;
			break;
			
		case 0x20:					/** jr	nz,#disp */
			if (!Z) pc += (int8_t)(imm8get());
			++pc;
			break;
			
		case 0x28:					/** jr	z,#disp */
			if (Z) pc += (int8_t)(imm8get());
			++pc;
			break;
			
		case 0x30:					/** jr	nc,#disp */
			if (!CY) pc += (int8_t)(imm8get());
			++pc;
			break;
			
		case 0x38:					/** jr	c,#disp */
			if (CY) pc += (int8_t)(imm8get());
			++pc;
			break;

		default:
			inherited::op_flow();
			break;
	}
}

void sim80vm_z80a::op_bits()
{
	opcode_set(imm8get()); 			/** skip lead-in (0xCB) part of opcode() and get the Z/80 opcode() */
	uint8_t disp=(index==reg_HL?0:imm8get()); 	/** displacement added to index IX,IY registers */
	switch(opcode()>>6) {
		case 0x00:							/** rotate... */
			switch(opcode()>>3) {
				case 0x00:					/** rlc reg8 */
				{
					uint8_t reg = reg8get(opcode()&0x7,disp);
					CY = ((reg)&0x80)>>7;	/** carry = msb */
					reg = reg << 1;			/** shift left one bit position */
					reg |= CY&1;			/** rotate msb to lsb position */
					AC=0;					/** set program status flags... */
					zero(reg);
					sign(reg);
					parity(reg);
					reg8put(opcode()&0x7,reg,disp);
				}
				break;
				
				case 0x01:					/** rrc reg8 */
				{
					uint8_t reg = reg8get(opcode()&0x7,disp);
					CY = (reg)&1;			/** carry = lsb */
					reg = reg >> 1;			/** shift right one bit position */
					reg |= CY&1;			/** rotate lsb to msb position */
					AC=0;					/** set program status flags... */
					zero(reg);
					sign(reg);
					parity(reg);
					reg8put(opcode()&0x7,reg,disp);
				}
				break;
				
				case 0x02:					/** rl reg8 */
				{
					uint8_t reg = reg8get(opcode()&0x7,disp);
					uint8_t t=CY;			/** save current carry flag */
					CY = ((reg)&0x80)>>7;	/** carry = msb */
					reg = reg << 1;			/** shift left one bit position */
					reg |= t&1;				/** rotate previous CY to lsb position */
					AC=0;					/** set program status flags... */
					zero(reg);
					sign(reg);
					parity(reg);
					reg8put(opcode()&0x7,reg,disp);
				}
				break;
				
				case 0x03:					/** rr reg8 */
				{
					uint8_t reg = reg8get(opcode()&0x7,disp);
					uint8_t t=CY;			/** save current carry flag */
					CY = (reg)&1;			/** carry = lsb */
					reg = reg >> 1;			/** shift right one bit position */
					reg |= t&1;				/** rotate previous CY to msb position */
					AC=0;					/** set program status flags... */
					zero(reg);
					sign(reg);
					parity(reg);
					reg8put(opcode()&0x7,reg,disp);
				}
				break;
				
				case 0x04:					/** sla reg8 */
				{
					uint8_t reg = reg8get(opcode()&0x7,disp);
					CY = ((reg)>>7)&1;		/** carry = msb */
					reg = reg << 1;			/** shift left one bit position */
					AC=0;					/** set program status flags... */
					N=0;
					zero(reg);
					sign(reg);
					parity(reg);
					reg8put(opcode()&0x7,reg,disp);
				}
				break;
				
				case 0x05:					/** sra reg8 */
				{
					uint8_t reg = reg8get(opcode()&0x7,disp);
					uint8_t t = (reg)&0x80;
					CY = (reg)&1;			/** carry = lsb */
					reg = reg >> 1;			/** shift left one bit position */
					reg |= t;				/** replace msb */
					AC=0;					/** set program status flags... */
					N=0;
					zero(reg);
					sign(reg);
					parity(reg);
					reg8put(opcode()&0x7,reg,disp);
				}
				break;
				
				default:
					bad_opcode(pc,opcode());
				break;
			}
			break;
		
		case 0x01:						/** bit b,reg */
		{
			uint8_t reg = reg8get(opcode()&0x7,disp);
			Z = (~((reg) >> ((opcode()>>3)&0x07))) & 0x01;
			AC=1;
			N=0;
		}
		break;
		
		case 0x02:						/** res b,reg */
		{
			uint8_t reg = reg8get(opcode()&0x7,disp);
			reg &= ~(0x01 << ((opcode()>>3)&0x07));
			reg8put(opcode()&0x7,reg,disp);
		}
		break;
		
		case 0x03:						/** set b,reg */
		{
			uint8_t reg = reg8get(opcode()&0x7,disp);
			reg |= (0x01 << ((opcode()>>3)&0x07));
			reg8put(opcode()&0x7,reg,disp);
		}
		break;
		default:
			bad_opcode(pc,opcode());
		break;
	}
}

void sim80vm_z80a::op_ix()
{
	/** switch to IX addressing mode.. */
	index = reg_IX;
	opcode_set(imm8get());
}

void sim80vm_z80a::op_iy()
{
	/** switch to IY addressing mode.. */
	index = reg_IY;
	opcode_set(imm8get());
}

void sim80vm_z80a::op_special()
{
	switch(opcode())
	{
		case 0x08:					/** EX af,af' - exchange af with af' */
			{
				uint8_t t;
				t=a; a=_a; _a=t;
				t=f; f=_f; _f=t;
			}
			break;
		case 0xd9:					/** EXX - exchange alternate register pairs bc,de,hl */
			{
				uint8_t t;
				t=b; b=_b; _b=t;
				t=c; c=_c; _c=t;
				t=d; d=_d; _d=t;
				t=e; e=_e; _e=t;
				t=h; h=_h; _h=t;
				t=l; l=_l; _l=t;
			}
			break;

		default:
			inherited::op_special();
			break;
	}
}

void sim80vm_z80a::op_interrupts()
{
	opcode_set(imm8get());
	switch(opcode())
	{
		case 0x44:					/** NEG - Two's complement accumulator */
		{
		}
		break;
		case 0x45:					/** RETN - return from non maskable interrupt */
		{
			/** FIXME: need to code z80 interrupt modes - just doing normal return here */
			pc = (mem()->get(sp++));
			pc |= (mem()->get(sp++)<<8);
			pc--;
		}
		break;
		case 0x46:					/** IM 0 - interrupt mode 0 */
		{
			/** FIXME: need to code z80 interrupt modes */
		}
		break;
		case 0x4d:					/** RETI - return from interrupt */
		{
			/** FIXME: need to code z80 interrupt modes - just doing normal return here */
			pc = (mem()->get(sp++));
			pc |= (mem()->get(sp++)<<8);
			pc--;
		}
		break;
		case 0x56:					/** IM 1 - interrupt mode 1 */
		{
			/** FIXME: need to code z80 interrupt modes */
		}
		break;
		case 0x5e:					/** IM 2 - interrupt mode 2 */
		{
			/** FIXME: need to code z80 interrupt modes */
		}
		break;
		case 0x67:					/** RRD */
		{
			uint16_t hl = (uint16_t)((h<<8)|l);
			uint8_t m = mem()->get(hl);
			uint8_t t = m;
			m &= ~0x03;
			m |= (m&0x0c)>>2;
			m &= ~0x0c;
			m |= (a&0x03)<<2;
			a &= ~0x03;
			a |= (t&0x03);
			zero(a);
			sign(a);
			parity(a);
			mem()->put(hl,m);
		}
		break;
		case 0x6f:					/** RLD */
		{
			uint16_t hl = (uint16_t)((h<<8)|l);
			uint8_t m = mem()->get(hl);
			uint8_t t = m;
			m &= ~0x03;
			m |= (a&0x03);
			m &= ~0x0c;
			m |= (t&0x03)<<2;
			a &= ~0x03;
			a |= (t&0x0c)>>2;
			zero(a);
			sign(a);
			parity(a);
			mem()->put(hl,m);
		}
		break;
		case 0xa0:					/** LDI */
		{
			uint16_t hl = (uint16_t)((h<<8)|l);
			uint16_t de = (uint16_t)((d<<8)|e);
			uint16_t bc = (uint16_t)((b<<8)|c);
			mem()->put(de++,mem()->get(hl++));
			--bc;
			b = (bc>>8)&0x00ff;
			c = (bc&0xff);
			h = (hl>>8)&0x00ff;
			l = (hl&0xff);
			d = (de>>8)&0x00ff;
			e = (de&0xff);
		}
		break;
		case 0xa1:					/** CPI */
		{
			uint16_t hl = (uint16_t)((h<<8)|l);
			uint16_t bc = (uint16_t)((b<<8)|c);
			uint8_t t = mem()->get(hl++);
			--bc;
			Z = 0;
			if (a == t)
				Z = 1;
			CY = 0;
			if (a < t)
				CY = 1;
			sign(a);
			auxcarry(a);
			b = (bc>>8)&0x00ff;
			c = (bc&0xff);
			h = (hl>>8)&0x00ff;
			l = (hl&0xff);
		}
		break;
		case 0xa8:					/** LDD */
		{
			uint16_t hl = (uint16_t)((h<<8)|l);
			uint16_t de = (uint16_t)((d<<8)|e);
			uint16_t bc = (uint16_t)((b<<8)|c);
			mem()->put(de--,mem()->get(hl--));
			--bc;
			b = (bc>>8)&0x00ff;
			c = (bc&0xff);
			h = (hl>>8)&0x00ff;
			l = (hl&0xff);
			d = (de>>8)&0x00ff;
			e = (de&0xff);
		}
		break;
		case 0xa9:					/** CPD */
		{
			uint16_t hl = (uint16_t)((h<<8)|l);
			uint16_t bc = (uint16_t)((b<<8)|c);
			uint8_t t = mem()->get(hl--);
			--bc;
			Z = 0;
			if (a == t)
				Z = 1;
			CY = 0;
			if (a < t)
				CY = 1;
			sign(a);
			auxcarry(a);
			b = (bc>>8)&0x00ff;
			c = (bc&0xff);
			h = (hl>>8)&0x00ff;
			l = (hl&0xff);
		}
		break;
		case 0xb0:					/** LDIR */
		{
			uint16_t hl = (uint16_t)((h<<8)|l);
			uint16_t de = (uint16_t)((d<<8)|e);
			uint16_t bc = (uint16_t)((b<<8)|c);
			do
			{
				mem()->put(de++,mem()->get(hl++));
			} while ( --bc != 0 );
			b = (bc>>8)&0x00ff;
			c = (bc&0xff);
			h = (hl>>8)&0x00ff;
			l = (hl&0xff);
			d = (de>>8)&0x00ff;
			e = (de&0xff);
		}
		break;
		case 0xb1:					/** CPIR */
		{
			uint16_t hl = (uint16_t)((h<<8)|l);
			uint16_t bc = (uint16_t)((b<<8)|c);
			uint8_t t;
			do
			{
				t = mem()->get(hl++);
				--bc;
				Z = 0;
				if (a == t)
					Z = 1;
				CY = 0;
				if (a < t)
					CY = 1;
				sign(a);
				auxcarry(a);
			} while ( bc != 0 && Z );
			b = (bc>>8)&0x00ff;
			c = (bc&0xff);
			h = (hl>>8)&0x00ff;
			l = (hl&0xff);
		}
		break;
		case 0xb8:					/** LDDR */
		{
			uint16_t hl = (uint16_t)((h<<8)|l);
			uint16_t de = (uint16_t)((d<<8)|e);
			uint16_t bc = (uint16_t)((b<<8)|c);
			do
			{
				mem()->put(de--,mem()->get(hl--));
			} while ( --bc != 0 );
			b = (bc>>8)&0x00ff;
			c = (bc&0xff);
			h = (hl>>8)&0x00ff;
			l = (hl&0xff);
			d = (de>>8)&0x00ff;
			e = (de&0xff);
		}
		break;
		case 0xb9:					/** CPDR */
		{
			uint16_t hl = (uint16_t)((h<<8)|l);
			uint16_t bc = (uint16_t)((b<<8)|c);
			uint8_t t;
			do
			{
				t = mem()->get(hl--);
				--bc;
				Z = 0;
				if (a == t)
					Z = 1;
				CY = 0;
				if (a < t)
					CY = 1;
				sign(a);
				auxcarry(a);
			} while ( bc != 0 && Z );
			b = (bc>>8)&0x00ff;
			c = (bc&0xff);
			h = (hl>>8)&0x00ff;
			l = (hl&0xff);
		}
		break;
		default:
		{
			++pc;
			switch(opcode()&0xCF)
			{
				case 0x43:			/** LD (addr),RP */
					mem()->put16(imm16get(),((opcode()>>4)&0x03));
				break;
				case 0x4B:			/** LD RP,(addr) */
					reg16put(((opcode()>>4)&0x03),mem()->get16(imm16get()));
				break;
				default:
				/** bad opcode() */
				opcode_set(mem()->get(--pc));
				inherited::exec_opcode();
			}
		}
		break;
	}
}

void sim80vm_z80a::op_stack()
{
	/* handle stack opcode()s */
	if ( index != reg_HL )
	{
		switch (opcode())
		{
			case 0xe5:				 /* PUSH index */
				mem()->put(--sp,getIndex()>>8);
				mem()->put(--sp,getIndex()&0xFF);
				break;
			case 0xe1:				 /* POP index */
				putIndex((mem()->get(sp)&0xFF)|(mem()->get(sp+1)<<8));
				sp += 2;
				break;
			case 0xf9:				 /* SP <- index */
				sp = getIndex();
				break;
			case 0xe3:				 /* XTHL */
				{
					uint16_t l = mem()->get(sp);
					mem()->put(sp,getIndex()&0xFF);
					uint16_t h = mem()->get(sp+1);
					mem()->put(sp+1,getIndex()>>8);
					putIndex((h<<8)|l);
				}
				break;
			default:
				bad_opcode(getRegPC(),opcode());
				break;
		}
	}
	else
	{
		inherited::op_stack();
	}
}

void sim80vm_z80a::op_compare()
{
	/* Compare operations */
	if ( index != reg_HL )
	{
		switch(opcode())
		{
			case 0xbe:				 /* CMP (HL) */
				Z = 0;
				if (a == mem()->get(getIndex(imm8get())))
					Z = 1;
				CY = 0;
				if (a < mem()->get(getIndex(imm8get())))
					CY = 1;
				sign(a);
				auxcarry(a);
				break;
			default:
				bad_opcode(getRegPC(),opcode());
				break;
		}
	}
	else
	{
		inherited::op_compare();
	}
}


void sim80vm_z80a::op_or()
{
	/* OR operations */
	if ( index != reg_HL )
	{
		switch(opcode())
		{
			case 0xb6:				 /* ORA (HL) */
				CY = 0;
				AC = 0;
				a = a | mem()->get(getIndex(imm8get()));
				zero(a);
				sign(a);
				auxcarry(a);
				break;
			default:
				bad_opcode(getRegPC(),opcode());
				break;
		}
	}
	else
	{
		inherited::op_or();
	}
}


void sim80vm_z80a::op_xor()
{
	/* XOR operations */
	if ( index != reg_HL )
	{
		switch(opcode())
		{
			case 0xae:				 /* XRA (HL) */
				CY = 0;
				AC = 0;
				a = a ^ mem()->get(getIndex(imm8get()));
				zero(a);
				sign(a);
				auxcarry(a);
				break;
			default:
				bad_opcode(getRegPC(),opcode());
				break;
		}
	}
	else
	{
		inherited::op_xor();
	}
}


void sim80vm_z80a::op_and()
{
	/* AND operations */
	if ( index != reg_HL )
	{
		switch(opcode())
		{
			case 0xa6:				 /* ANA (HL) */
				CY = 0;
				a = a & mem()->get(getIndex(imm8get()));
				zero(a);
				sign(a);
				auxcarry(a);
				break;
			default:
				bad_opcode(getRegPC(),opcode());
				break;
		}
	}
	else
	{
		inherited::op_and();
	}
}


void sim80vm_z80a::op_dad()
{
	int temp;
	/* 16 bit add */
	if ( index != reg_HL )
	{
		switch(opcode())
		{
			case 0x09:				 /* DAD BC */
				temp=getIndex()+((b<<8)| c);
				putIndex(temp&0xFFFF);
				CY = (temp>65535)?1:0;
				break;
	
			case 0x19:				 /* DAD DE */
				temp=getIndex()+((d<<8)| e);
				putIndex(temp&0xFFFF);
				CY = (temp>65535)?1:0;
				break;
	
			case 0x29:				 /* DAD HL */
				temp=getIndex()+getIndex();
				putIndex(temp&0xFFFF);
				CY = (temp>65535)?1:0;
				break;
	
			case 0x39:				 /* DAD SP */
				temp=getIndex()+sp;
				putIndex(temp&0xFFFF);
				CY = (temp>65535)?1:0;
				break;
	
			default:
				bad_opcode(getRegPC(),opcode());
				break;
		}
	}
	else
	{
		inherited::op_dad();
	}
}


void sim80vm_z80a::op_dcx()
{
	if ( index != reg_HL )
	{
		switch(opcode())
		{
			case 0x2b:				 /* DCX HL */
				putIndex(getIndex()-1);
				break;
			default:
				bad_opcode(getRegPC(),opcode());
				break;
		}
	}
	else
	{
		inherited::op_dcx();
	}
}


void sim80vm_z80a::op_inx()
{
	if ( index != reg_HL )
	{
		switch(opcode())
		{
			case 0x23:				 /* INX HL */
				putIndex(getIndex()+1);
				break;
			default:
				bad_opcode(getRegPC(),opcode());
				break;
		}
	}
	else
	{
		inherited::op_inx();
	}
}


void sim80vm_z80a::op_dcr()
{
	if ( index != reg_HL )
	{
		switch(opcode())
		{
			case 0x35:				 /* DCR (HL) */
			{
				uint16_t memory = getIndex(imm8get());
				carry(mem()->put(memory,mem()->get(memory)-1));
				zero(mem()->get(memory));
				sign(mem()->get(memory));
				auxcarry(mem()->get(memory));
									/* other flags */
			}
			break;
			default:
				bad_opcode(getRegPC(),opcode());
				break;
		}
	}
	else
	{
		inherited::op_dcr();
	}
}


void sim80vm_z80a::op_inr()
{
	if ( index != reg_HL )
	{
		switch(opcode())
		{
			case 0x34:				 /* INR (HL) */
			{
				uint16_t memory = getIndex(imm8get());
				carry(mem()->put( memory, mem()->get(memory)+1));
				zero(mem()->get(memory));
				sign(mem()->get(memory));
				auxcarry(mem()->get(memory));
									/* other flags */
			}
			break;
			default:
				bad_opcode(getRegPC(),opcode());
				break;
		}
	}
	else
	{
		inherited::op_inr();
	}
}


void sim80vm_z80a::op_sub()
{
	/* 8 bit subtraction */
	if ( index != reg_HL )
	{
		uint16_t memory;
		switch(opcode())
		{
			case 0x96:				 /* SUB (HL) */
				memory = getIndex(imm8get());
				temp=a;
				carry(temp - mem()->get(memory));
				a = a - mem()->get(memory);
				zero(a);
				sign(a);
				auxcarry(a);
									/* other flags */
				break;
			case 0x9e:				 /* SBB (HL) */
				memory = getIndex(imm8get());
				temp=a;
				a = a - mem()->get(memory) - CY;
				carry(temp-mem()->get(memory)-CY);
				zero(a);
				sign(a);
				auxcarry(a);
									/* other flags */
				break;
			default:
				bad_opcode(getRegPC(),opcode());
				break;
		}
	}
	else
	{
		inherited::op_sub();
	}

}


void sim80vm_z80a::op_add()
{
	/* 8 bit addition */
	if ( index != reg_HL )
	{
		switch (opcode())
		{
			case 0x8e:				 /* ADC (HL) */
				memory = getIndex(imm8get());
				temp=a;
				a = a + mem()->get(memory) + CY;
				carry(temp+a+mem()->get(memory)+CY);
				zero(a);
				sign(a);
				auxcarry(a);
									/* do other things */
				break;
			case 0x86:				 /* ADD (HL) */
				memory = getIndex(imm8get());
				temp=a;
				temp+=mem()->get(memory);
				carry(temp);
				a += mem()->get(memory);
				zero(a);
				sign(a);
				auxcarry(a);
									/* do CY etc flags */
				break;
			default:
				bad_opcode(getRegPC(),opcode());
				break;
		}
	}
	else
	{
		inherited::op_add();
	}
}


void sim80vm_z80a::op_mov()
{
	/* 8 bit move */
	if ( index != reg_HL )
	{
		uint16_t memory;
		switch(opcode())
		{
			case 0x46:				 /* MOV B,(HL) */
				memory = getIndex(imm8get());
				b = mem()->get(memory);
				break;
			case 0x4e:				 /* MOV C,(HL) */
				memory = getIndex(imm8get());
				c = mem()->get(memory);
				break;
			case 0x56:				 /* MOV D,(HL) */
				memory = getIndex(imm8get());
				d = mem()->get(memory);
				break;
			case 0x5e:				 /* MOV E,(HL) */
				memory = getIndex(imm8get());
				e = mem()->get(memory);
				break;
			case 0x66:				 /* MOV H,(HL) */
				memory = getIndex(imm8get());
				h = mem()->get(memory);
				break;
			case 0x6e:				 /* MOV L,(HL) */
				memory = getIndex(imm8get());
				l = mem()->get(memory);
				break;
			case 0x7e:				 /* MOV A,(HL) */
				memory = getIndex(imm8get());
				a = mem()->get(memory);
				break;
			case 0x70:				 /* MOV (HL),B */
				memory = getIndex(imm8get());
				mem()->put(memory,b);
				break;
			case 0x71:				 /* MOV (HL),C */
				memory = getIndex(imm8get());
				mem()->put(memory,c);
				break;
			case 0x72:				 /* MOV (HL),D */
				memory = getIndex(imm8get());
				mem()->put(memory,d);
				break;
			case 0x73:				 /* MOV (HL),E */
				memory = getIndex(imm8get());
				mem()->put(memory,e);
				break;
			case 0x74:				 /* MOV (HL),H */
				memory = getIndex(imm8get());
				mem()->put(memory,h);
				break;
			case 0x75:				 /* MOV (HL),L */
				memory = getIndex(imm8get());
				mem()->put(memory,l);
				break;
			case 0x77:				 /* MOV (HL),A */
				memory = getIndex(imm8get());
				mem()->put(memory,a);
				break;
			default:
				bad_opcode(getRegPC(),opcode());
				break;
		}
	}
	else
	{
		inherited::op_mov();
	}
}


void sim80vm_z80a::op_lxi()
{
	switch(opcode())
	{
		case 0x36:				 /* ld (i?),dddd */
			mem()->put( getIndex(), imm8get() );
			break;
		case 0x21:				 /* ld i?,dddd */
			putIndex(mem()->get(pc+1) | (mem()->get(pc+2)<<8));
			pc+=2;
			break;
		case 0x2a:				 /* ld i?,(addr) */
			{
				uint16_t memory = mem()->get(pc+1) | (mem()->get(pc+2)<<8);
				pc+=2;
				putIndex(mem()->get(memory)|mem()->get(memory+1));
			}
			break;
		case 0x22:				 /* ld (addr),i? */
			{
				uint16_t memory = mem()->get(pc+1) | (mem()->get(pc+2)<<8);
				pc+=2;
				mem()->put(memory,getIndex()&0xFF);
				mem()->put(memory+1,getIndex()>>8);
			}
			break;
		default:
			inherited::op_lxi();
			break;
	}
}


void sim80vm_z80a::exec_opcode()
{
	index = reg_HL;
	switch ( opcode() ) {
		case  0x08:		/** EX af,af' */
		case  0xd9: 	/** EXX */
			op_special();
			break;
		case  0x10:		/** djnz #disp  */
		case  0x18:		/** jr #disp  */
		case  0x20:		/** jr nz,#disp  */
		case  0x28:		/** jr z,#disp  */
		case  0x30:		/** jr nc,#disp  */
		case  0x38:		/** jr c,#disp  */
			op_flow();
			break;
		case  0xcb:		/** bit, res, rl, rlc, rr, rrc, set, sla, sra, srl */
			op_bits();
			break;
		case  0xdd: 	/** all instructions involving register IX */
			op_ix();
			inherited::exec_opcode();;
			break;
		case  0xed:		/** z80 interrupts and special accumulator ops */
			op_interrupts();
			break;
		case  0xfd:		/** all instructions involving register IY */
			op_iy();
			inherited::exec_opcode();;
			break;
		default:
			inherited::exec_opcode();
			break;
	}
}


