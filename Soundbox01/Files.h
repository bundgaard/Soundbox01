#pragma once
// TODO should separate all this into one file.
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <vector>
#include <string>
std::vector<std::wstring> ListMusicFiles(std::wstring const& Folder);