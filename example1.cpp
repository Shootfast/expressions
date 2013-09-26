// example1.cpp

#include <iostream>
#include <expressions/expressions.h>

int main()
{
	expr::Parser<float> parser;
	expr::Parser<float>::VariableMap vm;
	vm["pi"] = 3.14159f;
	vm["x"] = 10.0f;

	const char * expression = "1.0e2 + x * pi";
	expr::ASTNode* ast = parser.parse(expression, &vm);

	expr::Evaluator<float> eval;
	std::cout << eval.evaluate(ast) << std::endl;
	return 0;
}
