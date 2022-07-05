#include "ModelLoader.h"
#include "platform.h"
#include "TheGame.h"

#include "DrawJob.h"
#include "Texture.h"
#include "utils.h"
#include "Polygon.h"
#include <algorithm>

extern TheGame theGame;
extern float degrees2radians;

int ModelLoader::loadModel(std::vector<GameSubj*>* pSubjsVector0, std::string sourceFile, std::string subjClass) {
	//returns element's (Subj) number or -1
	//first - check if already loaded
	int totalSubjs = pSubjsVector0->size();
	for (int subjN0 = totalSubjs - 1; subjN0 >= 0; subjN0--) {
		GameSubj* pGS0 = pSubjsVector0->at(subjN0);
		if (pGS0 == NULL)
			continue;
		if (strcmp(pGS0->source, sourceFile.c_str()) != 0)
			continue;
		//if here - model was already loaded - copy
		int subjN = pSubjsVector0->size();
		for (int i = 0; i < pGS0->totalNativeElements; i++) {
			GameSubj* pGS = pSubjsVector0->at(subjN0 + i)->clone();
			pGS->nInSubjsSet = pSubjsVector0->size();
			pSubjsVector0->push_back(pGS);
		}
		GameSubj* pGS = pSubjsVector0->at(subjN);
		pGS->totalElements = pGS->totalNativeElements;
		pGS->d2parent = 0;
		pGS->d2headTo = 0;
		//restore original 1st DrawJob
		return subjN;
	}
	//if here - model wasn't loaded before - load
	int subjN = pSubjsVector0->size();
	GameSubj* pGS = theGame.newGameSubj(subjClass);
	pGS->pSubjsSet = pSubjsVector0;
	pGS->nInSubjsSet = subjN;
	myStrcpy_s(pGS->source, 256, sourceFile.c_str());
	pSubjsVector0->push_back(pGS);
	ModelLoader* pML = new ModelLoader(pSubjsVector0, subjN, NULL, sourceFile);
	processSource(pML);
	delete pML;
	pGS->totalNativeElements = pSubjsVector0->size() - subjN;
	pGS->totalElements = pGS->totalNativeElements;
	return subjN;
}

int ModelLoader::setIntValueFromHashMap(int* pInt, std::map<std::string, float> floatsHashMap, std::string varName, std::string tagStr) {
	if (!varExists(varName, tagStr))
		return 0;
	std::string str0 = getStringValue(varName, tagStr);
	if (floatsHashMap.find(str0) == floatsHashMap.end()) {
		mylog("ERROR in ModelLoader::setIntValueFromHashMap, %s not found, %s\n", varName.c_str(), tagStr.c_str());
		return -1;
	}
	*pInt = (int)floatsHashMap[str0];
	return 1;
}


int ModelLoader::setFloatValueFromHashMap(float* pFloat, std::map<std::string, float> floatsHashMap, std::string varName, std::string tagStr) {
	if (!varExists(varName, tagStr))
		return 0;
	std::string str0 = getStringValue(varName, tagStr);
	if (floatsHashMap.find(str0) == floatsHashMap.end()) {
		mylog("ERROR in ModelLoader::setIntValueFromHashMap, %s not found, %s\n", varName.c_str(), tagStr.c_str());
		return -1;
	}
	*pFloat = floatsHashMap[str0];
	return 1;
}
int ModelLoader::setTexture(ModelLoader* pML, int* pInt, std::string txName) {
	ModelBuilder* pMB = pML->pModelBuilder;
	std::string varName = txName + "_use";
	if (varExists(varName, pML->currentTag)) {
		if (setIntValueFromHashMap(pInt, pMB->floatsHashMap, varName, pML->currentTag) == 0) {
			mylog("ERROR in ModelLoader::setTexture: texture not in hashMap: %s\n", pML->currentTag.c_str());
			return -1;
		}
		return 1;
	}
	varName = txName + "_src";
	if (varExists(varName, pML->currentTag)) {
		std::string txFile = getStringValue(varName, pML->currentTag);
		varName = txName + "_ckey";
		unsigned int intCkey = 0;
		setUintColorValue(&intCkey, varName, pML->currentTag);
		int glRepeatH = GL_MIRRORED_REPEAT;
		varName = txName + "_glRepeatH";
		setGlRepeatValue(&glRepeatH, varName, pML->currentTag);
		int glRepeatV = GL_MIRRORED_REPEAT;
		varName = txName + "_glRepeatV";
		setGlRepeatValue(&glRepeatV, varName, pML->currentTag);
		*pInt = Texture::loadTexture(buildFullPath(pML, txFile), intCkey, glRepeatH, glRepeatV);
		return 1;
	}
	return 0; //texture wasn't reset
}
int ModelLoader::setMaterialTextures(ModelLoader* pML, Material* pMT) {
	if (setTexture(pML, &pMT->uTex0, "uTex0") > 0) {
		pMT->uColor.clear();
		pMT->uColor1.clear();
		pMT->uColor2.clear();
		pMT->uTex0translateChannelN = -1;
	}
	setTexture(pML, &pMT->uTex1mask, "uTex1mask");
	setTexture(pML, &pMT->uTex2nm, "uTex2nm");
	setTexture(pML, &pMT->uTex3, "uTex3");
	return 1;
}
int ModelLoader::fillProps_mt(Material* pMT, std::string tagStr, ModelLoader* pML) {
	ModelBuilder* pMB = pML->pModelBuilder;
	if (varExists("mt_use", tagStr)) {
		std::string mt_use = getStringValue("mt_use", tagStr);
		int optsN = pMB->materialsList.size();
		for (int mN = 0; mN <= optsN; mN++) {
			if (mN == optsN) {
				mylog("ERROR in ModelLoader::fillProps_mt: mt_use not found,tag %s\n   file %s\n", tagStr.c_str(), pML->fullPath.c_str());
				break;
			}
			Material* pMt0 = pMB->materialsList.at(mN);
			if (strcmp(pMt0->materialName, mt_use.c_str()) != 0)
				continue;
			memcpy(pMT, pMt0, sizeof(Material));
			break;
		}
	}
	setCharsValue(pMT->shaderType, 32, "mt_type", tagStr);
	setCharsValue(pMT->materialName, 32, "mt_name", tagStr);

	setMaterialTextures(pML, pMT);

	//color
	if (varExists("uColor", tagStr)) {
		unsigned int uintColor = 0;
		setUintColorValue(&uintColor, "uColor", tagStr);
		pMT->uColor.setUint32(uintColor);
		pMT->uTex0 = -1;
		pMT->uColor1.clear();
		pMT->uColor2.clear();
		pMT->uTex0translateChannelN = -1;
	}
	else if (varExists("uColor_use", tagStr)) {
		std::string clName = getStringValue("uColor_use", tagStr);
		MyColor* pCl = MyColor::findColor(clName.c_str(), &pMB->colorsList);
		memcpy(&pMT->uColor, pCl, sizeof(MyColor));
		pMT->uTex0 = -1;
		pMT->uColor1.clear();
		pMT->uColor2.clear();
		pMT->uTex0translateChannelN = -1;
	}
	if (varExists("uColor1", tagStr)) {
		unsigned int uintColor = 0;
		setUintColorValue(&uintColor, "uColor1", tagStr);
		pMT->uColor1.setUint32(uintColor);
		pMT->uColor.clear();
	}
	else if (varExists("uColor1_use", tagStr)) {
		std::string clName = getStringValue("uColor1_use", tagStr);
		MyColor* pCl = MyColor::findColor(clName.c_str(), &pMB->colorsList);
		memcpy(&pMT->uColor1, pCl, sizeof(MyColor));
		pMT->uColor.clear();
	}
	if (varExists("uColor2", tagStr)) {
		unsigned int uintColor = 0;
		setUintColorValue(&uintColor, "uColor2", tagStr);
		pMT->uColor2.setUint32(uintColor);
		pMT->uColor.clear();
	}
	else if (varExists("uColor2_use", tagStr)) {
		std::string clName = getStringValue("uColor2_use", tagStr);
		MyColor* pCl = MyColor::findColor(clName.c_str(), &pMB->colorsList);
		memcpy(&pMT->uColor2, pCl, sizeof(MyColor));
		pMT->uColor.clear();
	}

	setIntValue(&pMT->uTex1mask, "uTex1mask", tagStr);
	setIntValue(&pMT->uTex2nm, "uTex2nm", tagStr);
	//mylog("mt.uTex0=%d, mt.uTex1mask=%d\n", mt.uTex0, mt.uTex1mask);
	if (varExists("primitiveType", tagStr)) {
		std::string str0 = getStringValue("primitiveType", tagStr);
		if (str0.compare("GL_POINTS") == 0) pMT->primitiveType = GL_POINTS;
		else if (str0.compare("GL_LINES") == 0) pMT->primitiveType = GL_LINES;
		else if (str0.compare("GL_LINE_STRIP") == 0) pMT->primitiveType = GL_LINE_STRIP;
		else if (str0.compare("GL_LINE_LOOP") == 0) pMT->primitiveType = GL_LINE_LOOP;
		else if (str0.compare("GL_TRIANGLE_STRIP") == 0) pMT->primitiveType = GL_TRIANGLE_STRIP;
		else if (str0.compare("GL_TRIANGLE_FAN") == 0) pMT->primitiveType = GL_TRIANGLE_FAN;
		else pMT->primitiveType = GL_TRIANGLES;
	}
	setIntValue(&pMT->uTex1alphaChannelN, "uTex1alphaChannelN", tagStr);
	setIntValue(&pMT->uTex0translateChannelN, "uTex0translateChannelN", tagStr);
	setFloatValue(&pMT->uAlphaFactor, "uAlphaFactor", tagStr);
	if (pMT->uAlphaFactor < 1)
		pMT->uAlphaBlending = 1;
	setIntBoolValue(&pMT->uAlphaBlending, "uAlphaBlending", tagStr);
	if (pMT->uAlphaBlending > 0)
		pMT->zBufferUpdate = 0;
	setFloatValue(&pMT->uAmbient, "uAmbient", tagStr);
	setFloatValue(&pMT->uSpecularIntencity, "uSpecularIntencity", tagStr);
	setFloatValue(&pMT->uSpecularMinDot, "uSpecularMinDot", tagStr);
	setFloatValue(&pMT->uSpecularPowerOf, "uSpecularPowerOf", tagStr);
	setFloatValue(&pMT->uBleach, "uBleach", tagStr);

	setFloatValue(&pMT->lineWidth, "lineWidth", tagStr);
	setIntBoolValue(&pMT->zBuffer, "zBuffer", tagStr);
	if (pMT->zBuffer < 1)
		pMT->zBufferUpdate = 0;
	setIntBoolValue(&pMT->zBufferUpdate, "zBufferUpdate", tagStr);

	setIntBoolValue(&pMT->gem, "gem", tagStr);

	setCharsValue(pMT->layer2as, 32, "layer2as", tagStr);

	if (varExists("noShadow", tagStr))
		pMT->dropsShadow = 0;

	if (pML->tagName.compare("mt_adjust") != 0) {
		if (pMT->uTex0 < 0 && pMT->uColor.isZero())
			mylog("ERROR in ModelLoader::fillProps_mt: no tex no color,tag %s\n   file %s\n", tagStr.c_str(), pML->fullPath.c_str());
		if (strlen(pMT->shaderType) < 1)
			mylog("ERROR in ModelLoader::fillProps_mt: no shaderType,tag %s\n   file %s\n", tagStr.c_str(), pML->fullPath.c_str());
	}
	return 1;
}

