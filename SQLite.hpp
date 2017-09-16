#pragma once

#include "Handle.hpp"
#include "sqlite3.h"
#include <string>

/*
 * Asserting in Linux and Windows is very different.
 * Consider using the pre-processor to change the definitions
 * depending OS.
 * TODO
 */ 
#ifdef NDEBUG
#define assert(condition) ((void)0)
#else
#define assert(condition) /*implementation defined*/
#endif

/*
 * TODO
 *
 * Add macros to disable UTF-16 (wide character) functionality
 *
 * DISABLE_UTF_16_FUNC
 *
 * This may be unnecessary, but it could also cut down on
 * code that is not needed when the kernel/OS doesn't
 * the character encoding
 */
 
struct Exception
{
	int Result = 0;
	std::string Message;

	explicit Exception(sqlite3 *  const connection) :
		Result(sqlite3_extended_errcode(connection)),
		Message(sqlite3_errmsg(connection))
	{}
};

class Connection
{
	struct ConnectionHandleTraits : HandleTraits<sqlite3 *>
	{
		static void Close(Type value) noexcept
		{
			assert(SQLITE_OK ==  sqlite3_close(value));
		}
	};


	using ConnectionHandle = Handle<ConnectionHandleTraits>;
	ConnectionHandle m_handle;

	template <typename F, typename C>
	void InternalOpen(F open, C const * const filename)
	{
		Connection temp;
		
		if (SQLITE_OK != open(filename, temp.m_handle.Set()))
		{
			temp.ThrowLastError();
		}

		swap(m_handle, temp.m_handle);
	}

	public:
	
		Connection() noexcept = default;
		
		template <typename C>
		explicit Connection(C const * const filename)
		{
			Open(filename);
		}
		
		static Connection Memory()
		{
			return Connection(":memory:");
		}		

		static Connection WideMemory()
		{
			return Connection(L":memory:");
		}

		explicit operator bool() const noexcept
		{
			return static_cast<bool>(m_handle);
		}

		/*
 		 * ABI -> Application Binary Interface
		 */
		 sqlite3 * GetAbi() const noexcept
		 {
		 	return m_handle.Get();
		 }
	         
		/*
 		 * TODO 
 		 * this function is used in Microsoft to avoid conflicts with future extensions 	
		 * __declspec(noreturn) void ThrowLastError() const
		 * {
		 *	throw Exception(GetAbi());
		 * }
		 *
		 * If this library is to be used in Windows, comment out the above.
		 * In fact consider making a set of macros to make sure that if the library is 
		 * being built in Windows to enable the proper function.
		 */
	        [[ noreturn ]] void ThrowLastError() const
		{
			throw Exception(GetAbi());
		}

		void Open(char const * const filename)
		{
			InternalOpen(sqlite3_open, filename);			
		}
		
		/*
 		 * TODO 
 		 * I keep this Open function here for good practice.
 		 * The Linux kernel supports UTF-8 encoding for 
 		 * the filesystem thus there is no much use for it here
 		 * But Windows systems do support it so here it is.
 		 */ 
		void Open(wchar_t const * const filename)
		{
			InternalOpen(sqlite3_open16, filename);
		}	
};

template <typename T>
struct Reader
{
	int GetInt(int const column = 0) const noexcept
	{
		return sqlite3_column_int(static_cast<T const *>(this)->GetAbi(), column);
	}
	
	char const * GetString(int const column = 0) const noexcept
	{
		return reinterpret_cast<char const *>(
				sqlite3_column_text(
					static_cast<T const *>(this)->GetAbi(),
					column));
	}

	wchar_t const * GetWideString(int const column = 0) const noexcept
	{
		return static_cast<wchar_t const *>(
				sqlite3_column_text16(
					static_cast<T const *>(this)->GetAbi(),
					column));
	}

	int GetStringLength(int const column = 0) const noexcept
	{
		return sqlite3_column_bytes(static_cast<T const *>(this)->GetAbi(), column);
	}

	int GetWideStringLength(int const column = 0) const noexcept
	{
		return sqlite3_column_bytes16(static_cast<T const *>(this)->GetAbi(), column) / sizeof(wchar_t);
	}
}; 

class Row : public Reader<Row>
{
	sqlite3_stmt * m_statement = nullptr;

	public:
		sqlite3_stmt * GetAbi() const noexcept
		{
			return m_statement;
		}

		Row(sqlite3_stmt * const statement) noexcept :
			m_statement(statement)
		{}
};

class Statement : public Reader<Statement>
{
	struct StatementHandleTraits : HandleTraits<sqlite3_stmt *>
	{
		static void Close(Type value) noexcept
		{
			assert(SQLITE_OK == sqlite3_finalize(value));
		}	
	};

	using StatementHandle = Handle<StatementHandleTraits>;
	StatementHandle m_handle;

	template <typename F, typename C, typename ... Values>
	void InternalPrepare(Connection const & connection, F prepare, C const * const text, Values && ... values)
	{
		assert(connection);

		if (SQLITE_OK != prepare(connection.GetAbi(), text, -1, m_handle.Set(), nullptr))
		{
			connection.ThrowLastError();
		}

		BindAll(std::forward<Values>(values) ...);
	}

