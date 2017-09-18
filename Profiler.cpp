#include <stdio.h>
#include "SQLite.hpp"

static char const * TypeName(Type const type)
{
	switch(type)
	{
		case Type::Integer: return "Integer";
		case Type::Float: return "Float";
		case Type::Blob: return "Blob";
		case Type::Null: return "Null";
		case Type::Text: return "Text";
	}

	return "Invalid";
}

int main ()
{
	try
	{
		Connection connection = Connection::Memory();
		
		connection.Profile([](void *, 
					char const * const statement, 
					unsigned long long const time)		
				{
					unsigned long long const ms = time / 1000000;

					if (ms > 10)
					{
						printf("Profiler (%lld) %s\n", ms, statement);
					}	
				});
		
	}
	catch (Exception const & e)
	{
		printf("%s (%d)\n", e.Message.c_str(), e.Result);
	}
}

