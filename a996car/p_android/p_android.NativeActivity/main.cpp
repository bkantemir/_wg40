#include "platform.h"
#include "TheGame.h"

#include <string>
#include <vector>

#include <sys/stat.h>	//mkdir for Android

std::string filesRoot;

TheGame theGame;

struct android_app* androidApp;

ASensorManager* sensorManager;
const ASensor* accelerometerSensor;
ASensorEventQueue* sensorEventQueue;

EGLDisplay androidDisplay;
EGLSurface androidSurface;
EGLContext androidContext;

/**
* Initialize an EGL context for the current display.
*/
static int engine_init_display(struct engine* engine) {
	// initialize OpenGL ES and EGL

	/*
	* Here specify the attributes of the desired configuration.
	* Below, we select an EGLConfig with at least 8 bits per color
	* component compatible with on-screen windows
	*/
	const EGLint attribs[] = {
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_BLUE_SIZE, 8,
		EGL_GREEN_SIZE, 8,
		EGL_RED_SIZE, 8,
		EGL_DEPTH_SIZE, 16,
		EGL_NONE
	};
	EGLint format;
	EGLint numConfigs;
	EGLConfig config;
	EGLSurface surface;
	EGLContext context;

	EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

	eglInitialize(display, 0, 0);

	/* Here, the application chooses the configuration it desires. In this
	* sample, we have a very simplified selection process, where we pick
	* the first EGLConfig that matches our criteria */
	eglChooseConfig(display, attribs, &config, 1, &numConfigs);

	/* EGL_NATIVE_VISUAL_ID is an attribute of the EGLConfig that is
	* guaranteed to be accepted by ANativeWindow_setBuffersGeometry().
	* As soon as we picked a EGLConfig, we can safely reconfigure the
	* ANativeWindow buffers to match, using EGL_NATIVE_VISUAL_ID. */
	eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);

	ANativeWindow_setBuffersGeometry(androidApp->window, 0, 0, format);

	surface = eglCreateWindowSurface(display, config, androidApp->window, NULL);

	EGLint contextAttribs[] =
	{
		EGL_CONTEXT_CLIENT_VERSION, 3,
		EGL_NONE
	};
	context = eglCreateContext(display, config, NULL, contextAttribs);


	if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) {
		mylog("ERROR: Unable to eglMakeCurrent");
		return -1;
	}

	androidDisplay = display;
	androidContext = context;
	androidSurface = surface;

	return 0;
}

/**
* Tear down the EGL context currently associated with the display.
*/
static void engine_term_display() {

	if (androidDisplay != EGL_NO_DISPLAY) {
		eglMakeCurrent(androidDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
		if (androidContext != EGL_NO_CONTEXT) {
			eglDestroyContext(androidDisplay, androidContext);
		}
		if (androidSurface != EGL_NO_SURFACE) {
			eglDestroySurface(androidDisplay, androidSurface);
		}
		eglTerminate(androidDisplay);
	}
	androidDisplay = EGL_NO_DISPLAY;
	androidContext = EGL_NO_CONTEXT;
	androidSurface = EGL_NO_SURFACE;
}

/**
* Process the next input event.
*/
static int32_t engine_handle_input(struct android_app* app, AInputEvent* event) {
	if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
		//engine->state.x = AMotionEvent_getX(event, 0);
		//engine->state.y = AMotionEvent_getY(event, 0);
		return 1;
	}
	return 0;
}

