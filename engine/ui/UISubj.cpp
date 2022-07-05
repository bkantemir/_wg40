#include "UISubj.h"
#include "TheGame.h"

int UISubj::djNtex = -1;
int UISubj::djNclr = -1;
mat4x4 UISubj::mOrthoProjection;

std::vector<UISubj*> UISubj::uiSubjs;

extern TheGame theGame;
extern float degrees2radians;

int UISubj::init() {
    mat4x4_ortho(mOrthoProjection, -theGame.screenAspectRatio, theGame.screenAspectRatio, -1.f, 1.f, 1.f, -1.f);

 	//djTexture
    struct
    {
        float x, y, z, tu, tv;
    } vertsTex[4] =
    {
        { -0.5f,  0.5f, 0.f, 0.f, 0.f }, //top-left
        { -0.5f, -0.5f, 0.f, 0.f, 1.f }, //bottom-left
        {  0.5f,  0.5f, 0.f, 1.f, 0.f }, //top-right
        {  0.5f, -0.5f, 0.f, 1.f, 1.f }  //bottom-right
    };
    djNtex = DrawJob::drawJobs.size();
    DrawJob* pDJ = new DrawJob();
    pDJ->pointsN = 4;
    int vertex_buffer = DrawJob::newBufferId();
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertsTex), vertsTex, GL_STATIC_DRAW);
    //VBO is ready
    DrawJob::setAttribRef(&pDJ->aPos, vertex_buffer, 0, sizeof(float) * 5);
    DrawJob::setAttribRef(&pDJ->aTuv, vertex_buffer, sizeof(float) * 3, sizeof(float) * 5);

    Material* pMT = &pDJ->mt;
    pMT->primitiveType = GL_TRIANGLE_STRIP;
    pMT->uTex0 = 22;
    pMT->dropsShadow = 0;
    pMT->zBuffer = 0;
    pMT->zBufferUpdate = 0;
    pMT->assignShader("flat");

    DrawJob::buildVAOforShader(pDJ, pMT->shaderN);

	//djClr
    struct
    {
        float x, y, z;
    } vertsClr[4] =
    {
        { -0.5f,  0.5f, 0.f, }, //top-left
        { -0.5f, -0.5f, 0.f, }, //bottom-left
        {  0.5f,  0.5f, 0.f, }, //top-right
        {  0.5f, -0.5f, 0.f, }  //bottom-right
    };
    djNclr = DrawJob::drawJobs.size();
    pDJ = new DrawJob();
    pDJ->pointsN = 4;
    //unsigned int vertex_buffer;
    vertex_buffer = DrawJob::newBufferId();
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertsClr), vertsClr, GL_STATIC_DRAW);
    //VBO is ready
    DrawJob::setAttribRef(&pDJ->aPos, vertex_buffer, 0, sizeof(float) * 3);

    pMT = &pDJ->mt;
    pMT->primitiveType = GL_TRIANGLE_STRIP;
    pMT->uColor.setRGBA(255, 0, 0, 255);
    pMT->dropsShadow = 0;
    pMT->zBuffer = 0;
    pMT->zBufferUpdate = 0;
    pMT->assignShader("flat");

    DrawJob::buildVAOforShader(pDJ, pMT->shaderN);
    return 1;
}

void UISubj::buildModelMatrix(UISubj* pUI) {
    if (pUI->transformMatrixIsReady)
        return;
    memcpy(&pUI->absCoords, &pUI->ownCoords, sizeof(Coords2D));
    //count parent here
    //...

        float x_GL = pUI->absCoords.pos[0] / theGame.screenSize[0] * 2.0 - 1.0;
        float y_GL = -(pUI->absCoords.pos[1] / theGame.screenSize[1] * 2.0 - 1.0);
        float w_GL = pUI->scale[0] / theGame.screenSize[0] * 2;
        float h_GL = pUI->scale[1] / theGame.screenSize[1] * 2;
    mat4x4 m, mr;
    mat4x4_translate(m, x_GL, y_GL, 0);
    mat4x4_rotate_Z(mr, m, pUI->absCoords.aZdg * degrees2radians);
    mat4x4_scale_aniso(m, mr, w_GL, h_GL, 1);
    mat4x4_mul(pUI->transformMatrix, mOrthoProjection, m);
    pUI->transformMatrixIsReady = true;
}
int UISubj::renderStandard(UISubj* pUI) {
    if (theGame.screenSize[0] == 0)
        return 0;
    buildModelMatrix(pUI);

     //render subject
    for (int i = 0; i < pUI->djTotalN; i++) {
        DrawJob* pDJ = DrawJob::drawJobs.at(pUI->djStartN + i);
        Material* pMt = &pDJ->mt;
        if (i == 0)
            pMt = &pUI->mt0;
        pDJ->execute((float*)pUI->transformMatrix, NULL, NULL, NULL, NULL, NULL, 1, pMt);
    }
    return 1;
}
int UISubj::addTexSubj(std::string uiName, float x, float y, float w, float h, int uTex0) {
    UISubj* pUI = new UISubj();
    uiSubjs.push_back(pUI);

    pUI->ownCoords.pos[0] = x;
    pUI->ownCoords.pos[1] = y;
    pUI->scale[0] = w;
    pUI->scale[1] = h;
    pUI->djStartN = djNtex;
    memcpy(&pUI->mt0, &DrawJob::drawJobs.at(pUI->djStartN)->mt, sizeof(Material));
    pUI->mt0.uTex0 = uTex0;
    return (uiSubjs.size()-1);
}
int UISubj::addClrSubj(std::string uiName, float x, float y, float w, float h, unsigned int rgba) {
    UISubj* pUI = new UISubj();
    uiSubjs.push_back(pUI);

    pUI->ownCoords.pos[0] = x;
    pUI->ownCoords.pos[1] = y;
    pUI->scale[0] = w;
    pUI->scale[1] = h;

    pUI->djStartN = djNclr;
    memcpy(&pUI->mt0, &DrawJob::drawJobs.at(pUI->djStartN)->mt, sizeof(Material));
    pUI->mt0.uColor.setUint32(rgba);

    return (uiSubjs.size() - 1);
}
int UISubj::renderAll() {
    int subjsN = uiSubjs.size();
    for (int subjN = 0; subjN < subjsN; subjN++) {
        UISubj* pUI = uiSubjs.at(subjN);

        pUI->render();
    }
    return 1;
}
int UISubj::onScreenResize() {
    int subjsN = uiSubjs.size();
    for (int subjN = 0; subjN < subjsN; subjN++) {
        UISubj* pUI = uiSubjs.at(subjN);
        pUI->transformMatrixIsReady = false;
    }
    return 1;
}
int UISubj::cleanUp() {
    int subjsN = uiSubjs.size();
    for (int subjN = 0; subjN < subjsN; subjN++)
        delete uiSubjs.at(subjN);
    uiSubjs.clear();
    return 1;
}