#include "Files.h"

std::vector<std::wstring> ListMusicFiles(std::wstring const& Folder)
{
	std::vector<std::wstring> Files;
	WIN32_FIND_DATAW Block{};
	std::wstring Extension = Folder;
	Extension += L"\\*.wav";

	HANDLE FindHandle = FindFirstFileW(Extension.c_str(), &Block);
	// TODO(david): add error checking
	if (FindHandle)
	{
		do {
			Files.push_back(Block.cFileName);
		} while (FindNextFileW(FindHandle, &Block));
		FindClose(FindHandle);
	}

	return Files;
}