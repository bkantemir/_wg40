#include "FileLoader.h"
#include "platform.h"

extern std::string filesRoot;

FileLoader::FileLoader(std::string filePath, std::string readMode) {
    loadFile(this, filePath, readMode);
}
FileLoader::~FileLoader() {
    if (pData != NULL) {
        delete[] pData;
        pData = NULL;
    }
}
std::string FileLoader::getFullPath(std::string filePath) {
    if (filePath.find(filesRoot) == 0)
        return filePath;
    else
        return (filesRoot + filePath);
}
int FileLoader::translatePath(FileLoader* pFL, std::string filePath) {
    pFL->fullPath = getFullPath(filePath);
    int startPos = filesRoot.size();
    int lastSlashPos = pFL->fullPath.find_last_of('/');
    pFL->inAppFolder = pFL->fullPath.substr(startPos, lastSlashPos - startPos + 1);
    return 1;
}
int FileLoader::loadFile(FileLoader* pFL, std::string filePath, std::string readMode) {
    translatePath(pFL, filePath);
    FILE* pFile;
    myFopen_s(&pFile, pFL->fullPath.c_str(), readMode.c_str());
    if (pFile != NULL)
    {
        // obtain file size:
        fseek(pFile, 0, SEEK_END);
        pFL->dataSize = ftell(pFile);
        rewind(pFile);
        // size obtained, create buffer
        pFL->pData = new char[pFL->dataSize + 1];
        pFL->dataSize = fread(pFL->pData, 1, pFL->dataSize, pFile);
        pFL->pData[pFL->dataSize] = 0;
        fclose(pFile);
    }
    else {
        mylog("ERROR loading %s\n", pFL->fullPath.c_str());
        return -1;
    }
    return 1;
}


