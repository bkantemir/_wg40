#pragma once

typedef uint32_t myUint64;
typedef uint32_t myUint32;
typedef uint8_t  myUint16;
typedef uint8_t  myUint8;

void mylog(const char* _Format, ...);
void mySwapBuffers();
void myPollEvents();
int myFopen_s(FILE** pFile, const char* filePath, const char* mode);
int myMkDir(const char* outPath);
void myStrcpy_s(char* dst, int maxSize, const char* src);

