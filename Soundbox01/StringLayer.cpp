#include "StringLayer.h"
namespace tretton63
{
	std::wstring Prepend(std::wstring& String, std::wstring const& Prefix)
	{
		return Prefix + String;
	}
}