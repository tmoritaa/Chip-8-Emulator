/*
Known Issues:
	- Currently does not support sound
	- Some ROMS may not work. 
	ROMS found to not work:
		- VERS 
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <map>
#include "GL/freeglut.h"
#include "Defs.h"
#include "InstDefs.h"
#include "C8Ram.h"
#include "C8Stack.h"
#include "Logger.h"

#define PROGRAM_START_LOC 512 
#define GENERAL_REGISTER_SIZE 0x10
#define KEY_NUM 16
#define ORIGINAL_WIDTH 64
#define ORIGINAL_HEIGHT 32
#define MODIFIER 10
#define INST_PER_SEC 60

#define LOG 0 

BYTE generalRegisters[GENERAL_REGISTER_SIZE];
SHORT registerI;
BYTE registerDT;
BYTE registerST;
BYTE screenPixels[ORIGINAL_WIDTH][ORIGINAL_HEIGHT];
BYTE keys[KEY_NUM];
bool draw = false;

int SCREEN_WIDTH = ORIGINAL_WIDTH * MODIFIER;
int SCREEN_HEIGHT = ORIGINAL_HEIGHT * MODIFIER;

BYTE fontset[80] = 
{
	0xf0, 0x90, 0x90, 0x90, 0xf0,
	0x20, 0x60, 0x20, 0x20, 0x70,
	0xf0, 0x10, 0xf0, 0x80, 0xf0, 
	0xf0, 0x10, 0xf0, 0x10, 0xf0,
	0x90, 0x90, 0xf0, 0x10, 0x10, 
	0xf0, 0x80, 0xf0, 0x10, 0xf0,
	0xf0, 0x80, 0xf0, 0x90, 0xf0,
	0xf0, 0x10, 0x20, 0x40, 0x40,
	0xf0, 0x90, 0xf0, 0x90, 0xf0, 
	0xf0, 0x90, 0xf0, 0x10, 0xf0,
	0xf0, 0x90, 0xf0, 0x90, 0x90, 
	0xe0, 0x90, 0xe0, 0x90, 0xe0,
	0xf0, 0x80, 0x80, 0x80, 0xf0, 
	0xe0, 0x90, 0x90, 0x90, 0xe0,
	0xf0, 0x80, 0xf0, 0x80, 0xf0,
	0xf0, 0x80, 0xf0, 0x80, 0x80
};

int executeInstruction(SHORT instN) 
{
	char buffer[MAX_BUF_SIZE] = {0};
	int error = SUCCESS;
	bool drawDone = false;

	SHORT inst = instN;

	switch (inst & 0x00f0) { 

		case INST_CLASS_0:
		{
			if ((inst & 0xff00) == INST_CLS) {
				// clear screen
				for (int x = 0; x < ORIGINAL_WIDTH; x++) {
					for (int y = 0; y < ORIGINAL_HEIGHT; y++) {
						screenPixels[x][y] = 0;
					}
				}

				sprintf(buffer, "CLS\n");
				drawDone = true;
				draw = true;

			} else if ((inst & 0xff00) == INST_RET) {
				// return from subroutine
				C8Ram::instance()->setPC(C8Stack::instance()->top());	
				error = C8Stack::instance()->pop();	
				
				sprintf(buffer, "RET\n\tNew PC: %d\n", C8Ram::instance()->getPC());
			} else {
				printf(buffer, "No instruction found\n");
				return FAILURE;
			}

		 } break;

		 case INST_CLASS_1:
		 {
			if ((inst & INST_JP) == INST_JP) {
				SHORT tempAddress = 0;
				// jump to new address
				tempAddress = (((inst & 0x000f) << 8 | (inst & 0xff00) >> 8));
				C8Ram::instance()->setPC(tempAddress);
				C8Ram::instance()->incrementPC(-2);
				sprintf(buffer, "JP %d\n\tNew PC: %d\n", ((inst & 0x000f) << 8) | ((inst & 0xff00) >> 8), C8Ram::instance()->getPC());
			} else {
				printf(buffer, "No instruction found\n");
				return FAILURE;
			}
		} break;

		case INST_CLASS_2:
		{
			if ((inst & INST_CALL) == INST_CALL) {
				SHORT tempAddress = 0;
				// call subroutine
				tempAddress = (((inst & 0x000f) << 8 | (inst & 0xff00) >> 8));
				C8Stack::instance()->push(C8Ram::instance()->getPC());
				C8Ram::instance()->setPC(tempAddress);
				C8Ram::instance()->incrementPC(-2);
				sprintf(buffer, "CALL %d\n\tNew PC: %d\n", ((inst & 0x000f) << 8) | ((inst & 0xff00) >> 8), C8Ram::instance()->getPC());
			} else {
				printf(buffer, "No instruction found\n");
				return FAILURE;
			}
		} break;

		case INST_CLASS_3:
		{
			if ((inst & INST_SE_VB) == INST_SE_VB) {
				BYTE regXIndex = 0;
				BYTE valX;
				BYTE tempVal;
				// skip next instruction if...
				regXIndex = inst & 0x000f;
				valX = generalRegisters[regXIndex];
				tempVal = (inst & 0xff00) >> 8;
				if (valX == tempVal) {
					error = C8Ram::instance()->incrementPC();
				}
				sprintf(buffer, "SE V%X, %d\n", inst & 0x000f, (inst & 0xff00) >> 8);
			} else {
				printf(buffer, "No instruction found\n");
				return FAILURE;
			}
		} break;

		case INST_CLASS_4:
		{

			if ((inst & INST_SNE_VB) == INST_SNE_VB) {
				// skip next instruction if...
				BYTE regXIndex = inst & 0x000f;
				BYTE valX = generalRegisters[regXIndex];
				BYTE tempVal = (inst & 0xff00) >> 8;

				if (valX != tempVal) {
					error = C8Ram::instance()->incrementPC();
				}

				sprintf(buffer, "SNE V%X, %d\n", inst & 0x000f, (inst & 0xff00) >> 8);

			} else {
				printf(buffer, "No instruction found\n");
				return FAILURE;
			}

		} break;

		case INST_CLASS_5:
		{
			if ((inst & INST_SE_VV) == INST_SE_VV) {
				// skip next instruction if...
				BYTE regXIndex = inst & 0x000f;
				BYTE regYIndex = (inst & 0xf000) >> 12;
				BYTE valX = generalRegisters[regXIndex];
				BYTE valY = generalRegisters[regYIndex]; 

				if (valX == valY) {
					error = C8Ram::instance()->incrementPC();
				}
				sprintf(buffer, "SE V%X, V%X\n", inst & 0x000f, (inst & 0xf000) >> 12);
			} else {
				printf(buffer, "No instruction found\n");
				return FAILURE;
			}
		} break;

		case INST_CLASS_6:
		{
			if ((inst & INST_LD_VB) == INST_LD_VB) {
				// Load byte value to register 
				BYTE regXIndex = inst & 0x000f;
				BYTE tempVal = (inst & 0xff00) >> 8;
				generalRegisters[regXIndex] = tempVal;

				sprintf(buffer, "LD V%X, %d\n", inst & 0x000f, (inst & 0xff00) >> 8);
			} else {
				printf(buffer, "No instruction found\n");
				return FAILURE;
			}
		} break;

		case INST_CLASS_7:
		{
			if ((inst & INST_ADD_VB) == INST_ADD_VB) {
				// add byte value to register
				BYTE regXIndex = inst & 0x000f;
				BYTE valX = generalRegisters[regXIndex];
				BYTE tempVal = (inst & 0xff00) >> 8;
				generalRegisters[regXIndex] = valX + tempVal;	
				sprintf(buffer, "ADD V%X, %d\n\tV%X: %d\n", inst & 0x000f, (inst & 0xff00) >> 8, inst & 0x000f, generalRegisters[regXIndex]);
			} else {
				printf(buffer, "No instruction found\n");
				return FAILURE;
			}
		} break;

		case INST_CLASS_8:
		{
			if ((inst & 0x0ff0) == INST_LD_VV) {
				// load Vy to Vx
				BYTE regXIndex = inst & 0x000f;
				BYTE regYIndex = (inst & 0xf000) >> 12;
				BYTE valY = generalRegisters[regYIndex]; 
				generalRegisters[regXIndex] = valY;	
				sprintf(buffer, "LD V%X, V%X\n", inst & 0x000f, (inst & 0xf000) >> 12);

			} else if ((inst & 0x0ff0) == INST_OR_VV) {
				// bit-wise OR Vx and Vy, store in Vx
				BYTE regXIndex = inst & 0x000f;
				BYTE regYIndex = (inst & 0xf000) >> 12;
				BYTE valX = generalRegisters[regXIndex];
				BYTE valY = generalRegisters[regYIndex]; 
				generalRegisters[regXIndex] = valX | valY;	
				sprintf(buffer, "OR V%X, V%X\n", inst & 0x000f, (inst & 0xf000) >> 12);

			} else if ((inst & 0x0ff0) == INST_AND_VV) {
				// bit-wise AND Vx and Vy, store in Vx
				BYTE regXIndex = inst & 0x000f;
				BYTE regYIndex = (inst & 0xf000) >> 12;
				BYTE valX = generalRegisters[regXIndex];
				BYTE valY = generalRegisters[regYIndex]; 
				generalRegisters[regXIndex] = valX & valY;	
				sprintf(buffer, "AND V%X, V%X\n", inst & 0x000f, (inst & 0xf000) >> 12);

			} else if ((inst & 0x0ff0) == INST_XOR_VV) {
				// bit-wise XOR Vx and Vy, store in Vx
				BYTE regXIndex = inst & 0x000f;
				BYTE regYIndex = (inst & 0xf000) >> 12;
				BYTE valX = generalRegisters[regXIndex];
				BYTE valY = generalRegisters[regYIndex]; 
				generalRegisters[regXIndex] = valX ^ valY;	
				sprintf(buffer, "XOR V%X, V%X\n", inst & 0x000f, (inst & 0xf000) >> 12);

			} else if ((inst & 0x0ff0) == INST_ADD_VV) {
				// add Vx and Vy, and store in Vx. If Vx + Vy > 0xff, set Vf to 1
				BYTE regXIndex = inst & 0x000f;
				BYTE regYIndex = (inst & 0xf000) >> 12;
				BYTE valX = generalRegisters[regXIndex];
				BYTE valY = generalRegisters[regYIndex]; 

				if (((int)valX + (int)valY) > 0xff) {
					generalRegisters[0xf] = 1;
				} else {
					generalRegisters[0xf] = 0;
				}

				generalRegisters[regXIndex] = (valX + valY) & 0xff;

				sprintf(buffer, "ADD V%X, V%X\n", inst & 0x000f, (inst & 0xf000) >> 12);

			} else if ((inst & 0x0ff0) == INST_SUB_VV) {
				// sub Vy from Vx, and store in Vx. If Vx > Vy, set Vf to 1
				BYTE regXIndex = inst & 0x000f;
				BYTE regYIndex = (inst & 0xf000) >> 12;
				BYTE valX = generalRegisters[regXIndex];
				BYTE valY = generalRegisters[regYIndex]; 
				
				if (valX >= valY) {
					generalRegisters[0xf] = 1;
				} else {
					generalRegisters[0xf] = 0;
				}

				generalRegisters[regXIndex] = (valX - valY) & 0xff;
				sprintf(buffer, "SUB V%X, V%X\n", inst & 0x000f, (inst & 0xf000) >> 12);

			} else if ((inst & 0x0ff0) == INST_SHR_V) {
				// shift Vx right by one bit. If LSB is 1, set Vf to 1
				BYTE regXIndex = inst & 0x000f;
				BYTE valX = generalRegisters[regXIndex];

				if (valX & 0x01) {
					generalRegisters[0xf] = 1;
				} else {
					generalRegisters[0xf] = 0;
				}
				
				generalRegisters[regXIndex] = (valX >> 1) & 0xff;

				sprintf(buffer, "SHR V%X\n", inst & 0x000f);

			} else if ((inst & 0x0ff0) == INST_SUBN_VV) {
				// sub Vx from Vy, and store in Vx. If Vy > Vx, set Vf to 1
				BYTE regXIndex = inst & 0x000f;
				BYTE regYIndex = (inst & 0xf000) >> 12;
				BYTE valX = generalRegisters[regXIndex];
				BYTE valY = generalRegisters[regYIndex]; 
				
				if (valY >= valX) {
					generalRegisters[0xf] = 1;
				} else {
					generalRegisters[0xf] = 0;
				}

				generalRegisters[regXIndex] = (valY - valX) & 0xff;

				sprintf(buffer, "SUBN V%X, V%X\n", inst & 0x000f, (inst & 0xf000) >> 12);

			} else if ((inst & 0x0ff0) == INST_SHL_V) {
				// shift Vx left by one bit. If MSB is 1, set Vf to 1
				BYTE regXIndex = inst & 0x000f;
				BYTE valX = generalRegisters[regXIndex];

				if (valX & 0x80) {
					generalRegisters[0xf] = 1;
				} else {
					generalRegisters[0xf] = 0;
				}
				
				generalRegisters[regXIndex] = (valX << 1) & 0xff;
				sprintf(buffer, "SHL V%X\n", inst & 0x000f);
			} else {
				printf(buffer, "No instruction found\n");
				return FAILURE;
			}
		} break;

		case INST_CLASS_9:
		{
			if ((inst & INST_SNE_VV) == INST_SNE_VV) {
				// skip next instruction if...
				BYTE regXIndex = inst & 0x000f;
				BYTE regYIndex = (inst & 0xf000) >> 12;
				BYTE valX = generalRegisters[regXIndex];
				BYTE valY = generalRegisters[regYIndex]; 

				if (valX != valY) {
					error = C8Ram::instance()->incrementPC();
				}

				sprintf(buffer, "SNE V%X, V%X\n", inst & 0x000f, (inst & 0xf000) >> 12);
			} else {
				printf(buffer, "No instruction found\n");
				return FAILURE;
			}
		} break;

		case INST_CLASS_A:
		{
			if ((inst & INST_LD_IA) == INST_LD_IA) {
				// Load address to register I
				registerI = ((inst & 0x000f) << 8) | ((inst & 0xff00) >> 8);
				sprintf(buffer, "LD I, %d\n", ((inst & 0x000f) << 8) | ((inst & 0xff00) >> 8));
			} else {
				printf(buffer, "No instruction found\n");
				return FAILURE;
			}
		} break;

		case INST_CLASS_B:
		{
			if ((inst & INST_JP_VA) == INST_JP_VA) {
				// jump to address + V0
				BYTE val0 = generalRegisters[0x0];
				SHORT tempAddress = ((inst & 0x000f) << 8) | ((inst & 0xff00) >> 8);

				tempAddress = ((tempAddress + val0) - 2);

				C8Ram::instance()->setPC(tempAddress);
				sprintf(buffer, "JP V0, %d\n", ((inst & 0x000f) << 8) | ((inst & 0xff00) >> 8));
			} else {
				printf(buffer, "No instruction found\n");
				return FAILURE;
			}
		} break;

		case INST_CLASS_C:
		{
			if ((inst & INST_RND_VB) == INST_RND_VB) {
				// generate random number, then mask with byte
				BYTE regXIndex = inst & 0x000f;
				BYTE randNum = (rand() % 0x100);
				BYTE tempVal = (inst & 0xff00) >> 8;

				generalRegisters[regXIndex] = randNum & tempVal;
				sprintf(buffer, "RND V%X, %d\n", inst & 0x000f, (inst & 0xff00) >> 8);
			} else {
				printf(buffer, "No instruction found\n");
				return FAILURE;
			}
		} break;

		case INST_CLASS_D:
		{
			if ((inst & INST_DRW) == INST_DRW) {
				// draw stuff
				BYTE regXIndex = inst & 0x000f;
				BYTE regYIndex = (inst & 0xf000) >> 12;

				SHORT height = (inst & 0x0f00) >> 8;
				BYTE valx = generalRegisters[regXIndex];
				BYTE valy = generalRegisters[regYIndex];
				SHORT pixel = 0;

				generalRegisters[0xf] = 0;

				for (int yline = 0; yline < height; yline++) {
					pixel = C8Ram::instance()->getByteContent(registerI + yline); 

					if (((int)valy + (int)yline) >= ORIGINAL_HEIGHT) {
						break;
					}

					for (int xline = 0; xline < 8; xline++) {
						if (((int)valx + (int)xline) >= ORIGINAL_WIDTH) {
							break;	
						}

						if ((pixel & (0x80 >> xline)) != 0) {
							if (screenPixels[(valx + xline)][(valy + yline)] == 1) {
								generalRegisters[0xf] = 1;
							}

							screenPixels[(valx + xline)][(valy + yline)] ^= 1;
						}
					}
				}

				drawDone = true;
				draw = true;
				sprintf(buffer, "DRW V%X, V%X, %d\n\txVal: %d\n\tyVal: %d\n", inst & 0x000f, (inst & 0xf000) >> 12, (inst & 0x0f00) >> 8, 
								generalRegisters[regXIndex], generalRegisters[regYIndex]);

			} else {
				printf(buffer, "No instruction found\n");
				return FAILURE;
			}

		} break;

		case INST_CLASS_E:
		{
			if ((inst & 0xfff0) == INST_SKP) {
				// skip next instruction if certain key is pressed.
				BYTE regXIndex = inst & 0x000f;
				BYTE valX = generalRegisters[regXIndex];

				if (keys[valX]) {
					C8Ram::instance()->incrementPC();
				}
				sprintf(buffer, "SKP V%X\n", inst & 0x000f);

			} else if ((inst & 0xfff0) == INST_SKNP) {
				// skip next instruction if certain key is not pressed.
				BYTE regXIndex = inst & 0x000f;
				BYTE valX = generalRegisters[regXIndex];

				if (!keys[valX]) {
					C8Ram::instance()->incrementPC();
				}
				sprintf(buffer, "SKNP V%X\n", inst & 0x000f);

			} else {
				printf(buffer, "No instruction found\n");
				return FAILURE;
			}
		} break;

		case INST_CLASS_F:
		{
			if ((inst & 0xfff0) == INST_LD_VDT) {
				// Vx equals to delay timer 
				BYTE regXIndex = inst & 0x000f;
				generalRegisters[regXIndex] = registerDT;
				
				sprintf(buffer, "LD V%X, DT\n", inst & 0x000f);

			} else if ((inst & 0xfff0) == INST_LD_VK) {
				// wait for keypress and store K into Vx
				bool keyPressed = false;
				BYTE regXIndex = inst & 0x000f;

				for (int i = 0; i < KEY_NUM; i++) {
					if (keys[i] != 0) {
						generalRegisters[regXIndex] = i;
						keyPressed = true;
					}
				}

				if (!keyPressed) {
					C8Ram::instance()->incrementPC(-2);
				}
				
				sprintf(buffer, "LD V%X, K\n", inst & 0x000f);

			} else if ((inst & 0xfff0) == INST_LD_DTV) {
				// Delay timer equals Vx 
				BYTE regXIndex = inst & 0x000f;
				registerDT = generalRegisters[regXIndex];
				
				sprintf(buffer, "LD DT, V%X\n", inst & 0x000f);

			} else if ((inst & 0xfff0) == INST_LD_STV) {
				// Sound timer equals Vx
				BYTE regXIndex = inst & 0x000f;
				registerST = generalRegisters[regXIndex];
				
				sprintf(buffer, "LD ST, V%X\n", inst & 0x000f);

			} else if ((inst & 0xfff0) == INST_ADD_IV) {
				// I equals to I + Vx
				BYTE regXIndex = inst & 0x000f;
				registerI += generalRegisters[regXIndex];

				sprintf(buffer, "ADD I, V%X\n", inst & 0x000f);

			} else if ((inst & 0xfff0) == INST_LD_FV) {
				// I = Vx where Vx is where sprite to be drawn is.
				BYTE regXIndex = inst & 0x000f;
				registerI = generalRegisters[regXIndex] * 0x05;
				
				sprintf(buffer, "LD F, V%X\n", inst & 0x000f);

			} else if ((inst & 0xfff0) == INST_LD_BV) {
				// stores BCD representation in memory pointed by I
				BYTE regXIndex = inst & 0x000f;
				BYTE valX = generalRegisters[regXIndex];
				BYTE tempVal = 0;

				tempVal = valX / 100;
				C8Ram::instance()->CopyToRam(&tempVal, registerI, sizeof(BYTE));
				tempVal = (valX/10)%10;
				C8Ram::instance()->CopyToRam(&tempVal, registerI+1, sizeof(BYTE));
				tempVal = (valX%100)%10;
				C8Ram::instance()->CopyToRam(&tempVal, registerI+2, sizeof(BYTE));
				sprintf(buffer, "LD B, V%X\n", inst & 0x000f);

			} else if ((inst & 0xfff0) == INST_LD_IV) {
				// copy contents of registers to memory starting from I
				BYTE regXIndex = inst & 0x000f;
				BYTE valX = 0;

				for (int i = 0; i <= regXIndex; i++) {
					valX = generalRegisters[i];
					C8Ram::instance()->CopyToRam(&valX, registerI + i, sizeof(BYTE));
				}
				sprintf(buffer, "LD [I], V%X\n", inst & 0x000f);

			} else if ((inst & 0xfff0) == INST_LD_VI) {
				// copy contents of memory to registers
				BYTE regXIndex = inst & 0x000f;
				for (int i = 0; i <= regXIndex; i++) {
					generalRegisters[i] = C8Ram::instance()->getByteContent(registerI + i);
				}
				sprintf(buffer, "LD V%X, [I]\n", inst & 0x000f);
			}

		} break;

		default:
			printf(buffer, "No instruction found\n");
			return FAILURE;
	}

#if LOG
	Logger::instance()->writeLog(buffer);

	if (drawDone) {
		int num = 0;
		memset(buffer, 0, MAX_BUF_SIZE);
		for (int y = 0; y < ORIGINAL_HEIGHT; y++) {
			for (int x = 0; x < ORIGINAL_WIDTH; x++) {
				num += sprintf((buffer + num), "%c", screenPixels[x][y] + 48);
			}
			num += sprintf((buffer + num),  "\n");
		}
		num += sprintf((buffer + num), "\0");

		Logger::instance()->writeLog(buffer);
		drawDone = false;
	}
#endif

	return error;

}

void initialize() 
{
	C8Ram::instance()->init();

	for (int i = 0; i < GENERAL_REGISTER_SIZE; i++) {
		generalRegisters[i] = 0;
	}

	registerI = 0;
	registerDT = 0;
	registerST = 0;

	for (int x = 0; x < ORIGINAL_WIDTH; x++) {
		for (int y = 0; y < ORIGINAL_HEIGHT; y++) {
			screenPixels[x][y] = 0;
		}
	}

	for (int i = 0; i < KEY_NUM; i++) {
		keys[i] = 0;
	}

	C8Ram::instance()->CopyToRam((BYTE*)fontset, 0, sizeof(fontset));
	
	srand(time(NULL));

	return;
}

int loadProgramToRam(char* filename) 
{
	FILE *ifp;
	int num;
	BYTE* buffer;
	long size;

	ifp = fopen(filename, "r");

	if (ifp == NULL) {
		printf("File did not open\n");
		return FAILURE;
	}

	fseek(ifp, 0, SEEK_END);
	size = ftell(ifp);
	rewind(ifp);

	buffer = (BYTE *) malloc (sizeof(BYTE)*size);
	if (buffer == NULL) {
		printf("Malloc fail\n");
		fclose(ifp);
		return FAILURE;
	}

	memset(buffer, 0, size*sizeof(BYTE));
	
	num = fread(buffer, sizeof(BYTE), size, ifp);

	if (C8Ram::instance()->CopyToRam(buffer, PROGRAM_START_LOC, size)) {
		printf("Program does not fit memory\n");
		free(buffer);
		fclose(ifp);
		return FAILURE;
	}

	fclose(ifp);
	free (buffer);

	return SUCCESS;
}

void drawScreen() {
	for (int y = 0; y < ORIGINAL_HEIGHT; y++) {
		for (int x = 0; x < ORIGINAL_WIDTH; x++) {
			if (screenPixels[x][y] == 0) {
				glColor3f(0.0f, 0.0f, 0.0f);
			} else {
				glColor3f(1.0f, 1.0f, 1.0f);
			}

			glBegin(GL_QUADS);	

			glVertex3f((x*MODIFIER), (y*MODIFIER), 0.0f);
			glVertex3f((x*MODIFIER), (y*MODIFIER) + MODIFIER, 0.0f);
			glVertex3f((x*MODIFIER) + MODIFIER, (y*MODIFIER) + MODIFIER, 0.0f);
			glVertex3f((x*MODIFIER) + MODIFIER, (y*MODIFIER), 0.0f);

			glEnd();
		}
	}

	glutSwapBuffers();
}

void emulationLoop() 
{
	SHORT curInstruction;
	char *buffer;

	buffer = (char*) malloc(4096);

	curInstruction = C8Ram::instance()->getCurrentInstruction();

	if (executeInstruction(curInstruction)) {
		sprintf(buffer, "Instruction does not exist\n");
		Logger::instance()->writeLog(buffer);
		glutLeaveMainLoop();
	}

	if (C8Ram::instance()->incrementPC()) {
		sprintf(buffer, "PC has gone over RAM size");
		Logger::instance()->writeLog(buffer);
		glutLeaveMainLoop();
	}

	if (draw) {
		glClear(GL_COLOR_BUFFER_BIT);
		drawScreen();
		draw = false;
	}

	if (registerDT > 0) {
		registerDT--;
	}

	// currently does not support sound
	if (registerST > 0) {
		registerST--;
	}

	free (buffer);
		
	usleep(2*1000);
}

void keyUp(unsigned char key, int x, int y) 
{
	switch(key) {
		case '1':
			keys[0x1] = 0;
			break;
		case '2':
			keys[0x2] = 0;
			break;
		case '3':
			keys[0x3] = 0;
			break;
		case '4':
			keys[0xc] = 0;
			break;
		case 'q':
			keys[0x4] = 0;
			break;
		case 'w':
			keys[0x5] = 0;
			break;
		case 'e':
			keys[0x6] = 0;
			break;
		case 'r':
			keys[0xd] = 0;
			break;
		case 'a':
			keys[0x7] = 0;
			break;
		case 's':
			keys[0x8] = 0;
			break;
		case 'd':
			keys[0x9] = 0;
			break;
		case 'f':
			keys[0xe] = 0;
			break;
		case 'z':
			keys[0xa] = 0;
			break;
		case 'x':
			keys[0x0] = 0;
			break;
		case 'c':
			keys[0xb] = 0;
			break;
		case 'v':
			keys[0xf] = 0;
			break;
		default:
			break;
	}

	return;
}

void keyDown(unsigned char key, int x, int y) 
{
	switch(key) {
		case 27:
			glutLeaveMainLoop();
			break;
		case '1':
			keys[0x1] = 1;
			break;
		case '2':
			keys[0x2] = 1;
			break;
		case '3':
			keys[0x3] = 1;
			break;
		case '4':
			keys[0xc] = 1;
			break;
		case 'q':
			keys[0x4] = 1;
			break;
		case 'w':
			keys[0x5] = 1;
			break;
		case 'e':
			keys[0x6] = 1;
			break;
		case 'r':
			keys[0xd] = 1;
			break;
		case 'a':
			keys[0x7] = 1;
			break;
		case 's':
			keys[0x8] = 1;
			break;
		case 'd':
			keys[0x9] = 1;
			break;
		case 'f':
			keys[0xe] = 1;
			break;
		case 'z':
			keys[0xa] = 1;
			break;
		case 'x':
			keys[0x0] = 1;
			break;
		case 'c':
			keys[0xb] = 1;
			break;
		case 'v':
			keys[0xf] = 1;
			break;
		default:
			break;
	}

	return;
}

void reshapeWindow(GLsizei width, GLsizei height) 
{
	glClearColor(0.0f, 0.0f, 0.5f, 0.0f);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, width, height, 0, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glViewport(0, 0, width, height);

	SCREEN_WIDTH = width;
	SCREEN_HEIGHT = height;

}

int main(int argc, char** argv) 
{
	if (argc < 2) {
		printf("Please enter ROM to run on command line\n");
		exit(1);
	}

	initialize();

	if (loadProgramToRam(argv[1])) {
		printf("Loading program to RAM has failed\n");
		exit(1);
	}

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);

	glutInitWindowSize(SCREEN_WIDTH, SCREEN_HEIGHT);
	glutInitWindowPosition(320, 320);
	glutCreateWindow("Chip-8 Emulator");

	glutDisplayFunc(emulationLoop);
	glutIdleFunc(emulationLoop);
	glutReshapeFunc(reshapeWindow);
	glutKeyboardFunc(keyDown);
	glutKeyboardUpFunc(keyUp);

	glutMainLoop();

	return 0;
}