int ModelLoader::processTag(ModelLoader* pML) {
	ModelBuilder* pMB = pML->pModelBuilder;
	std::string tagStr = pML->currentTag;
	if (pML->tagName.compare("lastLineTexure") == 0)
		return processTag_lastLineTexure(pML);
	if (pML->tagName.compare("color_as") == 0)
		return processTag_color_as(pML);
	if (pML->tagName.compare("a2group") == 0)
		return processTag_a2group(pML);
	if (pML->tagName.compare("short") == 0)
		return processTag_short(pML);
	if (pML->tagName.compare("number_as") == 0) {
		std::string keyName = getStringValue("number_as", pML->currentTag);
		if (pMB->floatsHashMap.find(keyName) != pMB->floatsHashMap.end())
			return 0; //name exists
		else { //add new
			float val = 0;
			setFloatValue(&val, "val", pML->currentTag);
			pMB->floatsHashMap[keyName] = val;
			return 1;
		}
	}
	if (pML->tagName.compare("include") == 0)
		return processTag_include(pML);
	if (pML->tagName.compare("element") == 0)
		return processTag_element(pML);
	if (pML->tagName.compare("/element") == 0) {
		//restore previous useSubjN from stack
		int subjN = pMB->usingSubjsStack.back();
		pMB->usingSubjsStack.pop_back();
		pMB->useSubjN(pMB, subjN);
		return 1;
	}
	if (pML->tagName.compare("texture_as") == 0) {
		//saves texture N in texturesMap under given name
		std::string keyName = getStringValue("texture_as", pML->currentTag);
		if (pMB->floatsHashMap.find(keyName) != pMB->floatsHashMap.end())
			return (int)pMB->floatsHashMap[keyName];
		else { //add new
			std::string txFile = getStringValue("src", pML->currentTag);
			unsigned int intCkey = 0;
			setUintColorValue(&intCkey, "ckey", pML->currentTag);
			int glRepeatH = GL_MIRRORED_REPEAT;
			int glRepeatV = GL_MIRRORED_REPEAT;
			setGlRepeatValue(&glRepeatH, "glRepeatH", tagStr);
			setGlRepeatValue(&glRepeatV, "glRepeatV", tagStr);
			int txN = Texture::loadTexture(buildFullPath(pML, txFile), intCkey, glRepeatH, glRepeatV);
			pMB->floatsHashMap[keyName] = (float)txN;
			//mylog("%s=%d\n", keyName.c_str(), pMB->texturesMap[keyName]);
			return txN;
		}
	}
	if (pML->tagName.compare("mt_type") == 0 || pML->tagName.compare("mt_use") == 0) {
		//sets current material
		ModelBuilder* pMB = pML->pModelBuilder;
		if (!pML->closedTag) {
			//save previous material in stack
			if (pMB->usingMaterialN >= 0)
				pMB->materialsStack.push_back(pMB->usingMaterialN);
		}
		Material mt;
		fillProps_mt(&mt, tagStr, pML);
		pMB->usingMaterialN = pMB->getMaterialN(pMB, &mt);
		return 1;
	}
	if (pML->tagName.compare("/mt_type") == 0 || pML->tagName.compare("/mt_use") == 0) {
		//restore previous material
		if (pMB->materialsStack.size() > 0) {
			pMB->usingMaterialN = pMB->materialsStack.back();
			pMB->materialsStack.pop_back();
		}
		return 1;
	}
	if (pML->tagName.compare("vs") == 0) {
		//sets virtual shape
		ModelBuilder* pMB = pML->pModelBuilder;
		if (pML->closedTag) {
			if (pMB->pCurrentVShape != NULL)
				delete pMB->pCurrentVShape;
		}
		else { //open tag
			//save previous vshape in stack
			if (pMB->pCurrentVShape != NULL)
				pMB->vShapesStack.push_back(pMB->pCurrentVShape);
		}
		pMB->pCurrentVShape = new VirtualShape();
		fillProps_vs(pMB->pCurrentVShape, pMB->floatsHashMap, pML->currentTag);
		return 1;
	}
	if (pML->tagName.compare("/vs") == 0) {
		//restore previous virtual shape
		if (pMB->vShapesStack.size() > 0) {
			if (pMB->pCurrentVShape != NULL)
				delete(pMB->pCurrentVShape);
			pMB->pCurrentVShape = pMB->vShapesStack.back();
			pMB->vShapesStack.pop_back();
		}
		return 1;
	}
	if (pML->tagName.compare("group") == 0) {
		pMB->lockGroup(pMB);
		//mark
		if (varExists("mark", pML->currentTag)) {
			if (getStringValue("mark", pML->currentTag).empty())
				myStrcpy_s(pMB->pCurrentGroup->marks, 128, "");
			else
				addMark(pMB->pCurrentGroup->marks, getStringValue("mark", pML->currentTag));
		}
		return 1;
	}
	if (pML->tagName.compare("/group") == 0) {
		GroupTransform gt;
		fillProps_gt(&gt, pMB, pML->currentTag);
		gt.executeGroupTransform(pMB);

		pMB->releaseGroup(pMB);
		return 1;
	}
	if (pML->tagName.compare("a") == 0)
		return processTag_a(pML); //apply 
	if (pML->tagName.compare("clone") == 0)
		return processTag_clone(pML);
	if (pML->tagName.compare("/clone") == 0)
		return processTag_clone(pML);
	if (pML->tagName.compare("do") == 0)
		return processTag_do(pML);
	if (pML->tagName.compare("a2mesh") == 0)
		return processTag_a2mesh(pML);
	if (pML->tagName.compare("line2mesh") == 0)
		return processTag_line2mesh(pML);
	if (pML->tagName.compare("mt_adjust") == 0) {
		if (pML->pMaterialAdjust != NULL)
			mylog("ERROR in ModelLoader::processTag %s, pMaterialAdjust is still busy. File: %s\n", pML->currentTag.c_str(), pML->fullPath.c_str());
		pML->pMaterialAdjust = new (MaterialAdjust);
		fillProps_mt(pML->pMaterialAdjust, pML->currentTag, pML);
		pML->pMaterialAdjust->setWhat2adjust(pML->pMaterialAdjust, pML->currentTag);
		//save current material
		if (pMB->usingMaterialN >= 0)
			pMB->materialsStack.push_back(pMB->usingMaterialN);
		//adjust current material
		Material* pMt0 = pMB->materialsList.at(pMB->usingMaterialN);
		Material mt;
		memcpy(&mt, pMt0, sizeof(Material));
		//modify material
		MaterialAdjust::adjust(&mt, pML->pMaterialAdjust);
		pMB->usingMaterialN = pMB->getMaterialN(pMB, &mt);
		if (pML->closedTag){
			//clean up
			pMB->materialsStack.pop_back();
			//remove pMaterialAdjust
			delete pML->pMaterialAdjust;
			pML->pMaterialAdjust = NULL;
		}
		return 1;
	}
	if (pML->tagName.compare("/mt_adjust") == 0) {
		//restore original material
		if (pMB->materialsStack.size() > 0) {
			pMB->usingMaterialN = pMB->materialsStack.back();
			pMB->materialsStack.pop_back();
		}
		//remove pMaterialAdjust
		if (pML->pMaterialAdjust != NULL) {
			delete pML->pMaterialAdjust;
			pML->pMaterialAdjust = NULL;
		}
		return 1;
	}
	if (pML->tagName.compare("mt_adjust_as") == 0) {
		//save MaterialAdjust in MaterialAdjustsList
		std::vector<MaterialAdjust*>* pMaterialAdjustsList = &pMB->materialAdjustsList;
		std::string scope = getStringValue("scope", tagStr);
		if (scope.compare("app") == 0)
			pMaterialAdjustsList = &MaterialAdjust::materialAdjustsList;
		std::string keyName = getStringValue("mt_adjust_as", tagStr);
		//check if exists
		for (int i = pMaterialAdjustsList->size() - 1; i >= 0; i--)
			if (strcmp(pMaterialAdjustsList->at(i)->adjustmentName, keyName.c_str()) == 0)
				return 0; //name exists
		//add new
		MaterialAdjust* pMtA = new (MaterialAdjust);
		myStrcpy_s(pMtA->adjustmentName, 32, keyName.c_str());
		fillProps_mt(pMtA, pML->currentTag, pML);
		pMtA->setWhat2adjust(pMtA, tagStr);

		pMaterialAdjustsList->push_back(pMtA);
		return 1;
	}
	if (pML->tagName.compare("line") == 0)
		return processTag_lineStart(pML);
	if (pML->tagName.compare("/line") == 0)
		return processTag_lineEnd(pML);
	if (pML->tagName.compare("p") == 0) {
		//line point
		Vertex01* pV = new Vertex01();
		if ((int)pMB->vertices.size() > pML->lineStartsAt) {
			memcpy(pV, pMB->vertices.back(), sizeof(Vertex01));
			pV->endOfSequence = 0;
			pV->rad = 0;
		}
		pMB->vertices.push_back(pV);
		pV->subjN = pMB->usingSubjN;
		pV->materialN = pMB->usingMaterialN;
		setFloatArray(pV->aPos, 3, "pxyz", tagStr);
		setFloatValue(&pV->aPos[0], "px", tagStr);
		setFloatValue(&pV->aPos[1], "py", tagStr);
		setFloatValue(&pV->aPos[2], "pz", tagStr);
		float dPos[3] = { 0,0,0 };
		setFloatArray(dPos, 3, "dxyz", tagStr);
		setFloatValue(&dPos[0], "dx", tagStr);
		setFloatValue(&dPos[1], "dy", tagStr);
		setFloatValue(&dPos[2], "dz", tagStr);
		if (!v3equals(dPos, 0))
			for (int i = 0; i < 3; i++)
				pV->aPos[i] += dPos[i];
		setFloatValue(&pV->rad, "r", tagStr);
		if (varExists("passThrough", tagStr))
			pV->rad = -1;
		else if (varExists("curve", tagStr))
			pV->rad = -2;
		return 1;
	}
	if (pML->tagName.compare("tip") == 0)
		return processTag_lineTip(pML);
	if (pML->tagName.compare("dot") == 0)
	{
		Vertex01* pV = new Vertex01();
		pMB->vertices.push_back(pV);
		setFloatArray(pV->aPos, 3, "dot", tagStr);
		setFloatArray(pV->aNormal, 3, "nrm", tagStr);
		pV->subjN = pMB->usingSubjN;
		Material mt;
		memcpy(&mt, pMB->materialsList.at(pMB->usingMaterialN), sizeof(Material));
		mt.primitiveType = GL_LINES;
		myStrcpy_s(mt.shaderType, 32, "phong");
		mt.uDiscardNormalsOut = 1;
		pV->materialN = pMB->getMaterialN(pMB, &mt);

		pV = new Vertex01(*pV);
		pMB->vertices.push_back(pV);
		for (int i = 0; i < 3; i++)
			pV->aPos[i] += pV->aNormal[i]*mt.lineWidth/2;
		return 1;
	}
	if (pML->tagName.compare("ring") == 0) {
		//line ring
		VirtualShape vs;
		fillProps_vs(&vs, pMB->floatsHashMap, tagStr);
		float stepDg = (vs.angleFromTo[0] - vs.angleFromTo[1]) / vs.sectionsR;
		pMB->lockGroup(pMB);
		for (int rpn = 0; rpn <= vs.sectionsR; rpn++) {
			// rpn - radial point number
			float angleRd = (vs.angleFromTo[0] + stepDg * rpn) * degrees2radians;
			float kx = cosf(angleRd);
			float ky = sinf(angleRd);
			pMB->addVertex(pMB, kx, ky, 1);
		}
		GroupTransform gt;
		fillProps_gt(&gt, pMB, tagStr);
		v3set(gt.scale, vs.whl[0] / 2, vs.whl[1] / 2, vs.whl[2] / 2);
		gt.pGroup = pMB->pCurrentGroup;
		GroupTransform::executeGroupTransform(pMB, &gt);
		pMB->releaseGroup(pMB);
		return 1;
	}
	mylog("ERROR in ModelLoader::processTag, unhandled tag %s, file %s\n", pML->currentTag.c_str(), pML->fullPath.c_str());
	//mylog("======File:\n%s----------\n", pML->pData);
	return -1;
}
int ModelLoader::fillProps_vs(VirtualShape* pVS, std::map<std::string, float> floatsHashMap, std::string tagStr) {
	//sets virtual shape
	setCharsValue(pVS->shapeType, 32, "vs", tagStr);
	setFloatArray(pVS->whl, 3, "whl", tagStr);
	setFloatValue(&pVS->whl[0], "width", tagStr);
	setFloatValue(&pVS->whl[1], "height", tagStr);
	setFloatValue(&pVS->whl[2], "length", tagStr);
	//extensions
	float ext;
	if (varExists("ext", tagStr)) {
		setFloatValue(&ext, "ext", tagStr);
		pVS->setExt(ext);
	}
	if (varExists("extX", tagStr)) {
		setFloatValue(&ext, "extX", tagStr);
		pVS->setExtX(ext);
	}
	if (varExists("extY", tagStr)) {
		setFloatValue(&ext, "extY", tagStr);
		pVS->setExtY(ext);
	}
	if (varExists("extZ", tagStr)) {
		setFloatValue(&ext, "extZ", tagStr);
		pVS->setExtZ(ext);
	}
	setFloatValue(&pVS->extU, "extU", tagStr);
	setFloatValue(&pVS->extD, "extD", tagStr);
	setFloatValue(&pVS->extL, "extL", tagStr);
	setFloatValue(&pVS->extR, "extR", tagStr);
	setFloatValue(&pVS->extF, "extF", tagStr);
	setFloatValue(&pVS->extB, "extB", tagStr);
	//sections
	setIntValue(&pVS->sectionsR, "sectR", tagStr);
	setIntValueFromHashMap(&pVS->sectionsR, floatsHashMap, "sectR_use", tagStr);
	setIntValue(&pVS->sections[0], "sectX", tagStr);
	setIntValue(&pVS->sections[1], "sectY", tagStr);
	setIntValue(&pVS->sections[2], "sectZ", tagStr);

	setFloatArray(pVS->angleFromTo, 2, "angleFromTo", tagStr);
	setFloatValue(&pVS->angleFromTo[0], "angleFrom", tagStr);
	setFloatValue(&pVS->angleFromTo[1], "angleTo", tagStr);

	setCharsValue(pVS->side, 16, "side", tagStr);

	//mylog("pVS->shapeType=%s whl=%fx%fx%f\n", pVS->shapeType, pVS->whl[0], pVS->whl[1], pVS->whl[2]);
	return 1;
}

