#include "stdafx.h"

#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <fstream>
#include <numeric>
#include <cstdio>

#include "asms\colotracker.h"
#include "support\MouseSimulation.h"

#define shaking 1
#define winsize cv::Size(40*5, 30*5)
#define multiply 1.2
#define wwid 35*multiply
#define wheight 50*multiply
#define Wheelmultiple(x) x < 5 ? 2 * x : 4 * x

//设置入口地址 
#pragma comment( linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"" )  //设置入口地址不显示控制台

using namespace cv;
using namespace std;

cv::Mat image, frame;
cv::Rect selection, location;
cv::Point origin;
bool selectObject = false;
int trackObject = 0;
bool IsReInit = false, InitFromSelectedRect = false, wininit = true;

static void onMouse(int event, int x, int y, int, void*) {
	if (selectObject)
	{
		selection.x = MIN(x, origin.x);
		selection.y = MIN(y, origin.y);
		selection.width = std::abs(x - origin.x);
		selection.height = std::abs(y - origin.y);

		selection &= Rect(0, 0, image.cols, image.rows);
	}

	switch (event)
	{
	case EVENT_LBUTTONDOWN:
		origin = Point(x, y);
		selection = Rect(x, y, 0, 0);
		selectObject = true;
		break;
	case EVENT_LBUTTONUP:
		selectObject = false;
		if (selection.width > 0 && selection.height > 0)
			trackObject = -1;
		break;
	}
}

static inline cv::Point change2XY(cv::Rect a) {
	//static variable
	static double XS = ::GetSystemMetrics(SM_CXSCREEN) - 1;
	static double YS = ::GetSystemMetrics(SM_CYSCREEN) - 1;

	cv::Point temp = a.tl();
	double x = (frame.cols - location.width)*0.7;
	double y = (frame.rows - location.height)*0.7;

	return cv::Point(temp.x * XS / x, temp.y * YS / y);
}

int main(void) {

	std::string winname;
	std::string navigatorWindows = "1";

	//打开摄像头，默认外置
	VideoCapture cap;
	for (int i = 0; i >= 0; --i) {
		if (cap.open(i)) {
			winname = "CAM" + to_string(i) + "初始化窗口";
			if (!cap.set(CAP_PROP_FRAME_WIDTH, 1280))
				return -1;
			if (!cap.set(CAP_PROP_FRAME_HEIGHT, 720))
				return -1;
			break;
		}
	}
	if (!cap.isOpened())
		return -1;

	ColorTracker asms;

	MouseSimulation mouse;
	Rect last, initRect;

	//计算相关参数
	cap >> frame;
	initRect = Rect((frame.cols - wwid) / 2, (frame.rows - wheight) / 2-50, wwid, wheight);

	bool init = true;
	bool showNavigator = false;
	HWND navigatorHandle, MainWindowsHandle;

	//建立主窗口
	cv::namedWindow(winname);
	cv::moveWindow(winname, (::GetSystemMetrics(SM_CXSCREEN) - 1) / 2 - frame.cols / 2 - 10,
		(::GetSystemMetrics(SM_CYSCREEN) - 1) / 2 - frame.rows / 2 - 30);
	setMouseCallback(winname, onMouse, 0);

	//窗口句柄
	if (MainWindowsHandle = FindWindowA(0, winname.c_str()), MainWindowsHandle == NULL)
		return -1;

	while (cap.read(frame)) {
		Mat navigator(frame.size(), CV_8UC3, cv::Scalar(192, 192, 192));
		cv::flip(frame, frame, 0);
		image = frame.clone();
		if (trackObject) {
			if (trackObject < 0) {

				if (InitFromSelectedRect) {
					selection = initRect;
					InitFromSelectedRect = false;
				}

				asms.init(frame, selection);
				last = selection;
				trackObject = 1;
				showNavigator = true;
				wininit = true;
				//build navigator windows
				namedWindow(navigatorWindows);
				moveWindow(navigatorWindows, 10, 570);
				setWindowProperty(navigatorWindows, CV_WINDOW_FULLSCREEN, CV_WINDOW_FULLSCREEN);
				//cancel main windows
				//setMouseCallback(winname, 0, 0);
				cv::destroyWindow(winname);
				MainWindowsHandle = NULL;
			}
			location = asms.update(frame);
			cv::rectangle(image, location, cv::Scalar(0, 0, 255), 2);
			mouse.MouseMonitor();
			if (mouse.CanMouseMove) {
				mouse.MouseMove(change2XY(location).x-320, change2XY(location).y);
				last = location;
			}
			else
				mouse.MouseWheel(Wheelmultiple(last.y - location.y));
			//last = location;

			if (showNavigator) {
				if (wininit) {
					// Resize
					if (navigatorHandle = FindWindowA(0, navigatorWindows.c_str()), navigatorHandle == NULL)
						printf("Failed FindWindow\n");
					//borderless
					SetWindowLong(navigatorHandle, GWL_STYLE, GetWindowLong(navigatorHandle, GWL_EXSTYLE) | WS_EX_TOPMOST);
					ShowWindow(navigatorHandle, SW_SHOW);
					wininit = false;
				}
			}
			SetWindowPos(navigatorHandle, HWND_TOPMOST, 0, 0, winsize.width, winsize.height,
				(SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE) & ~SWP_NOSIZE);

			cv::Mat temp = navigator.clone();
			cv::ellipse(temp, (location.tl() + location.br()) / 2, cv::Size(location.width / 2, location.height / 2),
				0.f, 0.f, 360.f, cv::Scalar(0, 0, 0), -1);
			cv::resize(temp, temp, winsize);
			imshow(navigatorWindows, temp);
		}
		if (selectObject && selection.width > 0 && selection.height > 0) {
			Mat roi(image, selection);
			bitwise_not(roi, roi);
		}

		if (init)
			cv::rectangle(image, initRect, cv::Scalar(0, 255, 255), 3);

		if (!showNavigator) {
			SetWindowPos(MainWindowsHandle, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);
			cv::imshow(winname, image);
		}

		if ((0xff & waitKey(1)) == 27)  goto exit;

		if (0 == trackObject&&mouse.waitForInit()) {
			trackObject = -1;
			InitFromSelectedRect = true;
		}

		if (1 == trackObject &&IsReInit) {
			IsReInit = false;
			showNavigator = false;
			init = true;
			trackObject = 0;
			//destory navigator windows
			DestroyWindow(navigatorHandle);
			cv::destroyWindow(navigatorWindows);
			cv::namedWindow(winname);
			//setMouseCallback(winname, onMouse, 0);
			//窗口置顶
			MainWindowsHandle = FindWindowA(0, winname.c_str());
			SetWindowPos(MainWindowsHandle, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
			SwitchToThisWindow(MainWindowsHandle, TRUE);
		}
	}

exit:
	return 0;
}
