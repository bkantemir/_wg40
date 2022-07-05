#pragma once
#include <vector>
#include "GameSubj.h"
#include "UISubj.h"
#include "Camera.h"

class TheGame
{
public:
	int screenSize[2] = {0,0};
	float screenAspectRatio = 1;
	//synchronization
	long long int lastFrameMillis = 0;
	int targetFPS = 30;
	int millisPerFrame = 1000 / targetFPS;

	bool bExitGame;
	Camera mainCamera;
	float dirToMainLight[4] = { 1,1,1,0 };

	//static arrays (vectors) of active GameSubjs
	static std::vector<GameSubj*> gameSubjs;
public:
	int run();
	int getReady();
	int drawFrame();
	int cleanUp();
	int onScreenResize(int width, int height);
	static GameSubj* newGameSubj(std::string subjClass);

	static int run_blurred_white_noise();
};