int ModelLoader::processTag_a(ModelLoader* pML) {
	//apply
	ModelBuilder* pMB = pML->pModelBuilder;
	std::string tagStr = pML->currentTag;
	pMB->lockGroup(pMB);
	pMB->lockGroup(pMB);
	//mark
	if (varExists("mark", tagStr))
		addMark(pMB->pCurrentGroup->marks, getStringValue("mark", tagStr));

	std::vector<std::string> applyTosVector = splitString(pML->getStringValue("a", tagStr), ",");
	Material* pMT = pMB->materialsList.at(pMB->usingMaterialN);
	int texN = pMT->uTex1mask;
	if (texN < 0)
		texN = pMT->uTex0;
	float xywh[4] = { 0,0,1,1 };
	TexCoords* pTC = NULL;
	if (varExists("xywh", tagStr)) {
		setFloatArray(xywh, 4, "xywh", tagStr);
		std::string flipStr = getStringValue("flip", tagStr);
		TexCoords tc;
		tc.set(texN, xywh[0], xywh[1], xywh[2], xywh[3], flipStr);
		pTC = &tc;
	}
	else
		if (varExists("xywh_GL", tagStr)) {
			setFloatArray(xywh, 4, "xywh_GL", tagStr);
			std::string flipStr = getStringValue("flip", tagStr);
			TexCoords tc;
			tc.set_GL(&tc, xywh[0], xywh[1], xywh[2], xywh[3], flipStr);
			pTC = &tc;
		}
	TexCoords* pTC2nm = NULL;
	if (varExists("xywh2nm", tagStr)) {
		setFloatArray(xywh, 4, "xywh2nm", tagStr);
		std::string flipStr = getStringValue("flip2nm", tagStr);
		TexCoords tc2nm;
		tc2nm.set(pMT->uTex2nm, xywh[0], xywh[1], xywh[2], xywh[3], flipStr);
		pTC2nm = &tc2nm;
	}
	else if (varExists("xywh2nm_GL", tagStr)) {
		setFloatArray(xywh, 4, "xywh2nm_GL", tagStr);
		std::string flipStr = getStringValue("flip2nm", tagStr);
		TexCoords tc2nm;
		tc2nm.set_GL(&tc2nm, xywh[0], xywh[1], xywh[2], xywh[3], flipStr);
		pTC2nm = &tc2nm;
	}
	//adjusted VirtualShape
	VirtualShape* pVS_a = new VirtualShape(*pMB->pCurrentVShape);
	fillProps_vs(pVS_a, pMB->floatsHashMap, tagStr);

	for (int aN = 0; aN < (int)applyTosVector.size(); aN++) {
		pMB->buildFace(pMB, applyTosVector.at(aN), pVS_a, pTC, pTC2nm);
	}
	//mylog("vertsN=%d\n",pMB->vertices.size());

	if (strlen(pVS_a->side) > 0) {
		std::vector<std::string> sides = splitString(std::string(pVS_a->side), ",");
		for (int sideN = 0; sideN < (int)sides.size(); sideN++) {
			GroupTransform gt;
			if (sideN > 0) {
				//clone group
				pMB->releaseGroup(pMB);
				pMB->lockGroup(pMB);
				GroupTransform::flagGroup(pMB->pLastClosedGroup, &pMB->vertices, &pMB->triangles);
				GroupTransform::cloneFlagged(pMB);
			}
			//invert?
			if (sideN > 0 || sides[sideN].find("in") == 0) {
				GroupTransform::flagGroup(pMB->pCurrentGroup, &pMB->vertices, &pMB->triangles);
				GroupTransform::invertFlagged(&pMB->vertices, &pMB->triangles);
			}
			//adjust material?
			std::string mtName = sides[sideN];
			if (!mtName.empty()) {
				if (mtName.compare("in") == 0)
					mtName = "";
				else if (mtName.compare("inner") == 0)
					mtName = "";
				else if (mtName.find("in:") == 0)
					mtName = trimString(mtName.substr(3));
				else if (mtName.find("inner:") == 0)
					mtName = trimString(mtName.substr(6));
			}
			if (!mtName.empty()) {
				Material* pMt = pMB->materialsList.at(pMB->usingMaterialN);
				//locate materialAdjust
				MaterialAdjust* pMA = MaterialAdjust::findMaterialAdjust(mtName.c_str(), &pMB->materialAdjustsList);
				if (pMA == NULL) {
					mylog("ERROR in ModelLoader::processTag_a, %s not found, %s\n", mtName.c_str(), tagStr.c_str());
					return -1;
				}
				Material mt;
				memcpy(&mt, pMt, sizeof(Material));
				pMA->adjust(&mt, pMA);
				int mtN = pMB->getMaterialN(pMB, &mt);
				if (mtN != pMB->usingMaterialN) {
					//replace by adjusted material
					int totalN = pMB->vertices.size();
					for (int i = pMB->pCurrentGroup->fromVertexN; i < totalN; i++)
						pMB->vertices.at(i)->materialN = mtN;
					totalN = pMB->triangles.size();
					for (int i = pMB->pCurrentGroup->fromTriangleN; i < totalN; i++)
						pMB->triangles.at(i)->materialN = mtN;
				}
			}
		}
	}
	pMB->releaseGroup(pMB);
	delete pVS_a;

	GroupTransform GT_a;
	fillProps_gt(&GT_a, pMB, tagStr);
	GT_a.executeGroupTransform(pMB);

	pMB->releaseGroup(pMB);
	return 1;
}

