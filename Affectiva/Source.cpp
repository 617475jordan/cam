#include "affectiva.h"
#include <time.h>
#include "Header.h"
double k = 1.0;
//cv::Mat m_map;
Mat drawRectangle(Mat m_matInput, vector<coordinate> m_vecCoordinate);
vector<int> removeSimilarityRectangleAandA(vector<coordinate> m_vecFirstCoordinate);
vector<int> removeSimilarityRectangleAandB(vector<coordinate> m_vecFirstCoordinate,
	vector<coordinate> m_vecSecondCoordinate);
float computRectJoinUnion(int x0, int y0, int width0, int height0,
	int x1, int y1, int width1, int height1);

float m_areaThreshold = 0.1;
int main()
{
	/*string path;
	cin >> path;*/
	//cout << computRectJoinUnion(0, 0, 40, 40, 20, 20, 60, 60) << endl;
	int width = 320;
	int height = 240;
	int fps = 30;
	cv::VideoCapture capture;
	capture.open(0);
	//capture.set(CV_CAP_PROP_FRAME_WIDTH, width);
	//capture.set(CV_CAP_PROP_FRAME_HEIGHT, height);
	if (!capture.isOpened())
	{
		return -1;
	}
	affectiva *m_affectiva = new affectiva();
	Header *m_header = new Header();
	cv::Mat m_matImg;
	//m_map = imread("data\\map.jpg",CV_LOAD_IMAGE_UNCHANGED);
	capture >> m_matImg;
	if (m_matImg.empty() /*|| m_map.empty()*/)
	{
		return 0;
	}
	resize(m_matImg, m_matImg, Size(320, 240));
	m_header->Init(m_matImg);

	if (!m_affectiva->initialize())
	{
		return -2;
	}
	vector<coordinate> m_vecFirstCoordinate, m_vecSecondCoordinate;
	vector<coordinate> m_vecFirstResult, m_vecSecondResult;
	vector<int> m_vecIFirstResult, m_vecISecondResult, m_vecThirdIResult;
	vector<coordinate> m_vecUpdate;
	int m_count = 0;
	double m_currntTime = clock();
	while (1)
	{
		int m_time = clock();
		m_vecFirstCoordinate.clear();
		m_vecSecondCoordinate.clear();
		m_vecSecondResult.clear();
		m_vecFirstResult.clear();
		m_vecUpdate.clear();
		m_vecIFirstResult.clear();
		m_vecISecondResult.clear();
		m_vecThirdIResult.clear();
		capture >> m_matImg;
		if (m_matImg.empty())
		{
			return -3;
		}
		resize(m_matImg, m_matImg, Size(320, 240));
		char m_textCurrentTime[100];
		sprintf(m_textCurrentTime, "running time:%.1f s", (clock() - m_currntTime) / 1000.0);
		putText(m_matImg, m_textCurrentTime, Point(0, 10), 1, 0.5, Scalar(255, 0, 0));
		m_vecFirstCoordinate = m_affectiva->run(m_matImg);
		//int m_current = clock();
		m_vecIFirstResult = removeSimilarityRectangleAandA(m_vecFirstCoordinate);
	//	cout << "The total timeA:" << clock() - m_time << "ms" << endl;
		if (m_vecIFirstResult.size() == 0)
		{
			//m_matImg = drawRectangle(m_matImg, m_vecSecondResult);
			imshow("result", m_matImg);
			m_matImg.release();
			waitKey(1);
			cout << "The total time:" << clock() - m_time << "ms" << endl;

			continue;
		}
	
		sort(m_vecIFirstResult.begin(), m_vecIFirstResult.end());

		int m_m_vecIFirstResultlen = m_vecIFirstResult.size();
		for (int i = 0; i < m_m_vecIFirstResultlen; i++)
		{
			m_vecFirstResult.push_back(m_vecFirstCoordinate[m_vecIFirstResult[i]]);
		}
		m_vecSecondCoordinate = m_header->Run(m_matImg);
		//这段代码有个bug，暂时还没有调好
		m_vecISecondResult = removeSimilarityRectangleAandA(m_vecSecondCoordinate);
		//cout << "The total timeB:" << clock() - m_time << "ms" << endl;
		if (m_vecISecondResult.size()>0)
		{
			sort(m_vecISecondResult.begin(), m_vecISecondResult.end());
			for (int i = 0; i < m_vecISecondResult.size(); i++)
			{
				m_vecSecondResult.push_back(m_vecSecondCoordinate[m_vecISecondResult[i]]);
			}
			vector<int> m_uselessTrackObject;
			int m = 0;
			for (int i = 0; i < m_vecSecondCoordinate.size(); i++)
			{
				bool flag = true;
				int m_tmp=i;
				for (int j = m; j < m_vecISecondResult.size(); j++)
				{
					if (flag == false)
					{
						break;
					}
					if (m_vecISecondResult[j] == i)
					{
						flag = false;
						m++;
						m_tmp = m_vecISecondResult[j];
						break;
					}
				}
				if (flag == false)
				{
					continue;
				}
				else
				{
					m_uselessTrackObject.push_back(m_vecSecondCoordinate[i].ID);
				}
				i = m_tmp;
			}
			if (m_uselessTrackObject.size() > 0)
			{
				m_header->removeUselessTrackingObjectInfo(m_uselessTrackObject);
			}
		}
		//cout << "The total timeC:" << clock() - m_time << "ms" << endl;
		//cout << "totalA:" << clock() - m_current << "ms" << endl;
		//cout << m_vecSecondResult.size() << endl;
		m_matImg = drawRectangle(m_matImg, m_vecSecondResult);
		imshow("result", m_matImg);
		waitKey(1);
		//m_vecSecondCoordinate.clear();
		//m_vecFirstCoordinate.clear();
		m_vecThirdIResult = removeSimilarityRectangleAandB(m_vecFirstResult, m_vecSecondResult);
		sort(m_vecThirdIResult.begin(), m_vecThirdIResult.end());
		for (int i = 0; i < m_vecThirdIResult.size(); i++)
		{
			m_vecUpdate.push_back(m_vecFirstResult[m_vecThirdIResult[i]]);
		}
		m_header->update(m_vecUpdate);
		m_matImg.release();

		cout << "The total time:" << clock() - m_time << "ms" << endl;
		//cout << "The total time:" << clock() - m_time << "ms" << endl;
		if (waitKey(10) >= 0)
		{
			m_affectiva = NULL;
			delete[] m_affectiva;
			return -1;
		}
	}
	m_affectiva = NULL;
	delete[] m_affectiva;
	return 0;
}
Mat drawRectangle(Mat m_matInput, vector<coordinate> m_vecCoordinate)
{
	int m_len = m_vecCoordinate.size();
	if (m_len == 0)
	{
		return m_matInput;
	}
	//Mat tmpImg;
	//m_map.copyTo(tmpImg);
	for (int i = 0; i <m_len; i++)
	{
		 
		cv::Rect m_rect(m_vecCoordinate[i].x, m_vecCoordinate[i].y, m_vecCoordinate[i].width*k, m_vecCoordinate[i].height*k);
		cv::rectangle(m_matInput, m_rect, Scalar(255, 255, 0));
		//double m_k = m_vecCoordinate[i].width*1.0 / m_map.cols;
		

		/****************
		添加圣诞帽子
		**************/
		//int m_tmpwidth = m_vecCoordinate[i].x;
		//int m_tmpheight = m_vecCoordinate[i].y - m_vecCoordinate[i].height;
		//if (m_tmpheight >= 0 && m_tmpwidth >= 0)
		//{
		//	resize(m_map, tmpImg, Size(m_vecCoordinate[i].width
		//		, m_vecCoordinate[i].height));
		//	Mat m_dst = m_matInput(Rect(m_tmpwidth,
		//		m_tmpheight, tmpImg.cols, tmpImg.rows
		//		));
		//	
		//	addWeighted(tmpImg, 1, m_dst, 0.5, 1, m_dst);
		//	//m_dst.release();
		//	//imshow("dst", m_dst);
		//	tmpImg.release();
		//}
		//
		char currentId[10];
		sprintf(currentId, "Id:%d", m_vecCoordinate[i].ID);
		putText(m_matInput, currentId, Point(m_vecCoordinate[i].x + m_vecCoordinate[i].width*k,
			m_vecCoordinate[i].y + m_vecCoordinate[i].height*k), 1, 1, Scalar(255, 255, 0));
	}
	//imshow("test", m_matInput);
	return m_matInput;
}

