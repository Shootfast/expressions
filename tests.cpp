// tests.cpp

#include <expressions/expressions.h>
#include <iostream>
#include "math.h"

#include <cmath> // for fabs
#include <limits> // for epsilon

float pi = 3.41459f;
float x = 0;
float y = 0;


float execute(const char *expression)
{
	expr::Parser<float> parser;
	expr::Parser<float>::VariableMap vm;
	vm["pi"] = pi;
	vm["x"] = x;
	vm["y"] = y;

	float result;
	try
	{
		expr::ASTNode* ast = parser.parse(expression, &vm);
		expr::Evaluator<float> eval;
		result = eval.evaluate(ast);
	}
	catch (expr::ParserException &e)
	{
		throw;
	}
	catch (expr::EvaluatorException &e)
	{
		throw;
	}
	return result;
}


void assert(const char *expression, float result)
{
	float a = execute(expression);
	if (fabs( a - result) > std::numeric_limits<float>::epsilon()*2048) // ridiculously small difference
	{
		std::cerr << a << " != " << result << " for " << expression << " where x = "<<  x << " and y = " << y << std::endl;
	}
}


void syntaxErrors(const char *expression)
{
	bool error = false;
	try
	{
		execute(expression);
	}
	catch (expr::ParserException &e)
	{
		error = true;
	}
	if (!error)
	{
		std::cerr << "expression \"" << expression << "\" did not result in a syntax error" << std::endl;
	}
}


void clone()
{
	x = 10;
	y = 20;

	expr::Parser<float> parser;
	expr::Parser<float>::VariableMap vm;
	vm["pi"] = pi;
	vm["x"] = x;
	vm["y"] = y;


	float result;
	expr::ASTNode* ast = parser.parse("(x + y) * 10", &vm);

	expr::ASTNode* ast2 = ast->clone();
	delete ast;
	expr::Evaluator<float> eval;
	result = eval.evaluate(ast2);

	if (result != 300.0f)
	{
		std::cerr << "cloned expression did not evaluate correctly" << std::endl;
	}

}


void test()
{
	unsigned int count = 0;
	
	for (x=-10; x< 10; x+= 0.1f)
	{
		for (y=-10; y< 10; y+= 0.1f)
		{
			if (x==0 || y == 0)
				continue;
			assert("(y + x)", (y + x));
			assert("2 * (y + x)", 2 * (y + x));
			assert("(2 * y + 2 * x)", (2 * y + 2 * x));
			assert("(y + x / y) * (x - y / x)", (y + x / y) * (x - y / x));
			assert("x / ((x + y) * (x - y)) / y", x / ((x + y) * (x - y)) / y);
			assert("1 - ((x * y) + (y / x)) - 3", 1 - ((x * y) + (y / x)) - 3);
			assert("sin(2 * x) + cos(pi / y)", sin(2 * x) + cos(pi / y));
			assert("1 - sin(2 * x) + cos(pi / y)", 1 - sin(2 * x) + cos(pi / y));
			assert("sqrt(1 - sin(2 * x) + cos(pi / y) / 3)", sqrt(1 - sin(2 * x) + cos(pi / y) / 3));
			assert("(x^2 / sin(2 * pi / y)) -x / 2", (pow(x,2) / sin(2 * pi / y)) -x / 2);
			assert("x + (cos(y - sin(2 / x * pi)) - sin(x - cos(2 * y / pi))) - y", x + (cos(y - sin(2 / x * pi)) - sin(x - cos(2 * y / pi))) - y);
			assert("min(4,8) < max(4,8) && 10 % 4 == 2 ? (ceil(cos(60*pi/180) + sin(30*pi/180) + tan(45*pi/180)) + sqrt(floor(16.5)) + log2(16)) * log10(100) : 0", std::min(4,8) < std::max(4,8) && 10 % 4 == 2 ? (ceil(cos(60*pi/180) + sin(30*pi/180) + tan(45*pi/180)) + sqrt(floor(16.5)) + log2(16)) * log10(100) : 0);
			count+=12;
		}
	}

	syntaxErrors("x++"); count++;
	syntaxErrors("+"); count++;
	syntaxErrors("x y"); count++;
	syntaxErrors("sin x"); count++;
	syntaxErrors("min(x)"); count++;
	syntaxErrors("min(,1)"); count++;
	syntaxErrors(")))))))+x"); count++;
	syntaxErrors("x % "); count++;
	syntaxErrors("%x"); count++;
	syntaxErrors("1-*2"); count++;

	clone(); count++;

	std::cout << "Ran " << count << " tests successfully" << std::endl;;
}

int main()
{
	test();
	return 0;
}





