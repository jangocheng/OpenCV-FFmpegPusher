
#include "MediaCapture.h"
#include <opencv2/highgui.hpp>
#include <iostream>

#pragma comment(lib,"opencv_world320.lib")
using namespace std;

class MediaCaptureImpl :public MediaCapture
{
public:
	cv::VideoCapture cam;

	//初始化相机
	bool MediaCapture::Init(int cameraIndex)
	{
		///opencv打开流
		cam.open(cameraIndex);
		if (!cam.isOpened())
		{
			cout << "cam open failed!" << endl;
			return false;
		}
		cout << cameraIndex << " cam open success" << endl;
		//获取流宽高信息
		width = cam.get(cv::CAP_PROP_FRAME_WIDTH);
		height = cam.get(cv::CAP_PROP_FRAME_HEIGHT);
		fps = cam.get(cv::CAP_PROP_FPS);
		//fps有可能获取不到
		if (fps == 0) 
			fps = 25;

		return true;
	}

	//初始化网络流
	bool MediaCapture::Init(const char *url)
	{
		cam.open(url);
		if (!cam.isOpened())
		{
			cout << "cam open failed!" << endl;
			return false;
		}
		cout << url << " cam open success" << endl;
		width = cam.get(cv::CAP_PROP_FRAME_WIDTH);
		height = cam.get(cv::CAP_PROP_FRAME_HEIGHT);
		fps = cam.get(cv::CAP_PROP_FPS);
		if (fps == 0) 
			fps = 25;
		return true;
	}

	//停止
	void MediaCapture::Stop()
	{
		DataThread::Stop();
		if (cam.isOpened())
		{
			cam.release();
		}
	}

	void Main()
	{
		cout << "进入线程"<< endl;
		cv::Mat frame;
		while (!isExit)
		{
			if (!cam.read(frame))
			{
				Sleep(1);
				continue;
			}
			if (frame.empty())
			{
				Sleep(1);
				continue;
			}
			//确保数据是连续的 
			Data d((char*)frame.data, frame.cols*frame.rows*frame.elemSize());
			Push(d);
		}
		cout << "退出线程" << endl;
	}

};

MediaCapture *MediaCapture::Get(unsigned char index) 
{
	static MediaCaptureImpl vc[255];
	return &vc[index];
}

MediaCapture::MediaCapture()
{

}

MediaCapture::~MediaCapture()
{

}
