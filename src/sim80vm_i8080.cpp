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
#include <sim80vm_i8080.h>

#define inherited sim80vm

sim80vm_i8080::sim80vm_i8080(sim80mem* m,sim80io* io)
: sim80vm(m,io)
, f(0x2) /* bit 2 is always 1 */
, w(0)
, pc(0)
, I(0), S(0), Z(0), AC(0), P(0), CY(0)
{
}

sim80vm_i8080::~sim80vm_i8080()
{
}

void sim80vm_i8080::run2(void)
{
	if ( opcode() != 0x00 )		 /* Handle NOP case */
	{
		exec_opcode();
		f = (S<<5 | Z<<4 | AC<<3 | P<<2 | CY | 0x02);
	}
}

uint8_t sim80vm_i8080::imm8get()
{
	uint8_t t;
	t = mem()->get(++pc);
	return t;	
}

/** 
 * @brief Get value from an 8-bit register
 */
uint8_t sim80vm_i8080::reg8get(uint8_t i,uint8_t disp)
{
	uint8_t rc=0xFF;
	switch(i) {
		case 0x00: rc=b; break;
		case 0x01: rc=c; break;
		case 0x02: rc=d; break;
		case 0x03: rc=e; break;
		case 0x04: rc=h; break;
		case 0x05: rc=l; break;
		case 0x06: 
			{
				uint16_t addr=(uint16_t)((h<<8)|l);
				rc=mem()->get(addr); 
			}
		break;
		case 0x07: rc=a; break;
		default: 
			rc=0xff; 
			break;
	}
	return rc;
}

/** 
 * @brief Set value to an 8-bit register
 */
uint8_t sim80vm_i8080::reg8put(uint8_t i,uint8_t v,uint8_t disp)
{
	switch(i) {
		case 0x00: b=v; break;
		case 0x01: c=v; break;
		case 0x02: d=v; break;
		case 0x03: e=v; break;
		case 0x04: h=v; break;
		case 0x05: l=v; break;
		case 0x06: 
			{
				uint16_t addr=(uint16_t)((h<<8)|l);
				mem()->put(addr,v); 
			}
		break;
		case 0x07: a=v; break;
		default: break;
	}
	return v;
}

uint16_t sim80vm_i8080::imm16get()
{
	uint16_t t;
	t = imm8get();
	t |= (imm8get()<<8);
	return t;
}

uint16_t sim80vm_i8080::reg16get(uint8_t i)
{
	uint16_t rc=0xFFFF;
	switch(i) {
		case 0x00:
			rc = c;
			rc |= b<<8;
		break;
		case 0x01:
			rc = e;
			rc |= d<<8;
		break;
		case 0x02:
			rc = l;
			rc |= h<<8;
		break;
		case 0x03:
			rc = sp;
		break;
	}
	return rc;
}

uint16_t sim80vm_i8080::reg16put(uint8_t i,uint16_t v)
{
	switch(i) {
		case 0x00:
			c = (v&0xFF);
			b = (v>>8)&0xFF;
		break;
		case 0x01:
			e = (v&0xFF);
			d = (v>>8)&0xFF;
		break;
		case 0x02:
			l = (v&0xFF);
			h = (v>>8)&0xFF;
		break;
		case 0x03:
			sp = v;
		break;
	}
	return v;

}

void sim80vm_i8080::rst0()
{
	if ( I == 1 )
	{
		rst_vector_set(0xc7);		 /* RST 0 */
	}
}


void sim80vm_i8080::rst1()
{
	if ( I == 1 )
	{
		rst_vector_set(0xcf);		 /* RST 1 */
	}
}


void sim80vm_i8080::rst2()
{
	if ( I == 1 )
	{
		rst_vector_set(0xd7);		 /* RST 2 */
	}
}


void sim80vm_i8080::rst3()
{
	if ( I == 1 )
	{
		rst_vector_set(0xdf);		 /* RST 3 */
	}
}


void sim80vm_i8080::rst4()
{
	if ( I == 1 )
	{
		rst_vector_set(0xe7);		 /* RST 4 */
	}
}


void sim80vm_i8080::rst5()
{
	if ( I == 1 )
	{
		rst_vector_set(0xef);		 /* RST 5 */
	}
}


void sim80vm_i8080::rst6()
{
	if ( I == 1 )
	{
		rst_vector_set(0xf7);		 /* RST 6 */
	}
}


void sim80vm_i8080::rst7()
{
	if ( I == 1 )
	{
		rst_vector_set(0xff);		 /* RST 7 */
	}
}


void sim80vm_i8080::op_stack()
{
	/* handle stack opcode()s */
	switch (opcode())
	{
		case 0xc5:				 /* PUSH BC */
			mem()->put(--sp,b);
			mem()->put(--sp,c);
			break;

		case 0xd5:				 /* PUSH DE */
			mem()->put(--sp,d);
			mem()->put(--sp,e);
			break;

		case 0xe5:				 /* PUSH HL */
			mem()->put(--sp,h);
			mem()->put(--sp,l);
			break;

		case 0xc1:				 /* POP BC */
			c = mem()->get(sp++);
			b = mem()->get(sp++);
			break;

		case 0xd1:				 /* POP DE */
			e = mem()->get(sp++);
			d = mem()->get(sp++);
			break;

		case 0xe1:				 /* POP HL */
			l = mem()->get(sp++);
			h = mem()->get(sp++);
			break;

		case 0xf5:				 /* PUSH PSW */
			mem()->put(--sp, a);
								 /* flags */
			mem()->put(--sp, ((1*CY)+(2*1)+(4*P)+(8*0)+(16*AC)+(32*0)+(64*Z)+(128*S)));
			break;

		case 0xf1:				 /* POP PSW */
			temp = mem()->get(sp++);
			a = mem()->get(sp++);
			S = (temp & 128)/128;
			Z = (temp & 64)/64;
			AC = (temp & 16)/16;
			P = (temp & 4)/4;
			CY = (temp & 1)/1;
			break;

		case 0xf9:				 /* SPHL */
			sp = (h<<8)| l;
			break;

		case 0xe3:				 /* XTHL */
			temp = l;
			l = mem()->get(sp);
			mem()->put(sp,temp);

			temp = h;
			h = mem()->get(sp+1);
			mem()->put(sp+1,temp);
			break;

		default:
			bad_opcode(getRegPC(),opcode());
			break;
	}
}


void sim80vm_i8080::op_io()
{

	/* output port usage:
	port 0 sends all data to the display console wether it's printable or not
	if it is printable, ascii will be sent. otherwise hex codes will be sent.

	port 1 will display all ascii characters.
	*/

	switch(opcode())
	{
		case 0xd3:				 /* OUT port */
			/* get port */
			temp = imm8get();
			io()->put(temp,a);
			break;

		case 0xdb:				 /* IN port */
			/* get port */
			temp = imm8get();
			a=io()->get(temp);
			break;

		default:
			bad_opcode(getRegPC(),opcode());
			break;
	}
}


