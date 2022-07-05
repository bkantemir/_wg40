#pragma once
#include "linmath.h"

class Coords
{
public:
	float pos[4] = { 0,0,0,0 }; //x,y,z position + 4-th element for compatibility with 3D 4x4 matrices math
	float eulerDg[3] = { 0,0,0 }; //Euler angles (pitch, yaw, roll) in degrees
	mat4x4 rotationMatrix = { 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 };
public:
	static void getPositionFromMatrix(float* pos, mat4x4 m);
	static void eulerDgToMatrix(mat4x4 rotationMatrix, float* eulerDg);
	static void matrixToEulerDg(float* eulerDg, mat4x4 m);
};

