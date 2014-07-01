// tests.cpp
// g++ tests.cpp -o test -I. `llvm-config --cppflags --ldflags --libs core jit native` -Wall -DUSE_LLVM
#include <expressions/expressions.h>
#include <iostream>
#include "math.h"

#include <cmath> // for fabs
#include <limits> // for epsilon



#ifdef USE_LLVM
typedef expr::LLVMEvaluator Evaluator;
typedef expr::LLVMEvaluator::VariableMap VariableMap;
#else
typedef expr::Evaluator<float> Evaluator;
typedef expr::Evaluator<float>::VariableMap VariableMap;
#endif

// globals
float pi = 3.41459f;
float x = 0;
float y = 0;

// EvaluationTest object means we don't need to recompile the expression for every test
class EvaluationTest
{
	public:
		EvaluationTest(const char *expression)
		{
			m_vm["x"] = 0;
			m_vm["y"] = 0;
			m_vm["pi"] = pi;
			m_eval = new Evaluator(parser.parse(expression), &m_vm);
		}
		~EvaluationTest()
		{
			delete m_eval;
		}

		float evaluate(float x, float y)
		{
			m_vm["x"] = x;
			m_vm["y"] = y;
			return m_eval->evaluate();
		}

	private:
		expr::Parser<float> parser;
		VariableMap m_vm;
		Evaluator *m_eval;
};

typedef std::map<std::string, EvaluationTest*> TestMap;
TestMap tests;



float execute(const char *expression)
{
	EvaluationTest *t;
	if (tests.count(std::string(expression)))
	{
		t = tests[std::string(expression)];
	}
	else
	{
		t = new EvaluationTest(expression);
		tests[std::string(expression)] = t;
	}

	return t->evaluate(x,y);
}


void assertExp(const char *expression, float result)
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
	VariableMap vm;
	vm["pi"] = pi;
	vm["x"] = x;
	vm["y"] = y;


	float result;
	expr::ASTNodePtr ast = parser.parse("(x + y) * 10");

	expr::ASTNodePtr ast2 = ast->clone();
	Evaluator eval(ast2, &vm);
	result = eval.evaluate();

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
			assertExp("(y + x)", (y + x));
			assertExp("2 * (y + x)", 2 * (y + x));
			assertExp("(2 * y + 2 * x)", (2 * y + 2 * x));
			assertExp("(y + x / y) * (x - y / x)", (y + x / y) * (x - y / x));
			assertExp("x / ((x + y) * (x - y)) / y", x / ((x + y) * (x - y)) / y);
			assertExp("1 - ((x * y) + (y / x)) - 3", 1 - ((x * y) + (y / x)) - 3);
			assertExp("sin(2 * x) + cos(pi / y)", sin(2 * x) + cos(pi / y));
			assertExp("1 - sin(2 * x) + cos(pi / y)", 1 - sin(2 * x) + cos(pi / y));
			assertExp("sqrt(1 - sin(2 * x) + cos(pi / y) / 3)", sqrt(1 - sin(2 * x) + cos(pi / y) / 3));
			assertExp("(x^2 / sin(2 * pi / y)) -x / 2", (pow(x,2) / sin(2 * pi / y)) -x / 2);
			assertExp("x + (cos(y - sin(2 / x * pi)) - sin(x - cos(2 * y / pi))) - y", x + (cos(y - sin(2 / x * pi)) - sin(x - cos(2 * y / pi))) - y);
			assertExp("min(4,8) < max(4,8) && 10 % 4 == 2 ? (ceil(cos(60*pi/180) + sin(30*pi/180) + tan(45*pi/180)) + sqrt(floor(16.5)) + log2(16)) * log10(100) : 0", std::min(4,8) < std::max(4,8) && 10 % 4 == 2 ? (ceil(cos(60*pi/180) + sin(30*pi/180) + tan(45*pi/180)) + sqrt(floor(16.5)) + log2(16)) * log10(100) : 0);
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
	for(TestMap::iterator it=tests.begin(); it!=tests.end(); ++it)
	{
		delete it->second;
	}
	tests.clear();
	return 0;
}





