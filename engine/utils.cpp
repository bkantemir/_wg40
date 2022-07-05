#include "utils.h"
#include "platform.h"
#include <chrono>
#include <stdlib.h>     /* srand, rand */
#include <sys/stat.h> //if fileExists
#include <time.h> //for srand()

extern std::string filesRoot;
extern float radians2degrees;

int checkGLerrors(std::string ref) {
    //can be used after any GL call
    int res = glGetError();
    if (res == 0)
        return 0;
    std::string errCode;
    switch (res) {
        //case GL_NO_ERROR: errCode = "GL_NO_ERROR"; break;
        case GL_INVALID_ENUM: errCode = "GL_INVALID_ENUM"; break;
        case GL_INVALID_VALUE: errCode = "GL_INVALID_VALUE"; break;
        case GL_INVALID_OPERATION: errCode = "GL_INVALID_OPERATION"; break;
        case GL_INVALID_FRAMEBUFFER_OPERATION: errCode = "GL_INVALID_FRAMEBUFFER_OPERATION"; break;
        case GL_OUT_OF_MEMORY: errCode = "GL_OUT_OF_MEMORY"; break;
        default: errCode = "??"; break;
    }
    mylog("GL ERROR %d-%s in %s\n", res, errCode.c_str(), ref.c_str());
    return -1;
}
void mat4x4_mul_vec4plus(vec4 vOut, mat4x4 M, vec4 vIn, int v3) {
    vec4 v2;
    if (vOut == vIn) {
        memcpy(&v2, vIn, sizeof(vec4));
        vIn = v2;
    }
    vIn[3] = (float)v3;
    mat4x4_mul_vec4(vOut, M, vIn);
}
void v3set(float* vOut, float x, float y, float z) {
    vOut[0] = x;
    vOut[1] = y;
    vOut[2] = z;
}
void v3copy(float* vOut, float* vIn) {
    for (int i = 0; i < 3; i++)
        vOut[i] = vIn[i];
}
float v3yawRd(float* vIn) {
    return atan2f(vIn[0], vIn[2]);
}
float v3pitchRd(float* vIn) {
    return -atan2f(vIn[1], sqrtf(vIn[0] * vIn[0] + vIn[2] * vIn[2]));
}
float v3pitchDg(float* vIn) { 
    return v3pitchRd(vIn) * radians2degrees; 
}
float v3yawDg(float* vIn) { 
    return v3yawRd(vIn) * radians2degrees; 
}

long long int getSystemMillis() {
    auto currentTime = std::chrono::system_clock::now().time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(currentTime).count();
}
long long int getSystemNanos() {
    auto currentTime = std::chrono::system_clock::now().time_since_epoch();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(currentTime).count();
}
int randomCallN = 0;
int getRandom() {
    if (randomCallN % 1000 == 0)
        srand((unsigned int)getSystemNanos()); //re-initialize random seed:
    randomCallN++;
    return rand();
}
int getRandom(int fromN, int toN) {
    int randomN = getRandom();
    int range = toN - fromN + 1;
    return (fromN + randomN % range);
}
float getRandom(float fromN, float toN) {
    int randomN = getRandom();
    float range = toN - fromN;
    return (fromN + (float)randomN / RAND_MAX * range);
}
std::vector<std::string> splitString(std::string inString, std::string delimiter) {
    std::vector<std::string> outStrings;
    int delimiterSize = delimiter.size();
    outStrings.clear();
    while (1) {
        int delimiterPosition = inString.find(delimiter);
        if (delimiterPosition == std::string::npos) {
            //last element
            outStrings.push_back(trimString(inString));
            break;
        }
        std::string outString = inString.substr(0, delimiterPosition);
        outStrings.push_back(trimString(outString));
        int startAt = delimiterPosition + delimiterSize;
        inString = inString.substr(startAt, inString.size() - startAt);
    }
    return outStrings;
}
std::string trimString(std::string inString) {
    //Remove leading and trailing spaces
    int startsAt = inString.find_first_not_of(" ");
    if (startsAt == std::string::npos)
        return "";
    int endsAt = inString.find_last_not_of(" ") + 1;
    return inString.substr(startsAt, endsAt - startsAt);
}
bool fileExists(const char* filePath) {
    struct stat info;
    if (stat(filePath, &info) == 0)
        return true;
    else
        return false;
}
std::string getFullPath(std::string filePath) {
    if (filePath.find(filesRoot) == 0)
        return filePath;
    else
        return (filesRoot + filePath);
}
std::string getInAppPath(std::string filePath) {
    std::string inAppPath(filePath);
    if (inAppPath.find(filesRoot) == 0) {
        int startsAt = filesRoot.size();
        inAppPath = inAppPath.substr(startsAt, inAppPath.size() - startsAt);
    }
    if (inAppPath.find(".") != std::string::npos) {
        //cut off file name
        int endsAt = inAppPath.find_last_of("/");
        inAppPath = inAppPath.substr(0, endsAt + 1);
    }
    return inAppPath;
}
int makeDirs(std::string filePath) {
    filePath = getFullPath(filePath);
    std::string inAppPath = getInAppPath(filePath);
    std::vector<std::string> path = splitString(inAppPath, "/");
    int pathSize = path.size();
    filePath.assign(filesRoot);
    for (int i = 0; i < pathSize; i++) {
        filePath.append("/" + path.at(i));
        if (fileExists(filePath.c_str())) {
            continue;
        }
        //create dir
        myMkDir(filePath.c_str());
        mylog("Folder %d: %s created.\n", i, filePath.c_str());
    }
    return 1;
}
void v3inverse(float inV[]) {
    return v3inverse(inV, inV);
}
void v3inverse(float outV[], float inV[]) {
    for (int i = 0; i < 3; i++)
        outV[i] = -inV[i];
    return;
}

