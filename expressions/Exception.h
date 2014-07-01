#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <exception>
#include <stdexcept>

namespace expr
{

class Exception : public std::runtime_error
{
	public:
		Exception(const char* message)
			: std::runtime_error(message)
		{
		}
};

} //namespace expr

#endif
