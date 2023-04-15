#include "Files.h"
namespace tretton63
{
	// TODO: should move it into a thread to not block UI if the folder is too big.
	std::vector<std::wstring>
		ReadFilesIntoList(std::wstring const& Path)
	{
		WIN32_FIND_DATAW FindBlock{};
		std::vector<std::wstring> Result{};
		std::wstring Foo{};
		Foo += L"\\\\?\\";
		Foo += Path.c_str();
		Foo += L"\\*";

		HANDLE MyFile = FindFirstFileW(Foo.c_str(), &FindBlock);
		if (MyFile == nullptr)
		{
			DWORD dwError = GetLastError();
			wchar_t Buf[64] = { 0 };
			wsprintf(Buf, L"Error %x\n", dwError);
			OutputDebugString(Buf);
		}
		else
		{
			do {
				std::wstring Filename{ FindBlock.cFileName };
				if (Filename.ends_with(L".wav\0"))
				{
					OutputDebugStringW(Filename.c_str());
					std::wstring NewPath{};
					NewPath += Path.substr(0,Path.size()-1);
					NewPath += L"\\";
					NewPath += Filename;
					Result.push_back(NewPath);
					OutputDebugStringW(NewPath.c_str());
					OutputDebugStringW(L"\n");
				}
			} while (FindNextFileW(MyFile, &FindBlock));
		}
		return Result;
	}
}