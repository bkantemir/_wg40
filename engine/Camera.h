#pragma once
#include "Coords.h"
#include "linmath.h"

class Camera
{
public:
	Coords ownCoords;
	mat4x4 lookAtMatrix;
	float lookAtPoint[3] = { 0,0,0 };
	float focusDistance = 100;
	float viewRangeDg = 30;
	float stageSize[2] = { 500, 375 };
public:
	float pickDistance() { return pickDistance(this); };
	static float pickDistance(Camera* pCam);
	void setCameraPosition() { setCameraPosition(this); };
	static void setCameraPosition(Camera* pCam);
	void buildLookAtMatrix() { buildLookAtMatrix(this); };
	static void buildLookAtMatrix(Camera* pCam);
	void onScreenResize() { onScreenResize(this); };
	static void onScreenResize(Camera* pCam);
};
