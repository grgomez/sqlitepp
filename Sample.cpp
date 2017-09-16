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
		Execute(connection, "create table Hens(Name)");
		Execute(connection, "insert into Hens values (?)","Joe");
		printf("Inserted %lld\n", connection.RowId());
		Execute(connection, "insert into Hens values (?)","Martha");
		printf("Inserted %lld\n", connection.RowId());
		for (Row row : Statement(connection, "select RowId, Name from Hens"))
		{
			printf("(%d) %s\n", row.GetInt(0), row.GetString(1));
		}
		
	}
	catch (Exception const & e)
	{
		printf("%s (%d)\n", e.Message.c_str(), e.Result);
	}
}

