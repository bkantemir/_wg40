#include <android/log.h>
#include "stdio.h"
#include "TheGame.h"
#include <sys/stat.h>	//mkdir for Android

extern struct android_app* androidApp;
extern const ASensor* accelerometerSensor;
extern ASensorEventQueue* sensorEventQueue;

extern EGLDisplay androidDisplay;
extern EGLSurface androidSurface;
extern TheGame theGame;

void mylog(const char* _Format, ...) {
#ifdef _DEBUG
    char outStr[1024];
    va_list _ArgList;
    va_start(_ArgList, _Format);
    vsprintf(outStr, _Format, _ArgList);
    __android_log_print(ANDROID_LOG_INFO, "mylog", outStr, NULL);
    va_end(_ArgList);
#endif
};

void mySwapBuffers() {
	eglSwapBuffers(androidDisplay, androidSurface);
}
void myPollEvents() {
	// Read all pending events.
	int ident;
	int events;
	struct android_poll_source* source;

	// If not animating, we will block forever waiting for events.
	// If animating, we loop until all events are read, then continue
	// to draw the next frame of animation.
	while ((ident = ALooper_pollAll(0, NULL, &events,
		(void**)&source)) >= 0) {

		// Process this event.
		if (source != NULL) {
			source->process(androidApp, source);
		}

		// If a sensor has data, process it now.
		if (ident == LOOPER_ID_USER) {
			if (accelerometerSensor != NULL) {
				ASensorEvent event;
				while (ASensorEventQueue_getEvents(sensorEventQueue,
					&event, 1) > 0) {
					//LOGI("accelerometer: x=%f y=%f z=%f",
					//	event.acceleration.x, event.acceleration.y,
					//	event.acceleration.z);
				}
			}
		}

		// Check if we are exiting.
		if (androidApp->destroyRequested != 0) {
			theGame.bExitGame = true;
			break;
		}
	}
	//check screen size
	EGLint w, h;
	eglQuerySurface(androidDisplay, androidSurface, EGL_WIDTH, &w);
	eglQuerySurface(androidDisplay, androidSurface, EGL_HEIGHT, &h);
	theGame.onScreenResize(w, h);
}
int myFopen_s(FILE** pFile, const char* filePath, const char* mode) {
	*pFile = fopen(filePath, mode);
	if (*pFile == NULL) {
		mylog("ERROR: can't open file %s\n", filePath);
		return -1;
	}
	return 1;
}
int myMkDir(const char* outPath) {
	struct stat info;
	if (stat(outPath, &info) == 0)
		return 0; //exists already
	int status = mkdir(outPath, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	if (status == 0)
		return 1; //Successfully created
	mylog("ERROR creating, status=%d, errno: %s.\n", status, std::strerror(errno));
	return -1;
}
void myStrcpy_s(char* dst, int maxSize, const char* src) {
	strcpy(dst, src);
	//fill tail by zeros
	int strLen = strlen(dst);
	if (strLen < maxSize)
		for (int i = strLen; i < maxSize; i++)
			dst[i] = 0;
}
