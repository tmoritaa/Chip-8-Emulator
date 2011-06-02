all: Chip-8

Chip-8: Logger.o C8Stack.o C8Ram.o chip-8.o
	g++ Logger.o C8Stack.o C8Ram.o Chip-8.o -o Chip8-Emulator -lglut

C8Ram.o: C8Ram.cpp
	g++ -c -g C8Ram.cpp

C8Stack.o: C8Stack.cpp
	g++ -c -g C8Stack.cpp

chip-8.o: Logger.o C8Stack.o C8Ram.o Chip-8.cpp
	g++ -c -g Chip-8.cpp 

Logger.o: Logger.cpp
	g++ -c -g Logger.cpp

clean:
	rm -rf *.o Chip8-Emulator chip8log
