#pragma once


#include <memory>
#include <chrono>
#include <thread>
#include <mutex>
#include <fstream>
#include <map>
#include <opencv2.4.10_all.h>
#include "ImageListener.h"
#include <FaceListener.h>
#include <string>
#include <vector>
#include "structData.h"
using namespace affdex;


struct outputData
{
	//vector<float>          m_vecPersonSmile;
	//vector<float>          m_vecPersonEyeOpen;
	//vector<float>          m_vecPersonGenderSimilarity;
	vector<coordinate>       m_vecPersonCoordinate;
	vector<int>              m_vecPersonId;
	vector<string>           m_vecPersonGender;
	vector<string>           m_vecPersonAge;
	vector<string>           m_vecGlasses;
	vector<string>           m_vecEthnicityMap;
	void clear()
	{
		m_vecPersonAge.clear();
		m_vecPersonGender.clear();
		/*m_vecPersonGenderSimilarity.clear();*/
		m_vecPersonCoordinate.clear();
		m_vecPersonId.clear();
		m_vecGlasses.clear();
		m_vecEthnicityMap.clear();
		//m_vecPersonEyeOpen.clear();
		//m_vecPersonSmile.clear();
	}
};

class PlottingImageListener : public ImageListener
{
	outputData m_outputData;
    std::mutex mMutex;
    std::deque<std::pair<Frame, std::map<FaceId, Face> > > mDataArray;

    double mCaptureLastTS;
  //  double mCaptureFPS;
    double mProcessLastTS;
    double mProcessFPS;
    std::ofstream &fStream;
    std::chrono::time_point<std::chrono::system_clock> mStartT;
    const bool mDrawDisplay;
    const int spacing = 10;
    const float font_size = 0.5f;
    const int font = cv::FONT_HERSHEY_COMPLEX_SMALL;

    std::vector<std::string> expressions;
    std::vector<std::string> emotions;
    std::vector<std::string> emojis;
    std::vector<std::string> headAngles;

    std::map<affdex::Glasses, std::string> glassesMap;
    std::map<affdex::Gender, std::string> genderMap;
    std::map<affdex::Age, std::string> ageMap;
    std::map<affdex::Ethnicity, std::string> ethnicityMap;
	vector<cv::Scalar>         m_vecScalar;
public:


    PlottingImageListener(std::ofstream &csv, const bool draw_display)
        : fStream(csv), mDrawDisplay(draw_display), mStartT(std::chrono::system_clock::now()),
        mCaptureLastTS(-1.0f), /*mCaptureFPS(-1.0f),*/
        mProcessLastTS(-1.0f), mProcessFPS(-1.0f)
    {
        expressions = {
            "smile", "innerBrowRaise", "browRaise", "browFurrow", "noseWrinkle",
            "upperLipRaise", "lipCornerDepressor", "chinRaise", "lipPucker", "lipPress",
            "lipSuck", "mouthOpen", "smirk", "eyeClosure", "attention", "eyeWiden", "cheekRaise",
            "lidTighten", "dimpler", "lipStretch", "jawDrop"
        };

        emotions = {
            "joy", "fear", "disgust", "sadness", "anger",
            "surprise", "contempt", "valence", "engagement"
        };

        headAngles = { "pitch", "yaw", "roll" };


        emojis = std::vector<std::string> {
            "relaxed", "smiley", "laughing",
                "kissing", "disappointed",
                "rage", "smirk", "wink",
                "stuckOutTongueWinkingEye", "stuckOutTongue",
                "flushed", "scream"
        };

        genderMap = std::map<affdex::Gender, std::string> {
            { affdex::Gender::Male, "male" },
            { affdex::Gender::Female, "female" },
            { affdex::Gender::Unknown, "unknown" },

        };

        glassesMap = std::map<affdex::Glasses, std::string> {
            { affdex::Glasses::Yes, "Glasses:es" },
            { affdex::Glasses::No, "Glasses:no" }
        };

        ageMap = std::map<affdex::Age, std::string> {
            { affdex::Age::AGE_UNKNOWN, "unknown"},
            { affdex::Age::AGE_UNDER_18, "under 18" },
            { affdex::Age::AGE_18_24, "18-24" },
            { affdex::Age::AGE_25_34, "25-34" },
            { affdex::Age::AGE_35_44, "35-44" },
            { affdex::Age::AGE_45_54, "45-54" },
            { affdex::Age::AGE_55_64, "55-64" },
            { affdex::Age::AGE_65_PLUS, "65 plus" }
        };

        ethnicityMap = std::map<affdex::Ethnicity, std::string> {
            { affdex::Ethnicity::UNKNOWN, "unknown"},
            { affdex::Ethnicity::CAUCASIAN, "caucasian" },
            { affdex::Ethnicity::BLACK_AFRICAN, "black african" },
            { affdex::Ethnicity::SOUTH_ASIAN, "south asian" },
            { affdex::Ethnicity::EAST_ASIAN, "east asian" },
            { affdex::Ethnicity::HISPANIC, "hispanic" }
        };

        fStream << "TimeStamp,faceId,interocularDistance,glasses,age,ethnicity,gender,dominantEmoji,";
        for (std::string angle : headAngles) fStream << angle << ",";
        for (std::string emotion : emotions) fStream << emotion << ",";
        for (std::string expression : expressions) fStream << expression << ",";
        for (std::string emoji : emojis) fStream << emoji << ",";
        fStream << std::endl;
        fStream.precision(100);
        fStream << std::fixed;
		cv::RNG  m_rng = cv::RNG();
		for (int i = 0; i < 1000000; i++)
		{
			m_vecScalar.push_back(cv::Scalar(m_rng.uniform(0, 255), m_rng.uniform(0, 255), m_rng.uniform(0, 255)));
		}
    }

