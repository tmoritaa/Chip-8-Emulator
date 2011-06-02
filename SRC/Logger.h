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
