#ifndef AST_H
#define AST_H


#include <map>
#include <string>
#include <sstream>
#include "Exception.h"

namespace expr
{

class ASTNode
{
    public:
		enum ASTNodeType
		{
			OPERATION,
			FUNCTION1,
			FUNCTION2,
			COMPARISON,
			LOGICAL,
			BRANCH,
			NUMBER,
			VARIABLE
		};

        ASTNode(ASTNodeType nodeType)
			: m_type(nodeType)
        {
        }

        virtual ~ASTNode()
        {
        }

		virtual ASTNode * clone() const = 0;

        ASTNodeType type()
        {
        	return m_type;
        }

    protected:
        ASTNodeType m_type;

};


class OperationASTNode : public ASTNode
{
	public:
		enum OperationType
		{
			PLUS,
			MINUS,
			MUL,
			DIV,
			POW,
			MOD
		};

		OperationASTNode(OperationType operationType, ASTNode* leftNode, ASTNode* rightNode)
		: ASTNode(ASTNode::OPERATION)
		, m_operation(operationType)
		, m_left(leftNode)
		, m_right(rightNode)
		{}

		~OperationASTNode()
		{
			delete m_left;
			delete m_right;
		}

		virtual OperationASTNode * clone() const
		{
			ASTNode * leftNode = m_left->clone();
			ASTNode * rightNode = m_right->clone();
			return new OperationASTNode(m_operation, leftNode, rightNode);
		}

		OperationType operation()
		{
			return m_operation;
		}

		ASTNode* left()
		{
			return m_left;
		}

		ASTNode* right()
		{
			return m_right;
		}

	protected:
		OperationType m_operation;
		ASTNode* m_left;
		ASTNode* m_right;
};


class Function1ASTNode : public ASTNode
{
	public:
		enum Function1Type
		{
			SIN,
			COS,
			TAN,
			SQRT,
			LOG,
			LOG2,
			LOG10,
			CEIL,
			FLOOR
		};

		Function1ASTNode(Function1Type functionType, ASTNode* leftNode)
		: ASTNode(ASTNode::FUNCTION1)
		, m_function(functionType)
		, m_left(leftNode)
		{}

		~Function1ASTNode()
		{
			delete m_left;
		}

		virtual Function1ASTNode * clone() const
		{
			ASTNode * leftNode = m_left->clone();
			return new Function1ASTNode(m_function, leftNode);
		}

		Function1Type function()
		{
			return m_function;
		}

		ASTNode* left()
		{
			return m_left;
		}

	protected:
		Function1Type m_function;
		ASTNode* m_left;
};


class Function2ASTNode : public ASTNode
{
	public:
		enum Function2Type
		{
			MIN,
			MAX,
			POW
		};

		Function2ASTNode(Function2Type functionType, ASTNode* leftNode, ASTNode* rightNode)
		: ASTNode(ASTNode::FUNCTION2)
		, m_function(functionType)
		, m_left(leftNode)
		, m_right(rightNode)
		{}

		~Function2ASTNode()
		{
			delete m_left;
			delete m_right;
		}

		virtual Function2ASTNode * clone() const
		{
			ASTNode * leftNode = m_left->clone();
			ASTNode * rightNode = m_right->clone();
			return new Function2ASTNode(m_function, leftNode, rightNode);
		}

		Function2Type function()
		{
			return m_function;
		}

		ASTNode* left()
		{
			return m_left;
		}

		ASTNode* right()
		{
			return m_right;
		}

	protected:
		Function2Type m_function;
		ASTNode* m_left;
		ASTNode* m_right;
};


class ComparisonASTNode : public ASTNode
{
    public:
		enum ComparisonType
		{
			EQUAL,
			NOT_EQUAL,
			GREATER_THAN,
			GREATER_THAN_EQUAL,
			LESS_THAN,
			LESS_THAN_EQUAL
		};

		ComparisonASTNode(ComparisonType comparisonType, ASTNode* leftNode, ASTNode* rightNode)
			: ASTNode(ASTNode::COMPARISON)
			, m_comparison(comparisonType)
			, m_left(leftNode)
			, m_right(rightNode)
        {}

        ~ComparisonASTNode()
        {
        	delete m_left;
        	delete m_right;
        }

		virtual ComparisonASTNode * clone() const
		{
			ASTNode * leftNode = m_left->clone();
			ASTNode * rightNode = m_right->clone();
			return new ComparisonASTNode(m_comparison, leftNode, rightNode);
		}

        ComparisonType comparison()
        {
        	return m_comparison;
        }

        ASTNode* left()
		{
			return m_left;
		}

		ASTNode* right()
		{
			return m_right;
		}

    protected:
        ComparisonType m_comparison;
		ASTNode* m_left;
		ASTNode* m_right;
};

class LogicalASTNode : public ASTNode
{
	public:
		enum OperationType
		{
			AND,
			OR
		};
		LogicalASTNode(OperationType operationType, ASTNode* leftNode, ASTNode* rightNode)
			: ASTNode(ASTNode::LOGICAL)
			, m_operation(operationType)
			, m_left(leftNode)
			, m_right(rightNode)
		{}

		~LogicalASTNode()
		{
			delete m_left;
			delete m_right;
		}

		virtual LogicalASTNode * clone() const
		{
			ASTNode * leftNode = m_left->clone();
			ASTNode * rightNode = m_right->clone();
			return new LogicalASTNode(m_operation, leftNode, rightNode);
		}

		OperationType operation()
		{
			return m_operation;
		}

		ASTNode* left()
		{
			return m_left;
		}

		ASTNode* right()
		{
			return m_right;
		}

	protected:
		OperationType m_operation;
		ASTNode* m_left;
		ASTNode* m_right;
};


class BranchASTNode : public ASTNode
{
    public:

		BranchASTNode(ASTNode* conditionNode, ASTNode* yesNode, ASTNode* noNode)
			: ASTNode(ASTNode::BRANCH)
			, m_condition(conditionNode)
			, m_yes(yesNode)
			, m_no(noNode)
        {}

        ~BranchASTNode()
        {
        	delete m_condition;
        	delete m_yes;
        	delete m_no;
        }

		virtual BranchASTNode * clone() const
		{
			ASTNode * conditionNode = m_condition->clone();
			ASTNode * yesNode = m_yes->clone();
			ASTNode * noNode = m_no->clone();
			return new BranchASTNode(conditionNode, yesNode, noNode);
		}


        ASTNode* condition()
		{
			return m_condition;
		}

        ASTNode* yes()
		{
			return m_yes;
		}

		ASTNode* no()
		{
			return m_no;
		}

    protected:
		ASTNode* m_condition;
		ASTNode* m_yes;
		ASTNode* m_no;
};



template <typename T>
class NumberASTNode : public ASTNode
{
	public:
		NumberASTNode(T val)
		: ASTNode(ASTNode::NUMBER)
		, m_value(val)
		{}

		~NumberASTNode()
		{}

		virtual NumberASTNode * clone() const
		{
			return new NumberASTNode<T>(m_value);
		}

		T value()
		{
			return m_value;
		}

	protected:
		T m_value;
};


template <typename T>
class VariableASTNode : public ASTNode
{
	public:

		VariableASTNode(std::string k)
		: ASTNode(ASTNode::VARIABLE)
		, m_key(k)
		{}

		~VariableASTNode()
		{}

		virtual VariableASTNode * clone() const
		{
			return new VariableASTNode(m_key);
		}

		std::string variable()
		{
			return m_key;
		}

	protected:
		std::string m_key;
};


} // namespace expr

#endif
