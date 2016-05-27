// fbc_cv is free software and uses the same licence as OpenCV
// Email: fengbingchun@163.com

#include <windows.h>
#include "directory.hpp"

// reference: contrib/src/inputoutput.cpp (2.4.9)

namespace fbc{

	std::vector<std::string> Directory::GetListFiles(const std::string& path, const std::string & exten, bool addPath)
	{
		std::vector<std::string> list;
		list.clear();
		std::string path_f = path + "/" + exten;
		WIN32_FIND_DATAA FindFileData;
		HANDLE hFind;

		hFind = FindFirstFileA((LPCSTR)path_f.c_str(), &FindFileData);
		if (hFind == INVALID_HANDLE_VALUE) {
			return list;
		} else {
			do {
				if (FindFileData.dwFileAttributes == FILE_ATTRIBUTE_NORMAL ||
					FindFileData.dwFileAttributes == FILE_ATTRIBUTE_ARCHIVE ||
					FindFileData.dwFileAttributes == FILE_ATTRIBUTE_HIDDEN ||
					FindFileData.dwFileAttributes == FILE_ATTRIBUTE_SYSTEM ||
					FindFileData.dwFileAttributes == FILE_ATTRIBUTE_READONLY) {
					char* fname;
					fname = FindFileData.cFileName;

					if (addPath) {
						list.push_back(path + "/" + std::string(fname));
					} else {
						list.push_back(std::string(fname));
					}
				}
			} while (FindNextFileA(hFind, &FindFileData));

			FindClose(hFind);
		}

		return list;
	}

	std::vector<std::string> Directory::GetListFilesR(const std::string& path, const std::string & exten, bool addPath)
	{
		std::vector<std::string> list = Directory::GetListFiles(path, exten, addPath);
		std::vector<std::string> dirs = Directory::GetListFolders(path, exten, addPath);

		std::vector<std::string>::const_iterator it;
		for (it = dirs.begin(); it != dirs.end(); ++it) {
			std::vector<std::string> cl = Directory::GetListFiles(*it, exten, addPath);
			list.insert(list.end(), cl.begin(), cl.end());
		}

		return list;
	}

	std::vector<std::string> Directory::GetListFolders(const std::string& path, const std::string & exten, bool addPath)
	{
		std::vector<std::string> list;
		std::string path_f = path + "/" + exten;
		list.clear();

		WIN32_FIND_DATAA FindFileData;
		HANDLE hFind;

		hFind = FindFirstFileA((LPCSTR)path_f.c_str(), &FindFileData);
		if (hFind == INVALID_HANDLE_VALUE) {
			return list;
		} else {
			do {
				if (FindFileData.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY &&
					strcmp(FindFileData.cFileName, ".") != 0 &&
					strcmp(FindFileData.cFileName, "..") != 0) {
					char* fname;
					fname = FindFileData.cFileName;

					if (addPath) {
						list.push_back(path + "/" + std::string(fname));
					} else {
						list.push_back(std::string(fname));
					}
				}
			} while (FindNextFileA(hFind, &FindFileData));

			FindClose(hFind);
		}

		return list;
	}
}

