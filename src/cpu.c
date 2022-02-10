/******************************************************************************
 ________          ___ ________  ________          ________  ________  ___  ___     
|\_____  \        /  /|\   __  \|\   __  \        |\   ____\|\   __  \|\  \|\  \    
 \|___/  /|      /  //\ \  \|\  \ \  \|\  \       \ \  \___|\ \  \|\  \ \  \\\  \   
     /  / /     /  //  \ \   __  \ \  \\\  \       \ \  \    \ \   ____\ \  \\\  \  
    /  /_/__   /  //    \ \  \|\  \ \  \\\  \       \ \  \____\ \  \___|\ \  \\\  \ 
   |\________\/_ //      \ \_______\ \_______\       \ \_______\ \__\    \ \_______\
    \|_______|__|/        \|_______|\|_______|        \|_______|\|__|     \|_______|
                                                                                    
MIT License

Copyright (c) 2021 Mike Sharkey

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

#include <cpu.h>
#include <opcodes.h>
#include <string.h>

typedef union {
	uint16_t Word;
	struct {
		uint8_t L;
		uint8_t H;
	} Bytes;
} reg_t;

typedef struct _cpu_state_t_
{
	uint8_t IR;

	reg_t AF, SP, PC;
	reg_t BC, DE;
	reg_t HL, IX, IY;

	reg_t AF1;
	reg_t BC1, 	DE1;
	reg_t HL1;

	bool IFF1, 	IFF2;
	bool FlagZ, FlagNZ;
	bool FlagC, FlagNC;
	bool FlagPO,FlagPE;
	bool FlagM, FlagP;
	bool FlagN, FlagH;
	bool Flag3, Flag5;

	uint8_t 	mem_byte;
	uint16_t 	mem_addr;

	bool 		mem_write_indirect;
	bool 		mem_write;

	bool 		irq_pend;
	bool 		irq_en;
	uint32_t 	phl_t_states;
	uint32_t 	t_state;
	uint32_t 	clocks;

	bool		idx_ins;
	uint8_t		idx;

	reg_t* 		PointerReg; // make a get/put fn

	cpu_mem_rd_cb_t	cpu_mem_rd_cb;
	cpu_mem_wr_cb_t cpu_mem_wr_cb;
	cpu_io_rd_cb_t 	cpu_io_rd_cb;
	cpu_io_wr_cb_t 	cpu_io_wr_cb;

} cpu_state_t;

static cpu_state_t cpu_state;

static uint16_t	GetRegister(cpu_reg_t);
static void 	Push(uint16_t);
static void 	Pop(uint16_t*);

// Initialize the Z80 simulation.

void Init(	
			cpu_mem_rd_cb_t	cpu_mem_rd_cb,
			cpu_mem_wr_cb_t cpu_mem_wr_cb,
			cpu_io_rd_cb_t cpu_io_rd_cb,
			cpu_io_wr_cb_t cpu_io_wr_cb 
		)
{
	memset(&cpu_state,0,sizeof(cpu_state_t));

	cpu_state.cpu_mem_rd_cb = cpu_mem_rd_cb;
	cpu_state.cpu_mem_wr_cb = cpu_mem_wr_cb;
	cpu_state.cpu_io_rd_cb = cpu_io_rd_cb;
	cpu_state.cpu_io_wr_cb = cpu_io_wr_cb;
}

void RaiseIRQ() {
	cpu_state.irq_pend=true;
}

#if 0

// Return the contents of cell number Address in main memory.

inline uint8_t cpu_state.cpu_mem_rd_cb(uint16_t Address) {
	mem_addr=Address;
	cpu_state.mem_byte=Memory[Address];
	return cpu_state.mem_byte;
}


// Write the contents of Value to cell number Address in main memory,
// if this is allowed.

void cpu_state.cpu_mem_wr_cb(uint16_t Address, uint8_t Value) {
    mem_addr=Address;
	cpu_state.mem_byte=Value;
	cpu_state.mem_write=true;
    Memory[Address]=Value;
}

inline uint8_t cpu_state.cpu_io_rd_cb(uint8_t Port) {
	return 0;
}

void cpu_state.cpu_io_wr_cb(uint8_t Port, uint8_t Value) {
	fprintf(stdout, "WARNING: Writing %02x into port %02x\n", Value, Port);
}

#endif

// Process an interrupt request
void ProcessIRQ() {
	Push(cpu_state.PC.Word);
	cpu_state.PC.Word=0x0038;
	cpu_state.irq_pend=false;
}


// Return the bit specifying the sign in a 2's complement value of type 'byte'.

inline bool SignBit(uint8_t Value) {
	if(Value&0x80) return true; else return false;
}


// Return true if the parity of uint8_t is even, otherwise false.

inline bool Parity(uint8_t Byte) {
        uint8_t Temp;
	Temp=Byte;
        Temp=Temp^(Temp>>4);
        Temp=Temp^(Temp>>2);
        Temp=Temp^(Temp>>1);
        return ((Temp&1)!=0?false:true);
}


// Copy the contents of the Flag* bool variables into the actual F register.

void StoreFlags() {
	cpu_state.AF.Bytes.L=(
		(cpu_state.FlagC?(1<<0):0) |
		(cpu_state.FlagN?(1<<1):0) |
		(cpu_state.FlagPE?(1<<2):0)|
		(cpu_state.Flag3?(1<<3):0) |
		(cpu_state.FlagH?(1<<4):0) |
		(cpu_state.Flag5?(1<<5):0) |
		(cpu_state.FlagZ?(1<<6):0) |
		(cpu_state.FlagM?(1<<7):0)
	);
}


// Set the Flag* variables depending on the contents of Datum, except those
// that cannot be inferred from it alone. Don't set the actual F register.
// Also calculate parity and store it into cpu_state.FlagPE/cpu_state.FlagPO if GiveParity==true.

void SetFlags(uint8_t Datum) {
	cpu_state.Flag3=(Datum&0x08)!=0;
	cpu_state.Flag5=(Datum&0x20)!=0;
	if(Datum==0) cpu_state.FlagZ=1; else cpu_state.FlagZ=0;
	cpu_state.FlagNZ=!cpu_state.FlagZ;
	cpu_state.FlagP=!SignBit(Datum);
    cpu_state.FlagM=!cpu_state.FlagP;
	cpu_state.FlagPE=Parity(Datum);
	cpu_state.FlagPO=!cpu_state.FlagPE;
}


// Add Operand to *Register, and store the result into *Register. Set the Flag*
// variables accordingly.

void AddByte(uint8_t* Register, uint8_t Operand) {
	uint8_t Op1=*Register, Op2=Operand;
	long Sum;
	Sum=Op1+Op2;
	*Register=(uint8_t)Sum;
	SetFlags(*Register);
	if((Op1&0x08)^(Op2&0x08)^(Sum&0x08)) cpu_state.FlagH=1; else cpu_state.FlagH=0;
	if(Sum>(uint16_t)0xFF) cpu_state.FlagNC=!(cpu_state.FlagC=1); else cpu_state.FlagC=!(cpu_state.FlagNC=1);
	if(SignBit(Op1)==SignBit(Op2) && SignBit(Sum)!=SignBit(Op1)) cpu_state.FlagPO=!(cpu_state.FlagPE=1); else cpu_state.FlagPO=!(cpu_state.FlagPE=0);
	cpu_state.FlagN=0;
	cpu_state.t_state+=4;
}


// Subtract Operand from *Register, and store the result into *Register. Set the Flag*
// variables accordingly.

void SubByte(uint8_t* Register, uint8_t Operand) {
	uint8_t Op1=*Register, Op2=-Operand;
	long Sum;
	Sum=Op1+Op2;
	*Register=(uint8_t)Sum;
	SetFlags(*Register);
	if((Op1&0x08)^(Op2&0x08)^(Sum&0x08)) cpu_state.FlagH=0; else cpu_state.FlagH=1;
	if(Sum>(uint16_t)0xFF) cpu_state.FlagNC=!(cpu_state.FlagC=0); else cpu_state.FlagC=!(cpu_state.FlagNC=0);
	if(SignBit(Op1)==SignBit(Op2) && SignBit(Sum)!=SignBit(Op1)) cpu_state.FlagPO=!(cpu_state.FlagPE=1); else cpu_state.FlagPO=!(cpu_state.FlagPE=0);
	cpu_state.FlagN=0;
	cpu_state.t_state+=4;
}


// As per AddByte(), but do it for two words, and only set the flags required by
// a uint16_t addition.

void AddWord(uint16_t* Register, uint16_t Operand) {
	long Sum;
	Sum=*Register+Operand;
    if(((*Register)&0x0F00)+(Operand&0x0F00)>0x0F00) cpu_state.FlagH=1; else cpu_state.FlagH=0;
	*Register=(uint16_t)Sum;
	if(Sum>0xFFFF) cpu_state.FlagNC=!(cpu_state.FlagC=1); else cpu_state.FlagC=!(cpu_state.FlagNC=1);
	cpu_state.FlagN=0;
	cpu_state.t_state+=11;
}


// boolal And of two bytes, store result into *Register and set the Flag*s.

void And(uint8_t* Register, uint8_t Operand) {
	*Register=*Register&Operand;
	// Flags should be correct
	SetFlags(*Register);
	cpu_state.FlagNC=!(cpu_state.FlagC=0);
	cpu_state.FlagN=0;
	//StoreFlags();
	cpu_state.t_state+=4;
}


// boolal exclusive Or of two bytes, store result into *Register and set the Flag*s.

void XOr(uint8_t* Register, uint8_t Operand) {
	*Register=*Register^Operand;
	// Flags should be correct, but aren't
	SetFlags(*Register);
	cpu_state.FlagNC=!(cpu_state.FlagC=0);
	cpu_state.FlagN=0;
	cpu_state.FlagH=0;
	//StoreFlags();
	cpu_state.t_state+=4;
}


// boolal Or of two bytes, store result into *Register and set the Flag*s.

void Or(uint8_t* Register, uint8_t Operand) {
	*Register=*Register|Operand;
	// Flags should be correct
	SetFlags(*Register);
	cpu_state.FlagNC=!(cpu_state.FlagC=0);
	cpu_state.FlagN=0;
	//StoreFlags();
	cpu_state.t_state+=4;
}


// Subtract Operand to *Register; don't store the result anywhere, only set the Flag*s.

void Compare(uint8_t* Register, uint8_t Operand) {
	uint8_t Temp=*Register;
	SubByte(&Temp, Operand);
    cpu_state.FlagN=1;
	cpu_state.t_state+=4;
}


// Fetch a direct operand of the length of a uint16_t (16 bits).

uint16_t GetWordOperand() {
	reg_t WordOperand;
	WordOperand.Bytes.L=cpu_state.cpu_mem_rd_cb(cpu_state.PC.Word++);
	WordOperand.Bytes.H=cpu_state.cpu_mem_rd_cb(cpu_state.PC.Word++);
	return WordOperand.Word;
}


// Branch to NewAddress.

void JumpAbsolute(uint16_t NewAddress) {
	cpu_state.PC.Word=NewAddress;
	cpu_state.t_state+=10;
}


// Branch to PC+idx.

void JumpRelative(uint8_t idx) {
    cpu_state.PC.Word+=(uint16_t)(int8_t)cpu_state.idx;
	cpu_state.t_state+=12;
}


// Store PC into the stack, then branch to ProcAddress.

void Call(uint16_t ProcAddress) {
    Push(cpu_state.PC.Word);
	JumpAbsolute(ProcAddress);
	cpu_state.t_state-=4;
}


// Push Register into the stack.

static void Push(uint16_t Register) {
	cpu_state.cpu_mem_wr_cb(cpu_state.SP.Word-2, (Register&0x00FF)>>0);
    cpu_state.cpu_mem_wr_cb(cpu_state.SP.Word-1, (Register&0xFF00)>>8);
	cpu_state.SP.Word-=2;
	cpu_state.t_state+=11;
}


// Pop a value from the stack and store it into *Register.

static void Pop(uint16_t* Register) {
	*Register=cpu_state.cpu_mem_rd_cb(cpu_state.SP.Word++);
	(*Register)|=cpu_state.cpu_mem_rd_cb(cpu_state.SP.Word++)<<8;
	if(Register==&cpu_state.AF.Word) {
		cpu_state.FlagNC=!(cpu_state.FlagC=(cpu_state.AF.Bytes.L>>0)&1);
		cpu_state.FlagNZ=!(cpu_state.FlagZ=(cpu_state.AF.Bytes.L>>6)&1);
		cpu_state.FlagPO=!(cpu_state.FlagPE=(cpu_state.AF.Bytes.L>>2)&1);
		cpu_state.FlagP=!(cpu_state.FlagM=(cpu_state.AF.Bytes.L>>7)&1);
		cpu_state.FlagN=(cpu_state.AF.Bytes.L>>1)&1;
		cpu_state.FlagH=(cpu_state.AF.Bytes.L>>4)&1;
		cpu_state.Flag3=(cpu_state.AF.Bytes.L>>3)&1;
		cpu_state.Flag5=(cpu_state.AF.Bytes.L>>5)&1;
	}
	cpu_state.t_state+=10;
}


// Swap two words.

void Swap(uint16_t* Reg1, uint16_t* Reg2) {
	uint16_t Temp;
	Temp=*Reg1;
	*Reg1=*Reg2;
	*Reg2=Temp;
}


// Return the address of the register specified in Opcode, with Opcode being
// a Z80 opcode whose bits 3..5 specify a register.

uint8_t* OperandR(uint8_t Opcode) {
    if(OPARG_R_A(Opcode)) return &cpu_state.AF.Bytes.H;
	if(OPARG_R_B(Opcode)) return &cpu_state.BC.Bytes.H;
	if(OPARG_R_C(Opcode)) return &cpu_state.BC.Bytes.L;
	if(OPARG_R_D(Opcode)) return &cpu_state.DE.Bytes.H;
	if(OPARG_R_E(Opcode)) return &cpu_state.DE.Bytes.L;
	if(OPARG_R_H(Opcode)) return &cpu_state.HL.Bytes.H;
	if(OPARG_R_L(Opcode)) return &cpu_state.HL.Bytes.L;
	if(OPARG_R_PHL(Opcode)) {
		cpu_state.t_state+=cpu_state.phl_t_states;
		cpu_state.mem_write_indirect=true;
		cpu_state.mem_byte=cpu_state.cpu_mem_rd_cb(cpu_state.PointerReg->Word+(uint16_t)(int8_t)cpu_state.idx);
		return &cpu_state.mem_byte;
	}
	return &cpu_state.AF.Bytes.H;
}


// Return the address of the register specified in Opcode, with Opcode being
// a Z80 opcode whose bits 0..2 specify a register.

uint8_t* OperandS(uint8_t Opcode) {
	if(OPARG_S_A(Opcode)) return &cpu_state.AF.Bytes.H;
	if(OPARG_S_B(Opcode)) return &cpu_state.BC.Bytes.H;
	if(OPARG_S_C(Opcode)) return &cpu_state.BC.Bytes.L;
	if(OPARG_S_D(Opcode)) return &cpu_state.DE.Bytes.H;
	if(OPARG_S_E(Opcode)) return &cpu_state.DE.Bytes.L;
	if(OPARG_S_H(Opcode)) return &cpu_state.HL.Bytes.H;
	if(OPARG_S_L(Opcode)) return &cpu_state.HL.Bytes.L;
	if(OPARG_S_PHL(Opcode)) {
		cpu_state.t_state+=cpu_state.phl_t_states;
		cpu_state.mem_byte=cpu_state.cpu_mem_rd_cb(cpu_state.PointerReg->Word+(uint16_t)(int8_t)cpu_state.idx);
		return &cpu_state.mem_byte;
	}
	return &cpu_state.AF.Bytes.H;
}


// Return the address of the register pair specified in Opcode, with Opcode
// being a Z80 opcode whose bits 4 and 5 specify a register pair.

uint16_t* OperandP(uint8_t Opcode) {
	if(OPARG_P_BC(Opcode)) return &cpu_state.BC.Word;
	if(OPARG_P_DE(Opcode)) return &cpu_state.DE.Word;
	if(OPARG_P_HL(Opcode)) return &(cpu_state.PointerReg->Word);
	if(OPARG_P_SP(Opcode)) return &cpu_state.SP.Word;
	return NULL;
}


// Return the address of a Flag* variable as specified in Opcode, with
// Opcode being a z80 opcode whose bits 3..5 specify a flag.

bool* OperandF(uint8_t Opcode) {
	if(OPARG_F_NZ(Opcode)) 	return &cpu_state.FlagNZ;
	if(OPARG_F_Z(Opcode)) 	return &cpu_state.FlagZ;
	if(OPARG_F_NC(Opcode)) 	return &cpu_state.FlagNC;
	if(OPARG_F_C(Opcode)) 	return &cpu_state.FlagC;
	if(OPARG_F_PO(Opcode)) 	return &cpu_state.FlagPO;
	if(OPARG_F_PE(Opcode)) 	return &cpu_state.FlagPE;
	if(OPARG_F_P(Opcode)) 	return &cpu_state.FlagP;
	if(OPARG_F_M(Opcode)) 	return &cpu_state.FlagM;
	return NULL;
}


// As per OperandF(), but works with those opcode that only accept one
// of four flags instead of eight in bits 3 and 4

bool* OperandSF(uint8_t Opcode) {
	if(OPARG_SF_NZ(Opcode)) return &cpu_state.FlagNZ;
	if(OPARG_SF_Z(Opcode)) 	return &cpu_state.FlagZ;
	if(OPARG_SF_NC(Opcode)) return &cpu_state.FlagNC;
	if(OPARG_SF_C(Opcode)) 	return &cpu_state.FlagC;
	return NULL;
}

// Execute the Z80 instruction pointed to by PC.
void Step() {

    uint16_t Word;
	uint8_t Byte;
	
	cpu_state.IR=cpu_state.cpu_mem_rd_cb(cpu_state.PC.Word++);
	cpu_state.mem_write_indirect=cpu_state.mem_write=false;
	if(OP_IXPREFIX(cpu_state.IR)) {
	    cpu_state.IR=cpu_state.cpu_mem_rd_cb(cpu_state.PC.Word++);
		cpu_state.PointerReg=&cpu_state.IX;
		cpu_state.idx_ins=true;
		cpu_state.t_state+=4;
	} else if(OP_IYPREFIX(cpu_state.IR)) {
	    cpu_state.IR=cpu_state.cpu_mem_rd_cb(cpu_state.PC.Word++);
		cpu_state.PointerReg=&cpu_state.IY;
		cpu_state.idx_ins=true;
		cpu_state.t_state+=4;
	} else {
		cpu_state.PointerReg=&cpu_state.HL;
		cpu_state.idx_ins=false;
	}
	cpu_state.idx=0x00;
	cpu_state.phl_t_states=3;
	if(OP_HLT(cpu_state.IR)) {
		cpu_state.PC.Word--;
		cpu_state.t_state+=4;
	} else if(OP_LD_R_S(cpu_state.IR)) {
		if(cpu_state.idx_ins) cpu_state.idx=cpu_state.cpu_mem_rd_cb(cpu_state.PC.Word++);
		*OperandR(cpu_state.IR)=*OperandS(cpu_state.IR);
		cpu_state.t_state+=4;
	} else if(OP_LD_R_B(cpu_state.IR)) {
	    if(cpu_state.idx_ins) cpu_state.idx=cpu_state.cpu_mem_rd_cb(cpu_state.PC.Word++);
		*OperandR(cpu_state.IR)=cpu_state.cpu_mem_rd_cb(cpu_state.PC.Word++);
		cpu_state.t_state+=7;
	} else if(OP_LD_P_W(cpu_state.IR)) {
		*OperandP(cpu_state.IR)=cpu_state.cpu_mem_rd_cb(cpu_state.PC.Word++)|(cpu_state.cpu_mem_rd_cb(cpu_state.PC.Word++)<<8);
		cpu_state.t_state+=10;
	} else if(OP_LD_SP_HL(cpu_state.IR)) {
		cpu_state.SP.Word=cpu_state.PointerReg->Word;
		cpu_state.t_state+=10;
	} else if(OP_LD_A_PBC(cpu_state.IR)) {
		cpu_state.AF.Bytes.H=cpu_state.cpu_mem_rd_cb(cpu_state.BC.Word);
		cpu_state.t_state+=7;
	} else if(OP_LD_A_PDE(cpu_state.IR)) {
		cpu_state.AF.Bytes.H=cpu_state.cpu_mem_rd_cb(cpu_state.DE.Word);
		cpu_state.t_state+=7;
	} else if(OP_LD_A_PW(cpu_state.IR)) {
		cpu_state.AF.Bytes.H=cpu_state.cpu_mem_rd_cb(cpu_state.cpu_mem_rd_cb(cpu_state.PC.Word++)|(cpu_state.cpu_mem_rd_cb(cpu_state.PC.Word++)<<8));
		cpu_state.t_state+=13;
	} else if(OP_LD_PW_A(cpu_state.IR)) {
		cpu_state.cpu_mem_wr_cb(cpu_state.cpu_mem_rd_cb(cpu_state.PC.Word++)|(cpu_state.cpu_mem_rd_cb(cpu_state.PC.Word++)<<8), cpu_state.AF.Bytes.H);
		cpu_state.t_state+=13;
	} else if(OP_LD_HL_PW(cpu_state.IR)) {
		uint16_t Address=cpu_state.cpu_mem_rd_cb(cpu_state.PC.Word++)|(cpu_state.cpu_mem_rd_cb(cpu_state.PC.Word++)<<8);
		cpu_state.PointerReg->Word=cpu_state.cpu_mem_rd_cb(Address)|(cpu_state.cpu_mem_rd_cb(Address+1)<<8);
		cpu_state.t_state+=16;
	} else if(OP_LD_PBC_A(cpu_state.IR)) {
		cpu_state.cpu_mem_wr_cb(cpu_state.BC.Word, cpu_state.AF.Bytes.H);
		cpu_state.t_state+=7;
	} else if(OP_LD_PDE_A(cpu_state.IR)) {
		cpu_state.cpu_mem_wr_cb(cpu_state.DE.Word, cpu_state.AF.Bytes.H);
		cpu_state.t_state+=7;
	} else if(OP_ADD_S(cpu_state.IR)) {
		if(cpu_state.idx_ins) cpu_state.idx=cpu_state.cpu_mem_rd_cb(cpu_state.PC.Word++);
		if(OPMOD_CARRYIN(cpu_state.IR)) AddByte(&cpu_state.AF.Bytes.H, *OperandS(cpu_state.IR)+(cpu_state.FlagC?1:0)); else AddByte(&cpu_state.AF.Bytes.H, *OperandS(cpu_state.IR));
	} else if(OP_ADD_B(cpu_state.IR)) {
		if(OPMOD_CARRYIN(cpu_state.IR)) AddByte(&cpu_state.AF.Bytes.H, cpu_state.cpu_mem_rd_cb(cpu_state.PC.Word++)+(cpu_state.FlagC?1:0)); else AddByte(&cpu_state.AF.Bytes.H, cpu_state.cpu_mem_rd_cb(cpu_state.PC.Word++));
		cpu_state.t_state+=3;
	} else if(OP_SUB_S(cpu_state.IR)) {
		if(cpu_state.idx_ins) cpu_state.idx=cpu_state.cpu_mem_rd_cb(cpu_state.PC.Word++);
		if(OPMOD_CARRYIN(cpu_state.IR)) SubByte(&cpu_state.AF.Bytes.H, *OperandS(cpu_state.IR)+(cpu_state.FlagC?1:0)); else SubByte(&cpu_state.AF.Bytes.H, *OperandS(cpu_state.IR));
		cpu_state.FlagN=1;
	} else if(OP_SUB_B(cpu_state.IR)) {
		if(OPMOD_CARRYIN(cpu_state.IR)) /* ... */ ;
			SubByte(&cpu_state.AF.Bytes.H, cpu_state.cpu_mem_rd_cb(cpu_state.PC.Word++));
		cpu_state.FlagN=1;
		cpu_state.t_state+=3;
	} else if(OP_ADD_HL_P(cpu_state.IR)) {
		AddWord(&cpu_state.PointerReg->Word, *OperandP(cpu_state.IR));
	} else if(OP_INC_R(cpu_state.IR)) {
		uint8_t* Operand; bool Negative;
		if(cpu_state.idx_ins) cpu_state.idx=cpu_state.cpu_mem_rd_cb(cpu_state.PC.Word++);
                Operand=OperandR(cpu_state.IR);
		Negative=SignBit(*Operand);
		SetFlags(++(*Operand));
		if(Negative^(SignBit(*Operand))) cpu_state.FlagPO=!(cpu_state.FlagPE=true); else cpu_state.FlagPO=!(cpu_state.FlagPE=false);// Overflow
            cpu_state.FlagN=0;
		cpu_state.t_state+=4;
	} else if(OP_DEC_R(cpu_state.IR)) {
	    if(cpu_state.idx_ins) cpu_state.idx=cpu_state.cpu_mem_rd_cb(cpu_state.PC.Word++);
		SetFlags(--(*OperandR(cpu_state.IR)));
		cpu_state.FlagN=1;
		cpu_state.t_state+=4;
	} else if(OP_INC_P(cpu_state.IR)) {
		(*OperandP(cpu_state.IR))++;
		cpu_state.t_state+=6;
	} else if(OP_DEC_P(cpu_state.IR)) {
	        (*OperandP(cpu_state.IR))--;
		cpu_state.t_state+=6;
	} else if(OP_AND_S(cpu_state.IR)) {
	    if(cpu_state.idx_ins) cpu_state.idx=cpu_state.cpu_mem_rd_cb(cpu_state.PC.Word++);
		And(&cpu_state.AF.Bytes.H, *OperandS(cpu_state.IR));
	} else if(OP_AND_B(cpu_state.IR)) {
		And(&cpu_state.AF.Bytes.H, cpu_state.cpu_mem_rd_cb(cpu_state.PC.Word++));
	} else if(OP_XOR_S(cpu_state.IR)) {
		if(cpu_state.idx_ins) cpu_state.idx=cpu_state.cpu_mem_rd_cb(cpu_state.PC.Word++);
		XOr(&cpu_state.AF.Bytes.H, *OperandS(cpu_state.IR));
	} else if(OP_XOR_B(cpu_state.IR)) {
        XOr(&cpu_state.AF.Bytes.H, cpu_state.cpu_mem_rd_cb(cpu_state.PC.Word++));
	} else if(OP_OR_S(cpu_state.IR)) {
		if(cpu_state.idx_ins) cpu_state.idx=cpu_state.cpu_mem_rd_cb(cpu_state.PC.Word++);
		Or(&cpu_state.AF.Bytes.H, *OperandS(cpu_state.IR));
    } else if(OP_OR_B(cpu_state.IR)) {
        Or(&cpu_state.AF.Bytes.H, cpu_state.cpu_mem_rd_cb(cpu_state.PC.Word++));
	} else if(OP_CP_S(cpu_state.IR)) {
		if(cpu_state.idx_ins) cpu_state.idx=cpu_state.cpu_mem_rd_cb(cpu_state.PC.Word++);
		Compare(&cpu_state.AF.Bytes.H, *OperandS(cpu_state.IR));
	} else if(OP_CP_B(cpu_state.IR)) {
		Compare(&cpu_state.AF.Bytes.H, cpu_state.cpu_mem_rd_cb(cpu_state.PC.Word++));
		cpu_state.t_state+=3;
	} else if(OP_JP_W(cpu_state.IR)) {
		JumpAbsolute(GetWordOperand());
	} else if(OP_JP_F_W(cpu_state.IR)) {
		Word=GetWordOperand();
		if(*OperandF(cpu_state.IR)) JumpAbsolute(Word); else cpu_state.t_state+=10;
	} else if(OP_JP_PHL(cpu_state.IR)) {
		cpu_state.PC.Word=cpu_state.HL.Word;
		cpu_state.t_state+=4;
	} else if(OP_JR_B(cpu_state.IR)) {
		JumpRelative(cpu_state.cpu_mem_rd_cb(cpu_state.PC.Word++));
	} else if(OP_JR_SF_B(cpu_state.IR)) {
		Byte=cpu_state.cpu_mem_rd_cb(cpu_state.PC.Word++);
		if(*OperandF(cpu_state.IR)) JumpRelative(Byte); else cpu_state.t_state+=7;
	} else if(OP_DJNZ_B(cpu_state.IR)) {
		Byte=cpu_state.cpu_mem_rd_cb(cpu_state.PC.Word++);
		cpu_state.BC.Bytes.H--;
		if(cpu_state.BC.Bytes.H!=0) {
			JumpRelative(Byte);		
			cpu_state.t_state+=1;
		} else cpu_state.t_state+=8;
	} else if(OP_CPL(cpu_state.IR)) {
		cpu_state.AF.Bytes.H=~cpu_state.AF.Bytes.H;
		cpu_state.FlagN=cpu_state.FlagH=1;
		cpu_state.t_state+=4;
	} else if(OP_DI(cpu_state.IR)) {
		IFF1=false;
		cpu_state.t_state+=4;
	} else if(OP_EI(cpu_state.IR)) {
		irq_en=true;
		cpu_state.t_state+=4;
	} else if(OP_CALL_W(cpu_state.IR)) {
		Call(GetWordOperand());
	} else if(OP_CALL_F_W(cpu_state.IR)) {
		Word=GetWordOperand();
		if(*OperandF(cpu_state.IR)) Call(Word); else cpu_state.t_state+=10;
	} else if(OP_RET(cpu_state.IR)) {
		Pop(&(cpu_state.PC.Word));
	} else if(OP_RET_F(cpu_state.IR)) {
		if(*OperandF(cpu_state.IR)) {
			Pop(&cpu_state.PC.Word);
			cpu_state.t_state+=1;
		} else {
			cpu_state.t_state+=5;
		}
	} else if(OP_EXDEHL(cpu_state.IR)) {
		Swap(&cpu_state.DE.Word, &cpu_state.HL.Word);
		cpu_state.t_state+=4;
	} else if(OP_EXPSPHL(cpu_state.IR)) {
		uint8_t TempH, TempL;
		TempL=cpu_state.cpu_mem_rd_cb((uint16_t)(cpu_state.SP.Word+0));
		TempH=cpu_state.cpu_mem_rd_cb((uint16_t)(cpu_state.SP.Word+1));
		cpu_state.cpu_mem_wr_cb((uint16_t)(cpu_state.SP.Word+0), cpu_state.PointerReg->Bytes.L);
		cpu_state.cpu_mem_wr_cb((uint16_t)(cpu_state.SP.Word+1), cpu_state.PointerReg->Bytes.H);
		cpu_state.PointerReg->Bytes.H=TempH;
		cpu_state.PointerReg->Bytes.L=TempL;
		if(cpu_state.idx_ins) cpu_state.t_state+=23; else cpu_state.t_state+=4;
	} else if(OP_EXAFAF1(cpu_state.IR)) {
		Swap(&cpu_state.AF.Word, &cpu_state.AF1.Word);
		cpu_state.t_state+=4;
	} else if(OP_EXX(cpu_state.IR)) {
		Swap(&cpu_state.BC.Word, &cpu_state.BC1.Word);
		Swap(&cpu_state.DE.Word, &cpu_state.DE1.Word);
		Swap(&cpu_state.HL.Word, &cpu_state.HL1.Word);
		cpu_state.t_state+=4;
	} else if(OP_PUSH_P(cpu_state.IR)) {
		Push((OperandP(cpu_state.IR)==&cpu_state.SP.Word)?(cpu_state.AF.Word):(*OperandP(cpu_state.IR)));
	} else if(OP_POP_P(cpu_state.IR)) {
		if(OperandP(cpu_state.IR)==&cpu_state.SP.Word) Pop(&cpu_state.AF.Word); else Pop(OperandP(cpu_state.IR));
	} else if(OP_RLA(cpu_state.IR)) {
		bool Carry;
		Carry=SignBit(cpu_state.AF.Bytes.H);
		cpu_state.AF.Bytes.H=((cpu_state.AF.Bytes.H)<<1)+(cpu_state.FlagC?1:0);
		cpu_state.FlagNC=!(cpu_state.FlagC=Carry);
		cpu_state.FlagN=cpu_state.FlagH=0;
		cpu_state.t_state+=4;
	} else if(OP_RLCA(cpu_state.IR)) {
		if(SignBit(cpu_state.AF.Bytes.H)) cpu_state.FlagNC=!(cpu_state.FlagC=true); else cpu_state.FlagC=!(cpu_state.FlagNC=false);
		cpu_state.AF.Bytes.H=(cpu_state.AF.Bytes.H)<<1+(cpu_state.FlagC?1:0);
		cpu_state.FlagN=cpu_state.FlagH=0;
	} else if(OP_RRCA(cpu_state.IR)) {
		if(cpu_state.AF.Bytes.H&1) cpu_state.FlagNC=!(cpu_state.FlagC=true); else cpu_state.FlagC=!(cpu_state.FlagNC=false);
		cpu_state.AF.Bytes.H=(cpu_state.AF.Bytes.H)>>1+(cpu_state.FlagC?(1<<7):(0<<7));
		cpu_state.FlagN=cpu_state.FlagH=0;
		cpu_state.t_state+=4;
	} else if(OP_RRA(cpu_state.IR)) {
		bool Carry;
		Carry=cpu_state.AF.Bytes.H&1;
		cpu_state.AF.Bytes.H=((cpu_state.AF.Bytes.H)>>1)+(cpu_state.FlagC?(1<<7):(0<<7));
		cpu_state.FlagNC=!(cpu_state.FlagC=Carry);
		cpu_state.FlagN=cpu_state.FlagH=0;
		cpu_state.t_state+=4;
	} else if(OP_IN_B(cpu_state.IR)) {
		cpu_state.AF.Bytes.H=cpu_state.cpu_io_rd_cb(cpu_state.cpu_mem_rd_cb(cpu_state.PC.Word++));
		cpu_state.t_state+=11;
	} else if(OP_OUT_B(cpu_state.IR)) {
		cpu_state.cpu_io_wr_cb(cpu_state.cpu_mem_rd_cb(cpu_state.PC.Word++), cpu_state.AF.Bytes.H);
		cpu_state.t_state+=11;
	} else if(OP_RST00(cpu_state.IR)) {
		Push(cpu_state.PC.Word);
		cpu_state.PC.Word=0x0000;
	} else if(OP_RST08(cpu_state.IR)) {
		Push(cpu_state.PC.Word);
		cpu_state.PC.Word=0x0008;
	} else if(OP_RST10(cpu_state.IR)) {
		Push(cpu_state.PC.Word);
		cpu_state.PC.Word=0x0010;
	} else if(OP_RST18(cpu_state.IR)) {
		Push(cpu_state.PC.Word);
		cpu_state.PC.Word=0x0018;
	} else if(OP_RST20(cpu_state.IR)) {
		Push(cpu_state.PC.Word);
		cpu_state.PC.Word=0x0020;
	} else if(OP_RST28(cpu_state.IR)) {
		Push(cpu_state.PC.Word);
		cpu_state.PC.Word=0x0028;
	} else if(OP_RST30(cpu_state.IR)) {
		Push(cpu_state.PC.Word);
		cpu_state.PC.Word=0x0030;
	} else if(OP_RST38(cpu_state.IR)) {
		Push(cpu_state.PC.Word);
		cpu_state.PC.Word=0x0038;
	} else if(OP_NOP(cpu_state.IR)) {
		cpu_state.t_state+=4;
	} else if(OP_CB(cpu_state.IR)) {
		cpu_state.IR=cpu_state.cpu_mem_rd_cb(cpu_state.PC.Word++);
		cpu_state.phl_t_states=7;
		if(OP_CB_RLC(cpu_state.IR)) {
			if(SignBit(*OperandS(cpu_state.IR))) cpu_state.FlagNC=!(cpu_state.FlagC=true); else cpu_state.FlagNC=!(cpu_state.FlagC=false);
			*OperandS(cpu_state.IR)=(*OperandS(cpu_state.IR))<<1+(cpu_state.FlagC?1:0);
			SetFlags(*OperandS(cpu_state.IR));
			cpu_state.t_state+=8;
		} else if(OP_CB_RL(cpu_state.IR)) {
			bool Carry;
			if(SignBit(*OperandS(cpu_state.IR))) Carry=true; else Carry=false;
			*OperandS(cpu_state.IR)=(*OperandS(cpu_state.IR))<<1+(cpu_state.FlagC?1:0);
			cpu_state.FlagNC=!(cpu_state.FlagC=Carry);
			SetFlags(*OperandS(cpu_state.IR));
			cpu_state.t_state+=8;
		} else if(OP_CB_RRC(cpu_state.IR)) {
			if((*OperandS(cpu_state.IR))&0x01) cpu_state.FlagNC=!(cpu_state.FlagC=true); else cpu_state.FlagNC=!(cpu_state.FlagC=false);
			*OperandS(cpu_state.IR)=(*OperandS(cpu_state.IR))>>1+(cpu_state.FlagC?0x80:0);
			SetFlags(*OperandS(cpu_state.IR));
			cpu_state.t_state+=8;
		} else if(OP_CB_SLA(cpu_state.IR)) {
			if(SignBit(*OperandS(cpu_state.IR))) cpu_state.FlagNC=!(cpu_state.FlagC=true); else cpu_state.FlagNC=!(cpu_state.FlagC=false);
			*OperandS(cpu_state.IR)=(*OperandS(cpu_state.IR))<<1;
			SetFlags(*OperandS(cpu_state.IR));
			cpu_state.FlagN=0;
			cpu_state.FlagH=0;
			cpu_state.t_state+=8;
		} else if(OP_CB_BIT_N_S(cpu_state.IR)) {
			cpu_state.FlagZ=!(cpu_state.FlagNZ=(*OperandS(cpu_state.IR))&(1<<OPPARM_N(cpu_state.IR)));
			cpu_state.FlagN=0;
			cpu_state.FlagH=1;
			cpu_state.t_state+=8;
		}
	} else if(OP_ED(cpu_state.IR)) {
		cpu_state.IR=cpu_state.cpu_mem_rd_cb(cpu_state.PC.Word++);
		if(OP_ED_LD_P_PW(cpu_state.IR)) {
			int Addr;
			Addr=cpu_state.cpu_mem_rd_cb(cpu_state.PC.Word++)|(cpu_state.cpu_mem_rd_cb(cpu_state.PC.Word++)<<8);
			*OperandP(cpu_state.IR)=cpu_state.cpu_mem_rd_cb(Addr++)|(cpu_state.cpu_mem_rd_cb(Addr)<<8);
			cpu_state.t_state+=20;
		} else if(OP_ED_LD_PW_SP(cpu_state.IR)) {
	        int Addr;
			Addr=cpu_state.cpu_mem_rd_cb(cpu_state.PC.Word++)|(cpu_state.cpu_mem_rd_cb(cpu_state.PC.Word++)<<8);
			cpu_state.cpu_mem_wr_cb(Addr, cpu_state.SP.Bytes.L); Addr++;
			cpu_state.cpu_mem_wr_cb(Addr, cpu_state.SP.Bytes.L);
			cpu_state.t_state+=20;
		} else if(OP_ED_IM0(cpu_state.IR)) {
		} else if(OP_ED_IM1(cpu_state.IR)) {
        } else if(OP_ED_IM2(cpu_state.IR)) {
		} else if(OP_ED_IN_R_C(cpu_state.IR)) {
			cpu_state.BC.Bytes.L=cpu_state.cpu_io_rd_cb(*OperandR(cpu_state.IR));
			cpu_state.t_state+=12;
		}
	}
	StoreFlags();
	if(cpu_state.mem_write_indirect) {
		cpu_state.mem_addr=cpu_state.PointerReg->Word+(uint16_t)(int8_t)cpu_state.idx;
		cpu_state.cpu_mem_wr_cb(cpu_state.mem_addr, cpu_state.mem_byte);
	}
	cpu_state.clocks++;
	if(cpu_state.irq_pend && cpu_state.IFF1) 
		ProcessIRQ();
}

static uint16_t GetRegister(cpu_reg_t Reg) {
	switch(Reg) {
	case REG_PC: return cpu_state.PC.Word;
	case REG_SP: return cpu_state.SP.Word;
	case REG_AF: return cpu_state.AF.Word;
	case REG_BC: return cpu_state.BC.Word;
	case REG_DE: return cpu_state.DE.Word;
	case REG_HL: return cpu_state.HL.Word;
	case REG_IX: return IX.Word;
	case REG_IY: return IY.Word;
	}
}
