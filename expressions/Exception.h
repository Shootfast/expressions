#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <exception>

namespace expr
{

class Exception : public std::exception
{
	public:
		Exception(const char* message)
		{
			m_message = message;
		}

		virtual const char* what() const throw()
		{
			return m_message;
		}

	private:
		const char *m_message;
};

} //namespace expr

#endif
