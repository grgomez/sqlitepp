#include <stdio.h>
#include "SQLite.hpp"

int main ()
{
	try
	{
		Connection connection = Connection::Memory();
		
		Statement statement;
		
		statement.Prepare(connection, "select 'Hello' union all select 'world'");
		
		for (Row const & row : statement)
		{
			printf("%s\n", statement.GetString(0));
		}
	}
	catch (Exception const & e)
	{
		printf("%s (%d)\n", e.Message.c_str(), e.Result);
	}
}

