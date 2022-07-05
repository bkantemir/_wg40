#pragma once
#include <string>

class TexCoords
{
public:
	float tuvLeftTop[2] = { 0.0f,0.0f };
	float tuvRange[2] = { 1.0f,1.0f };
	char flipStr[4] = ""; //possible flips: "90" (CCW), "-90" (CW), "180", "h" (horizontal), "v" (vertical) 
public:
	void set(int texN, float x, float y, float w, float h, std::string flip) { set(this, texN, x, y, w, h, flip); };
	static void set(TexCoords* pTC, int texN, float x, float y, float w, float h, std::string flipStr);
	static void set_GL(TexCoords* pTC, float x, float y, float w, float h, std::string flipStr);
	static void setTUV(TexCoords* pTC, float* pDstTUV, float xRate, float yRate);
};
