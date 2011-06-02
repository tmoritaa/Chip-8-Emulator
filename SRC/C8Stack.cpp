#include <stdio.h>
#include "C8Stack.h"

#define STACK_SIZE 16

C8Stack* C8Stack::pInstance = NULL;

C8Stack* C8Stack::instance() 
{
	if (pInstance == NULL) {
		pInstance = new C8Stack();
	}

	return pInstance;
}

bool C8Stack::isEmpty()
{
	return stack.empty();
}

SHORT C8Stack::top()
{
	if (stack.empty()) {
		return 0;
	}

	return stack.top();
}

int C8Stack::push(SHORT val)
{
	if (stack.size() > STACK_SIZE) {
		return FAILURE;
	}

	stack.push(val);

	return SUCCESS;
}

int C8Stack::pop()
{
	if (stack.empty()) {
		return FAILURE;
	}

	stack.pop();

	return SUCCESS;
}
