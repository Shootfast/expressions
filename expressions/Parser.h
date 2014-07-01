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

        ASTNodePtr parse(const char* text)
        {
            try
            {
				std::deque<TokenPtr> tokens;
				Tokenizer<T>(text).tokenize(tokens);
                shuntingYard(tokens);
                ASTNodePtr node = rpnToAST(tokens);
				return node;
            }
            catch (TokenizerException &e)
            {
                throw ParserException(e.what());
            }
        }

        void print(std::deque<TokenPtr> &tokens)
        {
        	for (unsigned int i=0; i<tokens.size(); i++)
        	{
        		TokenPtr token = tokens[i];
        		std::cout << token->print();
        		std::cout << " ";
        	}
        	std::cout << std::endl;
        }


    private:

        void shuntingYard(std::deque<TokenPtr> &tokens)
		{
           	// This is an implementation of the shunting yard algorithm
			// to convert infix notation into reverse polish notation.
			// this ensures that the operator precedence is maintained
			// in our AST.

        	// Stacks for "shunting"
        	std::deque<TokenPtr> output;
        	std::deque<TokenPtr> stack;

        	// While there are tokens to be read
        	while (tokens.size() != 0)
        	{
        		// Read a token
        		TokenPtr token = tokens.front(); tokens.pop_front();

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
        					TokenPtr stackToken = stack.front();
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
        				TokenPtr stackToken = stack.front();
        				if (stackToken->getType() == Token::OPERATOR    ||
        					stackToken->getType() == Token::UNARY       ||
        					stackToken->getType() == Token::CONDITIONAL ||
        					stackToken->getType() == Token::LOGICAL     ||
        					stackToken->getType() == Token::TERNARY )
        				{
        					SHARED_PTR<PrecedenceOperator> op1 = STATIC_POINTER_CAST<PrecedenceOperator>(token);
        					SHARED_PTR<PrecedenceOperator> op2 = STATIC_POINTER_CAST<PrecedenceOperator>(stackToken);
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
        					TokenPtr stackToken = stack.front();
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
        	}

    		// When there are no more tokens to read
        	// While there are still operator tokens in the stack
        	while (stack.size() != 0)
        	{
        		TokenPtr stackToken = stack.front();
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

			// Replace the tokens in the original TokVec
			tokens = std::deque<TokenPtr>(output.begin(), output.end());
        }

        ASTNodePtr rpnToAST(std::deque<TokenPtr> &tokens)
        {
        	// To convert from RPN to AST, just push each number node onto the front of the stack
        	// and for each operator, pop the required operands, then push the resulting node
        	// to the front of the stack

        	std::deque<ASTNodePtr> stack;
        	for (unsigned int i=0; i<tokens.size(); i++)
        	{
        		TokenPtr token = tokens[i];

        		if (token->getType() == Token::NUMBER)
				{
					SHARED_PTR<NumberToken<T> > nt = STATIC_POINTER_CAST<NumberToken<T> >(token);
					ASTNodePtr n = ASTNodePtr(new NumberASTNode<T>(nt->getValue()));
					stack.push_front(n);
					continue;
				}

        		if (token->getType() == Token::VARIABLE)
        		{
					SHARED_PTR<VariableToken> v = STATIC_POINTER_CAST<VariableToken>(token);
        			std::string key = v->getValue();
        			ASTNodePtr n = ASTNodePtr(new VariableASTNode<T>(key));
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

        			SHARED_PTR<UnaryToken> u = STATIC_POINTER_CAST<UnaryToken>(token);

        			if (u->getDirection() == UnaryToken::NEGATIVE)
        			{

        				// Take the number from the top of the stack and make it negative
						SHARED_PTR<NumberASTNode<T> > n = STATIC_POINTER_CAST<NumberASTNode<T> >(stack.front());
        				stack.pop_front();

        				stack.push_front(ASTNodePtr(new NumberASTNode<T>(n->value() * -1)));
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

        			SHARED_PTR<OperatorToken> opt = STATIC_POINTER_CAST<OperatorToken>(token);
        			ASTNodePtr left = stack.front(); stack.pop_front();
        			ASTNodePtr right = stack.front(); stack.pop_front();
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
        			stack.push_front(ASTNodePtr(new OperationASTNode(type, left, right)));
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
        			SHARED_PTR<FunctionToken> f = STATIC_POINTER_CAST<FunctionToken>(token);
        			ASTNodePtr left = stack.front(); stack.pop_front();

        			switch(f->getFunction())
        			{
        				case FunctionToken::SIN:
        					stack.push_front(ASTNodePtr(new Function1ASTNode(Function1ASTNode::SIN, left)));
        					break;
        				case FunctionToken::COS:
        					stack.push_front(ASTNodePtr(new Function1ASTNode(Function1ASTNode::COS, left)));
        					break;
        				case FunctionToken::TAN:
        					stack.push_front(ASTNodePtr(new Function1ASTNode(Function1ASTNode::TAN, left)));
        					break;
        				case FunctionToken::SQRT:
							stack.push_front(ASTNodePtr(new Function1ASTNode(Function1ASTNode::SQRT, left)));
							break;
        				case FunctionToken::LOG:
							stack.push_front(ASTNodePtr(new Function1ASTNode(Function1ASTNode::LOG, left)));
							break;
        				case FunctionToken::LOG2:
							stack.push_front(ASTNodePtr(new Function1ASTNode(Function1ASTNode::LOG2, left)));
							break;
        				case FunctionToken::LOG10:
							stack.push_front(ASTNodePtr(new Function1ASTNode(Function1ASTNode::LOG10, left)));
							break;
        				case FunctionToken::CEIL:
							stack.push_front(ASTNodePtr(new Function1ASTNode(Function1ASTNode::CEIL, left)));
							break;
        				case FunctionToken::FLOOR:
							stack.push_front(ASTNodePtr(new Function1ASTNode(Function1ASTNode::FLOOR, left)));
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
        					ASTNodePtr right = stack.front(); stack.pop_front();
        					stack.push_front(ASTNodePtr(new Function2ASTNode(Function2ASTNode::MIN, left, right)));
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
							ASTNodePtr right = stack.front(); stack.pop_front();
							stack.push_front(ASTNodePtr(new Function2ASTNode(Function2ASTNode::MAX, left, right)));
						}
						break;
						case FunctionToken::POW:
        				{
							if (stack.size() == 0)
							{
								std::stringstream ss;
								ss << "Invalid syntax: function given with insuffucient operands, character: ";
								ss << token->getPosition();
								throw ParserException(ss.str().c_str());
							}
        					ASTNodePtr right = stack.front(); stack.pop_front();
        					stack.push_front(ASTNodePtr(new Function2ASTNode(Function2ASTNode::POW, left, right)));
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
        			SHARED_PTR<ConditionalToken> c = STATIC_POINTER_CAST<ConditionalToken>(token);
        			ASTNodePtr left = stack.front(); stack.pop_front();
        			ASTNodePtr right = stack.front(); stack.pop_front();
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
        			stack.push_front(ASTNodePtr(new ComparisonASTNode(type, left, right)));
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
        			SHARED_PTR<LogicalToken> l = STATIC_POINTER_CAST<LogicalToken>(token);
        			ASTNodePtr left = stack.front(); stack.pop_front();
        			ASTNodePtr right = stack.front(); stack.pop_front();
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
        			stack.push_front(ASTNodePtr(new LogicalASTNode(type, left, right)));
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
        			SHARED_PTR<TernaryToken> t = STATIC_POINTER_CAST<TernaryToken>(token);
        			if (t->getSymbol() == TernaryToken::TERNARY)
        			{
        				ASTNodePtr no =  stack.front(); stack.pop_front();
        				ASTNodePtr yes = stack.front(); stack.pop_front();
        				ASTNodePtr condition = stack.front(); stack.pop_front();

        				stack.push_front(ASTNodePtr(new BranchASTNode(condition, yes, no)));
        			}
        			continue;
        		}
        	}

        	if (stack.size() != 0)
        	{
        		return stack.front();
        	}

        	return ASTNodePtr();
        }
};

} // namespace expr

#endif
