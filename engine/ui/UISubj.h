#pragma once
#include <vector>
#include "Coords2D.h"
#include "linmath.h"
#include "DrawJob.h"

class UISubj
{
public:
	int d2parent = 0; //shift to parent object
	//char source[256] = "";
	char className[32] = "";
	char name[32] = "";

	Coords2D ownCoords;
	Coords2D absCoords;
	Coords2D ownSpeed;
	float scale[3] = { 1,1,1 };
	mat4x4 transformMatrix = { 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 };
	bool transformMatrixIsReady = false;

	int djStartN = 0; //first DJ N in DJs array (DrawJob::drawJobs)
	int djTotalN = 1; //number of DJs
	Material mt0;

	static int djNtex;
	static int djNclr;
	static mat4x4 mOrthoProjection;
	static std::vector<UISubj*> uiSubjs;

public:
	static int init();
	static int cleanUp();
	static int renderAll();
	static int onScreenResize();
	virtual int render() { return renderStandard(this); };
	static int renderStandard(UISubj* pUI);
	static int addTexSubj(std::string uiName, float x, float y, float w, float h, int uTex0);
	static int addClrSubj(std::string uiName, float x, float y, float w, float h, unsigned int rgba);
	static void buildModelMatrix(UISubj* pUI);
	/*
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
	*/
};
