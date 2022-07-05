#pragma once
#include <string>
#include <vector>
#include "platform.h"

class Texture
{
public:
    //texture's individual descriptor:
    unsigned int GLid = 0; // GL texture id
    int size[2] = { 0,0 };  // image size
    std::string source; //file name
    //if renderable ?
    unsigned int frameBufferId = 0;
    unsigned int depthBufferId = 0;
    //end of descriptor

    //static array (vector) of all loaded textures
    static std::vector<Texture*> textures;
public:
    static int loadTexture(std::string filePath, unsigned int ckey=0, 
        int glRepeatH = GL_MIRRORED_REPEAT, int glRepeatV = GL_MIRRORED_REPEAT);
    static int applyCkey(unsigned char* imgData, int w, int h, unsigned int ckey);
    static int findTexture(std::string filePath);
    static int cleanUp();
    static unsigned int getGLid(int texN) { return textures.at(texN)->GLid; };
    static int saveBMP(std::string filePath, unsigned char* buff, int w, int h, int bytesPerPixel=4);
    static int saveTGA(std::string filePath, unsigned char* buff, int w, int h, int bytesPerPixel=4);
    static int generateTexture(std::string imgID, int w, int h, unsigned char* imgData=NULL,
        int glRepeatH = GL_MIRRORED_REPEAT, int glRepeatV = GL_MIRRORED_REPEAT);
    static int detachRenderBuffer(Texture* pTex);
    static int attachRenderBuffer(int texN, bool zBuffer = false) { return attachRenderBuffer(textures.at(texN), zBuffer); };
    static int attachRenderBuffer(Texture* pTex, bool zBuffer = false);
    static int setRenderToTexture(int texN) { return setRenderToTexture(textures.at(texN)); };
    static int setRenderToTexture(Texture* pTex);
    static int getImageFromTexture(int texN, unsigned char* imgData);
    static int blurRGBA(unsigned char* imgData, int w, int h, int blurLevel);
};

