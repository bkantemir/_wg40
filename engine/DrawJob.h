#pragma once
#include "Material.h"
#include <vector>

struct AttribRef //attribute reference/description
{
	unsigned int glVBOid = 0; //buffer object id
	int offset = 0; //variable's offset inside of VBO's element
	int stride = 0; //Buffer's element size in bytes
};

class DrawJob
{
public:
	Material mt;
	Material mtLayer2;
	Material mtShadow;

	unsigned int glVAOid = 0; //will hold data stream attributes mapping/positions
	int pointsN = 0; //N of points to draw
	unsigned int glEBOid = 0; //Element Buffer Object (vertex indices)

	//common attributes
	AttribRef aPos;
	AttribRef aNormal;
	AttribRef aTuv;
	AttribRef aTuv2; //for normal map
	AttribRef aTangent; //for normal map
	AttribRef aBinormal; //for normal map

	//static arrays (vectors) of all loaded DrawJobs, VBO ids
	static std::vector<DrawJob*> drawJobs;
	static std::vector<unsigned int> buffersIds;
public:
	DrawJob();
	virtual ~DrawJob(); //destructor
	static int cleanUp();
	static int newBufferId();
	//int buildVAO() { return buildVAOforShader(this, mt.shaderN); };
	static int buildVAOforShader(DrawJob* pDJ, int shaderN);
	static int attachAttribute(int varLocationInShader, int attributeSizeInFloats, AttribRef* pAttribRef);
	static int setAttribRef(AttribRef* pAR, unsigned int glVBOid, int offset, int stride);

	virtual int setDesirableOffsets(int* pStride, int shaderN, int VBOid) { return setDesirableOffsetsForSingleVBO(this, pStride, shaderN, VBOid); };
	static int setDesirableOffsetsForSingleVBO(DrawJob* pDJ, int* pStride, int shaderN, int VBOid);

	int execute(float* uMVP, float* uMV, float* uMM, float* uMVP4dm, float* uVectorToLight, float* uCameraPosition, 
		float sizeUnitPixelsSize = 0, Material* pMt=NULL) { return executeDrawJob(this, uMVP, uMV, uMM, uMVP4dm, uVectorToLight, 
			uCameraPosition, sizeUnitPixelsSize, pMt); };
	static int executeDrawJob(DrawJob* pDJ, float* uMVP, float* uMV, float* uMM, float* uMVP4dm, float* uVectorToLight, 
		float* uCameraPosition, float sizeUnitPixelsSize = 0, Material* pMt = NULL);
	static bool lineWidthIsImportant(int primitiveType);
};



