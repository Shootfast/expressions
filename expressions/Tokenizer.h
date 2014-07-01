#ifndef TOKENIZER_H
#define TOKENIZER_H

#include "ctype.h" // for isspace, isdigit and isalnum
#include "string.h" // for memcpy
#include "Memory.h"
#include "Exception.h"
#include <string>
#include <deque>
#include <sstream>

namespace expr
{

class Token
{
    public:
		enum TokenType
		{
			UNARY,
		    OPERATOR,
		    FUNCTION,
		    COMMA,
		    CONDITIONAL,
		    LOGICAL,
		    TERNARY,
		    OPEN_PARENTHESIS,
		    CLOSE_PARENTHESIS,
		    NUMBER,
		    VARIABLE,
		    ENDOFTEXT
		};

		Token(TokenType type, int pos)
			: m_type(type)
			, m_pos(pos)
        {}

		~Token()
		{}

		TokenType getType()
		{
			return m_type;
		}

		int getPosition()
		{
			return m_pos;
		}

		virtual std::string print()
		{
			return "";
		}

    protected:
        TokenType	m_type;
		int         m_pos;
};
typedef SHARED_PTR<Token> TokenPtr;


template<typename T>
class NumberToken : public Token
{
	public:
		NumberToken(T value, int pos)
			: Token(NUMBER, pos)
			, m_value(value)
		{}

		~NumberToken()
		{}

		T getValue()
		{
			return m_value;
		}

		std::string print()
		{
			std::stringstream ss;
			ss << m_value;
			return ss.str();
		}

	protected:
		T m_value;
};

class VariableToken : public Token
{
	public:
		VariableToken(std::string value, int pos)
			: Token(VARIABLE, pos)
			, m_value(value)
		{}

		~VariableToken()
		{}

		std::string getValue()
		{
			return m_value;
		}

		std::string print()
		{
			return m_value;
		}

	protected:
		std::string m_value;
};



class FunctionToken : public Token
{
	public:
		enum FunctionType
			{
				SIN,
				COS,
				TAN,
				SQRT,
				LOG,
				LOG2,
				LOG10,
				CEIL,
				FLOOR,
				MIN,
				MAX,
				POW
			};
		FunctionToken(FunctionType function, int pos)
			: Token(FUNCTION, pos)
			, m_function(function)
		{}

		~FunctionToken()
		{}

		FunctionType getFunction()
		{
			return m_function;
		}

		std::string print()
		{
			switch(m_function)
			{
				case FunctionToken::SIN:
					return "sin";
				case FunctionToken::COS:
					return "cos";
				case FunctionToken::TAN:
					return "tan";
				case FunctionToken::SQRT:
					return "sqrt";
				case FunctionToken::LOG:
					return "log";
				case FunctionToken::LOG2:
					return "log2";
				case FunctionToken::LOG10:
					return "log10";
				case FunctionToken::CEIL:
					return "ceil";
				case FunctionToken::FLOOR:
					return "floor";
				case FunctionToken::MIN:
					return "min";
				case FunctionToken::MAX:
					return "max";
				case FunctionToken::POW:
					return "pow";
				default:
					return "";
			}
		}

	protected:
		FunctionType m_function;
};


class PrecedenceOperator: public Token
{
	public:
		PrecedenceOperator(Token::TokenType type, int pos)
			: Token(type, pos)
		{}
		virtual ~PrecedenceOperator()
		{}

		virtual int precedence()
		{
			return 0;
		}

		virtual bool leftAssociative()
		{
			return true;
		}
};


class OperatorToken : public PrecedenceOperator
{
	public:
		enum OperatorType
		{
			PLUS,
			MINUS,
			MUL,
			DIV,
			POW,
			MOD
		};

		OperatorToken(OperatorType op, int pos)
			: PrecedenceOperator(Token::OPERATOR, pos)
			, m_operator(op)
		{}

		~OperatorToken()
		{}

		OperatorType getOperator()
		{
			return m_operator;
		}

		std::string print()
		{
			switch(m_operator)
			{
				case OperatorToken::PLUS:
					return "+";
				case OperatorToken::MINUS:
					return "-";
				case OperatorToken::MUL:
					return "*";
				case OperatorToken::DIV:
					return "/";
				case OperatorToken::POW:
					return "^";
				case OperatorToken::MOD:
					return "%";
				default:
					return "";
			}

		}

		int precedence()
		{
			switch(m_operator)
			{
				case PLUS:
				case MINUS:
					return 20;
				case MUL:
				case DIV:
				case MOD:
					return 30;
				case POW:
					return 40;
				default:
					break;
			}
			return 0;
		}

