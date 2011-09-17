#ifndef _LOGGER_H_
#define _LOGGER_H_

class Logger {
public:
	static Logger* instance();
	void writeLog(char* text);

private:
	Logger(){};
	Logger(Logger const&){};
	Logger& operator= (Logger const&){};
	static Logger* pInstance;
};

#endif
