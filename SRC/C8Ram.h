#include "Defs.h"

#define RAM_SIZE 4096
#define PC_START_LOC 512

class C8Ram 
{
public:
	static C8Ram* instance();
	void init();
	int CopyToRam(BYTE* buffer, int location, int size);
	void PrintContents();
	SHORT getCurrentInstruction();
	int incrementPC(int inc = 2);
	void setPC(int i);
	int getPC();
	BYTE getByteContent(int loc);

private:
	C8Ram(){};
	C8Ram(C8Ram const&){};
	C8Ram& operator= (C8Ram const&){};
	static C8Ram* pInstance;

	BYTE ram[RAM_SIZE];
	int pc;
};
