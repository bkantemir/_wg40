#pragma once
#include "platform.h"

class VirtualShape
{
public:
	char shapeType[32] = "box";
	float whl[3] = { 0,0,0 }; //width/height/length (x,y,z sizes/dimensions)
	int sections[3] = { 1,1,1 }; //number of sections for each axis
	int sectionsR = 1; //number of radial sections
	//side extensions
	float extU = 0; //up
	float extD = 0; //down
	float extL = 0; //left
	float extR = 0; //right
	float extF = 0; //front
	float extB = 0; //back
	float angleFromTo[2] = { -180, 180 };
	char side[16] = "";

public:
	void setShapeType(std::string needType) { setShapeType(this, needType); };
	static void setShapeType(VirtualShape* pVS, std::string needType) { myStrcpy_s(pVS->shapeType, 32, (char*)needType.c_str()); };
	void setExt(float v) { setExt(this, v); };
	void setExtX(float v) { setExtX(this, v); };
	void setExtY(float v) { setExtY(this, v); };
	void setExtZ(float v) { setExtZ(this, v); };
	static void setExt(VirtualShape* pVS, float v) { pVS->extU = v; pVS->extD = v; pVS->extL = v; pVS->extR = v; pVS->extF = v; pVS->extB = v; };
	static void setExtX(VirtualShape* pVS, float v) { pVS->extL = v; pVS->extR = v; };
	static void setExtY(VirtualShape* pVS, float v) { pVS->extU = v; pVS->extD = v; };
	static void setExtZ(VirtualShape* pVS, float v) { pVS->extF = v; pVS->extB = v; };
};
