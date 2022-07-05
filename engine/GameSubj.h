#pragma once
#include "Coords.h"
#include "Material.h"
#include "Camera.h"
#include <string>
#include <vector>

class GameSubj
{
public:
	std::vector<GameSubj*>* pSubjsSet = NULL; //which vector/set this subj belongs to
	int nInSubjsSet = 0; //subj's number in pSubjsSet
	int rootN = 0; //model's root N
	int d2parent = 0; //shift to parent object
	int d2headTo = 0; //shift to headTo object
	int totalElements = 0; //elements N in the model
	int totalNativeElements = 0; //elements N in the model when initially loaded
	char source[256] = "";
	char className[32] = "";
	Coords absCoords;
	mat4x4 absModelMatrixUnscaled = { 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 };
	mat4x4 absModelMatrix = { 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 };
	mat4x4 ownModelMatrixUnscaled = { 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 };

	char name[32]="";
	Coords ownCoords;
	Coords ownSpeed;
	float scale[3] = { 1,1,1 };
	int djStartN = 0; //first DJ N in DJs array (DrawJob::drawJobs)
	int djTotalN = 0; //number of DJs
	Material mt0;
	int mt0isSet = 0;
	int dropsShadow = 1;
public:
	GameSubj();
	virtual ~GameSubj();
	virtual GameSubj* clone() { return new GameSubj(*this); };
	void buildModelMatrix() { buildModelMatrix(this); };
	static void buildModelMatrix(GameSubj* pGS);
	virtual int moveSubj() { return applySpeeds(this); };
	static int applySpeeds(GameSubj* pGS);
	virtual int render(mat4x4 mViewProjection, Camera* pCam, float* dirToMainLight) { return renderStandard(this, mViewProjection, pCam, dirToMainLight); };
	static int renderStandard(GameSubj* pGS, mat4x4 mViewProjection, Camera* pCam, float* dirToMainLight);
	static int renderDepthMap(GameSubj* pGS, mat4x4 mViewProjection, Camera* pCam);
};
