#include "MyColor.h"

std::vector<MyColor*> MyColor::colorsList;
float chanIntToFloat = 1.0f / 255;

int MyColor::cleanUp() {
	int itemsN = colorsList.size();
	//delete all colors
	for (int i = 0; i < itemsN; i++)
		delete colorsList.at(i);
	colorsList.clear();
	return 1;
}
void MyColor::cloneIntToFloat(MyColor* pCl) {
	for (int i = 0; i < 4; i++)
		pCl->rgba[i] = chanIntToFloat * pCl->RGBA.channel[i];
}
void MyColor::cloneFloatToInt(MyColor* pCl) {
	for (int i = 0; i < 4; i++)
		pCl->RGBA.channel[i] = (int)(pCl->rgba[i] * 255);
}
void MyColor::setRGBA(MyColor* pCl, float* rgba) {
	for (int i = 0; i < 4; i++)
		pCl->rgba[i] = rgba[i];
	cloneFloatToInt(pCl);
}
void MyColor::setRGBA(MyColor* pCl, float r, float g, float b, float a) {
	pCl->rgba[0] = r;
	pCl->rgba[1] = g;
	pCl->rgba[2] = b;
	pCl->rgba[3] = a;
	cloneFloatToInt(pCl);
}
void MyColor::setRGBA(MyColor* pCl, int R, int G, int B, int A) {
	pCl->RGBA.channel[0] = R;
	pCl->RGBA.channel[1] = G;
	pCl->RGBA.channel[2] = B;
	pCl->RGBA.channel[3] = A;
	cloneIntToFloat(pCl);
}
void MyColor::setRGBA(MyColor* pCl, unsigned char* RGBA) {
	for (int i = 0; i < 4; i++)
		pCl->RGBA.channel[i] = RGBA[i];
	cloneIntToFloat(pCl);
}
void MyColor::setUint32(MyColor* pCl, unsigned int RGBA) {
	pCl->RGBA.uint32value = RGBA;
	cloneIntToFloat(pCl);
}
MyColor* MyColor::findColor(const char colorName[], std::vector<MyColor*>* pList) {
	if (pList != NULL) {
		for (int maN = pList->size() - 1; maN >= 0; maN--) {
			MyColor* pCl = pList->at(maN);
			if (strcmp(pCl->colorName, colorName) == 0)
				return pCl;
		}
	}
	if (pList != &MyColor::colorsList) {
		pList = &MyColor::colorsList;
		for (int maN = pList->size() - 1; maN >= 0; maN--) {
			MyColor* pCl = pList->at(maN);
			if (strcmp(pCl->colorName, colorName) == 0)
				return pCl;
		}
	}
	return NULL;
}