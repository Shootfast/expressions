#ifndef AST_H
#define AST_H


#include <map>
#include <string>
#include <sstream>
#include "Exception.h"
#include "Memory.h"

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

		virtual SHARED_PTR<ASTNode> clone() const = 0;

        ASTNodeType type()
        {
        	return m_type;
        }

    protected:
        ASTNodeType m_type;

};
typedef SHARED_PTR<ASTNode> ASTNodePtr;


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

		OperationASTNode(OperationType operationType, ASTNodePtr leftNode, ASTNodePtr rightNode)
		: ASTNode(ASTNode::OPERATION)
		, m_operation(operationType)
		, m_left(leftNode)
		, m_right(rightNode)
		{}

		~OperationASTNode()
		{
		}

		virtual ASTNodePtr clone() const
		{
			ASTNodePtr leftNode = m_left->clone();
			ASTNodePtr rightNode = m_right->clone();
			return ASTNodePtr(new OperationASTNode(m_operation, leftNode, rightNode));
		}

		OperationType operation()
		{
			return m_operation;
		}

		ASTNodePtr left()
		{
			return m_left;
		}

		ASTNodePtr right()
		{
			return m_right;
		}

	protected:
		OperationType m_operation;
		ASTNodePtr m_left;
		ASTNodePtr m_right;
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

		Function1ASTNode(Function1Type functionType, ASTNodePtr leftNode)
		: ASTNode(ASTNode::FUNCTION1)
		, m_function(functionType)
		, m_left(leftNode)
		{}

		~Function1ASTNode()
		{
		}

		virtual ASTNodePtr clone() const
		{
			ASTNodePtr leftNode = m_left->clone();
			return ASTNodePtr(new Function1ASTNode(m_function, leftNode));
		}

		Function1Type function()
		{
			return m_function;
		}

		ASTNodePtr left()
		{
			return m_left;
		}

	protected:
		Function1Type m_function;
		ASTNodePtr m_left;
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

		Function2ASTNode(Function2Type functionType, ASTNodePtr leftNode, ASTNodePtr rightNode)
		: ASTNode(ASTNode::FUNCTION2)
		, m_function(functionType)
		, m_left(leftNode)
		, m_right(rightNode)
		{}

		~Function2ASTNode()
		{
		}

		virtual ASTNodePtr clone() const
		{
			ASTNodePtr leftNode = m_left->clone();
			ASTNodePtr rightNode = m_right->clone();
			return ASTNodePtr(new Function2ASTNode(m_function, leftNode, rightNode));
		}

		Function2Type function()
		{
			return m_function;
		}

		ASTNodePtr left()
		{
			return m_left;
		}

		ASTNodePtr right()
		{
			return m_right;
		}

	protected:
		Function2Type m_function;
		ASTNodePtr m_left;
		ASTNodePtr m_right;
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

		ComparisonASTNode(ComparisonType comparisonType, ASTNodePtr leftNode, ASTNodePtr rightNode)
			: ASTNode(ASTNode::COMPARISON)
			, m_comparison(comparisonType)
			, m_left(leftNode)
			, m_right(rightNode)
        {}

        ~ComparisonASTNode()
        {
        }

		virtual ASTNodePtr clone() const
		{
			ASTNodePtr leftNode = m_left->clone();
			ASTNodePtr rightNode = m_right->clone();
			return ASTNodePtr(new ComparisonASTNode(m_comparison, leftNode, rightNode));
		}

        ComparisonType comparison()
        {
        	return m_comparison;
        }

        ASTNodePtr left()
		{
			return m_left;
		}

		ASTNodePtr right()
		{
			return m_right;
		}

    protected:
        ComparisonType m_comparison;
		ASTNodePtr m_left;
		ASTNodePtr m_right;
};

class LogicalASTNode : public ASTNode
{
	public:
		enum OperationType
		{
			AND,
			OR
		};
		LogicalASTNode(OperationType operationType, ASTNodePtr leftNode, ASTNodePtr rightNode)
			: ASTNode(ASTNode::LOGICAL)
			, m_operation(operationType)
			, m_left(leftNode)
			, m_right(rightNode)
		{}

		~LogicalASTNode()
		{
		}

		virtual ASTNodePtr clone() const
		{
			ASTNodePtr leftNode = m_left->clone();
			ASTNodePtr rightNode = m_right->clone();
			return ASTNodePtr(new LogicalASTNode(m_operation, leftNode, rightNode));
		}

		OperationType operation()
		{
			return m_operation;
		}

		ASTNodePtr left()
		{
			return m_left;
		}

		ASTNodePtr right()
		{
			return m_right;
		}

	protected:
		OperationType m_operation;
		ASTNodePtr m_left;
		ASTNodePtr m_right;
};


class BranchASTNode : public ASTNode
{
    public:

		BranchASTNode(ASTNodePtr conditionNode, ASTNodePtr yesNode, ASTNodePtr noNode)
			: ASTNode(ASTNode::BRANCH)
			, m_condition(conditionNode)
			, m_yes(yesNode)
			, m_no(noNode)
        {}

        ~BranchASTNode()
        {
        }

		virtual ASTNodePtr clone() const
		{
			ASTNodePtr conditionNode = m_condition->clone();
			ASTNodePtr yesNode = m_yes->clone();
			ASTNodePtr noNode = m_no->clone();
			return ASTNodePtr(new BranchASTNode(conditionNode, yesNode, noNode));
		}


        ASTNodePtr condition()
		{
			return m_condition;
		}

        ASTNodePtr yes()
		{
			return m_yes;
		}

		ASTNodePtr no()
		{
			return m_no;
		}

    protected:
		ASTNodePtr m_condition;
		ASTNodePtr m_yes;
		ASTNodePtr m_no;
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

		virtual ASTNodePtr clone() const
		{
			return ASTNodePtr(new NumberASTNode<T>(m_value));
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

		virtual ASTNodePtr clone() const
		{
			return ASTNodePtr(new VariableASTNode(m_key));
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