		bool leftAssociative()
		{
			switch(m_operator)
			{
				case PLUS:
				case MINUS:
				case MUL:
				case DIV:
				case MOD:
					return true;
				case POW:
					return false;
				default:
					break;
			}
			return false;
		}

	protected:
		OperatorType m_operator;
};


class ConditionalToken : public PrecedenceOperator
{
	public:
		enum ConditionalType
		{
			EQUAL,
			NOT_EQUAL,
			GREATER_THAN,
			GREATER_THAN_EQUAL,
			LESS_THAN,
			LESS_THAN_EQUAL
		};

		ConditionalToken(ConditionalType conditional, int pos)
			: PrecedenceOperator(Token::CONDITIONAL, pos)
			, m_conditional(conditional)
		{}

		~ConditionalToken()
		{}

		ConditionalType getConditional()
		{
			return m_conditional;
		}

		std::string print()
		{
			switch(m_conditional)
			{
				case ConditionalToken::EQUAL:
					return "==";
				case ConditionalToken::NOT_EQUAL:
					return "!=";
				case ConditionalToken::LESS_THAN:
					return "<";
				case ConditionalToken::LESS_THAN_EQUAL:
					return "<=";
				case ConditionalToken::GREATER_THAN:
					return ">";
				case ConditionalToken::GREATER_THAN_EQUAL:
					return ">=";
				default:
					return "";
			}
		}

		int precedence()
		{
			switch(m_conditional)
			{
				case EQUAL:
				case NOT_EQUAL:
					return 10;
				case GREATER_THAN:
				case GREATER_THAN_EQUAL:
				case LESS_THAN:
				case LESS_THAN_EQUAL:
					return 15;
				default:
					break;
			}
			return 0;
		}

	protected:
		ConditionalType m_conditional;
};


class LogicalToken : public PrecedenceOperator
{
	public:
		enum OperatorType
		{
			AND,
			OR
		};

		LogicalToken(OperatorType op, int pos)
			: PrecedenceOperator(Token::LOGICAL, pos)
			, m_operator(op)
		{}

		~LogicalToken()
		{}

		OperatorType getOperator()
		{
			return m_operator;
		}

		std::string print()
		{
			switch(m_operator)
			{
				case LogicalToken::AND:
					return "&&";
				case LogicalToken::OR:
					return "||";
				default:
					return "";
			}
		}

		int precedence()
		{
			switch(m_operator)
			{
				case AND:
					return 9;
				case OR:
					return 8;
				default:
					break;
			}
			return 0;
		}

	private:
		OperatorType m_operator;
};

class TernaryToken : public PrecedenceOperator
{
	public:
		enum SymbolType
		{
			TERNARY,
			COLON
		};

		TernaryToken(SymbolType symbol, int pos)
			: PrecedenceOperator(Token::TERNARY, pos)
			, m_symbol(symbol)
		{}

		~TernaryToken()
		{}

		SymbolType getSymbol()
		{
			return m_symbol;
		}

		std::string print()
		{
			switch(m_symbol)
			{
				case TernaryToken::TERNARY:
					return "?";
				case TernaryToken::COLON:
					return ":";
				default:
					return "";
			}

		}

		int precedence()
		{
			return 5;
		}

		bool leftAssociative()
		{
			return false;
		}

	protected:
		SymbolType m_symbol;
};


class UnaryToken: public PrecedenceOperator
{
	public:
		enum UnaryType
		{
			POSITIVE,
			NEGATIVE
		};
		UnaryToken(UnaryType direction, int pos)
			: PrecedenceOperator(Token::UNARY, pos)
			, m_direction(direction)
		{}

		~UnaryToken()
		{}

		UnaryType getDirection()
		{
			return m_direction;
		}

		std::string print()
		{
			switch(m_direction)
			{
				case UnaryToken::POSITIVE:
					return "u+";;
				case UnaryToken::NEGATIVE:
					return "u-";
				default:
					return "";
			}

		}

		int precedence()
		{
			return 50;
		}

		bool leftAssociative()
		{
			return false;
		}


	protected:
		UnaryType m_direction;
};


class TokenizerException : public Exception
{
    public:
		TokenizerException(const char * message)
        : Exception(message)
        {
        }
};

template <typename T>
class Tokenizer
{
	typedef std::deque<TokenPtr> TokVec;
	public:
		Tokenizer(const char *text)
			: m_text(text)
			, m_index(0)
		{
		}

		~Tokenizer()
		{
		}