	void InternalBind(int) const noexcept
	{}

	template <typename First, typename ... Rest>
	void InternalBind(int const index, First && first, Rest && ... rest) const
	{
		Bind(index, std::forward<First>(first));
		InternalBind(index + 1, std::forward<Rest>(rest) ...);
	}

	public:
		
		Statement() noexcept = default;

		template <typename C, typename ... Values>
		Statement(Connection const & connection, C const * const text, Values && ... values)
		{
			Prepare(connection, text, std::forward<Values>(values) ...);
		}

		explicit operator bool() const noexcept
		{
			return static_cast<bool>(m_handle);
		}

		sqlite3_stmt * GetAbi() const noexcept
		{
			return m_handle.Get();
		}

		/*
 		 * TODO
 		 *
 		 * __declspec(noreturn) void ThrowLastError() const
 		 * {
 		 * 	throw Exception(sqlite3_db_handle(GetAbi()));
 		 * }
 		 *
 		 */
		[[ noreturn ]] void ThrowLastError() const
		{
			throw Exception(sqlite3_db_handle(GetAbi()));
		}   
		
		template <typename ... Values>
		void Prepare(Connection const & connection, char const * const text, Values && ... values)
		{
			InternalPrepare(connection, sqlite3_prepare_v2, text, std::forward<Values>(values) ...);
		}

		template <typename ... Values>
		void Prepare(Connection const & connection, wchar_t const * const text, Values && ... values)
		{
			InternalPrepare(connection, sqlite3_prepare16_v2, text, std::forward<Values>(values) ...);
		}

		bool Step() const
		{
			int const result = sqlite3_step(GetAbi());
			
			if (result == SQLITE_ROW) return true;
			if (result == SQLITE_DONE) return false;
			
			/*
 			 * Otherwise throw an error.
 			 * It is possible to have other recoverable states.
 			 * For instance, in a multi-threaded application
 			 * a busy state can be returned.
 			 * This case and others are app-specific and must
 			 * be considered as improvements to this library on a
 			 * per-app basis.
 			 */
			ThrowLastError();  
		}

		void Execute() const
		{
			Step();
		}

		void Bind(int const index, int const value) const
		{
			if(SQLITE_OK != sqlite3_bind_int(GetAbi(), index, value))
			{
				ThrowLastError();
			}
		}
		
		void Bind(int const index, char const * const value, int const size = -1) const
		{
			if (SQLITE_OK != sqlite3_bind_text(GetAbi(), 
							   index, 
							   value, 
							   size, 
							   SQLITE_STATIC))
			{
				ThrowLastError();
			}
		}


		void Bind(int const index, wchar_t const * const value, int const size = -1) const
		{
			if (SQLITE_OK != sqlite3_bind_text16(GetAbi(), 
							     index, 
 							     value, 
						 	     size, 
							     SQLITE_STATIC))
			{
				ThrowLastError();
			}
		}

		void Bind(int const index, std::string const & value) const
		{
	 		Bind(index, value.c_str(), value.size());
		}

		void Bind(int const index, std::wstring const & value) const
		{
			Bind(index, value.c_str(), value.size() * sizeof(wchar_t));
		}

		void Bind(int const index, std::string && value) const
		{
			if (SQLITE_OK != sqlite3_bind_text(GetAbi(),
							   index,
							   value.c_str(),
							   value.size(),
							   SQLITE_TRANSIENT))
			{
				ThrowLastError();
			}
		}

		void Bind(int const index, std::wstring && value) const
		{
			if (SQLITE_OK != sqlite3_bind_text16(GetAbi(), 
							     index,
							     value.c_str(),
							     value.size() * sizeof(wchar_t),
							     SQLITE_TRANSIENT))
			{
				ThrowLastError();
			}
		}
	
		template <typename ... Values>
		void BindAll(Values && ... values) const
		{
			InternalBind(1, std::forward<Values>(values) ...);
		}
};

class RowIterator
{
	Statement const * m_statement = nullptr;

	public:
		RowIterator() noexcept = default;
		RowIterator(Statement const & statement) noexcept
		{
			if (statement.Step())
			{
				m_statement = &statement;
			}
		}

		RowIterator & operator++() noexcept
		{
			if (!m_statement->Step())
			{
				m_statement = nullptr;
			}

			return *this;
		}

		bool operator !=(RowIterator const & other) const noexcept
		{
			return m_statement != other.m_statement;
		}

		bool operator ==(RowIterator const & other) const noexcept
		{
			return m_statement == other.m_statement;
		}

		Row operator *() const noexcept
		{
			return Row(m_statement->GetAbi());
		}
};

inline RowIterator begin(Statement const & statement) noexcept
{
	return RowIterator(statement);
}

inline RowIterator end(Statement const &) noexcept
{
	return RowIterator();
}

template <typename C, typename ... Values>
void Execute(Connection const & connection, C const * const text, Values && ... values)
{
	Statement(connection, text, std::forward<Values>(values) ...).Execute();
}
