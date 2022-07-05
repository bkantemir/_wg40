#pragma once
#include <string>

class FileLoader
{
public:
	std::string fullPath;
	std::string inAppFolder;
	char* pData = NULL;
	int dataSize = 0;
public:
	FileLoader(std::string filePath, std::string readMode="rt");
	virtual ~FileLoader();
	static std::string getFullPath(std::string filePath);
	static int translatePath(FileLoader* pFL, std::string filePath);
	static int loadFile(FileLoader* pFL, std::string filePath, std::string readMode="rb");
};
