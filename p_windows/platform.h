#pragma once
#include <glad/glad.h>
#include <stdio.h>

typedef unsigned _int64 myUint64;
typedef unsigned _int32 myUint32;
typedef unsigned _int16 myUint16;
typedef unsigned _int8  myUint8;

void mylog(const char* _Format, ...);
void mySwapBuffers();
void myPollEvents();
int myFopen_s(FILE** pFile, const char* filePath, const char* mode);
int myMkDir(const char* outPath);
void myStrcpy_s(char* dst, int maxSize, const char* src);
