#include "GameSubj.h"
#include "platform.h"
#include "utils.h"
#include "TheGame.h"
#include "DrawJob.h"
#include "MaterialAdjust.h"
#include "Shadows.h"
#include "Shader.h"

extern TheGame theGame;
extern float degrees2radians;

GameSubj::GameSubj() {
}
GameSubj::~GameSubj() {
}
void GameSubj::buildModelMatrix(GameSubj* pGS) {
    mat4x4_translate(pGS->ownModelMatrixUnscaled, pGS->ownCoords.pos[0], pGS->ownCoords.pos[1], pGS->ownCoords.pos[2]);
    //rotation order: Z-X-Y
    mat4x4_mul(pGS->ownModelMatrixUnscaled, pGS->ownModelMatrixUnscaled, pGS->ownCoords.rotationMatrix);

    if(pGS->d2parent == 0)
        memcpy(pGS->absModelMatrixUnscaled, pGS->ownModelMatrixUnscaled, sizeof(mat4x4));
    else {
        GameSubj* pParent = pGS->pSubjsSet->at(pGS->nInSubjsSet - pGS->d2parent);
        mat4x4_mul(pGS->absModelMatrixUnscaled, pParent->absModelMatrixUnscaled, pGS->ownModelMatrixUnscaled);
    }
    if (v3equals(pGS->scale, 1))
        memcpy(pGS->absModelMatrix, pGS->absModelMatrixUnscaled, sizeof(mat4x4));
    else
        mat4x4_scale_aniso(pGS->absModelMatrix, pGS->absModelMatrixUnscaled, pGS->scale[0], pGS->scale[1], pGS->scale[2]);

    //update absCoords
    if (pGS->d2parent == 0)
        memcpy(&pGS->absCoords, &pGS->ownCoords, sizeof(Coords));
    else {
        Coords::getPositionFromMatrix(pGS->absCoords.pos, pGS->absModelMatrixUnscaled);
    }
}
int GameSubj::applySpeeds(GameSubj* pGS) {
    bool angleChanged = false;
    for(int i=0;i<3;i++)
        if (pGS->ownSpeed.eulerDg[i] != 0) {
            angleChanged = true;
            pGS->ownCoords.eulerDg[i] += pGS->ownSpeed.eulerDg[i];
            if (pGS->ownCoords.eulerDg[i] > 180.0)
                pGS->ownCoords.eulerDg[i] -= 360.0;
            else if (pGS->ownCoords.eulerDg[i] <= -180.0)
                pGS->ownCoords.eulerDg[i] += 360.0;
        }
    if(angleChanged)
        Coords::eulerDgToMatrix(pGS->ownCoords.rotationMatrix, pGS->ownCoords.eulerDg);
    return 1;
}

int GameSubj::renderStandard(GameSubj* pGS, mat4x4 mViewProjection, Camera* pCam, float* dirToMainLight) {
    //build mMVP4dm (shadow MVP) matrix for given subject
    mat4x4 mMVP4dm;
    mat4x4_mul(mMVP4dm, Shadows::mViewProjection, pGS->absModelMatrix);

    //build MVP matrix for given subject
    mat4x4 mMVP;
    mat4x4_mul(mMVP, mViewProjection, pGS->absModelMatrix);
    //build Model-View (rotation) matrix for normals
    mat4x4 mMV4x4;
    mat4x4_mul(mMV4x4, pCam->lookAtMatrix, pGS->absModelMatrixUnscaled);
    //convert to 3x3 matrix
    float mMV3x3[3][3];
    for (int y = 0; y < 3; y++)
        for (int x = 0; x < 3; x++)
            mMV3x3[y][x] = mMV4x4[y][x];
    //subj's distance from camera
    float cameraSpacePos[4];
    mat4x4_mul_vec4plus(cameraSpacePos, pCam->lookAtMatrix, pGS->absCoords.pos, 1);
    float zDistance = abs(cameraSpacePos[2]);
    float cotangentA = 1.0f / tanf(degrees2radians * pCam->viewRangeDg / 2.0f);
    float halfScreenVertSizeInUnits = zDistance / cotangentA;
    float sizeUnitPixelsSize = (float)theGame.screenSize[1] / 2.0f / halfScreenVertSizeInUnits;
    //render subject
    for (int i = 0; i < pGS->djTotalN; i++) {
        DrawJob* pDJ = DrawJob::drawJobs.at(pGS->djStartN + i);
        Material* pMt = &pDJ->mt;
        if (i == 0)
            if (pGS->mt0isSet > 0)
                pMt = &pGS->mt0;
        pDJ->execute((float*)mMVP, *mMV3x3, (float*)pGS->absModelMatrix, (float*)mMVP4dm, dirToMainLight, pCam->ownCoords.pos,
            sizeUnitPixelsSize, pMt);

        //have 2-nd layer ?
        pMt = &pDJ->mtLayer2;
        if (pMt->shaderN >= 0) {
            pDJ->execute((float*)mMVP, *mMV3x3, (float*)pGS->absModelMatrix, (float*)mMVP4dm, dirToMainLight, pCam->ownCoords.pos,
                sizeUnitPixelsSize, pMt);
        }
    }
    return 1;
}
int GameSubj::renderDepthMap(GameSubj* pGS, mat4x4 mViewProjection, Camera* pCam) {
    if (pGS->dropsShadow < 1)
        return 0;
    mat4x4 mMVP;
    mat4x4_mul(mMVP, mViewProjection, pGS->absModelMatrix);
    mat4x4 mMV4x4;
    mat4x4_mul(mMV4x4, pCam->lookAtMatrix, pGS->absModelMatrixUnscaled);
    //convert to 3x3 matrix
    float mMV3x3[3][3];
    for (int y = 0; y < 3; y++)
        for (int x = 0; x < 3; x++)
            mMV3x3[y][x] = mMV4x4[y][x];
    //render subject
    for (int i = 0; i < pGS->djTotalN; i++) {
        DrawJob* pDJ = DrawJob::drawJobs.at(pGS->djStartN + i);
        if (pDJ->mt.dropsShadow < 1)
            continue;
        Material* pMT = &pDJ->mtShadow;
                    //Shader* pSh = Shader::shaders.at(pMT->shaderN);

        pDJ->executeDrawJob(pDJ, (float*)mMVP, *mMV3x3, NULL, NULL, NULL, NULL,
            Shadows::sizeUnitPixelsSize, pMT);
    }
    return 1;
}


