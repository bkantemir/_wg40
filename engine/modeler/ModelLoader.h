#pragma once
#include "XMLparser.h"
#include "ModelBuilder.h"
#include "GroupTransform.h"
#include "MaterialAdjust.h"

class ModelLoader : public XMLparser
{
public:
	ModelBuilder* pModelBuilder = NULL;
	bool ownModelBuilder = false;
	std::vector<GameSubj*>* pSubjsVector = NULL;
	MaterialAdjust* pMaterialAdjust = NULL;
	int lineStartsAt = -1;
public:
	ModelLoader(std::vector<GameSubj*>* pSubjsVector0, int subjN, ModelBuilder* pMB, std::string filePath) : XMLparser(filePath) {
		pSubjsVector = pSubjsVector0;
		if (pMB != NULL) {
			ownModelBuilder = false;
			pModelBuilder = pMB;
		}
		else {
			ownModelBuilder = true;
			pModelBuilder = new ModelBuilder();
			pModelBuilder->lockGroup(pModelBuilder);
		}
		pModelBuilder->useSubjN(pModelBuilder,subjN);
	};
	virtual ~ModelLoader() {
		if (!ownModelBuilder)
			return;
		pModelBuilder->buildDrawJobs(pModelBuilder, pSubjsVector);
		delete pModelBuilder;
	};
	virtual int processTag() { return processTag(this); };
	static int processTag(ModelLoader* pML);
	static int processTag_a(ModelLoader* pML); //apply
	static int setIntValueFromHashMap(int* pInt, std::map<std::string, float> floatsHashMap, std::string varName, std::string tagStr);
	static int setFloatValueFromHashMap(float* pFloat, std::map<std::string, float> floatsHashMap, std::string varName, std::string tagStr);
	static int setTexture(ModelLoader* pML, int* pInt, std::string txName);
	static int setMaterialTextures(ModelLoader* pML, Material* pMT);
	static int fillProps_vs(VirtualShape* pVS, std::map<std::string, float> floatsHashMap, std::string tagStr); //virtual shape
	static int fillProps_mt(Material* pMT, std::string tagStr, ModelLoader* pML); //Material
	static int fillProps_gt(GroupTransform* pGS, ModelBuilder* pMB, std::string tagStr);
	static int loadModel(std::vector<GameSubj*>* pSubjsVector0, std::string sourceFile, std::string subjClass);
	static int processTag_clone(ModelLoader* pML);
	static int addMark(char* marks, std::string newMark);
	static int processTag_do(ModelLoader* pML);
	static int processTag_a2mesh(ModelLoader* pML);
	static int processTag_line2mesh(ModelLoader* pML);
	static int processTag_points2mesh(ModelLoader* pML);
	static int processTag_element(ModelLoader* pML);
	static int processTag_include(ModelLoader* pML);
	static int processTag_short(ModelLoader* pML);
	static int processTag_lineStart(ModelLoader* pML);
	static int processTag_lineEnd(ModelLoader* pML);
	static int processTag_a2group(ModelLoader* pML);
	static int processTag_color_as(ModelLoader* pML);
	static int processTag_lastLineTexure(ModelLoader* pML);
	static int processTag_replaceLineByGroup(ModelLoader* pML);
	static int processTag_lineTip(ModelLoader* pML);

};