vector<int> removeSimilarityRectangleAandA(vector<coordinate> m_vecFirstCoordinate)
{
	vector<int> m_updateData;
	int m_len = m_vecFirstCoordinate.size();
	vector<int> m_similarityFlag;
	/*for (int i = 0; i < m_len; i++)
	{
	Rect r0(Point(m_vecFirstCoordinate[i].x, m_vecFirstCoordinate[i].y),
	Point(m_vecFirstCoordinate[i].x + m_vecFirstCoordinate[i].width,
	m_vecFirstCoordinate[i].y + m_vecFirstCoordinate[i].height));

	for (int j=0; j < m_len; j++)
	{
	if (i == j)
	{
	m_updateCoordinate.push_back(m_vecFirstCoordinate[i]);
	continue;
	}
	for (int k = 0; k < m_similarityFlag.size(); k++)
	{
	if (m_similarityFlag[k] == j)
	{
	continue;
	}
	}
	if (m_areaThreshold <= computRectJoinUnion(
	m_vecFirstCoordinate[i].x,m_vecFirstCoordinate[i].y,
	m_vecFirstCoordinate[i].width,m_vecFirstCoordinate[i].height,
	m_vecFirstCoordinate[j].x,m_vecFirstCoordinate[j].y,
	m_vecFirstCoordinate[j].width,m_vecFirstCoordinate[j].height))
	{
	m_similarityFlag.push_back(j);
	continue;
	}
	Rect r1(Point(m_vecFirstCoordinate[j].x, m_vecFirstCoordinate[j].y),
	Point(m_vecFirstCoordinate[j].x + m_vecFirstCoordinate[j].width,
	m_vecFirstCoordinate[j].y + m_vecFirstCoordinate[j].height));
	if (j != i && (r0&r1) == r0)
	{
	break;
	}
	if (j == m_len)
	{
	m_updateCoordinate.push_back(m_vecFirstCoordinate[i]);
	}
	}
	}*/

	if (m_len == 1)
	{
		m_updateData.push_back(0);
		//m_vecFirstCoordinate.clear();
		//return m_updateData;
	}
	else if (m_len == 0)
	{
		return m_updateData;
	}
	else
	{
		for (int i = 0; i < m_len; i++)
		{
			bool m_isOK = true;
			for (int k = 0; k < m_similarityFlag.size(); k++)
			{
				if (i == m_similarityFlag[k])
				{
					m_isOK = false;
					//break;
				}
				if (m_isOK == false)
				{
					break;
				}
			}
			if (m_isOK == true)
			{
				//int m_count = 0;
				for (int j = i + 1; j < m_len; j++)
				{
					float m_result = computRectJoinUnion(
						m_vecFirstCoordinate[i].x, m_vecFirstCoordinate[i].y,
						m_vecFirstCoordinate[i].width, m_vecFirstCoordinate[i].height,
						m_vecFirstCoordinate[j].x, m_vecFirstCoordinate[j].y,
						m_vecFirstCoordinate[j].width, m_vecFirstCoordinate[j].height);
					//cout << "similarity:" << m_result << endl;

					if (m_result>m_areaThreshold)
					{
						m_similarityFlag.push_back(j);
					}
				}
				m_updateData.push_back(i);
			}
			else
			{
				continue;
			}
		}
	}
	m_vecFirstCoordinate.clear();
	return m_updateData;
}