void sim80vm_i8080::op_flow()
{
	/* call instructions */

	switch(opcode())
	{
		case 0xcd:				 /* CALL addr */

			temp = pc + 3;		 /* bypass the call instr */
			mem()->put(--sp,(temp&0xFF00)>>8);
			mem()->put(--sp,temp&0x00FF);
			pc = (mem()->get(pc+1) | (mem()->get(pc+2)<<8));
			++pc;

			break;

		case 0xc4:				 /* CALL NZ addr */
			if (Z == 0)
			{
				temp = pc + 3;
				mem()->put(--sp, (temp&0xFF00)>>8);
				mem()->put(--sp, temp&0x00FF);
				pc = (mem()->get(pc+1) | (mem()->get(pc+2)<<8));
				++pc;
			}
			else
			{
				pc = pc + 2;
			}
			break;

		case 0xcc:				 /* CALL Z addr */
			if (Z == 1)
			{
				temp = pc + 3;
				mem()->put(--sp, (temp&0xFF00)>>8);
				mem()->put(--sp, temp&0x00FF);
				pc = (mem()->get(pc+1) | (mem()->get(pc+2)<<8));
				++pc;
			}
			else
			{
				pc += 2;
			}
			break;

		case 0xd4:				 /* CALL NC addr */
			if (CY == 0)
			{
				temp = pc + 3;
				mem()->put(--sp, (temp&0xFF00)>>8);
				mem()->put(--sp, temp&0x00FF);
				pc = (mem()->get(pc+1) | (mem()->get(pc+2)<<8));
				++pc;
			}
			else
			{
				pc += 2;
			}
			break;

		case 0xdc:				 /* CALL C addr */
			if (CY == 1)
			{
				temp = pc + 3;
				mem()->put(--sp, (temp&0xFF00)>>8);
				mem()->put(--sp, temp&0x00FF);
				pc = (mem()->get(pc+1) | (mem()->get(pc+2)<<8));
				++pc;
			}
			else
			{
				pc += 2;
			}
			break;

		case 0xe4:				 /* CALL PO addr */
			if (P == 0)
			{
				temp = pc + 3;
				mem()->put(--sp, (temp&0xFF00)>>8);
				mem()->put(--sp, temp&0x00FF);
				pc = (mem()->get(pc+1) | (mem()->get(pc+2)<<8));
				++pc;
			}
			else
			{
				pc += 2;
			}
			break;

		case 0xec:				 /* CALL PE addr */
			if (P == 1)
			{
				temp = pc + 3;
				mem()->put(--sp, (temp&0xFF00)>>8);
				mem()->put(--sp, temp&0x00FF);
				pc = (mem()->get(pc+1) | (mem()->get(pc+2)<<8));
				++pc;
			}
			else
			{
				pc += 2;
			}
			break;

		case 0xf4:				 /* CALL P addr */
			if (S == 0)
			{
				temp = pc + 3;
				mem()->put(--sp, (temp&0xFF00)>>8);
				mem()->put(--sp, temp&0x00FF);
				pc = (mem()->get(pc+1) | (mem()->get(pc+2)<<8));
				++pc;
			}
			else
			{
				pc += 2;
			}
			break;

		case 0xfc:				 /* CALL M addr */
			if (S == 1)
			{
				temp = pc + 3;
				mem()->put(--sp, (temp&0xFF00)>>8);
				mem()->put(--sp, temp&0x00FF);
				pc = (mem()->get(pc+1) | (mem()->get(pc+2)<<8));
				++pc;
			}
			else
			{
				pc += 2;
			}
			break;

		case 0xc9:				 /* RET */
			pc = (mem()->get(sp++));
			pc |= (mem()->get(sp++)<<8);
			pc--;

			break;

		case 0xc0:				 /* RET NZ */
			if (Z == 0)
			{
				pc = (mem()->get(sp++));
				pc |= (mem()->get(sp++)<<8);
				pc--;
			}
			break;

		case 0xc8:				 /* RET Z */
			if (Z == 1)
			{
				pc = (mem()->get(sp++));
				pc |= (mem()->get(sp++)<<8);
				pc--;
			}
			break;

		case 0xd0:				 /* RET NC */
			if (CY == 0)
			{
				pc = (mem()->get(sp++));
				pc |= (mem()->get(sp++)<<8);
				pc--;
			}
			break;

		case 0xd8:				 /* RET C */
			if (CY == 1)
			{
				pc = (mem()->get(sp++));
				pc |= (mem()->get(sp++)<<8);
				pc--;
			}
			break;

		case 0xe0:				 /* RET PO */
			if (P == 0)
			{
				pc = (mem()->get(sp++));
				pc |= (mem()->get(sp++)<<8);
				pc--;
			}
			break;

		case 0xe8:				 /* RET PE */
			if (P == 1)
			{
				pc = (mem()->get(sp++));
				pc |= (mem()->get(sp++)<<8);
				pc--;
			}
			break;

		case 0xf0:				 /* RET P */
			if (S == 0)
			{
				pc = (mem()->get(sp++));
				pc |= (mem()->get(sp++)<<8);
				pc--;
			}
			break;

		case 0xf8:				 /* RET M */
			if (S == 1)
			{
				pc = (mem()->get(sp++));
				pc |= (mem()->get(sp++)<<8);
				pc--;
			}
			break;

		case 0xe9:				 /* PCHL */
			pc = (h<<8)| l;
			pc--;				 /* do this to offset the pc++ at loop end */
			break;

		case 0xc7:				 /* RST 0 */
			++pc;
			mem()->put(--sp,(pc&0xFF00)>>8);
			mem()->put(--sp,pc&0x00FF);
			pc = 0;
			pc--;
			break;

		case 0xcf:				 /* RST 1 */
			++pc;
			mem()->put(--sp,(pc&0xFF00)>>8);
			mem()->put(--sp,pc&0x00FF);
			pc = 8;
			pc--;
			break;

		case 0xd7:				 /* RST 2 */
			++pc;
			mem()->put(--sp,(pc&0xFF00)>>8);
			mem()->put(--sp,pc&0x00FF);
			pc = 16;
			pc--;
			break;

		case 0xdf:				 /* RST 3 */
			++pc;
			mem()->put(--sp,(pc&0xFF00)>>8);
			mem()->put(--sp,pc&0x00FF);
			pc = 24;
			pc--;
			break;

		case 0xe7:				 /* RST 4 */
			++pc;
			mem()->put(--sp,(pc&0xFF00)>>8);
			mem()->put(--sp,pc&0x00FF);
			pc = 32;
			pc--;
			break;

		case 0xef:				 /* RST 5 */
			++pc;
			mem()->put(--sp,(pc&0xFF00)>>8);
			mem()->put(--sp,pc&0x00FF);
			pc = 40;
			pc--;
			break;

		case 0xf7:				 /* RST 6 */
			++pc;
			mem()->put(--sp,(pc&0xFF00)>>8);
			mem()->put(--sp,pc&0x00FF);
			pc = 48;
			pc--;
			break;

		case 0xff:				 /* RST 7 */
			++pc;
			mem()->put(--sp,(pc&0xFF00)>>8);
			mem()->put(--sp,pc&0x00FF);
			pc = 56;
			pc--;
			break;

		case 0xc3:				 /* JMP XXYY */
								 /* -1 since pc is incremented at end of this loop */
			pc = (mem()->get(pc+1) | ((mem()->get(pc+2))<<8) ) -1;
			break;

		case 0xc2:				 /* JNZ XXYY */
			if (Z==0)
				pc = (mem()->get(pc+1) | ((mem()->get(pc+2))<<8) ) -1;
			else
				pc+=2;			 /* else ignore next two op codes */
			break;

		case 0xca:				 /* JZ XXYY */
			if (Z==1)
				pc = (mem()->get(pc+1) | ((mem()->get(pc+2))<<8) ) -1;
			else
				pc += 2;
			break;

		case 0xd2:				 /* JNC XXYY */
			if (CY==0)
				pc = (mem()->get(pc+1) | ((mem()->get(pc+2))<<8) ) -1;
			else
				pc += 2;
			break;

		case 0xda:				 /* JC XXYY */
			if (CY == 1)
				pc = (mem()->get(pc+1) | ((mem()->get(pc+2))<<8) ) -1;
			else
				pc += 2;
			break;

		case 0xe2:				 /* JPO XXYY */
			if (P == 0)
				pc = (mem()->get(pc+1) | ((mem()->get(pc+2))<<8) ) -1;
			else
				pc += 2;
			break;

		case 0xea:				 /* JPE XXYY */
			if (P == 1)
				pc = (mem()->get(pc+1) | ((mem()->get(pc+2))<<8) ) -1;
			else
				pc += 2;
			break;

		case 0xf2:				 /* JP XXYY (S == 0, positive) */
			if (S == 0)
				pc = (mem()->get(pc+1) | ((mem()->get(pc+2))<<8) ) -1;
			else
				pc += 2;
			break;

		case 0xfa:				 /* JM XXYY (S == 1, minus) */
			if (S == 1)
				pc = (mem()->get(pc+1) | ((mem()->get(pc+2))<<8) ) -1;
			else
				pc += 2;
			break;

		default:
			bad_opcode(getRegPC(),opcode());
			break;
	}
}


