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


#ifndef __LIBZ80CPU_CPU_H__
#define __LIBZ80CPU_CPU_H__

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef uint8_t (*cpu_mem_rd_cb_t)(uint16_t addr);
typedef void (*cpu_mem_wr_cb_t)(uint16_t addr,uint8_t byte);

typedef uint8_t (*cpu_io_rd_cb_t)(uint8_t addr);
typedef void (*cpu_io_wr_cb_t)(uint8_t addr,uint8_t byte);

typedef enum {
	REG_NONE,
	REG_PC,
	REG_SP,
	REG_AF,
	REG_BC,
	REG_DE,
	REG_HL,
	REG_IX,
	REG_IY
} cpu_reg_t;

void Init(	
			cpu_mem_rd_cb_t	cpu_mem_rd_cb,
			cpu_mem_wr_cb_t cpu_mem_wr_cb,
			cpu_io_rd_cb_t cpu_io_rd_cb,
			cpu_io_wr_cb_t cpu_io_wr_cb 
		);

void Step(void);
void RaiseIRQ(void);

#ifdef __cplusplus
}
#endif

#endif