		void tokenize(TokVec &tokens)
		{
			while (m_text[m_index] != 0)
			{
				skipWhitespace();
				if (m_text[m_index] == 0)
				{
					break;
				}

				// Check for numbers
				if (isdigit(m_text[m_index]) || m_text[m_index] == '.')
				{
					tokens.push_back(TokenPtr(new NumberToken<T>(getNumber(), m_index)));
					continue;
				}

				// Check for single character operators
				{
					bool match = false;
					switch(m_text[m_index])
					{
						case '+':
						{
							TokenPtr token;

							// Attempt to detect unary minus
							if (tokens.size() == 0)
							{
								// No tokens before, so unary
								token = TokenPtr(new UnaryToken(UnaryToken::POSITIVE, m_index));
							}
							else
							{
								if (tokens.back()->getType() == Token::CLOSE_PARENTHESIS ||
									tokens.back()->getType() == Token::NUMBER            ||
									tokens.back()->getType() == Token::VARIABLE )
								{
									token = TokenPtr(new OperatorToken(OperatorToken::PLUS, m_index));
								}
								else if (tokens.back()->getType() == Token::FUNCTION)
								{
									std::stringstream ss;
									ss << "Invalid syntax: unary positive '+' following function declaration, character: ";
									ss << m_index;
									throw TokenizerException(ss.str().c_str());
								}
								else if (tokens.back()->getType() == Token::UNARY)
								{
									std::stringstream ss;
									ss << "Invalid syntax: unary positive '+' following unary declaration, character: ";
									ss << m_index;
									throw TokenizerException(ss.str().c_str());
								}
								else
								{
									token = TokenPtr(new UnaryToken(UnaryToken::POSITIVE, m_index));
								}
							}
							tokens.push_back(token);
							match = true;
							break;
						}
						case '-':
						{
							TokenPtr token;

							// Attempt to detect unary minus
							if (tokens.size() == 0)
							{
								// No tokens before, so unary
								token = TokenPtr(new UnaryToken(UnaryToken::NEGATIVE, m_index));
							}
							else
							{
								if (tokens.back()->getType() == Token::CLOSE_PARENTHESIS ||
									tokens.back()->getType() == Token::NUMBER            ||
									tokens.back()->getType() == Token::VARIABLE )
								{
									token = TokenPtr(new OperatorToken(OperatorToken::MINUS, m_index));
								}
								else if (tokens.back()->getType() == Token::FUNCTION)
								{
									std::stringstream ss;
									ss << "Invalid syntax: unary negative '-' following function declaration, character: ";
									ss << m_index;
									throw TokenizerException(ss.str().c_str());
								}
								else if (tokens.back()->getType() == Token::UNARY)
								{
									std::stringstream ss;
									ss << "Invalid syntax: unary negative '-' following unary declaration, character: ";
									ss << m_index;
									throw TokenizerException(ss.str().c_str());
								}
								else
								{
									token = TokenPtr(new UnaryToken(UnaryToken::NEGATIVE, m_index));
								}
							}
							tokens.push_back(token);
							match = true;
							break;
						}
						case '*':
							followsExpression("multiplication operator '*'", tokens);
							tokens.push_back(TokenPtr(new OperatorToken(OperatorToken::MUL, m_index)));
							match = true;
							break;
						case '/':
							followsExpression("division operator '/'", tokens);
							tokens.push_back(TokenPtr(new OperatorToken(OperatorToken::DIV, m_index)));
							match = true;
							break;
						case '^':
							followsExpression("power operator '^'", tokens);
							tokens.push_back(TokenPtr(new OperatorToken(OperatorToken::POW, m_index)));
							match = true;
							break;
						case '%':
							followsExpression("modulus operator '%'", tokens);
							tokens.push_back(TokenPtr(new OperatorToken(OperatorToken::MOD, m_index)));
							match = true;
							break;
						case '?':
							followsExpression("ternary declaration '?'", tokens);
							tokens.push_back(TokenPtr(new TernaryToken(TernaryToken::TERNARY, m_index)));
							match = true;
							break;
						case ':':
							followsExpression("ternary divider ':'", tokens);
							tokens.push_back(TokenPtr(new TernaryToken(TernaryToken::COLON, m_index)));
							match = true;
							break;
						case '(':
							tokens.push_back(TokenPtr(new Token(Token::OPEN_PARENTHESIS, m_index)));
							match = true;
							break;
						case ')':
							tokens.push_back(TokenPtr(new Token(Token::CLOSE_PARENTHESIS, m_index)));
							match = true;
							break;
						case ',':
							followsExpression("comma separator ','", tokens);
							tokens.push_back(TokenPtr(new Token(Token::COMMA, m_index)));
							match = true;
							break;
						default:
							break;
					}
					if (match)
					{
						m_index++;
						continue;
					}
				}

				// Look for known two character keywords
				{
					bool match = false;
					std::string word;
					if (m_text[m_index+1] != 0)
					{
						word = m_text[m_index];
						word += m_text[m_index+1];

						if (word == "==")
						{
							followsExpression("equality conditional '=='", tokens);
							tokens.push_back(TokenPtr(new ConditionalToken(ConditionalToken::EQUAL, m_index)));
							m_index++;
							match = true;
						}
						else if (word == "!=")
						{
							followsExpression("inequality conditional '!='", tokens);
							tokens.push_back(TokenPtr(new ConditionalToken(ConditionalToken::NOT_EQUAL, m_index)));
							m_index++;
							match = true;
						}
						else if (word == "<=")
						{
							followsExpression("less-than-or-equal conditional '<='", tokens);
							tokens.push_back(TokenPtr(new ConditionalToken(ConditionalToken::LESS_THAN_EQUAL, m_index)));
							m_index++;
							match = true;
						}
						else if (word == ">=")
						{
							followsExpression("greater-than-or-equal conditional '>='", tokens);
							tokens.push_back(TokenPtr(new ConditionalToken(ConditionalToken::GREATER_THAN_EQUAL, m_index)));
							m_index++;
							match = true;
						}
						else if (word == "&&")
						{
							followsExpression("logical and operator '&&'", tokens);
							tokens.push_back(TokenPtr(new LogicalToken(LogicalToken::AND, m_index)));
							m_index++;
							match = true;
						}
						else if (word == "||")
						{
							followsExpression("logical or operator '||'", tokens);
							tokens.push_back(TokenPtr(new LogicalToken(LogicalToken::OR, m_index)));
							m_index++;
							match = true;
						}
						else
						{
							// Look for single characters that are substrings of the words above
							switch(m_text[m_index])
							{
								case '<':
									followsExpression("less-than conditional '<'", tokens);
									tokens.push_back(TokenPtr(new ConditionalToken(ConditionalToken::LESS_THAN, m_index)));
									match = true;
									break;
								case '>':
									followsExpression("greater-than conditional '>'", tokens);
									tokens.push_back(TokenPtr(new ConditionalToken(ConditionalToken::GREATER_THAN, m_index)));
									match = true;
									break;
								default:
									break;
							}

						}
					}
					if (match)
					{
						m_index++; // 2 character words will have already incremented once
						continue;
					}
				}


				// Look for multi character keywords or variables
				{
					bool match = false;
					std::string word;

					while (m_text[m_index] != 0)
					{
						char c = m_text[m_index];

						// Only proceed if character is alphanumeric or underscore
						if (!isalnum(c) && c != '_')
						{
							break;
						}

						word +=c;
						m_index++;

						if (word ==  "sin")
						{
							expressionAllowed("sin", tokens);
							tokens.push_back(TokenPtr(new FunctionToken(FunctionToken::SIN, m_index)));
							match = true;
							break;
						}
						else if (word == "cos")
						{
							expressionAllowed("cos", tokens);
							tokens.push_back(TokenPtr(new FunctionToken(FunctionToken::COS, m_index)));
							match = true;
							break;
						}
						else if (word == "tan")
						{
							expressionAllowed("tan", tokens);
							tokens.push_back(TokenPtr(new FunctionToken(FunctionToken::TAN, m_index)));
							match = true;
							break;
						}
						else if (word == "sqrt")
						{
							expressionAllowed("sqrt", tokens);
							tokens.push_back(TokenPtr(new FunctionToken(FunctionToken::SQRT, m_index)));
							match = true;
							break;
						}
						else if (word == "ceil")
						{
							expressionAllowed("ceil", tokens);
							tokens.push_back(TokenPtr(new FunctionToken(FunctionToken::CEIL, m_index)));
							match = true;
							break;
						}
						else if (word == "floor")
						{
							expressionAllowed("floor", tokens);
							tokens.push_back(TokenPtr(new FunctionToken(FunctionToken::FLOOR, m_index)));
							match = true;
							break;
						}
						else if (word == "min")
						{
							expressionAllowed("min", tokens);
							tokens.push_back(TokenPtr(new FunctionToken(FunctionToken::MIN, m_index)));
							match = true;
							break;
						}
						else if (word == "max")
						{
							expressionAllowed("max", tokens);
							tokens.push_back(TokenPtr(new FunctionToken(FunctionToken::MAX, m_index)));
							match = true;
							break;
						}
						else if (word == "pow")
						{
							expressionAllowed("pow", tokens);
							tokens.push_back(TokenPtr(new FunctionToken(FunctionToken::POW, m_index)));
							match = true;
							break;
						}
						else if (word == "log")
						{
							// Check for other versions of log
							if (m_text[m_index] == '2')
							{
								expressionAllowed("log2", tokens);
								m_index++;
								tokens.push_back(TokenPtr(new FunctionToken(FunctionToken::LOG2, m_index)));
							}
							else if (m_text[m_index] == '1')
							{
								if (m_text[m_index + 1] == '0')
								{
								expressionAllowed("log10", tokens);
									m_index+=2;
									tokens.push_back(TokenPtr(new FunctionToken(FunctionToken::LOG10, m_index)));
								}
							}
							else
							{
									expressionAllowed("log", tokens);
									m_index+=2;
								tokens.push_back(TokenPtr(new FunctionToken(FunctionToken::LOG, m_index)));
							}
							match = true;
							break;
						}

					}

					if (match)
					{
						continue;
					}
					else
					{
						if (word != "")
						{
							expressionAllowed(std::string("variable '") + word + "'", tokens);
							tokens.push_back(TokenPtr(new VariableToken(word, m_index)));
							continue;
						}
					}
				}
				// Don't know what this token is
				std::stringstream ss;
				ss << "Unknown token '" << m_text[m_index] << "', character: ";
				ss << m_index;
				throw TokenizerException(ss.str().c_str());


			}

			// add the endoftext token
			if (tokens.size() != 0)
			{
				if (tokens.back()->getType() != Token::NUMBER         &&
					tokens.back()->getType() != Token::VARIABLE       &&
					tokens.back()->getType() != Token::CLOSE_PARENTHESIS )
				{
					std::stringstream ss;
					ss << "Unexpected end of expression, character: " << m_index;
					throw TokenizerException(ss.str().c_str());
				}
			}
			tokens.push_back(TokenPtr(new Token(Token::ENDOFTEXT, m_index)));
		}

