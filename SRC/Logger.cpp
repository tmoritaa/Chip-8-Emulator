#include <stdio.h>
#include <string.h>
#include "Defs.h"
#include "Logger.h"

Logger* Logger::pInstance = NULL;

Logger* Logger::instance() 
{
	if (pInstance == NULL) {
		pInstance = new Logger();
	}

	return pInstance;
}

void Logger::writeLog(char* s) 
{
	FILE *ofp;

	ofp = fopen("chip8log", "a");

	if (ofp == NULL) {
		printf("File did not open\n");
	}

	fwrite(s, 1, strlen(s), ofp);

	fclose(ofp);

	return;
}

