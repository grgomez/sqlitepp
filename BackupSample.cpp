#include <stdio.h>
#include "SQLite.hpp"

static void SaveToDisk(Connection const & source, char const * const filename)
{
	Connection destination(filename);
	Backup backup(destination, source);
	backup.Step();
}

int main ()
{
	try
	{
		Connection connection = Connection::Memory();
		Execute(connection, "create table Things (Content real)");
		Statement statement(connection, "insert into Things values (?)");
		for (int i = 0; i != 1000000; ++i)
		{
			statement.Reset(i);
			statement.Execute();
		}		
		Execute(connection, "delete from Things where Content > 10");
		Execute(connection, "vacuum");
		SaveToDisk(connection, "./backup.db");
	}
	catch (Exception const & e)
	{
		printf("%s (%d)\n", e.Message.c_str(), e.Result);
	}
}

