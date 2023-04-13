#pragma once
#include <functional>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

template<typename T, typename D>
class Defer
{
	std::function<void()> m_runnable;
	bool m_has_executed;
	T m_t;
public:
	Defer(T t, std::function<D> Runnable) : m_runnable(Runnable), m_has_executed(false), m_t (t)
	{

	};
	~Defer()
	{
		if (!m_has_executed)
			m_runnable();
		m_has_executed = true;
		OutputDebugString(L"Defer destructor\n");
	}
	T Get() 
	{
		return m_t;
	}
	Defer(Defer&) = delete;
	Defer& operator=(Defer&) = delete;
};

class Foo
{
public:
	Foo()
	{
		OutputDebugStringW(L"Foo constructor\n");
	}

	~Foo()
	{
		OutputDebugStringW(L"Foo destructor\n");
	}
};