	private:

		void skipWhitespace()
		{
			while(isspace(m_text[m_index]))
			{
				m_index++;
			}
		}

		void followsExpression(std::string descriptor, TokVec &tokens)
		{
			if (tokens.size() == 0)
			{
				std::stringstream ss;
				ss << "Invalid syntax: " << descriptor << " must follow expression, character: ";
				ss << m_index +1;
				throw TokenizerException(ss.str().c_str());
			}

			if (tokens.back()->getType() != Token::NUMBER         &&
				tokens.back()->getType() != Token::VARIABLE       &&
				tokens.back()->getType() != Token::CLOSE_PARENTHESIS )
			{
				std::stringstream ss;
				ss << "Invalid syntax: " << descriptor << " must follow expression, character: ";
				ss << m_index + 1;
				throw TokenizerException(ss.str().c_str());
			}
		}

		void expressionAllowed(std::string descriptor, TokVec &tokens)
		{
			if (tokens.size() != 0)
			{
				if (tokens.back()->getType() == Token::NUMBER         ||
					tokens.back()->getType() == Token::VARIABLE       ||
					tokens.back()->getType() == Token::FUNCTION       ||
					tokens.back()->getType() == Token::CLOSE_PARENTHESIS )
				{
					std::stringstream ss;
					ss << "Invalid syntax: " << descriptor << " cannot directly follow another expression without an operator between, character: ";
					ss << m_index;
					throw TokenizerException(ss.str().c_str());
				}
			}
		}

		T getNumber()
		{
			int index = m_index;
			std::stringstream ss;
			T value;

			// loop through till we find a non digit
			while(isdigit(m_text[m_index]))
			{
				m_index++;
			}
			// check for decimal point
			if(m_text[m_index] == '.')
			{
				m_index++;

				// Add the digits after the decimal point
				while(isdigit(m_text[m_index]))
				{
					m_index++;
				}
			}
			// check for exponent
			if(m_text[m_index] == 'e' || m_text[m_index] == 'E')
			{
				m_index++;
				if (m_text[m_index] == '+' || m_text[m_index] == '-')
				{
					m_index++;
				}

				// Add the rest of the digits
				while(isdigit(m_text[m_index]))
				{
					m_index++;
				}
			}

			// Convert from char *to T via stringstream
			char *number = new char[m_index - index +1];
			memcpy(number, &m_text[index], (m_index - index)*sizeof(char));
			number[m_index-index] = '\0';
			ss << number;
			ss >> value;
			delete[] number;
			return value;

		}

		const char * m_text;
		int m_index;
};

} // namespace expr

#endif
