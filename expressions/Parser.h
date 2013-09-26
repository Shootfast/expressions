#ifndef PARSER_H
#define PARSER_H

#include "AST.h"
#include "Tokenizer.h"
#include "Exception.h"
#include <map>
#include <iostream>

namespace expr
{

class ParserException : public Exception
{
    public:
        ParserException(const char * message)
        : Exception(message)
        {
        }
};



template <typename T>
class Parser
{
    public:
		typedef std::map<std::string, T> VariableMap;

        ASTNode* parse(const char* text, VariableMap *variables=0)
        {
            try
            {
                std::deque<Token*> tokens = Tokenizer<T>(text).tokenize();
                std::deque<Token*> rpnOrdered = shuntingYard(tokens);
                return rpnToAST(rpnOrdered, variables);
            }
            catch (TokenizerException &e)
            {
                throw ParserException(e.what());
            }
        }

        void print(std::deque<Token*> &tokens)
        {
        	for (unsigned int i=0; i<tokens.size(); i++)
        	{
        		Token* token = tokens[i];
        		std::cout << token->print();
        		std::cout << " ";
        	}
        	std::cout << std::endl;
        }


    private:

        std::deque<Token*> shuntingYard(std::deque<Token*> &tokens)
		{
           	// This is an implementation of the shunting yard algorithm
			// to convert infix notation into reverse polish notation.
			// this ensures that the operator precedence is maintained
			// in our AST.

        	// Stacks for "shunting"
        	std::deque<Token*> output;
        	std::deque<Token*> stack;

        	// While there are tokens to be read
        	while (tokens.size() != 0)
        	{
        		// Read a token
        		Token* token = tokens.front(); tokens.pop_front();

        		// If the token is a number, then add it to the output queue.
        		if (token->getType() == Token::NUMBER || token->getType() == Token::VARIABLE)
        		{
        			output.push_back(token);
        			continue;
        		}

        		// If the token is a function token, then push it onto the stack.
        		else if (token->getType() == Token::FUNCTION)
				{
					stack.push_front(token);
					continue;
				}

        		// If the token is a function argument separator (e.g., a comma):
        		else if (token->getType() == Token::COMMA)
        		{
        			// Until the token at the top of the stack is a left parenthesis,
        			while (true)
        			{
        				if (stack.size() != 0)
        				{
        					Token* stackToken = stack.front();
        					if (stackToken->getType() != Token::OPEN_PARENTHESIS)
        					{
        						// pop operators off the stack onto the output queue
        						output.push_back(stackToken);
        						stack.pop_front();
        					}
        					else
        					{
        						break;
        					}
        				}
        				else
        				{
        					// If no left parentheses are encountered, either the separator was misplaced
        					// or parentheses were mismatched.
        					delete token;
							std::stringstream ss;
							ss << "Misplaced separator or unmatched parenthesis, character: ";
							ss << token->getPosition();
							throw ParserException(ss.str().c_str());
        				}
        			}
        			continue;
        		}

        		// If the token is an operator, o1, then:
        		else if (	token->getType() == Token::OPERATOR    ||
							token->getType() == Token::UNARY       ||
							token->getType() == Token::CONDITIONAL ||
							token->getType() == Token::LOGICAL     ||
							token->getType() == Token::TERNARY )
        		{
        			// while there is an operator token, o2, at the top of the stack,
        			while (stack.size() != 0)
        			{
        				Token* stackToken = stack.front();
        				if (stackToken->getType() == Token::OPERATOR    ||
        					stackToken->getType() == Token::UNARY       ||
        					stackToken->getType() == Token::CONDITIONAL ||
        					stackToken->getType() == Token::LOGICAL     ||
        					stackToken->getType() == Token::TERNARY )
        				{
        					PrecedenceOperator *op1 = static_cast<PrecedenceOperator*>(token);
        					PrecedenceOperator *op2 = static_cast<PrecedenceOperator*>(stackToken);

        					// and either o1 is left-associative and its precedence is equal to that of o2,
        					// or o1 has precedence less than that of o2,
        					if ( (op1->leftAssociative() && op1->precedence() == op2->precedence()) ||
        					      op1->precedence() < op2->precedence())
        					{
        						// pop o2 off the stack, onto the output queue;
        						output.push_back(stackToken);
        						stack.pop_front();
        					}
        					else
        					{
        						break;
        					}
        				}
        				else
        				{
        					break;
        				}
        			}
        			// push o1 onto the stack.
        			stack.push_front(token);
        			continue;
        		}

        		// If the token is a left parenthesis, then push it onto the stack.
        		else if (token->getType() == Token::OPEN_PARENTHESIS)
        		{
        			stack.push_front(token);
        			continue;
        		}

        		// If the token is a right parenthesis:
        		else if (token->getType() == Token::CLOSE_PARENTHESIS)
				{
        			// Until the token at the top of the stack is a left parenthesis,
        			while (true)
        			{
        				if (stack.size() != 0)
        				{
        					Token* stackToken = stack.front();
        					if (stackToken->getType() != Token::OPEN_PARENTHESIS)
        					{
        						// pop operators off the stack onto the output queue
        						output.push_back(stackToken);
        						stack.pop_front();
        					}
        					else
        					{
        						break;
        					}
        				}
        				else
        				{
        					// If the stack runs out without finding a left parenthesis, then there are mismatched parentheses
        					delete token;
							std::stringstream ss;
							ss << "Mismatched parenthesis, character: ";
							ss << token->getPosition();
							throw ParserException(ss.str().c_str());
        				}
        			}

        			// Pop the left parenthesis from the stack, but not onto the output queue
        			stack.pop_front();

        			// If the token at the top of the stack is a function token,
        			if (stack.size() != 0)
        			{
        				if (stack.front()->getType() == Token::FUNCTION)
        				{
        					// pop it onto the output queue.
        					output.push_back(stack.front());
        					stack.pop_front();
        				}
        			}
        			continue;
				}
        		// If we get to here, then this token won't be needed in the RPN ordered deque
        		// So just delete it
        		else
        		{
        			delete token;
        		}
        	}

    		// When there are no more tokens to read
        	// While there are still operator tokens in the stack
        	while (stack.size() != 0)
        	{
        		Token* stackToken = stack.front();
        		if (stackToken->getType() == Token::OPEN_PARENTHESIS ||
        			stackToken->getType() == Token::CLOSE_PARENTHESIS )
        		{
        			throw ParserException("Mismatched parenthesis");
        		}
        		else
        		{
        			output.push_back(stackToken);
        			stack.pop_front();
        		}
        	}

        	return output;
        }

