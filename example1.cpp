// example1.cpp

#include <iostream>
#include <expressions/expressions.h>

int main()
{
	expr::Parser<float> parser;
	expr::Evaluator<float>::VariableMap vm;
	vm["pi"] = 3.14159f;
	vm["x"] = 10.0f;

	const char * expression = "1.0e2 + x * pi";
	expr::ASTNode* ast = parser.parse(expression);

	expr::Evaluator<float> eval(ast, &vm);
	std::cout << eval.evaluate() << std::endl;
	return 0;
}