int ModelLoader::processTag_clone(ModelLoader* pML) {
	ModelBuilder* pMB = pML->pModelBuilder;
	std::string tagStr = pML->currentTag;
	if (pML->tagName.compare("clone") == 0) {
		//mark what to clone
		GroupTransform gt;
		gt.pGroup = pMB->pLastClosedGroup;
		gt.flagSelection(&gt, &pMB->vertices, &pMB->triangles);

		//cloning
		pMB->lockGroup(pMB);
		//add mark?
		std::string cloneMark = getStringValue("mark", tagStr);
		if (!cloneMark.empty())
			addMark(pMB->pCurrentGroup->marks, cloneMark);
		gt.cloneFlagged(pMB, &pMB->vertices, &pMB->triangles, &pMB->vertices, &pMB->triangles);
	}
	/*
	GroupTransform gt;
	fillProps_gt(&gt, pMB, pML->currentTag);
	gt.executeGroupTransform(pMB);
	*/
	processTag_do(pML);

	if (pML->tagName.compare("/clone") == 0 || pML->closedTag) {
		pMB->releaseGroup(pMB);
	}
	return 1;
}
int ModelLoader::addMark(char* marks, std::string newMark) {
	if (newMark.empty())
		return 0;
	std::vector<std::string> newMarksList = splitString(newMark, ",");
	std::string allMarks;
	allMarks.assign(marks);
	for (int i = 0; i < (int)newMarksList.size(); i++)
		allMarks.append("<" + trimString(newMarksList.at(i)) + ">");
	myStrcpy_s(marks, 128, allMarks.c_str());
	return 1;
}
int ModelLoader::fillProps_gt(GroupTransform* pGT, ModelBuilder* pMB, std::string tagStr) {
	pGT->pGroup = pMB->pCurrentGroup;
	//position
	setFloatArray(pGT->shift, 3, "pxyz", tagStr);
	setFloatValue(&pGT->shift[0], "px", tagStr);
	setFloatValue(&pGT->shift[1], "py", tagStr);
	setFloatValue(&pGT->shift[2], "pz", tagStr);
	//angles
	setFloatArray(pGT->spinDg, 3, "axyz", tagStr);
	setFloatValue(&pGT->spinDg[0], "ax", tagStr);
	setFloatValue(&pGT->spinDg[1], "ay", tagStr);
	setFloatValue(&pGT->spinDg[2], "az", tagStr);
	setCharsValue(pGT->onThe,32,"onThe", tagStr);
	setCharsValue(pGT->align,32,"align", tagStr);
	setFloatArray(pGT->headZto, 3, "headZto", tagStr);
	//limit to
	if (varExists("all", tagStr))
		pGT->pGroup = NULL;
	if (varExists("lastClosedGroup", tagStr))
		pGT->pGroup = pMB->pLastClosedGroup;
	if (varExists("markedAs", tagStr))
		pGT->limit2mark(pGT, getStringValue("markedAs", tagStr));
	setFloatArray(pGT->pMin, 3, "xyzMin", tagStr);
	setFloatValue(&pGT->pMin[0], "fromX", tagStr);
	setFloatValue(&pGT->pMin[1], "fromY", tagStr);
	setFloatValue(&pGT->pMin[2], "fromZ", tagStr);
	setFloatArray(pGT->pMax, 3, "xyzMax", tagStr);
	setFloatValue(&pGT->pMax[0], "toX", tagStr);
	setFloatValue(&pGT->pMax[1], "toY", tagStr);
	setFloatValue(&pGT->pMax[2], "toZ", tagStr);
	//radiuses: fromRX - YZ plane
	setFloatValue(&pGT->rMin[0], "fromRX", tagStr);
	setFloatValue(&pGT->rMin[1], "fromRY", tagStr);
	setFloatValue(&pGT->rMin[2], "fromRZ", tagStr);
	setFloatValue(&pGT->rMax[0], "toRX", tagStr);
	setFloatValue(&pGT->rMax[1], "toRY", tagStr);
	setFloatValue(&pGT->rMax[2], "toRZ", tagStr);

	//scale
	setFloatArray(pGT->scale, 3, "scale", tagStr);
	if (varExists("scaleTo", tagStr)) { //re-size
		//bounding box
		pGT->flagSelection(pGT, &pMB->vertices, NULL);
		pGT->buildBoundingBoxFlagged(pGT, &pMB->vertices);
		float scaleTo[3];
		v3copy(scaleTo, pGT->bbSize);
		setFloatArray(scaleTo, 3, "scaleTo", tagStr);
		for (int i = 0; i < 3; i++)
			if (pGT->bbSize[i] == 0)
				pGT->scale[i] = 0;
			else
				pGT->scale[i] = scaleTo[i] / pGT->bbSize[i];
	}
	if (varExists("sizeD", tagStr)) { //re-size
		float sizeD[3];
		setFloatArray(sizeD, 3, "sizeD", tagStr);
		//bounding box
		pGT->flagSelection(pGT, &pMB->vertices, NULL);
		pGT->buildBoundingBoxFlagged(pGT, &pMB->vertices);
		for (int i = 0; i < 3; i++)
			pGT->scale[i] = (pGT->bbSize[i] + sizeD[i]) / pGT->bbSize[i];
	}
	if (varExists("do", tagStr))
		setCharsValue(pGT->action,32,"do", tagStr);
	if (varExists("clone", tagStr))
		setCharsValue(pGT->action,32,"clone", tagStr);
	setCharsValue(pGT->applyTo,32,"applyTo", tagStr);
	setFloatArray(pGT->xyz, 3, "xyz", tagStr);
	setFloatArray(pGT->refPoint, 3, "refPoint", tagStr);

	return 1;
}
int ModelLoader::processTag_a2mesh(ModelLoader* pML) {
	ModelBuilder* pMB = pML->pModelBuilder;
	std::string tagStr = pML->currentTag;
	GroupTransform gt;
	fillProps_gt(&gt, pMB, pML->currentTag);
	gt.flagSelection(&gt, &pMB->vertices, &pMB->triangles);
	//clone a copy
	std::vector<Vertex01*> vx1;
	std::vector<Triangle01*> tr1;
	gt.cloneFlagged(NULL, &vx1, &tr1, &pMB->vertices, &pMB->triangles);
	// build transform and inverted martrices
	mat4x4 transformMatrix;
	gt.buildTransformMatrix(&gt, &transformMatrix, 1);
	mat4x4 transformMatrixInverted;
	mat4x4_invert(transformMatrixInverted, transformMatrix);
	//move/rotate cloned
	gt.flagAll(&vx1, &tr1);
	//gt.transformFlagged(&pMB->vertices, &transformMatrixInverted);
	gt.transformFlaggedMx(&vx1, &transformMatrixInverted, gt.normalsToo);
	float wh[2];
	setFloatArray(wh, 2, "wh", tagStr);
	Polygon frame;
	frame.setRectangle(&frame, wh[0], wh[1]);
	//destination arrays
	std::vector<Vertex01*> vx2;
	std::vector<Triangle01*> tr2;
	Polygon triangle;
	for (int i = tr1.size() - 1; i >= 0; i--) {
		triangle.setTriangle(&triangle, tr1.at(i), &vx1);
		Polygon intersection;
		int pointsN = Polygon::xyIntersection(&intersection, &frame, &triangle);
		if (pointsN > 2) {
			Polygon::buildTriangles(&intersection);
			GroupTransform::flagAll(&intersection.vertices, &intersection.triangles);
			GroupTransform::cloneFlagged(NULL, &vx2, &tr2, &intersection.vertices, &intersection.triangles);
		}
	}
	if (vx2.size() < 3) {
		mylog("ERROR in ModelLoader::processTag_a2mesh: projection is empty. Tag %s\n   File %s\n",tagStr.c_str(),pML->fullPath.c_str());
	}
	gt.flagAll(&vx2, &tr2);
	//at this point we have cutted fragment facing us
	int vxTotal = vx2.size();
	int trTotal = tr2.size();

	//clean up marks
	for (int vN = 0; vN < vxTotal; vN++)
		myStrcpy_s(vx2.at(vN)->marks, 128, "");
	for (int tN = 0; tN < trTotal; tN++)
		myStrcpy_s(tr2.at(tN)->marks, 128, "");

	//apply adjusted material ?
	if (pML->pMaterialAdjust != NULL) {
		//scan vertices to find new (unupdated) material
		int materialNsrc = -1; //which N to replace
		int materialNdst = -1; //replace by N 
		for (int vN = 0; vN < vxTotal; vN++) {
			Vertex01* pV = vx2.at(vN);
			if (pV->flag < 0)
				continue;
			if (materialNsrc == pV->materialN)
				continue;
			//have new material
			materialNsrc = pV->materialN;
			Material mt;
			Material* pMt0 = pMB->materialsList.at(materialNsrc);
			memcpy(&mt, pMt0, sizeof(Material));
			//modify material
			MaterialAdjust::adjust(&mt, pML->pMaterialAdjust);
			materialNdst = pMB->getMaterialN(pMB, &mt);
			if (materialNsrc != materialNdst) {
				//replace mtN in vx and tr arrays
				for (int vN2 = vN; vN2 < vxTotal; vN2++) {
					Vertex01* pV2 = vx2.at(vN2);
					if (pV2->flag < 0)
						continue;
					if (materialNsrc == pV2->materialN)
						pV2->materialN = materialNdst;
				}
				for (int tN2 = 0; tN2 < trTotal; tN2++) {
					Triangle01* pT2 = tr2.at(tN2);
					if (pT2->flag < 0)
						continue;
					if (materialNsrc == pT2->materialN)
						pT2->materialN = materialNdst;
				}
				materialNsrc = materialNdst;
			}
		}
	}
	else { // pML->pMaterialAdjust == NULL, use pMB->usingMaterialN
		for (int vN2 = 0; vN2 < vxTotal; vN2++) {
			Vertex01* pV2 = vx2.at(vN2);
			if (pV2->flag < 0)
				continue;
			pV2->materialN = pMB->usingMaterialN;
		}
		for (int tN2 = 0; tN2 < trTotal; tN2++) {
			Triangle01* pT2 = tr2.at(tN2);
			if (pT2->flag < 0)
				continue;
			pT2->materialN = pMB->usingMaterialN;
		}
	}
	//apply xywh/2nm ?
	if (varExists("xywh", tagStr) || varExists("xywh2nm", tagStr) || varExists("xywh_GL", tagStr) || varExists("xywh2nm_GL", tagStr)) {
		Material* pMT = pMB->materialsList.at(vx2.at(0)->materialN);
		float xywh[4] = { 0,0,1,1 };
		TexCoords* pTC = NULL;
		if (varExists("xywh", tagStr)) {
			setFloatArray(xywh, 4, "xywh", tagStr);
			std::string flipStr = getStringValue("flip", tagStr);
			int texN = pMT->uTex1mask;
			if (texN < 0)
				texN = pMT->uTex0;
			TexCoords tc;
			tc.set(texN, xywh[0], xywh[1], xywh[2], xywh[3], flipStr);
			pTC = &tc;
		}
		else if (varExists("xywh_GL", tagStr)) {
			setFloatArray(xywh, 4, "xywh_GL", tagStr);
			std::string flipStr = getStringValue("flip", tagStr);
			TexCoords tc;
			tc.set_GL(&tc, xywh[0], xywh[1], xywh[2], xywh[3], flipStr);
			pTC = &tc;
		}
		TexCoords* pTC2nm = NULL;
		if (varExists("xywh2nm", tagStr)) {
			setFloatArray(xywh, 4, "xywh2nm", tagStr);
			std::string flipStr = getStringValue("flip2nm", tagStr);
			TexCoords tc2nm;
			tc2nm.set(pMT->uTex2nm, xywh[0], xywh[1], xywh[2], xywh[3], flipStr);
			pTC2nm = &tc2nm;
		}
		else if (varExists("xywh2nm_GL", tagStr)) {
			setFloatArray(xywh, 4, "xywh2nm_GL", tagStr);
			std::string flipStr = getStringValue("flip2nm", tagStr);
			TexCoords tc2nm;
			tc2nm.set_GL(&tc2nm, xywh[0], xywh[1], xywh[2], xywh[3], flipStr);
			pTC2nm = &tc2nm;
		}
		pMB->applyTexture2flagged(&vx2, "front", pTC, pTC2nm);
	}

	float detachBy = 0;
	setFloatValue(&detachBy, "detachBy", tagStr);
	if (detachBy != 0) {
		mat4x4 mx;
		mat4x4_translate(mx, 0, 0, detachBy);
		gt.transformFlaggedMx(&vx2, &mx, false);
	}
	//move/rotate back
	gt.transformFlaggedMx(&vx2, &transformMatrix, gt.normalsToo);
	//clone back to modelBuilder arrays
	gt.cloneFlagged(pMB, &pMB->vertices, &pMB->triangles, &vx2, &tr2);

	//clear memory
	for (int i = vx1.size() - 1; i >= 0; i--)
		delete vx1.at(i);
	vx1.clear();
	for (int i = tr1.size() - 1; i >= 0; i--)
		delete tr1.at(i);
	tr1.clear();
	for (int i = vx2.size() - 1; i >= 0; i--)
		delete vx2.at(i);
	vx2.clear();
	for (int i = tr2.size() - 1; i >= 0; i--)
		delete tr2.at(i);
	tr2.clear();

	return 1;
}
int ModelLoader::processTag_element(ModelLoader* pML) {
	ModelBuilder* pMB = pML->pModelBuilder;
	std::string tagStr = pML->currentTag;
	std::string sourceFile = getStringValue("element", tagStr);
	std::string subjClass = getStringValue("class", tagStr);
	std::vector<GameSubj*>* pSubjsVector0 = pML->pSubjsVector;
	int subjN = -1;
	if (!sourceFile.empty()) {
		sourceFile = buildFullPath(pML, sourceFile);
		subjN = loadModel(pSubjsVector0, sourceFile, subjClass);
	}
	else { //sourceFile not specified
		subjN = pML->pSubjsVector->size();
		GameSubj* pGS = theGame.newGameSubj(subjClass);
		pGS->pSubjsSet = pSubjsVector0;
		pGS->nInSubjsSet = subjN;
		pML->pSubjsVector->push_back(pGS);
		pGS->totalNativeElements = 1;
		pGS->totalElements = 1;
		if (!pML->closedTag) { //DrawJobs will follow
			pMB->usingSubjsStack.push_back(pMB->usingSubjN);
			pMB->useSubjN(pMB, subjN);
		}
	}
	//keep reading tag
	GameSubj* pGS = pSubjsVector0->at(subjN);
	int rootN = pMB->subjNumbersList.at(0);
	std::string attachTo = getStringValue("attachTo", tagStr);
	if (attachTo.empty()) //attach to root
		pGS->d2parent = subjN - rootN;
	else {
		//find parent by name
		for (int sN = subjN - 1; sN >= rootN; sN--) {
			if (strcmp(pSubjsVector0->at(sN)->name, attachTo.c_str()) == 0) {
				pGS->d2parent = subjN - sN;
				break;
			}
		}
	}
	std::string headTo = getStringValue("headTo", tagStr);
	if (!headTo.empty()) { //find headTo by name
		for (int sN = subjN - 1; sN >= rootN; sN--) {
			if (strcmp(pSubjsVector0->at(sN)->name, headTo.c_str()) == 0) {
				pGS->d2headTo = subjN - sN;
				break;
			}
		}
	}
	float xyz[3] = { 0,0,0 };
	//position
	setFloatArray(xyz, 3, "pxyz", tagStr);
	setFloatValue(&xyz[0], "px", tagStr);
	setFloatValue(&xyz[1], "py", tagStr);
	setFloatValue(&xyz[2], "pz", tagStr);
	v3copy(pGS->ownCoords.pos, xyz);
	//angles
	v3set(xyz, 0, 0, 0);
	setFloatArray(xyz, 3, "axyz", tagStr);
	setFloatValue(&xyz[0], "ax", tagStr);
	setFloatValue(&xyz[1], "ay", tagStr);
	setFloatValue(&xyz[2], "az", tagStr);
	v3set(pGS->ownCoords.eulerDg, xyz[0], xyz[1], xyz[2]);
	Coords::eulerDgToMatrix(pGS->ownCoords.rotationMatrix, pGS->ownCoords.eulerDg);
	
	if (varExists("noShadow", tagStr))
		pGS->dropsShadow = 0;
	return 1;
}
int ModelLoader::processTag_include(ModelLoader* pML) {
	ModelBuilder* pMB = pML->pModelBuilder;
	std::string tagStr = pML->currentTag;
	std::string sourceFile = getStringValue("include", tagStr);
	sourceFile = buildFullPath(pML, sourceFile);
	std::vector<GameSubj*>* pSubjsVector0 = pML->pSubjsVector;
	int subjN = pMB->usingSubjN;
	pMB->lockGroup(pMB);
	ModelLoader* pML2 = new ModelLoader(pSubjsVector0, subjN, pMB, sourceFile);
	processSource(pML2);
	delete pML2;
	GroupTransform gt;
	fillProps_gt(&gt, pMB, tagStr);
	gt.flagSelection(&gt, &pMB->vertices, &pMB->triangles);
	gt.transformFlagged(&gt, &pMB->vertices);
	pMB->releaseGroup(pMB);
	return 1;
}
int ModelLoader::processTag_do(ModelLoader* pML) {
	ModelBuilder* pMB = pML->pModelBuilder;
	GroupTransform gt;
	fillProps_gt(&gt, pMB, pML->currentTag);
	gt.flagSelection(&gt, &pMB->vertices, &pMB->triangles);
	if (strstr(gt.action,"normal") != NULL) {
		if (strstr(gt.action,"calc") != NULL)
			pMB->normalsCalc(pMB);
		else if (strstr(gt.action,"merge") != NULL)
			pMB->normalsMerge(pMB);
		else if (strstr(gt.action,"set") != NULL) {
			std::vector<Vertex01*>* pVx = &pMB->vertices;
			for (int vN = pVx->size() - 1; vN >= 0; vN--) {
				Vertex01* pV = pVx->at(vN);
				if (pV->flag < 0)
					continue;
				vec3_norm(pV->aNormal, gt.xyz);
			}
		}
		else if (strcmp(gt.action,"normalsTo") == 0) {
			std::vector<Vertex01*>* pVx = &pMB->vertices;
			for (int vN = pVx->size() - 1; vN >= 0; vN--) {
				Vertex01* pV = pVx->at(vN);
				if (pV->flag < 0)
					continue;
				for (int i = 0; i < 3; i++)
					pV->aNormal[i] = gt.xyz[i] - pV->aPos[i];
				vec3_norm(pV->aNormal, pV->aNormal);
			}
		}
		else if (strcmp(gt.action,"normalsFrom") == 0) {
			std::vector<Vertex01*>* pVx = &pMB->vertices;
			for (int vN = pVx->size() - 1; vN >= 0; vN--) {
				Vertex01* pV = pVx->at(vN);
				if (pV->flag < 0)
					continue;
				for (int i = 0; i < 3; i++)
					pV->aNormal[i] = pV->aPos[i] - gt.xyz[i];
				vec3_norm(pV->aNormal, pV->aNormal);
			}
		}
		else if (strcmp(gt.action,"normalsD") == 0) {
			std::vector<Vertex01*>* pVx = &pMB->vertices;
			for (int vN = pVx->size() - 1; vN >= 0; vN--) {
				Vertex01* pV = pVx->at(vN);
				if (pV->flag < 0)
					continue;
				for (int i = 0; i < 3; i++)
					pV->aNormal[i] += gt.xyz[i];
				vec3_norm(pV->aNormal, pV->aNormal);
			}
		}
		return 1;
	}
	else if (strcmp(gt.action,"reflect") == 0) {
		std::vector<Vertex01*>* pVx = &pMB->vertices;
		for (int vN = pVx->size() - 1; vN >= 0; vN--) {
			Vertex01* pV = pVx->at(vN);
			if (pV->flag < 0)
				continue;
			for (int i = 0; i < 3; i++)
				if (gt.xyz[i] != 0) {
					pV->aPos[i] = -pV->aPos[i];
					pV->aNormal[i] = -pV->aNormal[i];
				}
		}
		//rearrange?
		int sum = (int)(gt.xyz[0] + gt.xyz[1] + gt.xyz[2]);
		if (sum % 2 != 0) {
			//rearrange triangles
			GroupTransform::invertFlaggedTriangles(&pMB->triangles);

			//mark and copy line verts
			int mtN = -1;
			bool isLine = false;
			std::vector<Vertex01*> verts0;
			for (int vN = pVx->size() - 1; vN >= 0; vN--) {
				Vertex01* pV = pVx->at(vN);
				if (pV->flag < 0)
					continue;
				pV->flag = 0;
				int mN = pV->materialN;
				if (mtN != mN) {
					mtN = mN;
					Material* pMt = pMB->materialsList.at(mN);
					isLine = (pMt->primitiveType == GL_LINE_STRIP);
				}
				if (!isLine)
					continue;
				pV->flag = 1; //mark as line
				if (pV->endOfSequence != 0)
					pV->endOfSequence = -pV->endOfSequence;
				verts0.push_back(pV);
			}
			if (verts0.size() > 0) {
				//overwrite original verts in reversed order
				int replaceByN = verts0.size() - 1;
				for (int vN = pVx->size() - 1; vN >= 0; vN--) {
					Vertex01* pV = pVx->at(vN);
					if (pV->flag != 1)
						continue;
					pVx->at(vN) = verts0.at(replaceByN);
					replaceByN--;
				}
				verts0.clear();
			}
		}
		return gt.transformFlagged(&gt, &pMB->vertices);
	}
	else if (strcmp(gt.action,"points2mesh") == 0) {
		return processTag_points2mesh(pML);
	}
	else if (strcmp(gt.action,"replaceLineByGroup") == 0) {
		return processTag_replaceLineByGroup(pML);
	}
	else { //	if (gt.action.empty()) {
		if (strlen(gt.applyTo)==0)
			return gt.transformFlagged(&gt, &pMB->vertices);
		else
			return gt.transformFlaggedRated(&gt, &pMB->vertices);
	}
	return 1;
}

