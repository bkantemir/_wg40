#pragma once
#include <stdlib.h>
#include "ModelBuilder1base.h"

class GroupTransform
{
public:
	//common actions
	float shift[4] = { 0,0,0,0 };
	float spinDg[3] = { 0,0,0 }; //in degrees
	float scale[3] = { 1,1,1 };
	char align[32] = "";
	char onThe[32] = ""; //left/right/etc.
	float headZto[3] = { 0,0,0 };
	//limit to
	Group01* pGroup = NULL; //NULL-all, can also be pCurrentGroup or pLastClosedGroup
	char mark[32] = ""; //must be in <>, use limit2mark()
	//position min/max
	float pMin[3]{ -1000000 ,-1000000 ,-1000000 };
	float pMax[3]{ 1000000 , 1000000 , 1000000 };
	//radius min/max from axcis
	float rMin[3]{ -1000000 ,-1000000 ,-1000000 }; //plane: yz,xz,xy
	float rMax[3]{ 1000000 , 1000000 , 1000000 };
	//special actions
	char action[32] = "";
	char applyTo[32] = "";
	float xyz[3]{ 0,0,0 };
	float refPoint[3]{ 0,0,0 };
	//bounding box
	float bbMin[3];
	float bbMax[3];
	float bbSize[3];
	float bbMid[3];
	bool normalsToo = false;

public:
	static void clear(GroupTransform* pGT);
	int executeGroupTransform(ModelBuilder1base* pMB) { return executeGroupTransform(pMB, this); };
	static int executeGroupTransform(ModelBuilder1base* pMB, GroupTransform* pGT);
	//set limits
	static void limit2mark(GroupTransform* pGT, std::string mark0);
	static void flagAll(std::vector<Vertex01*>* pVx, std::vector<Triangle01*>* pTr);
	static void flagGroup(Group01* pGroup, std::vector<Vertex01*>* pVx, std::vector<Triangle01*>* pTr);
	static int flagSelection(GroupTransform* pGT, std::vector<Vertex01*>* pVertices, std::vector<Triangle01*>* pTriangles);
	static int cloneFlagged(ModelBuilder1base* pMB) { return cloneFlagged(pMB,
		&pMB->vertices, &pMB->triangles, &pMB->vertices, &pMB->triangles); };
	static int cloneFlagged(ModelBuilder1base* pMB,
		std::vector<Vertex01*>* pVxDst, std::vector<Triangle01*>* pTrDst,
		std::vector<Vertex01*>* pVxSrc, std::vector<Triangle01*>* pTrSrc);
	static int buildBoundingBoxFlagged(GroupTransform* pGT, std::vector<Vertex01*>* pVx);
	static int refactorTriangles(std::vector<Triangle01*>* pTrDst, int trianglesN0dst, std::vector<Vertex01*>* pVxSrc);
	static int invertFlaggedTriangles(std::vector<Triangle01*>* pTr);
	static int invertFlagged(std::vector<Vertex01*>* pVx, std::vector<Triangle01*>* pTr);

	static int transformFlagged(GroupTransform* pGT, std::vector<Vertex01*>* pVxDst);
	static int transformFlaggedMx(std::vector<Vertex01*>* pVx, mat4x4* pTransformMatrix,bool normalsToo);
	static int transformFlaggedRated(GroupTransform* pGT, std::vector<Vertex01*>* pVx);
	static int buildTransformMatrix(GroupTransform* pGT, mat4x4* pTransformMatrix, float rate);
	static float getTransformRate(GroupTransform* pGT, Vertex01* pV, float ratePow);
	static float dist2rate(float dist, float ratePow);

};