void sim80vm_i8080::op_rotate()
{
	switch(opcode())
	{
		case 0x07:				 /* RLC (rotate left) */
			temp = a;
			temp = temp << 1;
			CY = 0;
			if (temp > 255)
				CY = 1;
			a = a << 1;
			break;

		case 0x0f:				 /* RRC (rotate right) */
			temp = a;
			temp = temp >> 1;
			CY = 0;
			if (temp < 0)
				CY = 1;
			a = a >> 1;
			break;

		case 0x17:				 /* RAL (rotate left) */
			CY = (a&0x80)>>7;
			a = a<<1;
			a |= CY;
			break;

		case 0x1f:				 /* RAR (rotate right) */
			CY = (a&0x01);
			a = a>>1;
			a |= (CY<<7);
			break;

		default:
			bad_opcode(getRegPC(),opcode());
			break;
	}
}


void sim80vm_i8080::op_compare()
{
	/* Compare operations */
	switch(opcode())
	{
		case 0xb8:				 /* CMP B */
			Z = 0;
			if (a==b)
				Z = 1;
			CY = 0;
			if (a < b)
				CY = 1;
			sign(a);
			auxcarry(a);
			break;

		case 0xb9:				 /* CMP C */
			Z = 0;
			if (a == c)
				Z = 1;
			CY = 0;
			if (a < c)
				CY = 1;
			sign(a);
			auxcarry(a);
			break;

		case 0xba:				 /* CMP D */
			Z = 0;
			if (a == d)
				Z = 1;
			CY = 0;
			if (a < d)
				CY = 1;
			sign(a);
			auxcarry(a);
			break;

		case 0xbb:				 /* CMP E */
			Z = 0;
			if (a == e)
				Z = 1;
			CY = 0;
			if (a < e)
				CY = 1;
			sign(a);
			auxcarry(a);
			break;

		case 0xbc:				 /* CMP H */
			Z = 0;
			if (a == h)
				Z = 1;
			CY = 0;
			if (a < h)
				CY = 1;
			sign(a);
			auxcarry(a);
			break;

		case 0xbd:				 /* CMP L */
			Z = 0;
			if (a == l)
				Z = 1;
			CY = 0;
			if (a < l)
				CY = 1;
			sign(a);
			auxcarry(a);
			break;

		case 0xbe:				 /* CMP (HL) */
			Z = 0;
			if (a == mem()->get((h<<8)| l))
				Z = 1;
			CY = 0;
			if (a < mem()->get((h<<8)| l))
				CY = 1;
			sign(a);
			auxcarry(a);
			break;

		case 0xbf:				 /* CMP A */
			Z = 1;				 /* since a==a, Z always is 1 and CY = 0 */
			break;

		case 0xfe:				 /* CPI data */
			Z = 0;
			temp = imm8get();
			if (a == temp)
				Z = 1;
			CY = 0;
			if (a < temp)
				CY = 1;
			sign(a);
			auxcarry(a);
			break;

		default:
			bad_opcode(getRegPC(),opcode());
			break;
	}
}


void sim80vm_i8080::op_or()
{
	/* OR operations */
	switch(opcode())
	{
		case 0xb0:				 /* ORA B */
			CY = 0;
			AC = 0;
			a = a | b;
			zero(a);
			sign(a);
			auxcarry(a);
			break;

		case 0xb1:				 /* ORA C */
			CY = 0;
			AC = 0;
			a = a | c;
			zero(a);
			sign(a);
			auxcarry(a);
			break;

		case 0xb2:				 /* ORA D */
			CY = 0;
			AC = 0;
			a = a | d;
			zero(a);
			sign(a);
			auxcarry(a);
			break;

		case 0xb3:				 /* ORA E */
			CY = 0;
			AC = 0;
			a = a | e;
			zero(a);
			sign(a);
			auxcarry(a);
			break;

		case 0xb4:				 /* ORA H */
			CY = 0;
			AC = 0;
			a = a | h;
			zero(a);
			sign(a);
			auxcarry(a);
			break;

		case 0xb5:				 /* ORA L */
			CY = 0;
			AC = 0;
			a = a | l;
			zero(a);
			sign(a);
			auxcarry(a);
			break;

		case 0xb6:				 /* ORA (HL) */
			CY = 0;
			AC = 0;
			a = a | mem()->get((h<<8)| l);
			zero(a);
			sign(a);
			auxcarry(a);
			break;

		case 0xb7:				 /* ORA A */
			CY = 0;
			AC = 0;
			a = a | a;
			zero(a);
			sign(a);
			auxcarry(a);
			break;

		case 0xf6:				 /* ORI data */
			CY = 0;
			AC = 0;
			a = a | imm8get();
			zero(a);
			sign(a);
			auxcarry(a);
			break;

		default:
			bad_opcode(getRegPC(),opcode());
			break;
	}
}


void sim80vm_i8080::op_xor()
{
	/* XOR operations */
	switch(opcode())
	{
		case 0xa8:				 /* XRA B */
			CY = 0;
			AC = 0;
			a = a ^ b;
			zero(a);
			sign(a);
			auxcarry(a);
			break;

		case 0xa9:				 /* XRA C */
			CY = 0;
			AC = 0;
			a = a ^ c;
			zero(a);
			sign(a);
			auxcarry(a);
			break;

		case 0xaa:				 /* XRA D */
			CY = 0;
			AC = 0;
			a = a ^ d;
			zero(a);
			sign(a);
			auxcarry(a);
			break;

		case 0xab:				 /* XRA E */
			CY = 0;
			AC = 0;
			a = a ^ e;
			zero(a);
			sign(a);
			auxcarry(a);
			break;

		case 0xac:				 /* XRA H */
			CY = 0;
			AC = 0;
			a = a ^ h;
			zero(a);
			sign(a);
			auxcarry(a);
			break;

		case 0xad:				 /* XRA L */
			CY = 0;
			AC = 0;
			a = a ^ l;
			zero(a);
			sign(a);
			auxcarry(a);
			break;

		case 0xae:				 /* XRA (HL) */
			CY = 0;
			AC = 0;
			a = a ^ mem()->get((h<<8)| l);
			zero(a);
			sign(a);
			auxcarry(a);
			break;

		case 0xaf:				 /* XRA A  (clear a) */
			CY = 0;
			AC = 0;
			a = a ^ a;
			zero(a);
			sign(a);
			auxcarry(a);
			break;

		case 0xee:				 /* XRI data */
			CY = 0;
			AC = 0;
			a = a ^ imm8get();
			zero(a);
			sign(a);
			auxcarry(a);
			break;

		default:
			bad_opcode(getRegPC(),opcode());
			break;
	}
}