int ModelLoader::processTag_short(ModelLoader* pML) {
	ModelBuilder* pMB = pML->pModelBuilder;
	std::string tagStr = pML->currentTag;
	float p0[3];
	float p1[3];
	int vertsN = pMB->vertices.size();
	if (vertsN > 0)
		v3copy(p0, pMB->vertices.at(vertsN - 1)->aPos);
	setFloatArray(p0, 3, "p0", tagStr);
	v3copy(p1, p0);
	setFloatArray(p1, 3, "p1", tagStr);
	p1[0] += getFloatValue("dx", tagStr);
	p1[1] += getFloatValue("dy", tagStr);
	p1[2] += getFloatValue("dz", tagStr);
	Material mt;
	//save previous material in stack
	if (pMB->usingMaterialN >= 0) {
		pMB->materialsStack.push_back(pMB->usingMaterialN);
		memcpy(&mt, pMB->materialsList.at(pMB->usingMaterialN), sizeof(Material));
	}
	mt.primitiveType = GL_LINES;
	fillProps_mt(&mt, pML->currentTag, pML);
	pMB->usingMaterialN = pMB->getMaterialN(pMB, &mt);

	pMB->addVertex(pMB, p0[0], p0[1], p0[2]);
	pMB->addVertex(pMB, p1[0], p1[1], p1[2]);

	//restore previous material
	if (pMB->materialsStack.size() > 0) {
		pMB->usingMaterialN = pMB->materialsStack.back();
		pMB->materialsStack.pop_back();
	}
	return 1;
}
int	ModelLoader::processTag_lineStart(ModelLoader* pML) {
	ModelBuilder* pMB = pML->pModelBuilder;
	Material mt;
	//save previous material in stack
	if (pMB->usingMaterialN >= 0) {
		pMB->materialsStack.push_back(pMB->usingMaterialN);
		memcpy(&mt, pMB->materialsList.at(pMB->usingMaterialN), sizeof(Material));
	}
	mt.primitiveType = GL_LINE_STRIP;
	fillProps_mt(&mt, pML->currentTag, pML);
	pMB->usingMaterialN = pMB->getMaterialN(pMB, &mt);
	//line starts
	pML->lineStartsAt = pMB->vertices.size();
	return 1;
}

