 #pragma once
#include "ModelBuilder1base.h"
#include "TexCoords.h"

class ModelBuilder : public ModelBuilder1base
{
public:
	virtual ~ModelBuilder();
	static int buildFace(ModelBuilder* pMB, std::string applyTo, VirtualShape* pVS, TexCoords* pTC = NULL, TexCoords* pTC2nm = NULL);
	static int buildBoxFace(ModelBuilder* pMB, std::string applyTo, VirtualShape* pVS, TexCoords* pTC = NULL, TexCoords* pTC2nm=NULL);
	static int buildRoundFace(ModelBuilder* pMB, std::string applyTo, VirtualShape* pVS, TexCoords* pTC = NULL, TexCoords* pTC2nm = NULL);
	static int buildBoxFacePlain(ModelBuilder* pMB, std::string applyTo, VirtualShape* pVS);
	static int buildBoxFaceTank(ModelBuilder* pMB, std::string applyTo, VirtualShape* pVS);
	static int cylinderWrap(ModelBuilder* pMB, VirtualShape* pVS, float angleFrom, float angleTo, TexCoords* pTC = NULL, TexCoords* pTC2nm = NULL);
	static int capWrap(ModelBuilder* pMB, VirtualShape* pVS, float angleFrom, float angleTo, TexCoords* pTC = NULL, TexCoords* pTC2nm = NULL);
	static int coneWrap(ModelBuilder* pMB, VirtualShape* pVS, float angleFrom, float angleTo, TexCoords* pTC = NULL, TexCoords* pTC2nm = NULL);
	static int groupApplyTexture(ModelBuilder* pMB, std::string applyTo, TexCoords* pTC = NULL, TexCoords* pTC2nm = NULL);
	static int applyTexture2flagged(std::vector<Vertex01*>* pVX, std::string applyTo, TexCoords* pTC = NULL, TexCoords* pTC2nm = NULL);
	static int normalsCalc(ModelBuilder* pMB);
	static int normalsMerge(ModelBuilder* pMB);
	static int replaceLineByGroup(ModelBuilder* pMB, std::vector<Vertex01*>* pVx, std::vector<Triangle01*>* pTr, std::vector<Vertex01*>* pGuideLine);
	static int lineDirFromN2N(float* pDir, int vN0, int vN2, std::vector<Vertex01*>* pVx, int lineStartsAt);
	static int fillLineCurveTo(int vN, std::vector<Vertex01*>* pVx, int lineStartsAt);
	static int finalizeLine(ModelBuilder* pMB,int lineStartsAt);

};