vector<int> removeSimilarityRectangleAandB(vector<coordinate> m_vecFirstCoordinate,
	vector<coordinate> m_vecSecondCoordinate)
{

	vector<int> m_updateData;
	vector<int> m_similarityFlag;
	int m_firstLen = m_vecFirstCoordinate.size();
	int m_secondLen = m_vecSecondCoordinate.size();

	if (m_firstLen == 0)
	{
		return m_updateData;
	}
	else if (m_secondLen == 0)
	{
		for (int m = 0; m < m_firstLen; m++)
		{
			m_updateData.push_back(m);
		}
		//m_vecFirstCoordinate.clear();
		//return m_updateData;
	}
	else
	{
		for (int i = 0; i < m_firstLen; i++)
		{

			for (int k = 0; k < m_similarityFlag.size(); k++)
			{
				if (i == m_similarityFlag[k])
				{
					break;
				}
			}
			bool m_isOK = true;
			for (int j = 0; j < m_secondLen; j++)
			{
				float m_result = computRectJoinUnion(
					m_vecFirstCoordinate[i].x, m_vecFirstCoordinate[i].y,
					m_vecFirstCoordinate[i].width, m_vecFirstCoordinate[i].height,
					m_vecSecondCoordinate[j].x, m_vecSecondCoordinate[j].y,
					m_vecSecondCoordinate[j].width, m_vecSecondCoordinate[j].height);
				//cout << "similarity:" << m_result << endl;
				if (m_result>m_areaThreshold)
				{
					m_similarityFlag.push_back(j);
					m_isOK = false;
				}
			}
			if (m_isOK == true)
			{
				m_updateData.push_back(i);
			}
		}
	}
	m_vecFirstCoordinate.clear();
	m_vecSecondCoordinate.clear();
	return m_updateData;
}
float computRectJoinUnion(int x0, int y0, int width0, int height0,
	int x1, int y1, int width1, int height1)
{
	cv::Point p1, p2;                 //p1为相交位置的左上角坐标，p2为相交位置的右下角坐标
	p1.x = std::max(x0, x1);
	p1.y = std::max(y0, y1);

	p2.x = std::min(x0 + width0, x1 + width1);
	p2.y = std::min(y0 + height0, y1 + height1);

	float AJoin = 0;
	if (p2.x > p1.x && p2.y > p1.y)            //判断是否相交
	{
		AJoin = (p2.x - p1.x)*(p2.y - p1.y);    //如果先交，求出相交面积
	}
	float A1 = width0 * height0;
	float A2 = width1 * height1;
	float AUnion = (A1 + A2 - AJoin);                 //两者组合的面积
	float m_rate = (AJoin / A1 > AJoin / A2) ? AJoin / A1 : AJoin / A2;
	if (AUnion > 0)
		return m_rate;                  //相交面积与组合面积的比例
	else
		return 0;
}