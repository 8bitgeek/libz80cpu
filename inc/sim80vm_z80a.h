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
#ifndef SIM80VM_Z80A_H
#define SIM80VM_Z80A_H

#include <sim80vm_i8080.h>

class sim80vm_z80a : public sim80vm_i8080
{
	public:
		sim80vm_z80a(sim80mem* m,sim80io* io);
	
		~sim80vm_z80a();
		bool getFlagN()				{return N;}
		
		virtual uint8_t getRegA2() {return _a;}
		virtual uint8_t getRegF2() {return _f;}
		virtual uint8_t getRegB2() {return _b;}
		virtual uint8_t getRegC2() {return _c;}
		virtual uint8_t getRegD2() {return _d;}
		virtual uint8_t getRegE2() {return _e;}
		virtual uint8_t getRegH2() {return _h;}
		virtual uint8_t getRegL2() {return _l;}
		virtual uint16_t getRegIX(){return ix;}
		virtual uint16_t getRegIY(){return iy;}
		
	protected:
		virtual void run2();
		virtual void setFlagN(bool f)			{N=f;}
	
		virtual void setRegA2(uint8_t reg)		{_a=reg;}
		virtual void setRegF2(uint8_t reg)		{_f=reg;}
		virtual void setRegB2(uint8_t reg)		{_b=reg;}
		virtual void setRegC2(uint8_t reg)		{_c=reg;}
		virtual void setRegD2(uint8_t reg)		{_d=reg;}
		virtual void setRegE2(uint8_t reg)		{_e=reg;}
		virtual void setRegH2(uint8_t reg)		{_h=reg;}
		virtual void setRegL2(uint8_t reg)		{_l=reg;}
		virtual void setRegIX(uint16_t reg)		{ix=reg;}
		virtual void setRegIY(uint16_t reg)		{iy=reg;}
		
		virtual void rstINT(); /* INT */
		virtual void rstNMI(); /* NMI */
	
	protected:
		void op_bits();         					/** Z/80 bit operations */
		void op_ix();         						/** Z/80 IX operations */
		void op_iy();         						/** Z/80 IY operations */
		
		virtual void op_interrupts();				/** z80 interrupt and misc accum. opcodes */
		virtual void op_stack();		/* stack operations */
		virtual void op_flow();         /* program flow */
		virtual void op_compare();      /* compare operations */
		virtual void op_or();			/* OR operations */
		virtual void op_xor();			/* XOR operations */
		virtual void op_and();			/* AND operations */
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
				
		uint16_t getIndex(int8_t displacement);
		virtual	uint16_t getIndex();
		virtual void putIndex(uint16_t index);
		
		virtual void exec_opcode();
		virtual uint8_t* reg8ptr(uint8_t i,uint8_t disp=0);	/** get 8-bit register from opcode register index.. */
		
		typedef enum {								/** the index register involved in the current instruction */
			reg_HL=0,
   			reg_IX,
	  		reg_IY
		} t_index;
		

		t_index index;								/** current instruction 16 bit register addressing mode (HL),(IX),(IY) */
		uint8_t _f;							/** 8 bit (alternate) status register */
		uint8_t _a, _b, _c, _d, _e, _h, _l; 	/** 8 bit GP (alternate) registers */
		uint8_t r;							/** 8 bit refresh register */
		uint8_t N;							/** subtract flag */
		uint16_t ix,iy;						/** index registers */
	private:
};

#endif