void sim80vm_i8080::op_and()
{
	/* AND operations */
	switch(opcode())
	{
		case 0xa0:				 /* ANA B */
			CY = 0;				 /* always */
			a = a & b;
			zero(a);
			sign(a);
			auxcarry(a);
			break;

		case 0xa1:				 /* ANA C */
			CY = 0;
			a = a & c;
			zero(a);
			sign(a);
			auxcarry(a);
			break;

		case 0xa2:				 /* ANA D */
			CY = 0;
			a = a & d;
			zero(a);
			sign(a);
			auxcarry(a);
			break;

		case 0xa3:				 /* ANA E */
			CY = 0;
			a = a & e;
			zero(a);
			sign(a);
			auxcarry(a);
			break;

		case 0xa4:				 /* ANA H */
			CY = 0;
			a = a & h;
			zero(a);
			sign(a);
			auxcarry(a);
			break;

		case 0xa5:				 /* ANA L */
			CY = 0;
			a = a & l;
			zero(a);
			sign(a);
			auxcarry(a);
			break;

		case 0xa6:				 /* ANA (HL) */
			CY = 0;
			a = a & mem()->get((h<<8)| l);
			zero(a);
			sign(a);
			auxcarry(a);
			break;

		case 0xa7:				 /* ANA A */
			CY = 0;
			a = a & a;
			zero(a);
			sign(a);
			auxcarry(a);
			break;

		case 0xe6:				 /* ANI data */
			CY = 0;
			AC = 0;
			a = a & imm8get();
			zero(a);
			sign(a);
			auxcarry(a);
			break;

		default:
			bad_opcode(getRegPC(),opcode());
			break;
	}
}


void sim80vm_i8080::op_control()
{
	/* control operations */
	switch(opcode())
	{
		case 0x00:				 /* NOP - no operation */
			break;

		case 0xfb:				 /* EI - enable interrupts */
			I=1;
			break;

		case 0xf3:				 /* DI - disable interrupts */
			I=0;
			break;

		case 0x76:				 /* HALT - halt the processor */
			halt();
			break;

		default:
			bad_opcode(getRegPC(),opcode());
			break;
	}
}


void sim80vm_i8080::op_dad()
{
	/* 16 bit add */
	switch(opcode())
	{
		case 0x09:				 /* DAD BC */
			temp=((h<<8)| l)+((b<<8)| c);
			h=(temp&0xFF00)>>8;
			l=(temp&0x00FF);
			CY = (temp>65535)?1:0;
			break;

		case 0x19:				 /* DAD DE */
			temp=((h<<8)| l)+((d<<8)| e);
			h=(temp&0xFF00)>>8;
			l=(temp&0x00FF);
			CY = (temp>65535)?1:0;
			break;

		case 0x29:				 /* DAD HL */
			temp=((h<<8)| l)*2;
			h=(temp&0xFF00)>>8;
			l=(temp&0x00FF);
			CY = (temp>65535)?1:0;
			break;

		case 0x39:				 /* DAD SP */
			temp=((h<<8)| l)+sp;
			h=(temp&0xFF00)>>8;
			l=(temp&0x00FF);
			CY = (temp>65535)?1:0;
			break;

		default:
			bad_opcode(getRegPC(),opcode());
			break;
	}
}


void sim80vm_i8080::op_dcx()
{
	switch(opcode())
	{
		case 0x0b:				 /* DCX BC */
			c = c - 1;
			if (c == 255)
				b = b - 1;
			break;

		case 0x1b:				 /* DCX DE */
			e = e - 1;
			if (e == 255)
				d = d - 1;
			break;

		case 0x2b:				 /* DCX HL */
			l = l - 1;
			if (l == 255)
				h = h - 1;
			break;

		case 0x3b:				 /* DCX SP */
			sp--;
			break;

		default:
			bad_opcode(getRegPC(),opcode());
			break;
	}
}


void sim80vm_i8080::op_inx()
{
	switch(opcode())
	{
		case 0x03:				 /* INX BC */
			c = c + 1;
			if (c == 0)
			{
				b = b + 1;
			}
			break;

		case 0x13:				 /* INX DE */
			e = e + 1;
			if ( e == 0)
			{
				d = d + 1;
			}
			break;

		case 0x23:				 /* INX HL */
			l = l + 1;
			if (l == 0)
			{
				h = h + 1;
			}
			break;

		case 0x33:				 /* INX SP */
			sp++;
			break;

		default:
			bad_opcode(getRegPC(),opcode());
			break;
	}
}