int ModelLoader::processTag_line2mesh(ModelLoader* pML) {
	ModelBuilder* pMB = pML->pModelBuilder;
	std::string tagStr = pML->currentTag;
	GroupTransform gt;
	fillProps_gt(&gt, pMB, pML->currentTag);
	gt.flagSelection(&gt, &pMB->vertices, &pMB->triangles);
	//clone a copy
	std::vector<Vertex01*> vx1;
	std::vector<Triangle01*> tr1;
	gt.cloneFlagged(NULL, &vx1, &tr1, &pMB->vertices, &pMB->triangles);
	if (vx1.size() == 0) {
		mylog("ERROR in ModelLoader::processTag_line2mesh: vx1 size=0. %s\n", tagStr.c_str());
		return -1;
	}
	// build transform and inverted martrices
	mat4x4 transformMatrix;
	gt.buildTransformMatrix(&gt, &transformMatrix, 1);
	mat4x4 transformMatrixInverted;
	mat4x4_invert(transformMatrixInverted, transformMatrix);
	//move/rotate cloned
	gt.flagAll(&vx1, &tr1);
	gt.transformFlaggedMx(&vx1, &transformMatrixInverted, gt.normalsToo);

	//source line
	std::vector<Vertex01*> lineSrc;
	for (int vN = pMB->vertices.size() - 1; vN >= pMB->pCurrentGroup->fromVertexN; vN--) {
		Vertex01* pV = pMB->vertices.at(vN);
		lineSrc.insert(lineSrc.begin(), pV);
		pMB->vertices.pop_back();
	}
	int lineMaterialN = lineSrc.at(0)->materialN;

					//clone back to modelBuilder arrays
					//gt.cloneFlagged(pMB, &pMB->vertices, &pMB->triangles, &vx1, &tr1);

	//destination array
	std::vector<Vertex01*> vx2; //for 2-points lines
	std::vector<Vertex01*> vx3; //out
	for (int lN = 0; lN < (int)lineSrc.size() - 1; lN++) {
		//if (lN > 1)
		//	continue;

		vx2.clear();
		Polygon lineStep;
		lineStep.addVertex(&lineStep, lineSrc.at(lN));
		lineStep.addVertex(&lineStep, lineSrc.at(lN + 1));
		lineStep.finalizeLinePolygon(&lineStep);

		for (int tN = tr1.size() - 1; tN >= 0; tN--) {
			Polygon triangle;
			triangle.setTriangle(&triangle, tr1.at(tN), &vx1);
			Polygon intersection;

			int pointsN = Polygon::xy2pointsLineIntersection(&intersection, &lineStep, &triangle);
			if (pointsN > 1) {
				GroupTransform::flagAll(&intersection.vertices, NULL);
				GroupTransform::cloneFlagged(NULL, &vx2, NULL, &intersection.vertices, NULL);
			}
		}
		for (int pN = vx2.size() - 1; pN >= 0; pN--) {
			Vertex01* pV = vx2.at(pN);
			pV->aTangent[0] = v3xyLengthFromTo(lineStep.vertices.at(0)->aPos, pV->aPos);
		}
		std::sort(vx2.begin(), vx2.end(),
			[](Vertex01* pV0, Vertex01* pV1) {
				return pV0->aTangent[0] < pV1->aTangent[0]; });
		GroupTransform::flagAll(&vx2, NULL);
		GroupTransform::cloneFlagged(NULL, &vx3, NULL, &vx2, NULL);

		//break;
	}
	//round
	for (int pN = vx3.size() - 1; pN >= 0; pN--) {
		float* aPos = vx3.at(pN)->aPos;
		for (int i = 0; i < 3; i++)
			aPos[i] = (float)round(aPos[i] * 100.0f) / 100.0f;
	}
	//remove redundant points
	for (int pN = vx3.size() - 1; pN > 0; pN--) {
		Vertex01* pV = vx3.at(pN);
		Vertex01* pVprev = vx3.at(pN - 1);
		if (v3match(pV->aPos, pVprev->aPos)){
			delete vx3.at(pN);
			vx3.erase(vx3.begin() + pN);
		}
	}
	//finalize line			
	int vx3lastN = vx3.size() - 1;
	for (int pN = vx3lastN; pN >= 0; pN--) {
		Vertex01* pV = vx3.at(pN);
		if (pN == vx3lastN)
			pV->endOfSequence = 1;
		else if (pN == 0)
			pV->endOfSequence = -1;
		else
			pV->endOfSequence = 0;
		pV->subjN = pMB->usingSubjN;
		pV->materialN = lineMaterialN;
	}

	gt.flagAll(&vx3,NULL);
	//at this point we have bended line facing us

	float detachBy = 0;
	setFloatValue(&detachBy, "detachBy", tagStr);
	if (detachBy != 0) {
		mat4x4 mx;
		mat4x4_translate(mx, 0, 0, detachBy);
		gt.transformFlaggedMx(&vx3, &mx, false);
	}
	//move/rotate back
	gt.transformFlaggedMx(&vx3, &transformMatrix, gt.normalsToo);
	//clone back to modelBuilder arrays
	gt.cloneFlagged(pMB, &pMB->vertices, NULL, &vx3, NULL);
					/*
					mylog("==========final\n");
					for (int pN = vx3.size() - 1; pN >= 0; pN--) {
						Vertex01* pV = vx3.at(pN);
						mylog(" %d: ", pN);
						mylog_v3("", pV->aPos);
						mylog("\n");
					}
					*/
	//clear memory
	for (int i = vx1.size() - 1; i >= 0; i--)
		delete vx1.at(i);
	vx1.clear();
	for (int i = tr1.size() - 1; i >= 0; i--)
		delete tr1.at(i);
	tr1.clear();
	for (int i = vx2.size() - 1; i >= 0; i--)
		delete vx2.at(i);
	vx2.clear();
	for (int i = vx3.size() - 1; i >= 0; i--)
		delete vx3.at(i);
	vx3.clear();
	return 1;
}
int ModelLoader::processTag_a2group(ModelLoader* pML) {
	ModelBuilder* pMB = pML->pModelBuilder;
	std::string tagStr = pML->currentTag;
	GroupTransform gt;
	fillProps_gt(&gt, pMB, pML->currentTag);
	gt.flagSelection(&gt, &pMB->vertices, NULL);
	std::string applyTo = getStringValue("a2group", tagStr);
	//tuv coords
	Material* pMT = pMB->materialsList.at(pMB->usingMaterialN);
	int texN = pMT->uTex1mask;
	if (texN < 0)
		texN = pMT->uTex0;
	float xywh[4];
	TexCoords* pTC = NULL;
	if (varExists("xywh", tagStr)) {
		setFloatArray(xywh, 4, "xywh", tagStr);
		std::string flipStr = getStringValue("flip", tagStr);
		TexCoords tc;
		tc.set(texN, xywh[0], xywh[1], xywh[2], xywh[3], flipStr);
		pTC = &tc;
	}
	else
		if (varExists("xywh_GL", tagStr)) {
			setFloatArray(xywh, 4, "xywh_GL", tagStr);
			std::string flipStr = getStringValue("flip", tagStr);
			TexCoords tc;
			tc.set_GL(&tc, xywh[0], xywh[1], xywh[2], xywh[3], flipStr);
			pTC = &tc;
		}
	TexCoords* pTC2nm = NULL;
	if (varExists("xywh2nm", tagStr)) {
		setFloatArray(xywh, 4, "xywh2nm", tagStr);
		std::string flipStr = getStringValue("flip2nm", tagStr);
		TexCoords tc2nm;
		tc2nm.set(pMT->uTex2nm, xywh[0], xywh[1], xywh[2], xywh[3], flipStr);
		pTC2nm = &tc2nm;
	}
	else
		if (varExists("xywh2nm_GL", tagStr)) {
			setFloatArray(xywh, 4, "xywh2nm_GL", tagStr);
			std::string flipStr = getStringValue("flip2nm", tagStr);
			TexCoords tc2nm;
			tc2nm.set_GL(&tc2nm, xywh[0], xywh[1], xywh[2], xywh[3], flipStr);
			pTC2nm = &tc2nm;
		}

	pMB->applyTexture2flagged(&pMB->vertices, applyTo, pTC, pTC2nm);
	return 1;
}
int ModelLoader::processTag_color_as(ModelLoader* pML) {
	ModelBuilder* pMB = pML->pModelBuilder;
	std::string tagStr = pML->currentTag;

	std::string colorName = getStringValue("color_as", tagStr);
	std::string scope = getStringValue("scope", tagStr);
	std::vector<MyColor*>* pColorsList = &pMB->colorsList;
	if(scope.compare("app")==0)
		pColorsList = &MyColor::colorsList;
	//check if exists
	if (MyColor::findColor(colorName.c_str(), pColorsList) != NULL)
		return 0;
	//add new 
	MyColor* pCl = new MyColor();
	if (varExists("uColor_use", tagStr)) {
		std::string name = getStringValue("uColor_use", tagStr);
		MyColor* pCl0 = MyColor::findColor(name.c_str(),pColorsList);
		memcpy(pCl, pCl0, sizeof(MyColor));
	}
	else
	if (varExists("uColor", tagStr)) {
		unsigned int uintColor = 0;
		setUintColorValue(&uintColor, "uColor", tagStr);
		pCl->setUint32(uintColor);
	}
	myStrcpy_s(pCl->colorName, 32, colorName.c_str());
	pColorsList->push_back(pCl);
	return 1;
}