float v3dotProduct(float* a0, float* b0) {
    float a[3];
    float b[3];
    vec3_norm(a, a0);
    vec3_norm(b, b0);
    return v3dotProductNorm(a, b);
}
float v3dotProductNorm(float* a, float* b) {
    //assuming that vectors are normalized
    float dp = 0.0f;
    for (int i = 0; i < 3; i++)
        dp += a[i] * b[i];
    return dp;
}
float v3length(float* v) {
    float r = v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
    if (r == 0)
        return 0;
    return sqrtf(r);
}
float v3lengthXZ(float v[]) {
    float r = v[0] * v[0] + v[2] * v[2];
    if (r == 0)
        return 0;
    return sqrtf(r);
}
float v3lengthXY(float v[]) {
    float r = v[0] * v[0] + v[1] * v[1];
    if (r == 0)
        return 0;
    return sqrtf(r);
}
bool v3equals(float v[],float x) {
    for (int i = 0; i < 3; i++)
        if (v[i] != x)
            return false;
    return true;
}
bool v3match(float v0[], float v1[]) {
    for (int i = 0; i < 3; i++)
        if (v0[i] != v1[i])
            return false;
    return true;
}
void v3fromTo(float* v, float* v0, float* v1) {
    for (int i = 0; i < 3; i++)
        v[i] = v1[i] - v0[i];
}
float v3lengthFromTo(float* v0, float* v1) {
    float v[3];
    v3fromTo(v, v0, v1);
    float r = v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
    if (r == 0)
        return 0;
    return sqrtf(r);
}
float v3xyLengthFromTo(float* v0, float* v1) {
    float v[3];
    v3fromTo(v, v0, v1);
    float r = v[0] * v[0] + v[1] * v[1];
    if (r == 0)
        return 0;
    return sqrtf(r);
}
void v3dirFromTo(float* v, float* v0, float* v1) {
    v3fromTo(v, v0, v1);
    vec3_norm(v,v);
}
void mylog_v3(std::string vTitle, float* v) {
    mylog("%s%.2f:%.2f:%.2f", vTitle.c_str(), v[0], v[1], v[2]);
}
void v3setAll(float* vOut, float x) { 
    v3set(vOut, x, x, x);
}
void v3norm(float* v) {
    if (!v3equals(v, 0)) 
        vec3_norm(v, v); 
}
float angleDgNorm360(float a) {
    //returns angle in 0:360 range
    while (a < 0)
        a += 360;
    while (a > 360)
        a -= 360;
    return a;
}
float angleDgNorm180(float a) {
    //returns angle in -180:+180 range
    a = angleDgNorm360(a);
    if (a > 180)
        a -= 360;
    return a;
}
float angleDgFromTo(float a0, float a2) {
    float a = angleDgNorm180(a2 - a0);
    return a;
}
