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

	expr::ASTNode* ast = parser.parse(expression, &vm);
	expr::Evaluator<float> eval;
	return eval.evaluate(ast);
}


void assert(const char *expression, float result)
{
	float a = execute(expression);
	if (fabs( a - result) > std::numeric_limits<float>::epsilon()*2048) // ridiculously small difference
	{
		std::cerr << a << " != " << result << " for " << expression << " where x = "<<  x << " and y = " << y << std::endl;
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
	std::cout << "Ran " << count << " tests successfully" << std::endl;;
}


int main()
{
	test();
	return 0;
}





