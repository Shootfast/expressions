#ifndef EVALUATOR_H
#define EVALUATOR_H


#include "math.h"
#include <algorithm>

#include "AST.h"
#include "Exception.h"

namespace expr
{

class EvaluatorException : public Exception
{
    public:
       EvaluatorException(const char * message)
        : Exception(message)
       {
       }
};



template <typename T>
class Evaluator
{
	public:

		T evaluate(ASTNode* ast)
		{
			if(ast == NULL)
			{
				throw EvaluatorException("Incorrect abstract syntax tree");
			}

			return evaluateSubtree(ast);
		}

	private:
		T evaluateSubtree(ASTNode* ast)
		{
			if(ast->type() == ASTNode::NUMBER)
			{
				NumberASTNode<T>* n = static_cast<NumberASTNode<T>* >(ast);
				return n->value();
			}
			if(ast->type() == ASTNode::VARIABLE)
			{
				VariableASTNode<T>* v = static_cast<VariableASTNode<T>* >(ast);
				return v->value();
			}
			else if (ast->type() == ASTNode::OPERATION)
			{
				OperationASTNode* op = static_cast<OperationASTNode*>(ast);

				T v1 = evaluateSubtree(op->right()); // the operators are switched thanks to rpn notation
				T v2 = evaluateSubtree(op->left());
				switch(op->operation())
				{
					case OperationASTNode::PLUS:  return v1 + v2;
					case OperationASTNode::MINUS: return v1 - v2;
					case OperationASTNode::MUL:   return v1 * v2;
					case OperationASTNode::DIV:   return v1 / v2;
					case OperationASTNode::POW:   return (T) pow(v1,v2);
					case OperationASTNode::MOD:   return (T) fmod(v1,v2);
					default: throw EvaluatorException("Unknown operator in syntax tree");
				}
			}
			else if (ast->type() == ASTNode::FUNCTION1)
			{
				Function1ASTNode* f = static_cast<Function1ASTNode*>(ast);

				T v1 = evaluateSubtree(f->left());
				switch(f->function())
				{
					case Function1ASTNode::SIN:   return (T) sin(v1);
					case Function1ASTNode::COS:   return (T) cos(v1);
					case Function1ASTNode::TAN:   return (T) tan(v1);
					case Function1ASTNode::SQRT:  return (T) sqrt(v1);
					case Function1ASTNode::LOG:   return (T) log(v1);
					case Function1ASTNode::LOG2:  return (T) log2(v1);
					case Function1ASTNode::LOG10: return (T) log10(v1);
					case Function1ASTNode::CEIL:  return (T) ceil(v1);
					case Function1ASTNode::FLOOR: return (T) floor(v1);
					default: throw EvaluatorException("Unknown function in syntax tree");
				}
			}
			else if (ast->type() == ASTNode::FUNCTION2)
			{
				Function2ASTNode* f = static_cast<Function2ASTNode*>(ast);

				T v1 = evaluateSubtree(f->right()); // the operators are switched thanks to rpn notation
				T v2 = evaluateSubtree(f->left());
				switch(f->function())
				{
					case Function2ASTNode::MIN:  return std::min(v1, v2);
					case Function2ASTNode::MAX:  return std::max(v1, v2);
					default: throw EvaluatorException("Unknown function in syntax tree");
				}
			}
			else if (ast->type() == ASTNode::COMPARISON)
			{
				ComparisonASTNode* c = static_cast<ComparisonASTNode*>(ast);
				T v1 = evaluateSubtree(c->right()); // the operators are switched thanks to rpn notation
				T v2 = evaluateSubtree(c->left());
				switch(c->comparison())
				{
					case ComparisonASTNode::EQUAL:              return v1 == v2;
					case ComparisonASTNode::NOT_EQUAL:          return v1 != v2;
					case ComparisonASTNode::GREATER_THAN:       return v1 >  v2;
					case ComparisonASTNode::GREATER_THAN_EQUAL: return v1 >= v2;
					case ComparisonASTNode::LESS_THAN:          return v1 <  v2;
					case ComparisonASTNode::LESS_THAN_EQUAL:    return v1 <= v2;
					default: throw EvaluatorException("Unknown comparison in syntax tree");
				}
			}
			else if (ast->type() == ASTNode::LOGICAL)
			{
				LogicalASTNode *l = static_cast<LogicalASTNode*>(ast);
				T v1 = evaluateSubtree(l->right()); // the operators are switched thanks to rpn notation
				T v2 = evaluateSubtree(l->left());
				switch(l->operation())
				{
					case LogicalASTNode::AND: return v1 && v2;
					case LogicalASTNode::OR:  return v1 || v2;
					default: throw EvaluatorException("Unknown logical operator in syntax tree");
				}
			}
			else if (ast->type() == ASTNode::BRANCH)
			{
				BranchASTNode* b = static_cast<BranchASTNode*>(ast);
				if ((bool)evaluateSubtree(b->condition()) == true)
				{
					return evaluateSubtree(b->yes());
				}
				else
				{
					return evaluateSubtree(b->no());
				}
			}

			throw EvaluatorException("Incorrect syntax tree!");
		}


};

} // namespace expr

#endif
