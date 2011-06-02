#include <stack>
#include "Defs.h"

class C8Stack {
public:
	static C8Stack* instance();

	bool isEmpty();
	SHORT top();
	int push(SHORT val);
	int pop();

private:
	C8Stack(){};
	C8Stack(C8Stack const&){};
	C8Stack& operator= (C8Stack const&){};
	static C8Stack* pInstance;

	std::stack<SHORT> stack;




};
