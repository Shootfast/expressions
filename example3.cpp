// example3.cpp

// clang++ -g example3.cpp -I. `llvm-config --cppflags --ldflags --libs core jit native` -O3 -o example3

#include <iostream>
#define USE_LLVM
#include <expressions/expressions.h>

int main()
{
	expr::Parser<float> parser;
	expr::LLVMEvaluator<float>::VariableMap vm;
	vm["pi"] = 3.14159265359f;

	const char * expression = "sin(30 *pi/180) + cos(60 * pi/180)";
	expr::ASTNode* ast = parser.parse(expression);

	expr::LLVMEvaluator<float> eval(ast, &vm);
	std::cout << "result: " << eval.evaluate() << std::endl;
	return 0;
}