    FeaturePoint minPoint(VecFeaturePoint points)
    {
        VecFeaturePoint::iterator it = points.begin();
        FeaturePoint ret = *it;
        for (; it != points.end(); it++)
        {
            if (it->x < ret.x) ret.x = it->x;
            if (it->y < ret.y) ret.y = it->y;
        }
        return ret;
    };

    FeaturePoint maxPoint(VecFeaturePoint points)
    {
        VecFeaturePoint::iterator it = points.begin();
        FeaturePoint ret = *it;
        for (; it != points.end(); it++)
        {
            if (it->x > ret.x) ret.x = it->x;
            if (it->y > ret.y) ret.y = it->y;
        }
        return ret;
    };


    double getProcessingFrameRate()
    {
        std::lock_guard<std::mutex> lg(mMutex);
        return mProcessFPS;
    }

   /* double getCaptureFrameRate()
    {
        std::lock_guard<std::mutex> lg(mMutex);
        return mCaptureFPS;
    }*/

    int getDataSize()
    {
        std::lock_guard<std::mutex> lg(mMutex);
        return mDataArray.size();

    }

    std::pair<Frame, std::map<FaceId, Face>> getData()
    {
        std::lock_guard<std::mutex> lg(mMutex);
        std::pair<Frame, std::map<FaceId, Face>> dpoint = mDataArray.front();
        mDataArray.pop_front();
        return dpoint;
    }

    void onImageResults(std::map<FaceId, Face> faces, Frame image) override
    {
        std::lock_guard<std::mutex> lg(mMutex);
        mDataArray.push_back(std::pair<Frame, std::map<FaceId, Face>>(image, faces));
        std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
        std::chrono::milliseconds milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(now - mStartT);
        double seconds = milliseconds.count() / 1000.f;
        mProcessFPS = 1.0f / (seconds - mProcessLastTS);
        mProcessLastTS = seconds;
    };

    void onImageCapture(Frame image) override
    {
        std::lock_guard<std::mutex> lg(mMutex);
       // mCaptureFPS = 1.0f / (image.getTimestamp() - mCaptureLastTS);
        mCaptureLastTS = image.getTimestamp();
    };

    void outputToFile(const std::map<FaceId, Face> faces, const double timeStamp)
    {
        if (faces.empty())
        {
            fStream << timeStamp << ",nan,nan,no,unknown,unknown,unknown,unknown,";
            for (std::string angle : headAngles) fStream << "nan,";
            for (std::string emotion : emotions) fStream << "nan,";
            for (std::string expression : expressions) fStream << "nan,";
            for (std::string emoji : emojis) fStream << "nan,";
            fStream << std::endl;
        }
        for (auto & face_id_pair : faces)
        {
            Face f = face_id_pair.second;

            fStream << timeStamp << ","
                << f.id << ","
                << f.measurements.interocularDistance << ","
                << glassesMap[f.appearance.glasses] << ","
                << ageMap[f.appearance.age] << ","
                << ethnicityMap[f.appearance.ethnicity] << ","
                << genderMap[f.appearance.gender] << ","
                << affdex::EmojiToString(f.emojis.dominantEmoji) << ",";

            float *values = (float *)&f.measurements.orientation;
            for (std::string angle : headAngles)
            {
                fStream << (*values) << ",";
                values++;
            }

            values = (float *)&f.emotions;
            for (std::string emotion : emotions)
            {
                fStream << (*values) << ",";
                values++;
            }

            values = (float *)&f.expressions;
            for (std::string expression : expressions)
            {
                fStream << (*values) << ",";
                values++;
            }

            values = (float *)&f.emojis;
            for (std::string emoji : emojis)
            {
                fStream << (*values) << ",";
                values++;
            }

            fStream << std::endl;
        }
    }

