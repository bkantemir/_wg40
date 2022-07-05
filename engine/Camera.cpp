#include "Camera.h"
#include "TheGame.h"
#include "utils.h"

extern TheGame theGame;
extern float degrees2radians;

float Camera::pickDistance(Camera* pCam) {
	float cotangentA = 1.0f / tanf(degrees2radians * pCam->viewRangeDg / 2);
	float cameraDistanceV = pCam->stageSize[1] / 2 * cotangentA;
	float cameraDistanceH = pCam->stageSize[0] / 2 * cotangentA / theGame.screenAspectRatio;
	pCam->focusDistance = (float)fmax(cameraDistanceV, cameraDistanceH);
	return pCam->focusDistance;
}
void Camera::setCameraPosition(Camera* pCam) {
	v3set(pCam->ownCoords.pos, 0, 0, -pCam->focusDistance);
	mat4x4_mul_vec4plus(pCam->ownCoords.pos, pCam->ownCoords.rotationMatrix, pCam->ownCoords.pos, 1);
	for (int i = 0; i < 3; i++)
		pCam->ownCoords.pos[i] += pCam->lookAtPoint[i];
}
void Camera::buildLookAtMatrix(Camera* pCam) {
	float cameraUp[4] = { 0,1,0,0 }; //y - up
	mat4x4_mul_vec4plus(cameraUp, pCam->ownCoords.rotationMatrix, cameraUp, 0);
	mat4x4_look_at(pCam->lookAtMatrix, pCam->ownCoords.pos, pCam->lookAtPoint, cameraUp);
}
void Camera::onScreenResize(Camera* pCam) {
	pickDistance(pCam);
	setCameraPosition(pCam);
	buildLookAtMatrix(pCam);
}
