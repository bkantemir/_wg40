#include "TheGame.h"
#include "platform.h"
#include "utils.h"
#include "linmath.h"
#include "Texture.h"
#include "Shader.h"
#include "DrawJob.h"
#include "ModelBuilder.h"
#include "TexCoords.h"
#include "ModelLoader.h"
#include "model_car/CarWheel.h"
#include "Shadows.h"


extern std::string filesRoot;
extern float degrees2radians;

std::vector<GameSubj*> TheGame::gameSubjs;

int TheGame::getReady() {
    bExitGame = false;
    Shader::loadShaders();
    UISubj::init();

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);//default
    glFrontFace(GL_CCW);//default

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_TRUE);

    int subjN = ModelLoader::loadModel(&gameSubjs, "/dt/models/cars/999_1935_deusenberg_ssj_speedster/root01.txt", "");
    GameSubj* pGS = gameSubjs.at(subjN);
    pGS->ownSpeed.eulerDg[1] = 1;

    //===== set up camera
    v3set(mainCamera.ownCoords.eulerDg, 25, 180, 0); //set camera angles/orientation
    Coords::eulerDgToMatrix(mainCamera.ownCoords.rotationMatrix, mainCamera.ownCoords.eulerDg);

    mainCamera.viewRangeDg = 20;
    mainCamera.stageSize[0] = 90;
    mainCamera.stageSize[1] = 60;
    memcpy(mainCamera.lookAtPoint, pGS->ownCoords.pos, sizeof(float) * 3);
    mainCamera.onScreenResize();

    //===== set up light
    v3set(dirToMainLight, -1, 1, 0.5);
    v3norm(dirToMainLight);

    Shadows::init();
    //======= set UI
    //UISubj::addTexSubj("test", 210, 210, 400, 400, Shadows::depthMapTexN);

    return 1;
}
int TheGame::drawFrame() {
    myPollEvents(); 

    //scan subjects
    int subjsN = gameSubjs.size();
    for (int subjN = 0; subjN < subjsN; subjN++) {
        GameSubj* pGS = gameSubjs.at(subjN);
        //behavior - apply rotation speed
        pGS->moveSubj();
        //prepare subject for rendering
        pGS->buildModelMatrix(pGS);
    }

    Shadows::renderDepthMap(&gameSubjs);

    //set render to screen
    glViewport(0, 0, screenSize[0], screenSize[1]);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //glClearColor(0.0, 0.0, 0.5, 1.0);
    glDepthMask(GL_TRUE);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    mat4x4 mProjection, mViewProjection;
    float nearClip = mainCamera.focusDistance - 55;
    float farClip = mainCamera.focusDistance + 55;
    if (nearClip < 0) nearClip = 0;
    //mat4x4_ortho(mProjection, -50, 50, -30, 30, nearClip, farClip);
    mat4x4_perspective(mProjection, mainCamera.viewRangeDg * degrees2radians, screenAspectRatio, nearClip, farClip);
    mat4x4_mul(mViewProjection, mProjection, mainCamera.lookAtMatrix);
    mViewProjection[1][3] = 0; //keystone effect

    //draw subjects
    for (int subjN = 0; subjN < subjsN; subjN++) {
        GameSubj* pGS = gameSubjs.at(subjN);

        pGS->render(mViewProjection, &mainCamera, dirToMainLight);
    }
    
    UISubj::renderAll();

    //synchronization
    while (1) {
        long long int currentMillis = getSystemMillis();
        long long int millisSinceLastFrame = currentMillis - lastFrameMillis;
        if (millisSinceLastFrame >= millisPerFrame) {
            lastFrameMillis = currentMillis;
            break;
        }
    }
    mySwapBuffers();
    return 1;
}
int TheGame::cleanUp() {
    int itemsN = gameSubjs.size();
    //delete all UISubjs
    for (int i = 0; i < itemsN; i++)
        delete gameSubjs.at(i);
    gameSubjs.clear();
    //clear all other classes
    MaterialAdjust::cleanUp();
    Texture::cleanUp();
    Shader::cleanUp();
    DrawJob::cleanUp();
    MyColor::cleanUp();
    UISubj::cleanUp();
    return 1;
}
int TheGame::onScreenResize(int width, int height) {
    if (screenSize[0] == width && screenSize[1] == height)
        return 0;
    screenSize[0] = width;
    screenSize[1] = height;
    screenAspectRatio = (float)width / height;
    glViewport(0, 0, width, height);
    mainCamera.onScreenResize();
    UISubj::onScreenResize();
    mylog(" screen size %d x %d\n", width, height);
    return 1;
}
GameSubj* TheGame::newGameSubj(std::string subjClass) {
    GameSubj* pGS = NULL;
    if (subjClass.compare("") == 0)
        pGS = (new GameSubj());
    else if (subjClass.find("Car") == 0) {
        if (subjClass.compare("CarWheel") == 0)
            pGS = (new CarWheel());
    }
    if (pGS == NULL) {
        mylog("ERROR in TheGame::newGameSubj. %s class not found\n", subjClass.c_str());
        return NULL;
    }
    myStrcpy_s(pGS->className, 32, subjClass.c_str());
    return pGS;
}
int TheGame::run_blurred_white_noise() {
    Shader::loadShaders();

    //int wh[2] = { 1024,1024 };// 64, 64};
    int wh[2] = { 64, 64};
    int bytesPerPixel = 4;
    unsigned char* imgData = new unsigned char[wh[1] * wh[0] * 4];
    for (int y = 0; y < wh[1]; y++)
        for (int x = 0; x < wh[0]; x++) {
            int idx = (y * wh[1] + x) * bytesPerPixel;
            for (int i = 0; i < 4; i++)
                imgData[idx + i] = getRandom(0, 1) * 255;
        }
    //background generated, generate texture:
    int texN = Texture::generateTexture("", wh[0], wh[1], imgData);
    Texture::attachRenderBuffer(texN);
    Texture::setRenderToTexture(texN);

    //Texture* pT = Texture::textures.at(texN);

    mat4x4 mProjection, mMVP;
    mat4x4_ortho(mProjection, -wh[0] / 2, wh[0] / 2, -wh[1] / 2, wh[1] / 2, -1.f, 1.f);
    unsigned char RGBA[4];

    glDepthMask(GL_FALSE);

    //create a simple 1x1 ucolor square
    GameSubj* pSquare = new GameSubj();
    gameSubjs.push_back(pSquare);
    ModelBuilder* pMB = new ModelBuilder();
    pMB->useSubjN(pMB,gameSubjs.size() - 1);
    //define VirtualShape
    VirtualShape vs;
    vs.setShapeType("box");
    v3set(vs.whl, 1, 1, 0);
    Material mt;
    //define material - flat red
    //mt.setShaderType("flat");
    mt.primitiveType = GL_TRIANGLES;
    mt.uColor.setRGBA(255, 0, 0, 255); //red
    mt.assignShader("flat");
    //pMB->useMaterial(&mt);
    pMB->usingMaterialN = pMB->getMaterialN(pMB, &mt);
    pMB->buildBoxFace(pMB, "front", &vs);
    pMB->buildDrawJobs(pMB, &gameSubjs);
    delete pMB;
    //copy mt to pAltMaterial
    //pSquare->pAltMaterial = new Material(mt);
    memcpy(&pSquare->mt0, &mt, sizeof(Material));
    pSquare->mt0isSet = 1;
    //--end of pSquare
    DrawJob* pDJ = DrawJob::drawJobs.back();

    //-------------blurLevel 3
    //glClearColor(1.0, 0.0, 1.0, 1.0);
    //glClear(GL_COLOR_BUFFER_BIT);

    int blurLevel = 3;
    std::string fileName = "wn64_blur3";
    int dotSize = blurLevel * 2;
    v3set(pSquare->scale, dotSize, dotSize, 1);

    for (int i = 0; i < 500; i++) {
        pSquare->ownCoords.pos[0] = getRandom(-wh[0] / 2, wh[0] / 2);
        pSquare->ownCoords.pos[1] = getRandom(-wh[1] / 2, wh[1] / 2);
        pSquare->ownCoords.eulerDg[2] = getRandom(0, 90);
        Coords::eulerDgToMatrix(pSquare->ownCoords.rotationMatrix, pSquare->ownCoords.eulerDg);
        for (int ch = 0; ch < 4; ch++)
            RGBA[ch] = (unsigned char)(getRandom(0, 1) * 255);
        pSquare->mt0.uColor.setRGBA(RGBA);

        //prepare subject for rendering
        pSquare->buildModelMatrix(pSquare);
        //build MVP matrix for given subject
        mat4x4_mul(mMVP, mProjection, pSquare->absModelMatrix);
        //render subject
        pDJ->execute((float*)mMVP, NULL, NULL, NULL, NULL, NULL, 1, &pSquare->mt0);
    }
    Texture::getImageFromTexture(texN, imgData);
    Texture::blurRGBA(imgData, wh[0], wh[1], blurLevel);

    Texture::saveBMP("/dt/out/" + fileName + ".bmp", imgData, wh[0], wh[1]);
    //-------------blurLevel 2
    blurLevel = 2;
    fileName = "wn64_blur2";
    dotSize = blurLevel * 2;
    v3set(pSquare->scale, dotSize, dotSize, 1);

    for (int i = 0; i < 500; i++) {
        pSquare->ownCoords.pos[0] = getRandom(-wh[0] / 2, wh[0] / 2);
        pSquare->ownCoords.pos[1] = getRandom(-wh[1] / 2, wh[1] / 2);
        pSquare->ownCoords.eulerDg[2] = getRandom(0, 90);
        Coords::eulerDgToMatrix(pSquare->ownCoords.rotationMatrix, pSquare->ownCoords.eulerDg);
        for (int ch = 0; ch < 4; ch++)
            RGBA[ch] = (unsigned char)(getRandom(0, 1) * 255);
        pSquare->mt0.uColor.setRGBA(RGBA);

        //prepare subject for rendering
        pSquare->buildModelMatrix(pSquare);
        //build MVP matrix for given subject
        mat4x4_mul(mMVP, mProjection, pSquare->absModelMatrix);
        //render subject
        pDJ->execute((float*)mMVP, NULL, NULL, NULL, NULL, NULL, 1, &pSquare->mt0);
    }
    Texture::getImageFromTexture(texN, imgData);
    Texture::blurRGBA(imgData, wh[0], wh[1], blurLevel);

    Texture::saveBMP("/dt/out/" + fileName + ".bmp", imgData, wh[0], wh[1]);
    //-------------blurLevel 1
    blurLevel = 1;
    fileName = "wn64_blur1";
    dotSize = blurLevel * 2;
    v3set(pSquare->scale, dotSize, dotSize, 1);

    for (int i = 0; i < 500; i++) {
        pSquare->ownCoords.pos[0] = getRandom(-wh[0] / 2, wh[0] / 2);
        pSquare->ownCoords.pos[1] = getRandom(-wh[1] / 2, wh[1] / 2);
        pSquare->ownCoords.eulerDg[2] = getRandom(0, 90);
        Coords::eulerDgToMatrix(pSquare->ownCoords.rotationMatrix, pSquare->ownCoords.eulerDg);
        for (int ch = 0; ch < 4; ch++)
            RGBA[ch] = (unsigned char)(getRandom(0, 1) * 255);
        pSquare->mt0.uColor.setRGBA(RGBA);

        //prepare subject for rendering
        pSquare->buildModelMatrix(pSquare);
        //build MVP matrix for given subject
        mat4x4_mul(mMVP, mProjection, pSquare->absModelMatrix);
        //render subject
        pDJ->execute((float*)mMVP, NULL, NULL, NULL, NULL, NULL, 1, &pSquare->mt0);
    }
    Texture::getImageFromTexture(texN, imgData);
    Texture::blurRGBA(imgData, wh[0], wh[1], blurLevel);

    Texture::saveBMP("/dt/out/" + fileName + ".bmp", imgData, wh[0], wh[1]);

    delete[] imgData;
    mylog("Ready\n");
    return 1;
}
int TheGame::run() {
    //run_blurred_white_noise();
    
    getReady();
    while (!bExitGame) {
        drawFrame();
    }
    cleanUp();
    
    return 1;
}
