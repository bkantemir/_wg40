#pragma once
#include "FileLoader.h"

class XMLparser : public FileLoader
{
public:
	char* readFrom;
	std::string currentTag = "";
	int tagLength = 0;
	std::string tagName = "";
	bool closedTag = false; // > or />
public:
	XMLparser(std::string filePath) : FileLoader(filePath) { removeComments(this); readFrom = pData; };
	static int removeComments(XMLparser* pXP);
	static int processSource(XMLparser* pXP);
	int nextTag() { return nextTag(this); }; //returns 0 if no more tags, 1 - tag extractedb
	static bool nextTag(XMLparser* pXP); //returns 0 if no more tags, 1 - tag extractedb
	static int nameEndsAt(std::string varName, std::string tag);
	virtual int processTag() { return 1; };
	static std::string buildFullPath(XMLparser* pXP, std::string filePath);

	static bool varExists(std::string varName, std::string tag);
	static std::string getStringValue(std::string varName, std::string tag);
	static int setCharsValue(char* pChars, int charsLength, std::string varName, std::string tag);
	static int setIntValue(int* pInt, std::string varName, std::string tag);
	static int getIntValue(std::string varName, std::string tag);
	static unsigned int getUintValue(std::string varName, std::string tag);
	static int setFloatValue(float* pFloat, std::string varName, std::string tag);
	static float getFloatValue(std::string varName, std::string tag);
	static int setIntArray(int* pInts, int arrayLength, std::string varName, std::string tag);
	static int setFloatArray(float* pFloats, int arrayLength, std::string varName, std::string tag);
	static int setUintColorValue(unsigned int* pInt, std::string varName, std::string tag);
	static int setIntBoolValue(int* pInt, std::string varName, std::string tag);
	static int setGlRepeatValue(int* pInt, std::string varName, std::string tagStr);

};