/**
* Process the next main command.
*/
static void engine_handle_cmd(struct android_app* app, int32_t cmd) {
	struct engine* engine = (struct engine*)app->userData;
	switch (cmd) {
	case APP_CMD_INIT_WINDOW:
		// The window is being shown, get it ready.
		if (androidApp->window != NULL) {
			engine_init_display(engine);
			//engine_draw_frame(engine);
		}
		break;
	case APP_CMD_TERM_WINDOW:
		// The window is being hidden or closed, clean it up.
		engine_term_display();
		break;
	case APP_CMD_GAINED_FOCUS:
		// When our app gains focus, we start monitoring the accelerometer.
		if (accelerometerSensor != NULL) {
			ASensorEventQueue_enableSensor(sensorEventQueue,
				accelerometerSensor);
			// We'd like to get 60 events per second (in microseconds).
			ASensorEventQueue_setEventRate(sensorEventQueue,
				accelerometerSensor, (1000L / 60) * 1000);
		}
		break;
	case APP_CMD_LOST_FOCUS:
		// When our app loses focus, we stop monitoring the accelerometer.
		// This is to avoid consuming battery while not being used.
		if (accelerometerSensor != NULL) {
			ASensorEventQueue_disableSensor(sensorEventQueue,
				accelerometerSensor);
		}
		// Also stop animating.
		//engine_draw_frame(engine);
		break;
	}
}
static std::vector<std::string> list_assets(android_app* app, const char* asset_path)
{ //by Marcel Smit, stolen from https://github.com/android/ndk-samples/issues/603
	std::vector<std::string> result;

	JNIEnv* env = nullptr;
	app->activity->vm->AttachCurrentThread(&env, nullptr);

	auto context_object = app->activity->clazz;
	auto getAssets_method = env->GetMethodID(env->GetObjectClass(context_object), "getAssets", "()Landroid/content/res/AssetManager;");
	auto assetManager_object = env->CallObjectMethod(context_object, getAssets_method);
	auto list_method = env->GetMethodID(env->GetObjectClass(assetManager_object), "list", "(Ljava/lang/String;)[Ljava/lang/String;");

	jstring path_object = env->NewStringUTF(asset_path);
	auto files_object = (jobjectArray)env->CallObjectMethod(assetManager_object, list_method, path_object);
	env->DeleteLocalRef(path_object);
	auto length = env->GetArrayLength(files_object);

	for (int i = 0; i < length; i++)
	{
		jstring jstr = (jstring)env->GetObjectArrayElement(files_object, i);
		const char* filename = env->GetStringUTFChars(jstr, nullptr);
		if (filename != nullptr)
		{
			result.push_back(filename);
			env->ReleaseStringUTFChars(jstr, filename);
		}
		env->DeleteLocalRef(jstr);
	}
	app->activity->vm->DetachCurrentThread();
	return result;
}
int updateAssets() {
	//get APK apkLastUpdateTime timestamp
	JNIEnv* env = nullptr;
	androidApp->activity->vm->AttachCurrentThread(&env, nullptr);
	jobject context_object = androidApp->activity->clazz;
	jmethodID getPackageNameMid_method = env->GetMethodID(env->GetObjectClass(context_object), "getPackageName", "()Ljava/lang/String;");
	jstring packageName = (jstring)env->CallObjectMethod(context_object, getPackageNameMid_method);
	jmethodID getPackageManager_method = env->GetMethodID(env->GetObjectClass(context_object), "getPackageManager", "()Landroid/content/pm/PackageManager;");
	jobject packageManager_object = env->CallObjectMethod(context_object, getPackageManager_method);
	jmethodID getPackageInfo_method = env->GetMethodID(env->GetObjectClass(packageManager_object), "getPackageInfo", "(Ljava/lang/String;I)Landroid/content/pm/PackageInfo;");
	jobject packageInfo_object = env->CallObjectMethod(packageManager_object, getPackageInfo_method, packageName, 0x0);
	jfieldID updateTimeFid = env->GetFieldID(env->GetObjectClass(packageInfo_object), "lastUpdateTime", "J");
	long int apkLastUpdateTime = env->GetLongField(packageInfo_object, updateTimeFid);
	// APK updateTime timestamp retrieved
	// compare with saved timestamp
	std::string updateTimeFilePath = filesRoot + "/dt/apk_update_time.bin";
	FILE* inFile = fopen(updateTimeFilePath.c_str(), "r");
	if (inFile != NULL)
	{
		long int savedUpdateTime;
		fread(&savedUpdateTime, 1, sizeof(savedUpdateTime), inFile);
		fclose(inFile);
		if (savedUpdateTime == apkLastUpdateTime) {
			mylog("Assets are up to date.\n");
			return 0;
		}
	}
	// if here - need to update assets
	AAssetManager* am = androidApp->activity->assetManager;
	int buffSize = 1000000; //guess, should be enough?
	char* buff = new char[buffSize];

	std::vector<std::string> dirsToCheck; //list of assets folders to check
	dirsToCheck.push_back("dt"); //root folder
	while (dirsToCheck.size() > 0) {
		//open last element from directories vector
		std::string dirPath = dirsToCheck.back();
		dirsToCheck.pop_back(); //delete last element
		//mylog("Scanning directory <%s>\n", dirPath.c_str());
		//make sure folder exists on local drive
		std::string outPath = filesRoot + "/" + dirPath; // .c_str();
		struct stat info;
		int statRC = stat(outPath.c_str(), &info);
		if (statRC == 0)
			mylog("%s folder exists.\n", outPath.c_str());
		else {
			// mylog("Try to create %s\n", outPath.c_str());
			int status = mkdir(outPath.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
			if (status == 0)
				mylog("%s folder added.\n", outPath.c_str());
			else {
				mylog("ERROR creating, status=%d, errno: %s.\n", status, std::strerror(errno));
			}
		}
		//get folder's content
		std::vector<std::string> dirItems = list_assets(androidApp, dirPath.c_str());
		int itemsN = dirItems.size();
		//scan directory items
		for (int i = 0; i < itemsN; i++) {
			std::string itemPath = dirPath + "/" + dirItems.at(i).c_str();
			//mylog("New item: <%s> - ", itemPath.c_str());
			//try to open it to see if it's a file
			AAsset* asset = AAssetManager_open(am, itemPath.c_str(), AASSET_MODE_UNKNOWN);
			if (asset != NULL) {
				long size = AAsset_getLength(asset);
				//mylog("It's a file, size = %d - ", size);
				if (size > buffSize) {
					mylog("ERROR in main.cpp->updateAssets(): File %s is too big, skipped.\n", itemPath.c_str());
				}
				else {
					AAsset_read(asset, buff, size);
					outPath = filesRoot + "/" + itemPath;
					FILE* outFile = fopen(outPath.c_str(), "wb");
					if (outFile != NULL)
					{
						fwrite(buff, 1, size, outFile);
						fflush(outFile);
						fclose(outFile);
						mylog("%s saved\n", outPath.c_str());
					}
					else
						mylog("ERROR in main.cpp->updateAssets(): Can't create file %s\n", itemPath.c_str());
				}
				AAsset_close(asset);
			}
			else {
				dirsToCheck.push_back(itemPath);
				//mylog("It's a folder, add to folders list to check.\n");
			}
		}
		dirItems.clear();
	}
	delete[] buff;
	// save updateTime
	FILE* outFile = fopen(updateTimeFilePath.c_str(), "w+");
	if (outFile != NULL)
	{
		fwrite(&apkLastUpdateTime, 1, sizeof(apkLastUpdateTime), outFile);
		fflush(outFile);
		fclose(outFile);
	}
	else
		mylog("ERROR creating %s\n", updateTimeFilePath.c_str());
	return 1;
}
/**
* This is the main entry point of a native application that is using
* android_native_app_glue.  It runs in its own thread, with its own
* event loop for receiving input events and doing other things.
*/
void android_main(struct android_app* state) {

	//state->userData = &engine;
	state->onAppCmd = engine_handle_cmd;
	state->onInputEvent = engine_handle_input;
	androidApp = state;

	// Prepare to monitor accelerometer
	sensorManager = ASensorManager_getInstance();
	accelerometerSensor = ASensorManager_getDefaultSensor(sensorManager,
		ASENSOR_TYPE_ACCELEROMETER);
	sensorEventQueue = ASensorManager_createEventQueue(sensorManager,
		state->looper, LOOPER_ID_USER, NULL, NULL);

	// Read all pending events.
	int ident;
	int events;
	struct android_poll_source* source;
	//wait for display
	while (androidDisplay == NULL) {
		// No display yet.
		//std::this_thread::sleep_for(std::chrono::seconds(1));
		//mylog("No display yet\n");
		//wait for event
		while ((ident = ALooper_pollAll(0, NULL, &events,
			(void**)&source)) >= 0) {
			// Process this event.
			if (source != NULL) {
				source->process(state, source);
			}
		}
	}

	EGLint w, h;
	eglQuerySurface(androidDisplay, androidSurface, EGL_WIDTH, &w);
	eglQuerySurface(androidDisplay, androidSurface, EGL_HEIGHT, &h);
	theGame.onScreenResize(w, h);

	//retrieving files root
	filesRoot.assign(androidApp->activity->internalDataPath);
	mylog("filesRoot = %s\n", filesRoot.c_str());

	updateAssets();

	theGame.run();

	engine_term_display();
}

