#include <stdlib.h>
#include <stdio.h>
#include <time.h>
//#include "cv.h" 
//#include "highgui.h" 
#include <opencv2.4.10_all.h>
#include "MultiObjectTLD.h"

#define LOADCLASSIFIERATSTART 0
#define CLASSIFIERFILENAME "test.moctld"

//struct headcoordinateData
//{
//	int x;
//	int y;
//	int width;
//	int height;
//};
class Header
{
public:
	int                       drawMode = 255;
	bool                      learningEnabled = true;
	bool                      save = false;
	bool                      load = false;
	bool                      reset = false;
private:
	//uncomment if you have a high resolution camera and want to speed up tracking
#define                       FORCE_RESIZING
#define                       RESOLUTION_X 320
#define                       RESOLUTION_Y 240
	//CvCapture                 * capture;
	IplImage                  * curImage = NULL;
	//bool                      ivQuit = false;
	int                       ivWidth, ivHeight;
	int                       m_size;
	MultiObjectTLD            *p;
public:
	Header()
	{
		int                   drawMode = 255;
		bool                  learningEnabled = true;
		bool                  save = false;
		bool                  load = false;
		bool                  reset = false;
	};
	~Header()
	{
		int                   drawMode = 255;
		bool                  learningEnabled = true;
		bool                  save = false;
		bool                  load = false;
		bool                  reset = false;
	};
	void Init(cv::Mat m_matImg)
	{
		//capture = cvCaptureFromCAM(CV_CAP_ANY);
		//if (!capture){
		//	std::cout << "error starting video capture" << std::endl;
		//	exit(0);
		//}
		////propose a resolution
		//cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH, 640);
		//cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT, 480);
		//get the actual (supported) resolution
		/*ivWidth = cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH);
		ivHeight = cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT);
		std::cout << "camera/video resolution: " << ivWidth << "x" << ivHeight << std::endl;*/
		ivWidth = m_matImg.cols;
		ivHeight = m_matImg.rows;
#ifdef FORCE_RESIZING
		ivWidth = RESOLUTION_X;
		ivHeight = RESOLUTION_Y;
#endif

		//cvNamedWindow("MOCTLD", 0); //CV_WINDOW_AUTOSIZE );

		//CvSize wsize = { ivWidth, ivHeight };
		//curImage = cvCreateImage(wsize, IPL_DEPTH_8U, 3);

		//cvResizeWindow("MOCTLD", ivWidth, ivHeight);
		//cvSetMouseCallback("MOCTLD", MouseHandler);

		m_size = ivWidth*ivHeight;

		// Initialize MultiObjectTLD
#if LOADCLASSIFIERATSTART
		p = new MultiObjectTLD::loadClassifier((char*)CLASSIFIERFILENAME);
#else
		MOTLDSettings settings(COLOR_MODE_RGB);
		settings.useColor = true;
		p = new MultiObjectTLD(ivWidth, ivHeight, settings);
#endif
	};

	vector<coordinate> Run(cv::Mat m_matImg)
	{
		
		//unsigned char img[size*3];
		unsigned char img[320 * 240 * 3];
//#ifdef FORCE_RESIZING
		CvSize wsize = { ivWidth, ivHeight };

		IplImage* frame = &IplImage(m_matImg);
		/*curImage = frame;*/
//#endif
		float m_runTime = clock();

			for (int j = 0; j<m_size; j++)
			{
				img[j] = frame->imageData[j * 3 + 2];
				img[j + m_size] = frame->imageData[j * 3 + 1];
				img[j + 2 * m_size] = frame->imageData[j * 3];
			}
			p->processFrame(img);
			
			// Process it with motld
			
			// Add new box
	/*		if (mouseMode == MOUSE_MODE_ADD_BOX)
			{*/
				
			//	mouseMode = MOUSE_MODE_IDLE;
			//}

			// Display result
		//	HandleInput(1);
			/*Matrix maRed;
			Matrix maGreen;
			Matrix maBlue;*/
			vector<coordinate> m_vecCoordiante;
			m_vecCoordiante = p->getCurrentImage(/*img, maRed, maGreen, maBlue, drawMode*/);
			/*FromRGB(maRed, maGreen, maBlue);
			float m_currentTime = clock() - m_runTime;
			char m_charCurrentTime[100];
			time_t  m_tTime;
			m_tTime = time(0);
			strftime(m_charCurrentTime, sizeof(m_charCurrentTime), "%Y-%m-%d %X %A ", localtime(&m_tTime));
			CvFont font;
			cvInitFont(&font, CV_FONT_HERSHEY_COMPLEX, 0.3, 0.3, 0.5, 1, 8);
			cvPutText(curImage, m_charCurrentTime, Point(0, 10), &font, CV_RGB(0,255,255));
			char m_char[100];
			sprintf(m_char, "runTime:%.1fs", m_currentTime/1000);
			cvPutText(curImage, m_char, Point(0, 20), &font, CV_RGB(0, 255, 255));
			cvShowImage("MOCTLD", curImage);*/
			p->enableLearning(learningEnabled);
			return m_vecCoordiante;
			/*if (save)
			{
				p->saveClassifier((char*)CLASSIFIERFILENAME);
				save = false;
			}*/
		
			//waitKey(1);
	
		//delete[] img;
		/*cvReleaseCapture(&capture);*/
			//return m_objectBoxes;
	};
	bool update(vector<coordinate> m_coordinateData)
	{
		ObjectBox mouseBox;
		for (int i = 0; i < m_coordinateData.size(); i++)
		{
			if (m_coordinateData[i].x<0 || m_coordinateData[i].y<0 ||
				m_coordinateData[i].x>ivWidth || m_coordinateData[i].y>ivHeight ||
				(m_coordinateData[i].x + m_coordinateData[i].width)>ivWidth ||
				(m_coordinateData[i].y + m_coordinateData[i].height>ivHeight))
			{
				continue;
			}
			mouseBox.x = m_coordinateData[i].x;
			mouseBox.y = m_coordinateData[i].y;
			mouseBox.width = m_coordinateData[i].width;
			mouseBox.height = m_coordinateData[i].height;
			p->addObject(mouseBox);
			//p->enableLearning(learningEnabled);
		}
		return true;
	};
	void removeUselessTrackingObjectInfo(vector<int> m_uselessTrackObjectInfo)
	{
		p->removeUselessTrackingObjectBoxInfo(m_uselessTrackObjectInfo);
	};
	bool isOver()
	{
		p = NULL;
		delete[] p;
	};
	//private:
	//	void FromRGB(Matrix& maRed, Matrix& maGreen, Matrix& maBlue)
	//	{
	//		for (int i = 0; i < ivWidth*ivHeight; ++i){
	//			curImage->imageData[3 * i + 2] = maRed.data()[i];
	//			curImage->imageData[3 * i + 1] = maGreen.data()[i];
	//			curImage->imageData[3 * i + 0] = maBlue.data()[i];
	//		}
	//		//at this place you could save the images using
	//		//cvSaveImage(filename, curImage);
	//		/*if (mouseMode == MOUSE_MODE_MARKER)
	//		{
	//			CvPoint pt1; pt1.x = mouseBox.x; pt1.y = mouseBox.y;
	//			CvPoint pt2; pt2.x = mouseBox.x + mouseBox.width; pt2.y = mouseBox.y + mouseBox.height;
	//			cvRectangle(curImage, pt1, pt2, CV_RGB(0, 0, 255));
	//		}*/
	//	};
};




