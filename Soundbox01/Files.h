#pragma once
// TODO should separate all this into one file.
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <vector>
#include <string>
namespace tretton63
{
	std::vector<std::wstring> ReadFilesIntoList(std::wstring const& Path);
}
