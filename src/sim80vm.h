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
#ifndef SIM80VM_H
#define SIM80VM_H

#include <sim80mem.h>

class sim80vm 
{
	public:
		sim80vm(sim80mem* m);
	
		~sim80vm();
	
		/** get registers... */
		virtual uint8_t getRegA()=0;
		virtual uint8_t getRegF()=0;
		virtual uint8_t getRegB()=0;
		virtual uint8_t getRegC()=0;
		virtual uint8_t getRegD()=0;
		virtual uint8_t getRegE()=0;
		virtual uint8_t getRegH()=0;
		virtual uint8_t getRegL()=0;
		virtual uint16_t getRegSP()=0;
		virtual uint16_t getRegPC()=0;
		
		virtual uint8_t getRegA2() {return 0;}
		virtual uint8_t getRegF2() {return 0;}
		virtual uint8_t getRegB2() {return 0;}
		virtual uint8_t getRegC2() {return 0;}
		virtual uint8_t getRegD2() {return 0;}
		virtual uint8_t getRegE2() {return 0;}
		virtual uint8_t getRegH2() {return 0;}
		virtual uint8_t getRegL2() {return 0;}
		virtual uint16_t getRegIX(){return 0;}
		virtual uint16_t getRegIY(){return 0;}
		
		/** get boolean flags... */
		virtual bool getFlagI()=0;
		virtual bool getFlagS()=0;
		virtual bool getFlagZ()=0;
		virtual bool getFlagAC()=0;
		virtual bool getFlagP()=0;
		virtual bool getFlagN()=0;
		virtual bool getFlagCY()=0;
	
	protected:
		virtual void bad_opcode(uint16_t addr, uint8_t) {halt();}
		virtual void halt() {for(;;);}
    
	protected:
		/** set registers... */
		virtual void setRegA(uint8_t reg)=0;
		virtual void setRegF(uint8_t reg)=0;
		virtual void setRegB(uint8_t reg)=0;
		virtual void setRegC(uint8_t reg)=0;
		virtual void setRegD(uint8_t reg)=0;
		virtual void setRegE(uint8_t reg)=0;
		virtual void setRegH(uint8_t reg)=0;
		virtual void setRegL(uint8_t reg)=0;
		virtual void setRegSP(uint16_t reg)=0;
		virtual void setRegPC(uint16_t reg)=0;
		
		virtual void setRegA2(uint8_t reg)		{}
		virtual void setRegF2(uint8_t reg)		{}
		virtual void setRegB2(uint8_t reg)		{}
		virtual void setRegC2(uint8_t reg)		{}
		virtual void setRegD2(uint8_t reg)		{}
		virtual void setRegE2(uint8_t reg)		{}
		virtual void setRegH2(uint8_t reg)		{}
		virtual void setRegL2(uint8_t reg)		{}
		virtual void setRegIX(uint16_t reg)		{}
		virtual void setRegIY(uint16_t reg)		{}
		
		/** set any boolean flags... */
		virtual void setFlagI(bool f)=0;
		virtual void setFlagS(bool f)=0;
		virtual void setFlagZ(bool f)=0;
		virtual void setFlagAC(bool f)=0;
		virtual void setFlagP(bool f)=0;
		virtual void setFlagN(bool f)=0;
		virtual void setFlagCY(bool f)=0;
		
		virtual void run1();
		virtual void run2()=0;
		
		/** interrupt vectors... */
		virtual void rst0()=0; /* RST 0 */
		virtual void rst1()=0; /* RST 1 */
		virtual void rst2()=0; /* RST 2 */
		virtual void rst3()=0; /* RST 3 */
		virtual void rst4()=0; /* RST 4 */
		virtual void rst5()=0; /* RST 5 */
		virtual void rst6()=0; /* RST 6 */
		virtual void rst7()=0; /* RST 7 */
		virtual void rstINT() {rst7();} /* INT */
		virtual void rstNMI() {rst7();} /* NMI */

	protected:
		virtual	uint16_t getIndex()=0;
		virtual void putIndex(uint16_t index)=0;
		virtual void exec_opcode()=0;
		virtual uint8_t* reg8ptr(uint8_t i,uint8_t disp=0)=0;	/** get 8-bit register from opcode register index.. */
		
		sim80mem*	mem; 						/** memory class */
		uint8_t 	opcode;						/** opcode read from program memory */
		int 		rst_vector;					/** reset (interrupt) vector */
};

#endif
