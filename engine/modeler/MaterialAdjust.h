#pragma once
#include "Material.h"
#include <vector>

class MaterialAdjust : public Material
{
public:
	char adjustmentName[32] = ""; //"clearcoat", etc
	bool b_shaderType = false;
	bool b_primitiveType = false;
	bool b_uColor = false;
	bool b_uColor1 = false;
	bool b_uColor2 = false;
	bool b_uTex0 = false;
	bool b_uTex1mask = false;
	bool b_uTex2nm = false;
	bool b_uTex3 = false;
	bool b_uTex1alphaChannelN = false;
	bool b_uTex1alphaNegative = false;
	bool b_uTex0translateChannelN = false;

	bool b_uAlphaBlending = false;
	bool b_uAlphaFactor = false;
	bool b_uAmbient = false;
	//specular light parameters
	bool b_uSpecularIntencity = false;
	bool b_uSpecularMinDot = false;
	bool b_uSpecularPowerOf = false;

	bool b_lineWidth = false;

	bool b_layer2as = false;

	bool b_dropsShadow = false;
	bool b_uBleach = false;

	static std::vector<MaterialAdjust*> materialAdjustsList;

public:
	static int cleanUp();
	static int adjust(Material* pMT, MaterialAdjust* pMA);
	static int setWhat2adjust(MaterialAdjust* pMA, std::string tagStr);
	static MaterialAdjust* findMaterialAdjust(const char adjustmentName[], std::vector<MaterialAdjust*>* pList);
};