void sim80vm_i8080::op_dcr()
{
	switch(opcode())
	{
		case 0x05:				 /* DCR B */
			carry(--b);
			sign(b);
			zero(b);
			auxcarry(b);
			/* other flags */
			break;

		case 0x0d:				 /* DCR C */
			carry(--c);
			zero(c);
			sign(c);
			auxcarry(c);
			/* other flags */
			break;

		case 0x15:				 /* DCR D */
			carry(--d);
			zero(d);
			sign(d);
			auxcarry(d);
			/* other flags */
			break;

		case 0x1d:				 /* DCR E */
			carry(--e);
			zero(e);
			sign(e);
			auxcarry(e);
			/* other flags */
			break;

		case 0x25:				 /* DCR H */
			carry(--h);
			zero(h);
			sign(h);
			auxcarry(h);
			/* other flags */
			break;

		case 0x2d:				 /* DCR L */
			carry(--l);
			zero(l);
			sign(l);
			auxcarry(l);
			/* other flags */
			break;

		case 0x35:				 /* DCR (HL) */
			memory = getIndex();
			carry(mem()->put(memory,mem()->get(memory)-1));
			zero(mem()->get(memory));
			sign(mem()->get(memory));
			auxcarry(mem()->get(memory));
								 /* other flags */
			break;

		case 0x3d:				 /* DCR A */
			carry(--a);
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


void sim80vm_i8080::op_inr()
{
	switch(opcode())
	{
		case 0x04:				 /* INR B */
			carry(++b);
			zero(b);
			sign(b);
			auxcarry(b);
			/* other flags */
			break;

		case 0x0c:				 /* INR C */
			carry(++c);
			zero(c);
			sign(c);
			auxcarry(c);
			/* other flags */
			break;

		case 0x14:				 /* INR D */
			carry(++d);
			zero(d);
			sign(d);
			auxcarry(d);
			/* other flags */
			break;

		case 0x1c:				 /* INR E */
			carry(++e);
			zero(e);
			sign(e);
			auxcarry(e);
			/* other flags */
			break;

		case 0x24:				 /* INR H */
			carry(++h);
			zero(h);
			sign(h);
			auxcarry(h);
			/* other flags */
			break;

		case 0x2c:				 /* INR L */
			carry(++l);
			zero(l);
			sign(l);
			auxcarry(l);
			/* other flags */
			break;

		case 0x34:				 /* INR (HL) */
			memory = getIndex();
			carry(mem()->put( memory, mem()->get(memory)+1 ));
			zero(mem()->get(memory));
			sign(mem()->get(memory));
			auxcarry(mem()->get(memory));
								 /* other flags */
			break;

		case 0x3c:				 /* INR A */
			carry(++a);
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


void sim80vm_i8080::op_sub()
{
	/* 8 bit subtraction */
	switch(opcode())
	{
		case 0x90:				 /* SUB B */
			temp=a;
			carry(temp-b);
			a = a - b;
			zero(a);
			sign(a);
			auxcarry(a);
			/* do other flags */
			break;

		case 0x91:				 /* SUB C */
			temp=a;
			carry(temp-c);
			a = a - c;
			zero(a);
			sign(a);
			auxcarry(a);
			/* and do others */
			break;

		case 0x92:				 /* SUB D */
			temp=a;
			carry(temp-d);
			a = a - d;
			zero(a);
			sign(a);
			auxcarry(a);
			/* and other flags */
			break;

		case 0x93:				 /* SUB E */
			temp=a;
			carry(temp-e);
			a = a - e;
			zero(a);
			sign(a);
			auxcarry(a);
			/* other flags */
			break;

		case 0x94:				 /* SUB H */
			temp=a;
			carry(temp-h);
			a = a - h;
			zero(a);
			sign(a);
			auxcarry(a);
			/* other flags */
			break;

		case 0x95:				 /* SUB L */
			temp=a;
			carry(temp-l);
			a = a - l;
			zero(a);
			sign(a);
			auxcarry(a);
			/* other flags */
			break;

		case 0x96:				 /* SUB (HL) */
			memory = getIndex();
			temp=a;
			carry(temp - mem()->get(memory));
			a = a - mem()->get(memory);
			zero(a);
			sign(a);
			auxcarry(a);
								 /* other flags */
			break;

		case 0x97:				 /* SUB A */
			a = a - a;
			Z = 1;				 /* always will be */
			S = 0;				 /* always will be */
			CY = 0;
			AC = 0;
								 /* do other flags */
			break;
								 /*----------------------*/

		case 0xd6:				 /* SUI data */
			temp=a;
			a = a - imm8get();
			carry(temp-mem()->get(pc+1));
			zero(a);
			sign(a);
			auxcarry(a);
								 /* do other flags */
			break;

		case 0x98:				 /* SBB B */
			temp=a;
			a = a - b - CY;
			carry(temp-b-CY);
			zero(a);
			sign(a);
			auxcarry(a);
								 /* other flags */
			break;

		case 0x99:				 /* SBB C */
			temp=a;
			a = a - c - CY;
			carry(temp-c-CY);
			zero(a);
			sign(a);
			auxcarry(a);
								 /* other flags */
			break;

		case 0x9a:				 /* SBB D */
			temp=a;
			a = a - d - CY;
			carry(temp-d-CY);
			zero(a);
			sign(a);
			auxcarry(a);
								 /* other flags */
			break;

		case 0x9b:				 /* SBB E */
			temp=a;
			a = a - e - CY;
			carry(temp-e-CY);
			zero(a);
			sign(a);
			auxcarry(a);
								 /* other flags */
			break;

		case 0x9c:				 /* SBB H */
			temp=a;
			a = a - h - CY;
			carry(temp-h-CY);
			zero(a);
			sign(a);
			auxcarry(a);
								 /* other flags */
			break;

		case 0x9d:				 /* SBB L */
			temp=a;
			a = a - l - CY;
			carry(temp-l-CY);
			zero(a);
			sign(a);
			auxcarry(a);
								 /* other flags */
			break;

		case 0x9e:				 /* SBB (HL) */
			memory = getIndex();
			temp=a;
			a = a - mem()->get(memory) - CY;
			carry(temp-mem()->get(memory)-CY);
			zero(a);
			sign(a);
			auxcarry(a);
								 /* other flags */
			break;

		case 0x9f:				 /* SBB A */
			temp=a;
			a = a - a - CY;
			carry(temp-temp-CY);
			zero(a);
			sign(a);
			auxcarry(a);
								 /* other flags */
			break;

		case 0xde:				 /* SBI data */
			temp=a;
			a = a - imm8get() - CY;
			carry(temp-mem()->get(pc+1)-CY);
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


void sim80vm_i8080::op_add()
{
	/* 8 bit addition */
	switch (opcode())
	{
		case 0xc6:				 /* ADI data */
			temp=a;
			carry(temp+mem()->get(pc+1));
			a += imm8get();
			zero(a);
			sign(a);
			auxcarry(a);
			/* do CY etc flags */
			break;

		case 0x88:				 /* ADC B */
			temp=a;
			a = a + b + CY;
			carry(temp + a + b + CY);
			zero(a);
			sign(a);
			auxcarry(a);
			/* do other flags */
			break;

		case 0x89:				 /* ADC C */
			temp=a;
			a = a + c + CY;
			carry(temp+a+c+CY);
			zero(a);
			sign(a);
			auxcarry(a);
			/* do other flags */
			break;

		case 0x8a:				 /* ADC D */
			temp=a;
			a = a + d + CY;
			carry(temp+a+d+CY);
			zero(a);
			sign(a);
			auxcarry(a);
			/* do other flags */
			break;

		case 0x8b:				 /* ADC E */
			temp=a;
			a = a + e + CY;
			carry(temp+a+e+CY);
			zero(a);
			sign(a);
			auxcarry(a);
			/* do other things */
			break;

		case 0x8c:				 /* ADC H */
			temp=a;
			a = a + h + CY;
			carry(temp+a+h+CY);
			zero(a);
			sign(a);
			auxcarry(a);
			/* do other things */
			break;

		case 0x8d:				 /* ADC L */
			temp=a;
			a = a + l + CY;
			carry(temp+a+l+CY);
			zero(a);
			sign(a);
			auxcarry(a);
								 /* do other things */
			break;

		case 0x8e:				 /* ADC (HL) */
			memory = getIndex();
			temp=a;
			a = a + mem()->get(memory) + CY;
			carry(temp+a+mem()->get(memory)+CY);
			zero(a);
			sign(a);
			auxcarry(a);
								 /* do other things */
			break;

		case 0x8f:				 /* ADC A */
			temp=a;
			a = a + a + CY;
			carry(temp+a+a+CY);
			zero(a);
			sign(a);
			auxcarry(a);
								 /* do other stuff */
			break;

		case 0xce:				 /* ACI data */
			temp=a;
			a = a + imm8get() + CY;
			carry(temp+mem()->get(pc+1) + CY);
			zero(a);
			sign(a);
			auxcarry(a);
								 /* do other flags */
			break;

		case 0x80:				 /* ADD B */
			temp = a;
			temp += b;			 /* use temp for CY flag test */
			carry(temp);
			a += b;
			zero(a);			 /* set Z based on a */
			sign(a);			 /* set S based on a */
								 /* parity flag P - do */
			auxcarry(a);		 /* and aux carry flag */
			break;

		case 0x81:				 /* ADD C */
			temp = a; temp += c;
			carry(temp);		 /* check carry flag */
			a += c;
			zero(a);
			sign(a);
								 /* do CY flag, P flag, and AC flag */
			auxcarry(a);
			break;

		case 0x82:				 /* ADD D */
			temp = a; temp += d;
			carry(temp);
			a += d;
			zero(a);
			sign(a);
			auxcarry(a);
								 /* do CY, P and AC flags */
			break;

		case 0x83:				 /* ADD E */
			temp = a; temp += e;
			carry(temp);
			a += e;
			zero(a);
			sign(a);
			auxcarry(a);
								 /* do CY, P and AC flags */
			break;

		case 0x84:				 /* ADD H */
			temp = a; temp += h;
			carry(temp);
			a += h;
			zero(a);
			sign(a);
			auxcarry(a);
								 /* do CY etc flags */
			break;

		case 0x85:				 /* ADD L */
			temp=a; temp+=l;carry(temp);
			a += l;
			zero(a);
			sign(a);
			auxcarry(a);
								 /* do CY etc flags */
			break;

		case 0x87:				 /* ADD A */
			temp=a;temp+=a;carry(temp);
			a += a;
			zero(a);
			sign(a);
			auxcarry(a);
								 /* do CY etc flags */
			break;

		case 0x86:				 /* ADD (HL) */
			memory = getIndex();
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


void sim80vm_i8080::op_mov()
{
	/* 8 bit move */
	switch(opcode())
	{
		case 0x40:				 /* MOV B,B */
			b = b;
			break;

		case 0x41:				 /* MOV B,C */
			b = c;
			break;

		case 0x42:				 /* MOV B,D */
			b = d;
			break;

		case 0x43:				 /* MOV B,E */
			b = e;
			break;

		case 0x44:				 /* MOV B,H */
			b = h;
			break;

		case 0x45:				 /* MOV B,L */
			b = l;
			break;

		case 0x46:				 /* MOV B,(HL) (memory) */
			memory = getIndex(); /* set pointer from HL */
			b = mem()->get(memory);
			break;

		case 0x47:				 /* MOV B,A */
			b = a;
			break;

								 /*------------------------ */

		case 0x48:				 /* MOV C,B */
			c = b;
			break;

		case 0x49:				 /* MOV C,C */
			c = c;
			break;

		case 0x4a:				 /* MOV C,D */
			c = d;
			break;

		case 0x4b:				 /* MOV C,E */
			c = e;
			break;

		case 0x4c:				 /* MOV C,H */
			c = h;
			break;

		case 0x4d:				 /* MOV C,L */
			c = l;
			break;

		case 0x4e:				 /* MOV C,(HL) */
			memory = getIndex();
			c = mem()->get(memory);
			break;

		case 0x4f:				 /* MOV C,A */
			c = a;
			break;
								 /*---------------------- */

		case 0x50:				 /* MOV D,B */
			d = b;
			break;

		case 0x51:				 /* MOV D,C */
			d = c;
			break;

		case 0x52:				 /* MOV D,D */
			d = d;
			break;

		case 0x53:				 /* MOV D,E */
			d = e;
			break;

		case 0x54:				 /* MOV D,H */
			d = h;
			break;

		case 0x55:				 /* MOV D,L */
			d = l;
			break;

		case 0x56:				 /* MOV D,(HL) */
			memory = getIndex();
			d = mem()->get(memory);
			break;

		case 0x57:				 /* MOV D,A */
			d = a;
			break;

								 /*-----------------------*/

		case 0x58:				 /* MOV E,B */
			e = b;
			break;

		case 0x59:				 /* MOV E,C */
			e = c;
			break;

		case 0x5a:				 /* MOV E,D */
			e = d;
			break;

		case 0x5b:				 /* MOV E,E */
			e = e;
			break;

		case 0x5c:				 /* MOV E,H */
			e = h;
			break;

		case 0x5d:				 /* MOV E,L */
			e = l;
			break;

		case 0x5e:				 /* MOV E,(HL) */
			memory = getIndex();
			e = mem()->get(memory);
			break;

		case 0x5f:				 /* MOV E,A */
			e = a;
			break;

								 /*------------------------*/

		case 0x60:				 /* MOV H,B */
			h = b;
			break;

		case 0x61:				 /* MOV H,C */
			h = c;
			break;

		case 0x62:				 /* MOV H,D */
			h = d;
			break;

		case 0x63:				 /* MOV H,E */
			h = e;
			break;

		case 0x64:				 /* MOV H,H */
			h = h;
			break;

		case 0x65:				 /* MOV H,L */
			h = l;
			break;

		case 0x66:				 /* MOV H,(HL) */
			memory = getIndex();
			h = mem()->get(memory);
			break;

		case 0x67:				 /* MOV H,A */
			h = a;
			break;
								 /*-----------------------*/

		case 0x68:				 /* MOV L,B */
			l = b;
			break;

		case 0x69:				 /* MOV L,C */
			l = c;
			break;

		case 0x6a:				 /* MOV L,D */
			l = d;
			break;

		case 0x6b:				 /* MOV L,E */
			l = e;
			break;

		case 0x6c:				 /* MOV L,H */
			l = h;
			break;

		case 0x6d:				 /* MOV L,L */
			l = l;
			break;

		case 0x6e:				 /* MOV L,(HL) */
			memory = getIndex();
			l = mem()->get(memory);
			break;

		case 0x6f:				 /* MOV L,A */
			l = a;
			break;
								 /*------------------------*/

		case 0x78:				 /* MOV A,B */
			a = b;
			break;

		case 0x79:				 /* MOV A,C */
			a = c;
			break;

		case 0x7a:				 /* MOV A,D */
			a = d;
			break;

		case 0x7b:				 /* MOV A,E */
			a = e;
			break;

		case 0x7c:				 /* MOV A,H */
			a = h;
			break;

		case 0x7d:				 /* MOV A,L */
			a = l;
			break;

		case 0x7e:				 /* MOV A,(HL) */
			memory = getIndex();
			a = mem()->get(memory);
			break;

		case 0x7f:				 /* MOV A,A */
			a = a;
			break;
								 /*------------------------*/

		case 0x70:				 /* MOV (HL),B */
			memory = getIndex();
			mem()->put(memory,b);
			break;

		case 0x71:				 /* MOV (HL),C */
			memory = getIndex();
			mem()->put(memory,c);
			break;

		case 0x72:				 /* MOV (HL),D */
			memory = getIndex();
			mem()->put(memory,d);
			break;

		case 0x73:				 /* MOV (HL),E */
			memory = getIndex();
			mem()->put(memory,e);
			break;

		case 0x74:				 /* MOV (HL),H */
			memory = getIndex();
			mem()->put(memory,h);
			break;

		case 0x75:				 /* MOV (HL),L */
			memory = getIndex();
			mem()->put(memory,l);
			break;

		case 0x77:				 /* MOV (HL),A */
			memory = getIndex();
			mem()->put(memory,a);
			break;

		default:
			bad_opcode(getRegPC(),opcode());
			break;
	}
}


void sim80vm_i8080::op_lxi()
{
	switch(opcode())
	{
		case 0x06:				 /* MVI B */
			b = imm8get();
			break;

		case 0x0e:				 /* MVI C */
			c = imm8get();
			break;

		case 0x16:				 /* MVI D */
			d = imm8get();
			break;

		case 0x1e:				 /* MVI E */
			e = imm8get();
			break;

		case 0x26:				 /* MVI H */
			h = imm8get();
			break;

		case 0x2e:				 /* MVI L */
			l = imm8get();
			break;

		case 0x36:				 /* MVI (HL) - load data to memory */
			memory = getIndex();
			mem()->put( memory, imm8get() );
			break;

		case 0x3e:				 /* MVI A */
			a = imm8get();
			break;

								 /*----------------------------*/

		case 0x01:				 /* LXI BC,data-l,data-h */
			c = imm8get();
			b = imm8get();
			break;

		case 0x11:				 /* LXI DE,data-l,data-h */
			e = imm8get();
			d = imm8get();
			break;

		case 0x21:				 /* LXI HL,data-l,data-h */
			l = imm8get();
			h = imm8get();
			break;

		case 0x31:				 /* LXI SP,data-l,data-h */
			sp = mem()->get(pc+1) | (mem()->get(pc+2)<<8);
			pc+=2;
			break;
								 /*----------------------------*/

		case 0x3a:				 /* LDA addr */
			memory = mem()->get(pc+1) | (mem()->get(pc+2)<<8);
			pc+=2;
			a = mem()->get(memory);
			break;
								 /*-----------------------------*/

		case 0x32:				 /* STA addr */
			memory = mem()->get(pc+1) | (mem()->get(pc+2)<<8);
			pc+=2;
			mem()->put(memory,a);
			break;
								 /*------------------------------*/

		case 0x2a:				 /* LHLD addr */
			memory = mem()->get(pc+1) | (mem()->get(pc+2)<<8);
			pc+=2;
			l = mem()->get(memory);
			h = mem()->get(memory+1);
			break;
								 /*------------------------------*/

		case 0x22:				 /* SHLD addr */
			memory = mem()->get(pc+1) | (mem()->get(pc+2)<<8);
			pc+=2;
			mem()->put(memory,l);
			mem()->put(memory+1,h);
			break;
								 /*-----------------------------*/

		case 0x0a:				 /* LDAX BC */
			memory = (b<<8) | c;
			a = mem()->get(memory);
			break;

		case 0x1a:				 /* LDAX DE */
			memory = (d<<8) | e;
			a = mem()->get(memory);
			break;
								 /*-----------------------------*/

		case 0x02:				 /* STAX BC */
			memory = (b<<8) | c;
			mem()->put(memory,a);
			break;

		case 0x12:				 /* STAX DE */
			memory = (d<<8) | e;
			mem()->put(memory,a);
			break;
								 /*---------------------------*/

		case 0xeb:				 /* XCHG */
			w = h;				 /* swap HL and DE */
			h = d;
			d = w;
			w = l;
			l = e;
			e = w;
			break;
								 /*----------------------------*/

		default:
			bad_opcode(getRegPC(),opcode());
			break;
	}
}


void sim80vm_i8080::op_special()
{
	switch(opcode())
	{
		/* special commands */
		case 0x2f:				 /* CMA - compliment Acc */
			a = ~a;
			break;

		case 0x37:				 /* STC - Set carry flag */
			CY = 1;
			break;

		case 0x3f:				 /* CMC - compliment the carry flag */
			CY = !CY;
			break;

		case 0x27:				 /* DAA - decimal adjust acc */
			if ( (a&0x0F)>9 ||AC )
			{
				temp = a;
				temp += 6;		 /* add 6 to accumulator */
				carry(temp);
				a += 6;
				zero(a);		 /* set Z based on a */
				sign(a);		 /* set S based on a */
				/* parity flag P - do */
				auxcarry(a);	 /* and aux carry flag */
			}
			if ( ((a&0xF0)>>4)>9 ||AC )
			{
				temp = (a&0x0f) | (((a&0xf0)>>4)+6)<<4;
				carry(temp);
				a = (a&0x0f) | (((a&0xf0)>>4)+6)<<4;
				zero(a);		 /* set Z based on a */
				sign(a);		 /* set S based on a */
				/* parity flag P - do */
				auxcarry(a);	 /* and aux carry flag */
			}
			break;

		default:
			bad_opcode(getRegPC(),opcode());
			break;
	}
}

void sim80vm_i8080::exec_opcode()
{
	switch ( opcode() ) {
		
		/** stack operations */
		
		case 0xc5:  /** PUSH BC */
		case 0xd5:  /** PUSH DE */
		case 0xe5:  /** PUSH DE */
		case 0xc1:  /** POP BC */
		case 0xd1:  /** POP DE */
		case 0xe1:  /** POP HL */
		case 0xf5:  /** PUSH PSW */
		case 0xf1:  /** POP PSW */
		case 0xf9:  /** SPHL */
		case 0xe3:  /** XTHL */
			op_stack();
			break;
		
		/** input/output */
	
		case 0xd3:  /** OUT port */
		case 0xdb:  /** IN port */
			op_io();
			break;
			
		/** program flow */
	
		case 0xcd:  /** CALL addr */
		case 0xc4:  /** CALL NZ addr */
		case 0xcc:  /** CALL Z addr */
		case 0xd4:  /** CALL NC addr */
		case 0xdc:  /** CALL C addr */
		case 0xe4:  /** CALL PO addr */
		case 0xec:  /** CALL PE addr */
		case 0xf4:  /** CALL P addr */
		case 0xfc:  /** CALL M addr */
		case 0xc9:  /** RET */
		case 0xc0:  /** RET NZ */
		case 0xc8:  /** RET Z */
		case 0xd0:  /** RET NC */
		case 0xd8:  /** RET C */
		case 0xe0:  /** RET PO */
		case 0xe8:  /** RET PE */
		case 0xf0:  /** RET P */
		case 0xf8:  /** RET M */
		case 0xe9:  /** PCHL */
		case 0xc7:  /** RST 0 */
		case 0xcf:  /** RST 1 */
		case 0xd7:  /** RST 2 */
		case 0xdf:  /** RST 3 */
		case 0xe7:  /** RST 4 */
		case 0xef:  /** RST 5 */
		case 0xf7:  /** RST 6 */
		case 0xff:  /** RST 7 */
		case 0xc3:  /** JMP XXYY */
		case 0xc2:  /** JNZ XXYY */
		case 0xca:  /** JZ XXYY */
		case 0xd2:  /** JNC XXYY */
		case 0xda:  /** JC XXYY */
		case 0xe2:  /** JPO XXYY */
		case 0xea:  /** JPE XXYY */
		case 0xf2:  /** JP XXYY (S == 0, positive) */
		case 0xfa:  /** JM XXYY (S == 1, minus) */
			op_flow();
			break;
			
		/** compare opcode()s */
		
		case 0xb8:  /** CMP B */
		case 0xb9:  /** CMP C */
		case 0xba:  /** CMP D */
		case 0xbb:  /** CMP E */
		case 0xbc:  /** CMP H */
		case 0xbd:  /** CMP L */
		case 0xbe:  /** CMP (HL) */
		case 0xbf:  /** CMP A */
		case 0xfe:  /** CPI data */
			op_compare();
			break;
			
		/** rotate opcode()s */
			
		case 0x07:  /** RLC (rotate left) */
		case 0x0f:  /** RRC (rotate right) */
		case 0x17:  /** RAL (rotate left) */
		case 0x1f:  /** RAR (rotate right) */
			op_rotate();
			break;
	
		/** OR operations */
		
		case 0xb0:  /** ORA B */
		case 0xb1:  /** ORA C */
		case 0xb2:  /** ORA D */
		case 0xb3:  /** ORA E */
		case 0xb4:  /** ORA H */
		case 0xb5:  /** ORA L */
		case 0xb6:  /** ORA (HL) */
		case 0xb7:  /** ORA A */
		case 0xf6:  /** ORI data */
			op_or();
			break;
			
		/** XOR operations */
		
		case 0xa8:  /** XRA B */
		case 0xa9:  /** XRA C */
		case 0xaa:  /** XRA D */
		case 0xab:  /** XRA E */
		case 0xac:  /** XRA H */
		case 0xad:  /** XRA L */
		case 0xae:  /** XRA (HL) */
		case 0xaf:  /** XRA A  (clear a) */
		case 0xee:  /** XRI data */
			op_xor();
			break;
			
		/** AND operations */
		/** ANA B */
		case 0xa0:  
		/** ANA C */
		case 0xa1:  
		/** ANA D */
		case 0xa2:  
		/** ANA E */
		case 0xa3:  
		/** ANA H */
		case 0xa4:  
		/** ANA L */
		case 0xa5:  
		/** ANA (HL) */
		case 0xa6:  
		/** ANA A */
		case 0xa7:  
		/** ANI data */
		case 0xe6:  
			op_and();
			break;
			
		/** control operations */
		/** NOP - no operation */
		case 0x00:  
		/** EI - enable interrupts */
		case 0xfb:  
		/** DI - disable interrupts */
		case 0xf3:  
		/** HALT - halt the processor */
		case 0x76:  
			op_control();
			break;
			
		/** 16 bit add operations */
		/** DAD BC */
		case 0x09:  
		/** DAD DE */
		case 0x19:  
		/** DAD HL */
		case 0x29:  
		/** DAD SP */
		case 0x39:  
			op_dad();
			break;
			
		/** 16 bit decrement operations */
		/** DCX BC */
		case 0x0b:  
		/** DCX DE */
		case 0x1b:  
		/** DCX HL */
		case 0x2b:  
		/** DCX SP */
		case 0x3b:  
			op_dcx();
			break;
			
		/** 16 bit increment operations */
		/** INX BC */
		case 0x03:  
		/** INX DE */
		case 0x13:  
		/** INX HL */
		case 0x23:  
		/** INX SP */
		case 0x33:  
			op_inx();
			break;
			
		/** 8 bit decrement operations */
		/** DCR B */
		case 0x05:  
		/** DCR C */
		case 0x0d:  
		/** DCR D */
		case 0x15:  
		/** DCR E */
		case 0x1d:  
		/** DCR H */
		case 0x25:  
		/** DCR L */
		case 0x2d:  
		/** DCR (HL) */
		case 0x35:  
		/** DCR A */
		case 0x3d:  
			op_dcr();
			break;
		
		/** 8 bit increment operations */
		/** INR B */
		case 0x04:  
		/** INR C */
		case 0x0c:  
		/** INR D */
		case 0x14:  
		/** INR E */
		case 0x1c:  
		/** INR H */
		case 0x24:  
		/** INR L */
		case 0x2c:  
		/** INR (HL) */
		case 0x34:  
		/** INR A */
		case 0x3c:  
			op_inr();
			break;
			
		/** 8 bit subtraction */
		/** SUB B */
		case 0x90:  
		/** SUB C */
		case 0x91:  
		/** SUB D */
		case 0x92:  
		/** SUB E */
		case 0x93:  
		/** SUB H */
		case 0x94:  
		/** SUB L */
		case 0x95:  
		/** SUB (HL) */
		case 0x96:  
		/** SUB A */
		case 0x97:  
		/** SUI data */
		case 0xd6:  
		/** SBB B */
		case 0x98:  
		/** SBB C */
		case 0x99:  
		/** SBB D */
		case 0x9a:  
		/** SBB E */
		case 0x9b:  
		/** SBB H */
		case 0x9c:  
		/** SBB L */
		case 0x9d:  
		/** SBB (HL) */
		case 0x9e:  
		/** SBB A */
		case 0x9f:  
		/** SBI data */
		case 0xde:  
			op_sub();
			break;
			
		/** 8 bit addition */
		/** ADI data */
		case 0xc6:  
		/** ADC B */
		case 0x88:  
		/** ADC C */
		case 0x89:  
		/** ADC D */
		case 0x8a:  
		/** ADC E */
		case 0x8b:  
		/** ADC H */
		case 0x8c:  
		/** ADC L */
		case 0x8d:  
		/** ADC (HL) */
		case 0x8e:  
		/** ADC A */
		case 0x8f:  
		/** ACI data */
		case 0xce:  
		/** ADD B */
		case 0x80:  
		/** ADD C */
		case 0x81:  
		/** ADD D */
		case 0x82:  
		/** ADD E */
		case 0x83:  
		/** ADD H */
		case 0x84:  
		/** ADD L */
		case 0x85:  
		/** ADD A */
		case 0x87:  
		/** ADD (HL) */
		case 0x86:  
			op_add();
			break;
			
		/** 8 bit move */
		/** MOV B,B */
		case 0x40:  
		/** MOV B,C */
		case 0x41:  
		/** MOV B,D */
		case 0x42:  
		/** MOV B,E */
		case 0x43:  
		/** MOV B,H */
		case 0x44:  
		/** MOV B,L */
		case 0x45:  
		/** MOV B,(HL) */
		case 0x46:  
		/** MOV B,A */
		case 0x47:  
		/** MOV C,B */
		case 0x48:  
		/** MOV C,C */
		case 0x49:  
		/** MOV C,D */
		case 0x4a:  
		/** MOV C,E */
		case 0x4b:  
		/** MOV C,H */
		case 0x4c:  
		/** MOV C,L */
		case 0x4d:  
		/** MOV C,(HL) */
		case 0x4e:  
		/** MOV C,A */
		case 0x4f:  
		/** MOV D,B */
		case 0x50:  
		/** MOV D,C */
		case 0x51:  
		/** MOV D,D */
		case 0x52:  
		/** MOV D,E */
		case 0x53:  
		/** MOV D,H */
		case 0x54:  
		/** MOV D,L */
		case 0x55:  
		/** MOV D,(HL) */
		case 0x56:  
		/** MOV D,A */
		case 0x57:  
		/** MOV E,B */
		case 0x58:  
		/** MOV E,C */
		case 0x59:  
		/** MOV E,D */
		case 0x5a:  
		/** MOV E,E */
		case 0x5b:  
		/** MOV E,H */
		case 0x5c:  
		/** MOV E,L */
		case 0x5d:  
		/** MOV E,(HL) */
		case 0x5e:  
		/** MOV E,A */
		case 0x5f:  
		/** MOV H,B */
		case 0x60:  
		/** MOV H,C */
		case 0x61:  
		/** MOV H,D */
		case 0x62:  
		/** MOV H,E */
		case 0x63:  
		/** MOV H,H */
		case 0x64:  
		/** MOV H,L */
		case 0x65:  
		/** MOV H,(HL) */
		case 0x66:  
		/** MOV H,A */
		case 0x67:  
		/** MOV L,B */
		case 0x68:  
		/** MOV L,C */
		case 0x69:  
		/** MOV L,D */
		case 0x6a:  
		/** MOV L,E */
		case 0x6b:  
		/** MOV L,H */
		case 0x6c:  
		/** MOV L,L */
		case 0x6d:  
		/** MOV L,(HL) */
		case 0x6e:  
		/** MOV L,A */
		case 0x6f:  
		/** MOV A,B */
		case 0x78:  
		/** MOV A,C */
		case 0x79:  
		/** MOV A,D */
		case 0x7a:  
		/** MOV A,E */
		case 0x7b:  
		/** MOV A,H */
		case 0x7c:  
		/** MOV A,L */
		case 0x7d:  
		/** MOV A,(HL) */
		case 0x7e:  
		/** MOV A,A */
		case 0x7f:  
		/** MOV (HL),B */
		case 0x70:  
		/** MOV (HL),C */
		case 0x71:  
		/** MOV (HL),D */
		case 0x72:  
		/** MOV (HL),E */
		case 0x73:  
		/** MOV (HL),H */
		case 0x74:  
		/** MOV (HL),L */
		case 0x75:  
		/** MOV (HL),A */
		case 0x77:  
			op_mov();
			break;
			
		/** 8/16 bit load */
		/** MVI B */
		case 0x06:  
		/** MVI C */
		case 0x0e:  
		/** MVI D */
		case 0x16:  
		/** MVI E */
		case 0x1e:  
		/** MVI H */
		case 0x26:  
		/** MVI L */
		case 0x2e:  
		/** MVI (HL) - load data to memory */
		case 0x36:  
		/** MVI A */
		case 0x3e:  
		/** LXI BC,data-l,data-h */
		case 0x01:  
		/** LXI DE,data-l,data-h */
		case 0x11:  
		/** LXI HL,data-l,data-h */
		case 0x21:  
		/** LXI SP,data-l,data-h */
		case 0x31:  
		/** LDA addr */
		case 0x3a:  
		/** STA addr */
		case 0x32:  
		/** SHLD addr */
		case 0x22:  
		/** LHLD addr */
		case 0x2a:  
		/** LDAX BC */
		case 0x0a:  
		/** LDAX DE */
		case 0x1a:  
		/** STAX BC */
		case 0x02:  
		/** STAX DE */
		case 0x12:  
		/** XCHG */
		case 0xeb:  
			op_lxi();
			break;
			
		/** special opcode()s */
	
		/** CMA - compliment Acc */
		case 0x2f:  
		/** STC - Set carry flag */
		case 0x37:  
		/** CMC - compliment the carry flag */
		case 0x3f:  
		/** DAA - decimal adjust acc */
		case 0x27:  
			op_special();
			break;
		default:
			bad_opcode(getRegPC(),opcode());
			break;
	}

}

