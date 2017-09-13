#pragma once

#ifdef NDEBUG
#define assert(condition) ((void)0)
#else
#define assert(condition) /*implementation defined*/
#endif

template <typename T>
struct HandleTraits
{
	using Type = T;
	
	static Type Invalid() noexcept
	{
		return nullptr;
	}

	// static void Close(Type value) noexcept
};

template <typename Traits>
class Handle
{
	using Type = decltype(Traits::Invalid());
	Type m_value;	
	
	void Close() noexcept
	{
		if (*this)
		{
			Traits::Close(m_value);
		}
	}

public:

	Handle(Handle const &) = delete;
	Handle & operator=(Handle const &) = delete;
	
	explicit Handle(Type value =  Traits::Invalid()) noexcept :
		m_value(value)
	{}

	Handle(Handle && other) noexcept :
		m_value(other.Detach())
	{}

	Handle & operator=(Handle && other) noexcept
	{
		if (this != &other)
		{
			Reset(other.Detach());
		}

		return *this;
	}

	~Handle() noexcept
	{
		Close();
	}

	explicit operator bool() const noexcept
	{
		return m_value != Traits::Invalid();
	}

	Type Get() const noexcept
	{
		return m_value;
	}

	Type * Set() noexcept
	{
		assert(!*this);
		return &m_value;
	}
	
	Type Detach() noexcept
	{
		Type value = m_value;
		m_value = Traits::Invalid();
		return value;
	}

	bool Reset(Type value = Traits::Invalid()) noexcept
	{
		if (m_value != value)
		{
			Close();
			m_value = value;
		}
		
		return static_cast<bool>(*this);
	}

	void Swap(Handle<Traits> & other) noexcept
	{
		Type temp = m_value;
		m_value = other.m_value;
		other.m_value = temp;
	}
};

template <typename Traits>
void swap(Handle<Traits> & left, Handle<Traits> & right) noexcept
{
	left.Swap(right);
}

template <typename Traits>
bool operator==(Handle<Traits> const & left, Handle<Traits> const & right) noexcept
{
	return left.Get() == right.Get();
}

template <typename Traits>
bool operator !=(Handle<Traits> const & left, Handle<Traits> const & right) noexcept
{
	return !(left == right);
}