        ASTNode* rpnToAST(std::deque<Token*> &tokens, VariableMap *variables)
        {
        	// To convert from RPN to AST, just push each number node onto the front of the stack
        	// and for each operator, pop the required operands, then push the resulting node
        	// to the front of the stack

        	std::deque<ASTNode*> stack;
        	for (unsigned int i=0; i<tokens.size(); i++)
        	{
        		Token* token = tokens[i];

        		if (token->getType() == Token::NUMBER)
				{
					ASTNode *n = new NumberASTNode<T>(static_cast<NumberToken<T>* >(token)->getValue());
					stack.push_front(n);
					continue;
				}

        		if (token->getType() == Token::VARIABLE)
        		{
        			std::string key = static_cast<VariableToken*>(token)->getValue();
        			ASTNode *n = new VariableASTNode<T>(key, variables);
        			stack.push_front(n);
        			continue;
        		}



        		if (token->getType() == Token::UNARY)
        		{
					if (stack.size() == 0)
					{
						std::stringstream ss;
						ss << "Invalid syntax: unary operator given without variable, character: ";
						ss << token->getPosition();
						throw ParserException(ss.str().c_str());
					}

        			UnaryToken *u = static_cast<UnaryToken*>(token);

        			if (u->getDirection() == UnaryToken::NEGATIVE)
        			{

        				// Take the number from the top of the stack and make it negative
        				NumberASTNode<T>* n = static_cast<NumberASTNode<T>* >(stack.front());
        				stack.pop_front();

        				stack.push_front(new NumberASTNode<T>(n->value() * -1));

        				// Remove the old ASTNode
        				delete n;
        			}
        			continue;
        		}

        		if (token->getType() == Token::OPERATOR)
        		{
					if (stack.size() < 2)
					{
						std::stringstream ss;
						ss << "Invalid syntax: operator given with insufficient operands, character: ";
						ss << token->getPosition();
						throw ParserException(ss.str().c_str());
					}

        			OperatorToken *opt = static_cast<OperatorToken*>(token);
        			ASTNode *left = stack.front(); stack.pop_front();
        			ASTNode *right = stack.front(); stack.pop_front();
        			OperationASTNode::OperationType type;
        			switch(opt->getOperator())
        			{
        				case OperatorToken::PLUS:  type = OperationASTNode::PLUS;  break;
        				case OperatorToken::MINUS: type = OperationASTNode::MINUS; break;
        				case OperatorToken::MUL:   type = OperationASTNode::MUL;   break;
        				case OperatorToken::DIV:   type = OperationASTNode::DIV;   break;
        				case OperatorToken::POW:   type = OperationASTNode::POW;   break;
        				case OperatorToken::MOD:   type = OperationASTNode::MOD;   break;
        				default:
							std::stringstream ss;
							ss << "Unknown operator token, character: ";
							ss << token->getPosition();
							throw ParserException(ss.str().c_str());
        			}
        			stack.push_front(new OperationASTNode(type, left, right));
        			continue;
        		}

        		if (token->getType() == Token::FUNCTION)
        		{
					if (stack.size() == 0)
					{
						std::stringstream ss;
						ss << "Invalid syntax: function given with insufficient operands, character: ";
						ss << token->getPosition();
						throw ParserException(ss.str().c_str());
					}
        			FunctionToken *f = static_cast<FunctionToken*>(token);
        			ASTNode *left = stack.front(); stack.pop_front();

        			switch(f->getFunction())
        			{
        				case FunctionToken::SIN:
        					stack.push_front(new Function1ASTNode(Function1ASTNode::SIN, left));
        					break;
        				case FunctionToken::COS:
        					stack.push_front(new Function1ASTNode(Function1ASTNode::COS, left));
        					break;
        				case FunctionToken::TAN:
        					stack.push_front(new Function1ASTNode(Function1ASTNode::TAN, left));
        					break;
        				case FunctionToken::SQRT:
							stack.push_front(new Function1ASTNode(Function1ASTNode::SQRT, left));
							break;
        				case FunctionToken::LOG:
							stack.push_front(new Function1ASTNode(Function1ASTNode::LOG, left));
							break;
        				case FunctionToken::LOG2:
							stack.push_front(new Function1ASTNode(Function1ASTNode::LOG2, left));
							break;
        				case FunctionToken::LOG10:
							stack.push_front(new Function1ASTNode(Function1ASTNode::LOG10, left));
							break;
        				case FunctionToken::CEIL:
							stack.push_front(new Function1ASTNode(Function1ASTNode::CEIL, left));
							break;
        				case FunctionToken::FLOOR:
							stack.push_front(new Function1ASTNode(Function1ASTNode::FLOOR, left));
							break;
        				case FunctionToken::MIN:
        				{
							if (stack.size() == 0)
							{
								std::stringstream ss;
								ss << "Invalid syntax: function given with insuffucient operands, character: ";
								ss << token->getPosition();
								throw ParserException(ss.str().c_str());
							}
        					ASTNode *right = stack.front(); stack.pop_front();
        					stack.push_front(new Function2ASTNode(Function2ASTNode::MIN, left, right));
        				}
        				break;
        				case FunctionToken::MAX:
						{
							if (stack.size() == 0)
							{
								std::stringstream ss;
								ss << "Invalid syntax: function given with insuffucient operands, character: ";
								ss << token->getPosition();
								throw ParserException(ss.str().c_str());
							}
							ASTNode *right = stack.front(); stack.pop_front();
							stack.push_front(new Function2ASTNode(Function2ASTNode::MAX, left, right));
						}
						case FunctionToken::POW:
        				{
							if (stack.size() == 0)
							{
								std::stringstream ss;
								ss << "Invalid syntax: function given with insuffucient operands, character: ";
								ss << token->getPosition();
								throw ParserException(ss.str().c_str());
							}
        					ASTNode *right = stack.front(); stack.pop_front();
        					stack.push_front(new Function2ASTNode(Function2ASTNode::POW, left, right));
        				}
						break;
        				default:
							std::stringstream ss;
							ss << "Unknown function token, character: ";
							ss << token->getPosition();
							throw ParserException(ss.str().c_str());
        			}
        			continue;
        		}

        		if (token->getType() == Token::CONDITIONAL)
        		{
					if (stack.size() < 2)
					{
						std::stringstream ss;
						ss << "Invalid syntax: conditional operator given with insuffucient operands, character: ";
						ss << token->getPosition();
						throw ParserException(ss.str().c_str());
					}
        			ConditionalToken *c = static_cast<ConditionalToken*>(token);
        			ASTNode *left = stack.front(); stack.pop_front();
        			ASTNode *right = stack.front(); stack.pop_front();
        			ComparisonASTNode::ComparisonType type;
        			switch(c->getConditional())
        			{
        				case ConditionalToken::EQUAL:
        					type = ComparisonASTNode::EQUAL; break;
        				case ConditionalToken::NOT_EQUAL:
        					type = ComparisonASTNode::NOT_EQUAL; break;
        				case ConditionalToken::GREATER_THAN:
        					type = ComparisonASTNode::GREATER_THAN; break;
        				case ConditionalToken::GREATER_THAN_EQUAL:
        					type = ComparisonASTNode::GREATER_THAN_EQUAL; break;
        				case ConditionalToken::LESS_THAN:
        					type = ComparisonASTNode::LESS_THAN; break;
        				case ConditionalToken::LESS_THAN_EQUAL:
        					type = ComparisonASTNode::LESS_THAN_EQUAL; break;
        				default:
							std::stringstream ss;
							ss << "Unknown conditional operator token, character: ";
							ss << token->getPosition();
							throw ParserException(ss.str().c_str());
        			}
        			stack.push_front(new ComparisonASTNode(type, left, right));
        			continue;
        		}

        		if (token->getType() == Token::LOGICAL)
        		{
					if (stack.size() < 2)
					{
						std::stringstream ss;
						ss << "Invalid syntax: logical operator given with insuffucient operands, character: ";
						ss << token->getPosition();
						throw ParserException(ss.str().c_str());
					}
        			LogicalToken *l = static_cast<LogicalToken*>(token);
        			ASTNode *left = stack.front(); stack.pop_front();
        			ASTNode *right = stack.front(); stack.pop_front();
        			LogicalASTNode::OperationType type;
        			switch(l->getOperator())
        			{
        				case LogicalToken::AND:
        					type = LogicalASTNode::AND; break;
        				case LogicalToken::OR:
        					type = LogicalASTNode::OR; break;
        				default:
							std::stringstream ss;
							ss << "Unknown logical operator token, character: ";
							ss << token->getPosition();
							throw ParserException(ss.str().c_str());
        			}
        			stack.push_front(new LogicalASTNode(type, left, right));
					continue;
        		}

        		if (token->getType() == Token::TERNARY)
        		{
					if (stack.size() < 3)
					{
						std::stringstream ss;
						ss << "Invalid syntax: ternary operator given with insuffucient operands, character: ";
						ss << token->getPosition();
						throw ParserException(ss.str().c_str());
					}
        			TernaryToken *t = static_cast<TernaryToken*>(token);
        			if (t->getSymbol() == TernaryToken::TERNARY)
        			{
        				ASTNode *no =  stack.front(); stack.pop_front();
        				ASTNode *yes = stack.front(); stack.pop_front();
        				ASTNode *condition = stack.front(); stack.pop_front();

        				stack.push_front(new BranchASTNode(condition, yes, no));
        			}
        			continue;
        		}
        	}

        	if (stack.size() != 0)
        	{
        		return stack.front();
        	}

        	return NULL;
        }
};

} // namespace expr

#endif
