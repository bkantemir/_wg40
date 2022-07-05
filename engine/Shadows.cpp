#include "Shadows.h"
#include "TheGame.h"
#include "utils.h"

float Shadows::shadowLight = 0.5f;
int Shadows::depthMapTexN = -1;
Texture* Shadows::pDepthMap = NULL;
Camera Shadows::shadowCamera;
mat4x4 Shadows::mViewProjection;
float Shadows::sizeUnitPixelsSize = 1;

extern TheGame theGame;

float Shadows::uDepthBias[16]; //z-value shifts depeding on normal

int Shadows::init() {
	//depthMapTexN = Texture::generateTexture("depthMap", 1024,1024, NULL, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

	depthMapTexN = Texture::textures.size();
	Texture* pTex = new Texture();
	Texture::textures.push_back(pTex);
	pTex->size[0] = 1024;
	pTex->size[1] = 1024;
	pTex->source.assign("depthMap");

	glGenTextures(1, (GLuint*)&pTex->GLid);
	glBindTexture(GL_TEXTURE_2D, pTex->GLid);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		//pTex->size[0], pTex->size[1], 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexStorage2D(GL_TEXTURE_2D, 1, GL_R16F, pTex->size[0], pTex->size[1]);
	checkGLerrors("Depth tex");

	glBindTexture(GL_TEXTURE_2D, 0);
	/////////////////

	Texture::attachRenderBuffer(depthMapTexN, true);
	pDepthMap = Texture::textures.at(depthMapTexN);
	float worldRadius = 45;
	sizeUnitPixelsSize = pDepthMap->size[0] / (worldRadius * 2);

	//set up camera
	Camera* pCam = &shadowCamera;
	pCam->focusDistance = worldRadius;
	for (int i = 0; i < 3; i++) {
		//pCam->lookAtPoint[i] = ?;
		pCam->ownCoords.pos[i] = theGame.dirToMainLight[i] * worldRadius;
	}
	//build pCam->ownCoords.eulerDg[]
	pCam->ownCoords.eulerDg[0] = v3pitchDg(pCam->lookAtPoint);
	pCam->ownCoords.eulerDg[1] = v3yawDg(pCam->lookAtPoint);
	//build pCam->ownCoords.rotationMatrix
	pCam->ownCoords.eulerDgToMatrix(pCam->ownCoords.rotationMatrix, pCam->ownCoords.eulerDg);
	pCam->buildLookAtMatrix(pCam);

	//set up mViewProjection matrix
	mat4x4 mProjection;
	float nearClip = 0;
	float farClip = worldRadius * 2;
	mat4x4_ortho(mProjection, -worldRadius, worldRadius, -worldRadius, worldRadius, nearClip, farClip);
	mat4x4_mul(mViewProjection, mProjection, shadowCamera.lookAtMatrix);

	//calculate uBias (for depth map)
	int totalN = 16;
	float normalStep = 1.0f / (totalN +1);
	for (int i = 0; i < totalN; i++) {
		float normalZ = normalStep * (i + 1);
		float bias = sqrtf(1.0f - normalZ * normalZ) / normalZ;
		uDepthBias[i] = bias / 1024.0 * 3.0;
	}
	return 1;
}



int Shadows::renderDepthMap(std::vector<GameSubj*>* pGSubjs){
	Texture::setRenderToTexture(depthMapTexN);
				Texture* pT = Texture::textures.at(depthMapTexN);

	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //RenderScene
	int subjsN = pGSubjs->size();
	for (int subjN = 0; subjN < subjsN; subjN++) {
		GameSubj* pGS = pGSubjs->at(subjN);
		pGS->renderDepthMap(pGS, mViewProjection, &shadowCamera);
				//checkGLerrors("pGS->renderDepthMap");
	}

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	return 1;
}
