#include "affectiva.h"


affectiva::affectiva()
{
}


affectiva::~affectiva()
{
}

bool affectiva::initialize()
{
	

	std::ofstream csvFileStream;
	faceListenPtr = shared_ptr<FaceListener>(new AFaceListener());
	listenPtr = shared_ptr<PlottingImageListener>(new PlottingImageListener(csvFileStream, true));  
	videoListenPtr = shared_ptr<StatusListener>(new StatusListener());
	frameDetector = make_shared<FrameDetector>(buffer_length, process_framerate, nFaces,
		                   (affdex::FaceDetectorMode)faceDetectorMode);        // Init the FrameDetector Class


	frameDetector->setDetectAllEmotions(true);
	frameDetector->setDetectAllExpressions(true);
	frameDetector->setDetectAllEmojis(true);
	frameDetector->setDetectAllAppearances(true);
	frameDetector->setClassifierPath(DATA_FOLDER);
	frameDetector->setImageListener(listenPtr.get());
	frameDetector->setFaceListener(faceListenPtr.get());
	frameDetector->setProcessStatusListener(videoListenPtr.get());
	frameDetector->start();
	m_firstTime = clock();
	return true;
}

vector<coordinate> affectiva::run(cv::Mat m_matImg)
{

	vector<coordinate> m_vecPersonCoordinate;
#ifdef _WIN32
	if (!GetAsyncKeyState(VK_ESCAPE) && videoListenPtr->isRunning())
#else //  _WIN32
	if (videoListenPtr->isRunning())//(cv::waitKey(20) != -1);
#endif
	{

		// Create a frame
		Frame m_frame(m_matImg.size().width, m_matImg.size().height, m_matImg.data, Frame::COLOR_FORMAT::BGR, (clock() - m_firstTime)/1000);

		frameDetector->process(m_frame);  //Pass the frame to detector

		// For each frame processed
		if (listenPtr->getDataSize() > 0)
		{

			std::pair<Frame, std::map<FaceId, Face> > dataPoint = listenPtr->getData();
			Frame frame = dataPoint.first;
			std::map<FaceId, Face> faces = dataPoint.second;

			// Draw metrics to the GUI
			if (draw_display)
			{
				m_vecPersonCoordinate=listenPtr->draw(faces, m_matImg).m_vecPersonCoordinate;
			}
		}
	}
	return m_vecPersonCoordinate;
}
