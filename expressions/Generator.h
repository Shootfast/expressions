#ifndef GENERATOR_H
#define GENERATOR_H

#include <iostream>
#include <set>
#include "AST.h"
#include "Memory.h"
#include "Exception.h"

namespace expr
{

class GeneratorException : public Exception
{
    public:
		GeneratorException(const char * message)
        : Exception(message)
       {
       }
};

template <typename T>
class ShaderGenerator
{
	public:
		enum Language
		{
			
			GLSLv1_0,
			GLSLv1_3
		};
		std::string generate(ASTNodePtr ast, Language lang=GLSLv1_3)
		{
			m_language = lang;

			if(ast == NULL)
			{
				throw GeneratorException("Incorrect abstract syntax tree");
			}

			std::string code = generateSubtree(ast);
			return code;
		}

	private:
		std::string type()
		{
			return "double";
		}

		std::string generateSubtree(ASTNodePtr ast)
		{
			std::stringstream ss;

			if(ast->type() == ASTNode::NUMBER)
			{
				SHARED_PTR<NumberASTNode<T> > n = STATIC_POINTER_CAST<NumberASTNode<T> >(ast);
				ss << n->value();
				if (type() == "float")
				{
					ss << "f";
				}
				else if (type() == "double")
				{
					ss << "lf";
				}
				return ss.str();
			}
			else if(ast->type() == ASTNode::VARIABLE)
			{
				SHARED_PTR<VariableASTNode<T> > v = STATIC_POINTER_CAST<VariableASTNode<T> >(ast);
				std::string var = v->variable();
				ss << var;
				return ss.str();
			}
			else if (ast->type() == ASTNode::OPERATION)
			{
				SHARED_PTR<OperationASTNode> op = STATIC_POINTER_CAST<OperationASTNode>(ast);

				std::string v1 = generateSubtree(op->right()); // the operators are switched thanks to rpn notation
				std::string v2 = generateSubtree(op->left());
				switch(op->operation())
				{
					case OperationASTNode::PLUS:  return std::string("(") + v1 + "+" + v2 + ")";
					case OperationASTNode::MINUS: return std::string("(") + v1 + "-" + v2 + ")";
					case OperationASTNode::MUL:   return std::string("(") + v1 + "*" + v2 + ")";
					case OperationASTNode::DIV:   return std::string("(") + v1 + "/" + v2 + ")";
					case OperationASTNode::POW:   return std::string("pow(") + v1 + "," + v2 + ")";
					case OperationASTNode::MOD:
					{
						if(m_language == GLSLv1_0)
						{
							// GLSLv1_0 has no trunc or fmod operator
							// operation should return (v1 - v2 * ((v1/v2>0) ? floor(v1/v2) : ceil(v1/v2)))
							return std::string("(") + v1 + " - " + v2 + "* ((" + v1 + "/" + v2 + ">0) ? floor(" + v1 + "/" + v2 + ") : ceil(" + v1 + "/" + v2 + ")))";
						}
						else if (m_language == GLSLv1_3)
						{
							// GLSLv1_3 has no fmod operator
							// operation should return (v1 - v2 * trunc(v1/v2))
							return std::string("(") + v1 + " - " + v2 + " * trunc(" + v1 + "/" + v2 + "))";
						}
						return std::string("fmod(") + v1 + "," + v2 + ")";
					}
					default: throw GeneratorException("Unknown operator in syntax tree");
				}
			}
			else if (ast->type() == ASTNode::FUNCTION1)
			{
				SHARED_PTR<Function1ASTNode> f = STATIC_POINTER_CAST<Function1ASTNode>(ast);

				std::string v1 = generateSubtree(f->left());
				switch(f->function())
				{
					case Function1ASTNode::SIN:   return std::string("sin(") + v1 + ")";
					case Function1ASTNode::COS:   return std::string("cos(") + v1 + ")";
					case Function1ASTNode::TAN:   return std::string("tan(") + v1 + ")";
					case Function1ASTNode::SQRT:  return std::string("sqrt(") + v1 + ")";
					case Function1ASTNode::LOG:   return std::string("log(") + v1 + ")";
					case Function1ASTNode::LOG2:  return std::string("log2(") + v1 + ")";
					case Function1ASTNode::LOG10:
						if (m_language == GLSLv1_0 || m_language == GLSLv1_3)
						{
							// GLSLv1_0 and GLSLv1_3 have no log10 operator
							// operation should return (log(v1)/log(10f))
							return std::string("(log(") + v1 + ")/log(10f))";
						}
						return std::string("log10(") + v1 + ")";
					case Function1ASTNode::CEIL:  return std::string("ceil(") + v1 + ")";
					case Function1ASTNode::FLOOR: return std::string("floor(") + v1 + ")";
					default: throw GeneratorException("Unknown function in syntax tree");
				}
			}
			else if (ast->type() == ASTNode::FUNCTION2)
			{
				SHARED_PTR<Function2ASTNode> f = STATIC_POINTER_CAST<Function2ASTNode>(ast);

				std::string v1 = generateSubtree(f->right()); // the operators are switched thanks to rpn notation
				std::string v2 = generateSubtree(f->left());
				switch(f->function())
				{
					case Function2ASTNode::MIN:  return std::string("min(") + v1 + "," + v2 + ")";
					case Function2ASTNode::MAX:  return std::string("max(") + v1 + "," + v2 + ")";
					case Function2ASTNode::POW:  return std::string("pow(") + v1 + "," + v2 + ")";
					default: throw GeneratorException("Unknown function in syntax tree");
				}
			}
			else if (ast->type() == ASTNode::COMPARISON)
			{
				SHARED_PTR<ComparisonASTNode> c = STATIC_POINTER_CAST<ComparisonASTNode>(ast);
				std::string v1 = generateSubtree(c->right()); // the operators are switched thanks to rpn notation
				std::string v2 = generateSubtree(c->left());
				switch(c->comparison())
				{
					case ComparisonASTNode::EQUAL:              return v1 + "==" +  v2;
					case ComparisonASTNode::NOT_EQUAL:          return v1 + "!=" +  v2;
					case ComparisonASTNode::GREATER_THAN:       return v1 + ">"  +  v2;
					case ComparisonASTNode::GREATER_THAN_EQUAL: return v1 + ">=" +  v2;
					case ComparisonASTNode::LESS_THAN:          return v1 + "<"  +  v2;
					case ComparisonASTNode::LESS_THAN_EQUAL:    return v1 + "<=" +  v2;
					default: throw GeneratorException("Unknown comparison in syntax tree");
				}
			}
			else if (ast->type() == ASTNode::LOGICAL)
			{
				SHARED_PTR<LogicalASTNode> l = STATIC_POINTER_CAST<LogicalASTNode>(ast);
				std::string v1 = generateSubtree(l->right()); // the operators are switched thanks to rpn notation
				std::string v2 = generateSubtree(l->left());
				switch(l->operation())
				{
					case LogicalASTNode::AND: return v1 + "&&" +  v2;
					case LogicalASTNode::OR:  return v1 + "||" +  v2;
					default: throw GeneratorException("Unknown logical operator in syntax tree");
				}
			}
			else if (ast->type() == ASTNode::BRANCH)
			{
				SHARED_PTR<BranchASTNode> b = STATIC_POINTER_CAST<BranchASTNode>(ast);
				std::string condition = generateSubtree(b->condition());
				std::string yes = generateSubtree(b->yes());
				std::string no = generateSubtree(b->no());

				return std::string("((bool(") + condition + ")) ? " + yes + ":" + no + ")";
			}

			throw GeneratorException("Incorrect syntax tree!");
		}

	private:
		Language m_language;
};

template <> std::string ShaderGenerator<int>::type()
{
	return "int";
}

template <> std::string ShaderGenerator<float>::type()
{
	return "float";
}

} // namespace expr

#endif
