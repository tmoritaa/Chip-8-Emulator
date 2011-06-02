#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "C8Ram.h"
#include "Logger.h"

C8Ram* C8Ram::pInstance = NULL;

C8Ram* C8Ram::instance() 
{
	if (pInstance == NULL) {
		pInstance = new C8Ram();
	}

	return pInstance;
}

void C8Ram::init() 
{
	pc = PC_START_LOC;

	memset(ram, 0, RAM_SIZE*sizeof(BYTE));
}

int C8Ram::CopyToRam(BYTE* buffer, int location, int size) 
{
	if (sizeof(buffer) + location > RAM_SIZE) {
		return FAILURE;
	}

	memcpy(ram+location, buffer, size);
	
	return SUCCESS;
}

void C8Ram::PrintContents() 
{
	char* buffer;
	buffer = (char*) malloc(MAX_BUF_SIZE);

	sprintf(buffer, "RAM Content:\n");
	Logger::instance()->writeLog(buffer);

	for (int i = 0; i < RAM_SIZE; i++) {
		if (ram[i] != 0) {
			sprintf(buffer, "%d (ram[%d]): %X\n", i, i, ram[i]);
			Logger::instance()->writeLog(buffer);
		}
	}

	sprintf(buffer, "////////////////////////////////////////////////////////////////////////\n");
	Logger::instance()->writeLog(buffer);
	
	free(buffer);

	return;
}

SHORT C8Ram::getCurrentInstruction() 
{
	SHORT inst = 0;

	inst = (ram[pc]) | (ram[pc+1] << 8);

	return inst;	
}

int C8Ram::incrementPC(int inc) 
{
	pc += inc;

	if (pc > RAM_SIZE + 2) {
		return FAILURE;
	}

	return SUCCESS;
}

void C8Ram::setPC(int i) 
{
	pc = i;
}

int C8Ram::getPC()
{
	return pc;
}

BYTE C8Ram::getByteContent(int loc)
{
	return ram[loc];

}
