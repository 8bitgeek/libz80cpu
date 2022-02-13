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
#include "sim80vm.h"

sim80vm::sim80vm(sim80mem* m,sim80io* io)
 : rst_vector(0)
 , opcode(0)
 , m_mem(m)
 , m_io(io)
{
}


sim80vm::~sim80vm()
{
}


void sim80vm::run1(void)
{

	if ( rst_vector > 0 )
	{
		setFlagI(0);
		opcode = rst_vector;
		rst_vector = (-1);
	}
	else
	{
		opcode = mem()->get(getRegPC());
	}
	run2();
	setRegPC( getRegPC()+1 );	 /** increment the PC */
}


