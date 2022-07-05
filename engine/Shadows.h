#pragma once
#include "Camera.h"
#include "GameSubj.h"
#include "Texture.h"

class Shadows
{
public:
	static float shadowLight;

	static int depthMapTexN;
	static Texture* pDepthMap;

	static Camera shadowCamera;
	static mat4x4 mViewProjection;
	static float sizeUnitPixelsSize;

	static float uDepthBias[16]; //z-value shifts depeding on normal
public:
	static int init();
	static int renderDepthMap(std::vector<GameSubj*>* pGSubjs);
};


