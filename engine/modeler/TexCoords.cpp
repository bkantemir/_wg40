#include "TexCoords.h"
#include "Texture.h"

void TexCoords::set(TexCoords* pTC, int texN, float x, float y, float w, float h, std::string flipStr) {
	if (texN < 0)
		return;
	Texture* pTex = Texture::textures.at(texN);
	x = x / pTex->size[0];
	y = y / pTex->size[1];
	w = w / pTex->size[0];
	h = h / pTex->size[1];
	set_GL(pTC, x, y, w, h, flipStr);
}
void TexCoords::set_GL(TexCoords* pTC, float x, float y, float w, float h, std::string flipStr) {
	//assuming that x,y,w,h - in GL 0-to-1 format
	float tuvRightBottom[2];
	pTC->tuvLeftTop[0] = x;
	pTC->tuvLeftTop[1] = y;
	tuvRightBottom[0] = x + w;
	tuvRightBottom[1] = y + h;
	for (int i = 0; i < 2; i++)
		pTC->tuvRange[i] = tuvRightBottom[i] - pTC->tuvLeftTop[i];
	myStrcpy_s(pTC->flipStr, 4, flipStr.c_str()); 
}
void TexCoords::setTUV(TexCoords* pTC, float* pDstTUV, float ratioX, float ratioY){
	//possible flips: "90" (CCW), "-90" (CW), "180", "h" (horizontal), "v" (vertical) 
	if (pTC == NULL)
		return;
	float rx; //out ratio
	float ry;
	if (strlen(pTC->flipStr)==0) {
		//straight
		rx = ratioX;
		ry = ratioY;
	}
	else if (strstr(pTC->flipStr,"h") == pTC->flipStr) {
		rx = 1.0f - ratioX;
		ry = ratioY;
	}
	else if (strstr(pTC->flipStr, "v") == pTC->flipStr) {
		rx = ratioX;
		ry = 1.0f - ratioY;
	}
	else if (strstr(pTC->flipStr, "-90") == pTC->flipStr) {
		rx = ratioY;
		ry = 1.0f - ratioX;
	}
	else if (strstr(pTC->flipStr, "90") == pTC->flipStr) {
		rx = 1.0f - ratioY;
		ry = ratioX;
	}
	else if (strstr(pTC->flipStr, "180") == pTC->flipStr) {
		rx = 1.0f - ratioX;
		ry = 1.0f - ratioY;
	}
	else {
		//straight
		rx = ratioX;
		ry = ratioY;
	}
	pDstTUV[0] = pTC->tuvLeftTop[0] + pTC->tuvRange[0] * rx;
	pDstTUV[1] = pTC->tuvLeftTop[1] + pTC->tuvRange[1] * ry;
}

