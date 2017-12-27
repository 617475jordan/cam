#pragma once
#include "AFaceListener.hpp"
#include "PlottingImageListener.hpp"
#include "StatusListener.hpp"
#include <FrameDetector.h>
#include <memory>
#include <chrono>
#include <time.h>
using namespace std;
#if _DEBUG
#pragma  comment(lib,"..\\lib\\debug\\affdex-native.lib")
#else
#pragma  comment(lib,"..\\lib\\release\\affdex-native.lib")
#endif

class affectiva
{
public:
	affectiva();
	~affectiva();
	bool                        initialize();
	vector<coordinate>           run(cv::Mat m_matImg);
private:

	affdex::path DATA_FOLDER = L"data";

	int process_framerate = 30;
	int buffer_length = 2;
	int camera_id = 0;
	unsigned int nFaces = 20;
	bool draw_display = true;
	int faceDetectorMode = (int)FaceDetectorMode::LARGE_FACES;

	shared_ptr<PlottingImageListener> listenPtr;
    shared_ptr<FrameDetector> frameDetector;
    shared_ptr<StatusListener> videoListenPtr;
    shared_ptr<FaceListener> faceListenPtr; 
	double m_firstTime = -1;
};

