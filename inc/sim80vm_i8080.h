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
#ifndef SIM80VM_I8080_H
#define SIM80VM_I8080_H

#include <sim80vm.h>

class sim80vm_i8080 : public sim80vm
{
	public:
		sim80vm_i8080(sim80mem* msim80mem,sim80io* io);
		virtual ~sim80vm_i8080();
		
		/** get registers... */
		uint8_t getRegA()		{return a;}
		uint8_t getRegF()		{return f;}
		uint8_t getRegB()		{return b;}
		uint8_t getRegC()		{return c;}
		uint8_t getRegD()		{return d;}
		uint8_t getRegE()		{return e;}
		uint8_t getRegH()		{return h;}
		uint8_t getRegL()		{return l;}
		uint16_t getRegSP()	{return sp;}
		uint16_t getRegPC()	{return pc;}
		/** get boolean flags... */
		bool getFlagI()				{return I;}
		bool getFlagS()				{return S;}
		bool getFlagZ()				{return Z;}
		bool getFlagAC()			{return AC;}
		bool getFlagP()				{return P;}
		bool getFlagN()				{return true;} /* always 1 on i8080 */
		bool getFlagCY()			{return CY;}
	
	protected:
		virtual void setRegA(uint8_t reg)		{a=reg;}
		virtual void setRegF(uint8_t reg)		{f=reg;}
		virtual void setRegB(uint8_t reg)		{b=reg;}
		virtual void setRegC(uint8_t reg)		{c=reg;}
		virtual void setRegD(uint8_t reg)		{d=reg;}
		virtual void setRegE(uint8_t reg)		{e=reg;}
		virtual void setRegH(uint8_t reg)		{h=reg;}
		virtual void setRegL(uint8_t reg)		{l=reg;}
		virtual void setRegSP(uint16_t reg)	{sp=reg;}
		virtual void setRegPC(uint16_t reg)	{pc=reg;}
		/** set any boolean flags in the VM - from the form... */
		virtual void setFlagI(bool f)				{I=f;}
		virtual void setFlagS(bool f)				{S=f;}
		virtual void setFlagZ(bool f)				{Z=f;}
		virtual void setFlagAC(bool f)				{AC=f;}
		virtual void setFlagP(bool f)				{P=f;}
		virtual void setFlagN(bool f)				{}
		virtual void setFlagCY(bool f)				{CY=f;}
		/** reset (interrupt) vectors... */
		virtual void rst0(); /* RST 0 */
		virtual void rst1(); /* RST 1 */
		virtual void rst2(); /* RST 2 */
		virtual void rst3(); /* RST 3 */
		virtual void rst4(); /* RST 4 */
		virtual void rst5(); /* RST 5 */
		virtual void rst6(); /* RST 6 */
		virtual void rst7(); /* RST 7 */
		/** run... */
		virtual void run2();
	
	protected:
		
		virtual void op_stack();		/* stack operations */
		virtual void op_io();			/* I/O operations */
		virtual void op_flow();         /* program flow */
		virtual void op_compare();      /* compare operations */
		virtual void op_or();			/* OR operations */
		virtual void op_rotate();		/* rotate operations */
		virtual void op_xor();			/* XOR operations */
		virtual void op_and();			/* AND operations */
		virtual void op_control();		/* control operations */
		virtual void op_dad();			/* 16 bit ADD */
		virtual void op_dcx();			/* 16 bit decrement */
		virtual void op_inx();			/* 16 bit increment */
		virtual void op_dcr();			/* 8 bit decrement */
		virtual void op_inr();			/* 8 bit increment */
		virtual void op_sub();			/* 8 bit subtraction */
		virtual void op_add();			/* 8 bit addition */
		virtual void op_mov();			/* 8 bit move */
		virtual void op_lxi();			/* 8/16 bit load */
		virtual void op_special();		/* misc. opcodes */
        
	protected:
		virtual	uint16_t getIndex() 			{return (h<<8)|l;}
		virtual void putIndex(uint16_t index)	{l=(index&0xff); h=((index&0xff00)>>8);}
		
		virtual void exec_opcode();

		virtual uint8_t  reg8get(uint8_t i,uint8_t disp=0);
		virtual uint8_t  reg8put(uint8_t i,uint8_t v,uint8_t disp=0);

		inline void sign(uint8_t num) {
			S=(num>>7);			/** transfer the sign bit */
		}
		inline void auxcarry(uint8_t num) {
			AC=(num>>4)&1;		/** transfer the AC bit */ 
		}
		inline void zero(int num) {
			(num==0)?Z=1:Z=0;
		}
		inline void carry(int num) {
			((num>255)||(num<0))?CY=1:CY=0;
		}
		inline void parity(int num) {
			int p=0;
			while(num) { p+=(num&1); num=num>>1; }
			P=p;
		}
		
		uint8_t f;					/* 8 bit status register */
		uint8_t a, b, c, d, e, h, l;  /* 8 bit GP registers */
		uint8_t w;					/* 8 bit internal registers */
		uint16_t sp;				/* 16 bit stack pointer */
		uint16_t pc;				/* 16 bit Program Counter */
    
		uint8_t I;					/* interrupt flag */
		uint8_t S;					/* sign flag: S=0 (plus) or S=1 (minus) */
		uint8_t Z;					/* zero flag: Z=0 (not zero) or Z=1 (zero) */
		uint8_t AC;					/* Aux carry: if bit 3 rolled to bit 4, set, else unset */
		uint8_t P;					/* parity flag: P=0 (parity odd) or P=1 (parity even) */
		uint8_t CY;					/* carry flag: CY=0 (no carry) or CY=1 (carry) */
    
		uint16_t memory;			/* address of HL */
    
		int temp;							/* use temp for CY flag test */

};

#endif
