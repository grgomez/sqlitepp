#include <stdio.h>
#include "SQLite.hpp"

int main ()
{
	try
	{
		Connection connection = Connection::Memory();
		Execute(connection, "create table users(name)");
		Execute(connection, "insert into users values (?)", "Joe");	
		Execute(connection, "insert into users values (?)", "Mary");
		for (Row row : Statement(connection, "select Name from users"))
		{
			printf("%s\n", row.GetString());
		}
	}
	catch (Exception const & e)
	{
		printf("%s (%d)\n", e.Message.c_str(), e.Result);
	}
}

