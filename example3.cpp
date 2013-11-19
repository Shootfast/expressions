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
	vm["v"] = 0.5;

	const char * expression = "(v < 0.0404482362771082) ? v/12.92 : ((v+0.055)/1.055)^2.4";
	expr::ASTNode* ast = parser.parse(expression);

	expr::LLVMEvaluator eval(ast, &vm); 
	//expr::Evaluator<float> eval(ast, &vm);

	for (int x=0; x< 1920; x++)
	{
		for (int y=0; y<1080; y++)
		{
			vm["v"] = 0.1;
			//std::cout << eval.evaluate() << std::endl;
		}
	}
	return 0;
}