int ModelLoader::processTag_points2mesh(ModelLoader* pML) {
	ModelBuilder* pMB = pML->pModelBuilder;
	std::string tagStr = pML->currentTag;
	GroupTransform gt;
	fillProps_gt(&gt, pMB, tagStr);
	gt.flagSelection(&gt, &pMB->vertices, &pMB->triangles);
	//clone a copy
	std::vector<Vertex01*> vx1;
	std::vector<Triangle01*> tr1;
	gt.cloneFlagged(NULL, &vx1, &tr1, &pMB->vertices, &pMB->triangles);
	// build transform and inverted martrices
	mat4x4 transformMatrix;
	gt.buildTransformMatrix(&gt, &transformMatrix, 1);
	mat4x4 transformMatrixInverted;
	mat4x4_invert(transformMatrixInverted, transformMatrix);
	//move/rotate cloned
	gt.flagAll(&vx1, &tr1);
	gt.transformFlaggedMx(&vx1, &transformMatrixInverted, gt.normalsToo);

	//scan involved (group) points
	int pointsN = pMB->vertices.size();
	int startPointN = pMB->pCurrentGroup->fromVertexN;
	for (int pN = startPointN; pN < pointsN; pN++) {
		Vertex01* pV = pMB->vertices.at(pN);
		int newPointsN = 0;
		for (int tN = tr1.size() - 1; tN >= 0; tN--) {
			Polygon triangle;
			triangle.setTriangle(&triangle, tr1.at(tN), &vx1);
			Polygon intersection;

			int addedPoints = Polygon::xyPointProjection(&intersection, pV, &triangle);
			if (addedPoints > 0) {
				newPointsN += addedPoints;
				Vertex01* pV2 = intersection.vertices.at(0);
				//pV2 has material and tUVs from "2mesh"
				//copy coords
				for (int i = 0; i < 3; i++) {
					pV->aPos[i] = pV2->aPos[i];
					pV->aNormal[i] = pV2->aNormal[i];
				}
				break;
			}
		}
		if (newPointsN < 1) {
			mylog("ERROR in ModelLoader::processTag_points2mesh: point out of range, tag %s\n   file: %s\n", tagStr.c_str(), pML->fullPath.c_str());
		}
	}
	GroupTransform::flagGroup(pMB->pCurrentGroup, &pMB->vertices, &pMB->triangles);
	float detachBy = 0;
	setFloatValue(&detachBy, "detachBy", tagStr);
	if (detachBy != 0) {
		mat4x4 mx;
		mat4x4_translate(mx, 0, 0, detachBy);
		gt.transformFlaggedMx(&pMB->vertices, &mx, false);
	}
	//move/rotate back
	gt.transformFlaggedMx(&pMB->vertices, &transformMatrix, gt.normalsToo);

	//clear memory
	for (int i = vx1.size() - 1; i >= 0; i--)
		delete vx1.at(i);
	vx1.clear();
	for (int i = tr1.size() - 1; i >= 0; i--)
		delete tr1.at(i);
	tr1.clear();
	return 1;
}
int ModelLoader::processTag_lineEnd(ModelLoader* pML) {
	ModelBuilder* pMB = pML->pModelBuilder;
	//int lineMaterialN = pMB->usingMaterialN;
	int lineStartsAt = pML->lineStartsAt;

	pMB->vertices.back()->endOfSequence = 1;
	pMB->vertices.at(pML->lineStartsAt)->endOfSequence = -1; //first point
	pML->lineStartsAt = -1;
	//int lineMaterialN = pMB->usingMaterialN;
	//restore previous material
	if (pMB->materialsStack.size() > 0) {
		pMB->usingMaterialN = pMB->materialsStack.back();
		pMB->materialsStack.pop_back();
	}
	pMB->finalizeLine(pMB, lineStartsAt);
	/*
	//assuming that the line is uninterrupted
	int totalN = pMB->vertices.size();
	for (int vN = pMB->vertices.size() - 1; vN >= lineStartsAt; vN--) {
		Vertex01* pV = pMB->vertices.at(vN);
		if (pV->rad > 0) {
			//have a curve
			//next point
			Vertex01* pV2 = pV;
			if (vN < totalN - 1) {
				pV2 = pMB->vertices.at(vN + 1);
				float dist0 = v3lengthFromTo(pV->aPos, pV2->aPos);
				if (dist0 > pV->rad) {
					//dist > rad - add shifted back point
					pV2 = new Vertex01(*pV2);
					pMB->vertices.insert(pMB->vertices.begin() + vN + 1, pV2);
					float k = pV->rad / dist0;
					for (int i = 0; i < 3; i++) {
						float fromV = pV2->aPos[i] - pV->aPos[i];
						pV2->aPos[i] = pV->aPos[i] + (fromV * k);
					}
					pV2->endOfSequence = 0;
					pV2->rad = 0;
				}
			}
			//prev point
			Vertex01* pV0 = pV;
			if (vN > lineStartsAt) {
				pV0 = pMB->vertices.at(vN - 1);
				float dist0 = v3lengthFromTo(pV->aPos, pV0->aPos);
				if (dist0 > pV->rad) {
					//dist > rad - add shifted back point
					pV0 = new Vertex01(*pV0);
					pMB->vertices.insert(pMB->vertices.begin() + vN, pV0);
					vN++;
					float k = pV->rad / dist0;
					for (int i = 0; i < 3; i++) {
						float fromV = pV0->aPos[i] - pV->aPos[i];
						pV0->aPos[i] = pV->aPos[i] + (fromV * k);
					}
					pV0->endOfSequence = 0;
					pV0->rad = 0;
				}
			}
			if (pV != pV0 && pV != pV2) {
				delete pV;
				pMB->vertices.erase(pMB->vertices.begin() + vN);
			}
			//calculate pV0 dir from prev point
			lineDirFromN2N(pV0->aNormal, vN - 2, vN - 1, &pMB->vertices, lineStartsAt);
			//calculate pV2 dir to next point
			lineDirFromN2N(pV2->aNormal, vN, vN + 1, &pMB->vertices, lineStartsAt);

			fillLineCurveTo(vN, &pMB->vertices, lineStartsAt);
		}
		else if (pV->rad == -2) {
			//just curve
			delete pV;
			pMB->vertices.erase(pMB->vertices.begin() + vN);
			Vertex01* pV0 = pMB->vertices.at(vN - 1);
			Vertex01* pV2 = pMB->vertices.at(vN);
			//calculate pV0 dir from prev point
			lineDirFromN2N(pV0->aNormal, vN - 2, vN - 1, &pMB->vertices, lineStartsAt);
			//calculate pV2 dir to next point
			lineDirFromN2N(pV2->aNormal, vN, vN + 1, &pMB->vertices, lineStartsAt);

			fillLineCurveTo(vN, &pMB->vertices, lineStartsAt);
		}
		else if (pV->rad == -1) {
			//pass-through point
			//next point
			Vertex01* pV2 = pV;
			float dToV2 = 0;
			float dirToV2[3] = { 0,0,0 };
			if (vN < totalN - 1) {
				pV2 = pMB->vertices.at(vN + 1);
				dToV2 = v3lengthFromTo(pV->aPos, pV2->aPos);
				v3dirFromTo(dirToV2, pV->aPos, pV2->aPos);
				lineDirFromN2N(pV2->aNormal, vN + 1, vN + 2, &pMB->vertices, lineStartsAt);
			}
			//prev point
			Vertex01* pV0 = pV;
			float dFromV0 = 0;
			float dirFromV0[3] = { 0,0,0 };
			if (vN > lineStartsAt) {
				pV0 = pMB->vertices.at(vN - 1);
				dFromV0 = v3lengthFromTo(pV0->aPos, pV->aPos);
				v3dirFromTo(dirFromV0, pV0->aPos, pV->aPos);
				lineDirFromN2N(pV0->aNormal, vN - 2, vN - 1, &pMB->vertices, lineStartsAt);
			}
			float k0 = dToV2 / (dFromV0 + dToV2);
			float k2 = 1.0f - k0;
			for (int i = 0; i < 3; i++)
				pV->aNormal[i] = dirFromV0[i] * k0 + dirToV2[i] * k2;
			v3norm(pV->aNormal);

			fillLineCurveTo(vN+1, &pMB->vertices, lineStartsAt);
			fillLineCurveTo(vN, &pMB->vertices, lineStartsAt);
		}
	}
	*/
	return 1;
}
int ModelLoader::processTag_lastLineTexure(ModelLoader* pML) {
	ModelBuilder* pMB = pML->pModelBuilder;
	std::string tagStr = pML->currentTag;
	if (pML->lineStartsAt >= 0) {
		mylog("ERROR in ModelLoader::processTag_lineTexture: tag is inside of <line>: %s\n", tagStr.c_str());
		return -1;
	}
	float vStep_GL = getFloatValue("vStep_GL", tagStr);
	float vStep2nm_GL = getFloatValue("vStep2nm_GL", tagStr);
	std::vector<Vertex01*>* pVx = &pMB->vertices;
	int totalN = pVx->size();

	int startPoint = -1;
	for (int vN = totalN - 1; vN >= 0; vN--) {
		Vertex01* pV = pVx->at(vN);
		if (pV->endOfSequence != -1)
			continue;
		startPoint = vN;
		break;
	}
	if (startPoint < 0) {
		mylog("ERROR in ModelLoader::processTag_lineTexture: line startPoint not found: %s\n", tagStr.c_str());
		return -1;
	}
	for (int vN = startPoint+1; vN < totalN; vN++) {
		Vertex01* pV0 = pVx->at(vN-1);
		Vertex01* pV = pVx->at(vN);
		float dist0 = v3lengthFromTo(pV0->aPos, pV->aPos);
		pV->aTuv[1] = pV0->aTuv[1] + vStep_GL * dist0;
		pV->aTuv2[1] = pV0->aTuv2[1] + vStep2nm_GL * dist0;
		if (pV->endOfSequence > 0)
			break;
	}
	return 1;
}
int ModelLoader::processTag_replaceLineByGroup(ModelLoader* pML) {
	ModelBuilder* pMB = pML->pModelBuilder;
	//std::string tagStr = pML->currentTag;
	//move line and group to work vectors
	std::vector<Vertex01*> guideLine;
	std::vector<Vertex01*> vx0;
	std::vector<Triangle01*> tr0;
	//move group verts
	int totalN = pMB->vertices.size();
	for (int vN = totalN - 1; vN >= pMB->pCurrentGroup->fromVertexN; vN--) {
		Vertex01* pV = pMB->vertices.at(vN);
		myStrcpy_s(pV->marks, 128, "");
		pV->altN = vN;
		vx0.insert(vx0.begin(), pV);
		pMB->vertices.pop_back();
	}
	//move group triangles
	totalN = pMB->triangles.size();
	for (int tN = totalN - 1; tN >= pMB->pCurrentGroup->fromTriangleN; tN--) {
		Triangle01* pT = pMB->triangles.at(tN);
		myStrcpy_s(pT->marks, 128, "");
		tr0.insert(tr0.begin(), pT);
		pMB->triangles.pop_back();
	}
	//re-factor triangles
	int vxN = vx0.size();
	int trN = tr0.size();
	for (int tN =0; tN <trN; tN++) {
		Triangle01* pT = tr0.at(tN);
		for (int i = 0; i < 3; i++) {
			int idx = pT->idx[i];
			for (int vN = 0; vN < vxN; vN++) {
				Vertex01* pV = vx0.at(vN);
				if (pV->altN == idx) {
					pT->idx[i] = vN;
					break;
				}
			}
		}
	}
	//move guideLine verts
	totalN = pMB->vertices.size();
	for (int vN = totalN - 1; vN >= 0; vN--) {
		Vertex01* pV = pMB->vertices.at(vN);
		myStrcpy_s(pV->marks, 128, "");
		guideLine.insert(guideLine.begin(), pV);
		pMB->vertices.pop_back();
		if (pV->endOfSequence == -1) //lines's start
			break;
	}
	//redefine current group
	pMB->pCurrentGroup->fromVertexN = pMB->vertices.size();
	pMB->pCurrentGroup->fromTriangleN = pMB->triangles.size();

	pMB->replaceLineByGroup(pMB, &vx0, &tr0, &guideLine);

	//clear memory
	totalN = vx0.size();
	for (int vN = totalN - 1; vN >= 0; vN--)
		delete vx0.at(vN);
	vx0.clear();
	totalN = tr0.size();
	for (int tN = totalN - 1; tN >= 0; tN--)
		delete tr0.at(tN);
	tr0.clear();
	totalN = guideLine.size();
	for (int vN = totalN - 1; vN >= 0; vN--)
		delete guideLine.at(vN);
	guideLine.clear();

	return 1;
}
int ModelLoader::processTag_lineTip(ModelLoader* pML) {
	ModelBuilder* pMB = pML->pModelBuilder;
	std::string tagStr = pML->currentTag;
	int onLineEnd = 1; //0-start, 1-end
	if (getStringValue("tip", tagStr).compare("start") == 0)
		onLineEnd = 0;
	int vertsN = pMB->vertices.size();
	int lineStart = -1;
	int lineEnd = -1;
	//find line end
	Material* pMt = NULL;
	for (int vN = vertsN - 1; vN >= 0; vN--) {
		Vertex01* pV = pMB->vertices.at(vN);
		pMt = pMB->materialsList.at(pV->materialN);
		if (pMt->primitiveType == GL_LINES || pMt->primitiveType == GL_LINE_STRIP)
		if(strcmp(pMt->shaderType,"wire")==0) {
			lineEnd = vN;
			lineStart = vN-1;
			break;
		}
	}
	if (pMt == NULL)
		return -1;
	if (onLineEnd==0 && pMt->primitiveType == GL_LINE_STRIP) {
		//look for line start
		for (int vN = lineStart; vN >= 0; vN--) {
			Vertex01* pV = pMB->vertices.at(vN);
			if (pV->endOfSequence < 0) {
				lineStart = vN;
				lineEnd = vN + 1;
				break;
			}
		}
	}
	Vertex01* pV0 = pMB->vertices.at(lineStart);
	Vertex01* pV2 = pMB->vertices.at(lineEnd);
	if (onLineEnd == 0) { //on line start
		pV0 = pMB->vertices.at(lineEnd);
		pV2 = pMB->vertices.at(lineStart);
	}
	Vertex01* pV = new Vertex01(*pV2);
	pMB->vertices.push_back(pV);
	pV->endOfSequence = 0;
	v3dirFromTo(pV->aNormal, pV0->aPos, pV2->aPos);
	pV->subjN = pMB->usingSubjN;
	Material mt;
	memcpy(&mt, pMt, sizeof(Material));
	mt.primitiveType = GL_LINES;
	myStrcpy_s(mt.shaderType, 32, "phong");
	mt.uDiscardNormalsOut = 1;
	pV->materialN = pMB->getMaterialN(pMB, &mt);

	Vertex01* pV22 = new Vertex01(*pV);
	pMB->vertices.push_back(pV22);
	for (int i = 0; i < 3; i++)
		pV22->aPos[i] = pV22->aPos[i] + pV->aNormal[i] * mt.lineWidth * 0.5;
	return 1;
}