    void drawValues(const float * first, const std::vector<std::string> names,
        const int x, int &padding, const cv::Scalar clr,
        cv::Mat img)
    {
        for (std::string name : names)
        {
            if (std::abs(*first) > 5.0f)
            {
                char m[50];
                sprintf(m, "%s: %3.2f", name.c_str(), (*first));
                cv::putText(img, m, cv::Point(x, padding += spacing), font, font_size, clr);
            }
            first++;
        }
    }

	outputData  draw(const std::map<FaceId, Face> faces, cv::Mat m_matImg)
	{

		m_outputData.clear();
		const int left_margin = 30;
		cv::Scalar clr = cv::Scalar(0, 0, 255);
		cv::Scalar header_clr = cv::Scalar(255, 0, 0);
		int  m_currentId = -1;
		for (auto & face_id_pair : faces)
		{
			++m_currentId;
			vector<cv::Point> m_tmpPoints;
			m_tmpPoints.clear();
			Face m_currentFace = face_id_pair.second;
			VecFeaturePoint points = m_currentFace.featurePoints;
			for (auto& point : points)    //Draw face feature points.
			{
			//	cv::circle(m_matImg, cv::Point(point.x, point.y), 2.0f, cv::Scalar(0, 0, 255));
				m_tmpPoints.push_back(cv::Point(point.x, point.y));
			}

			//=============【1】寻找最小包围矩形================  
			cv::RotatedRect minRect = cv::minAreaRect(cv::Mat(m_tmpPoints));
			m_tmpPoints.clear();
			cv::Point2f vertex[4];//用于存放最小矩形的四个顶点  
			minRect.points(vertex);//返回矩形的四个顶点给vertex  
			//绘制最小面积包围矩形  
			int m_mincoordinate_x = vertex[0].x, m_mincoordinate_y = vertex[0].y;
			int m_maxcoordinate_x = vertex[0].x, m_maxcoordinate_y = vertex[0].y;
			for (int i = 0; i < 4; i++)
			{
				//line(m_matImg, vertex[i], vertex[(i + 1) % 4], cv::Scalar(0, 255, 0), 1, 8);//非常巧妙的表达式  
				if (vertex[i].x>m_maxcoordinate_x)
				{
					m_maxcoordinate_x = vertex[i].x;
				}
				if (vertex[i].y > m_maxcoordinate_y)
				{
					m_maxcoordinate_y = vertex[i].y;
				}

				if (vertex[i].x < m_mincoordinate_x)
				{
					m_mincoordinate_x = vertex[i].x;
				}
				if (vertex[i].y < m_mincoordinate_y)
				{
					m_mincoordinate_y = vertex[i].y;
				}
			}

			cv::Point m_startPoint = cv::Point(m_mincoordinate_x, m_mincoordinate_y);
			cv::Point m_endPoint = cv::Point(m_maxcoordinate_x, m_maxcoordinate_y);
			if (m_startPoint.x < 0)
			{
				m_startPoint.x = 0;
			}
			if (m_startPoint.y < 0)
			{
				m_startPoint.y = 0;
			}
			if (m_endPoint.x > m_matImg.cols)
			{
				m_endPoint.x = m_matImg.cols;
			}
			if (m_endPoint.y > m_matImg.rows)
			{
				m_endPoint.y = m_matImg.rows;
			}
		
			coordinate m_coordinateData;
			m_coordinateData.x = m_startPoint.x;
			m_coordinateData.y = m_startPoint.y;
			m_coordinateData.width = abs(m_startPoint.x - m_endPoint.x);
			m_coordinateData.height = abs(m_startPoint.y - m_endPoint.y);

			m_outputData.m_vecPersonCoordinate.push_back(m_coordinateData);
			m_outputData.m_vecPersonGender.push_back(genderMap[m_currentFace.appearance.gender]);
			m_outputData.m_vecPersonAge.push_back(ageMap[m_currentFace.appearance.age]);
			m_outputData.m_vecGlasses.push_back(glassesMap[m_currentFace.appearance.glasses]);
			m_outputData.m_vecPersonId.push_back(m_currentFace.id);
			m_outputData.m_vecEthnicityMap.push_back(ethnicityMap[m_currentFace.appearance.ethnicity]);
			cv::Rect m_rect(m_startPoint.x, m_startPoint.y, abs(m_startPoint.x - m_endPoint.x), abs(m_startPoint.y - m_endPoint.y));
			cv::rectangle(m_matImg, m_rect, m_vecScalar[m_currentId],2);
			
			//cv::Rect m_rect(cv::Point(vertex[3].x, vertex[0]), cv::Point(m_maxcoordinate_x, m_maxcoordinate_y));
			/*cv::Mat m_matImg = m_matImg(m_rect);
			imshow("roi", m_matImg);*/



			FeaturePoint tl = minPoint(points);
			FeaturePoint br = maxPoint(points);

			//Output the results of the different classifiers.
			int padding = tl.y + 10;

		/*	cv::putText(m_matImg, "APPEARANCE", cv::Point(br.x, padding += (spacing * 2)), font, font_size, header_clr);
			cv::putText(m_matImg, genderMap[m_currentFace.appearance.gender], cv::Point(br.x, padding += spacing), font, font_size, clr);
			cv::putText(m_matImg, glassesMap[m_currentFace.appearance.glasses], cv::Point(br.x, padding += spacing), font, font_size, clr);
			cv::putText(m_matImg, ageMap[m_currentFace.appearance.age], cv::Point(br.x, padding += spacing), font, font_size, clr);
			cv::putText(m_matImg, ethnicityMap[m_currentFace.appearance.ethnicity], cv::Point(br.x, padding += spacing), font, font_size, clr);*/



			//Orientation headAngles = m_currentFace.measurements.orientation;

			//char strAngles[100];
			//sprintf(strAngles, "Pitch: %3.2f Yaw: %3.2f Roll: %3.2f Interocular: %3.2f",
			//	headAngles.pitch, headAngles.yaw, headAngles.roll, m_currentFace.measurements.interocularDistance);



			char fId[10];
			sprintf(fId, "ID: %i", m_currentFace.id);
			//cv::putText(m_matImg, fId, cv::Point(br.x, padding += spacing), font, font_size, clr);
			cv::putText(m_matImg, fId, cv::Point(m_startPoint.x + abs(m_startPoint.x - m_endPoint.x), 
				m_startPoint.y + abs(m_startPoint.y - m_endPoint.y) ), font, font_size, clr);

			/*
			暂时不显示出来
			*/
			
			/*	cv::putText(m_matImg, "MEASUREMENTS", cv::Point(br.x, padding += (spacing * 2)), font, font_size, header_clr);

				cv::putText(m_matImg, strAngles, cv::Point(br.x, padding += spacing), font, font_size, clr);

				cv::putText(m_matImg, "EMOJIS", cv::Point(br.x, padding += (spacing * 2)), font, font_size, header_clr);

				cv::putText(m_matImg, "dominantEmoji: " + affdex::EmojiToString(m_currentFace.emojis.dominantEmoji),
				cv::Point(br.x, padding += spacing), font, font_size, clr);

				drawValues((float *)&m_currentFace.emojis, emojis, br.x, padding, clr, m_matImg);

				cv::putText(m_matImg, "EXPRESSIONS", cv::Point(br.x, padding += (spacing * 2)), font, font_size, header_clr);

				drawValues((float *)&m_currentFace.expressions, expressions, br.x, padding, clr, m_matImg);

				cv::putText(m_matImg, "EMOTIONS", cv::Point(br.x, padding += (spacing * 2)), font, font_size, header_clr);

				drawValues((float *)&m_currentFace.emotions, emotions, br.x, padding, clr, m_matImg);*/
				
		}
		char fps_str[50];
		//sprintf(fps_str, "capture fps: %2.0f", mCaptureFPS);
		//cv::putText(m_matImg, fps_str, cv::Point(m_matImg.cols - 110, m_matImg.rows - left_margin - spacing), font, font_size, clr);
		sprintf(fps_str, "process fps: %2.0f", mProcessFPS);
		cv::putText(m_matImg, fps_str, cv::Point(m_matImg.cols - 110, m_matImg.rows - left_margin), font, font_size, clr);

		cv::imshow("analyze video", m_matImg);
		std::lock_guard<std::mutex> lg(mMutex);
		cv::waitKey(1);
		m_matImg.release();
		return m_outputData;
	}

};
