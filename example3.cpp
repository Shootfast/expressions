// example3.cpp

// clang++ -g example3.cpp -I. `llvm-config --cppflags --ldflags --libs core jit native` -O3 -o example3

#include <iostream>
#include <ctime>
#define USE_LLVM
#include <expressions/expressions.h>

int main()
{
	expr::Parser<float> parser;
	expr::LLVMEvaluator::VariableMap vm;
	vm["pi"] = 3.14159265359f;
	vm["x"] = 0.5;
	vm["y"] = 0.5;

	const char * expression = "(x + y) * 10";
	expr::ASTNode* ast = parser.parse(expression);

	expr::LLVMEvaluator eval(ast, &vm); 

	std::cout << eval.evaluate() << std::endl;

	return 0;
}
