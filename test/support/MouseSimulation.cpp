#include "MouseSimulation.h"
#include <Windows.h>
#include <iostream>
#include <cstdlib>
#include <assert.h>
#include <WinUser.h>

#define changeCursor 1

using namespace std;

extern bool IsReInit;

MouseSimulation::MouseSimulation()
	:CanMouseMove(true) {
	for (int i = 1; i <= 5; ++i) {
		char comName[8];
		memset(comName, 0, sizeof(comName));
		sprintf(comName, "com%d", i);
		std::string tmp(comName);
		if (serial.OpenCOM(tmp)) {
			serial.ConfigCOM(115200);
			cout << tmp << "波特率为115200" << endl;
			break;
		}
		if (5 == i) {
			cout << "打开COM1-COM5失败!!!" << endl;
		}
	}
	memset(buf, 0, sizeof(buf));
	
	//移动至中心
	INPUT  Input = { 0 };
	Input.type = INPUT_MOUSE;
	Input.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE;
	Input.mi.dx = 65535.0f / 2;
	Input.mi.dy = 65535.0f / 2;
	SendInput(1, &Input, sizeof(INPUT));

#if changeCursor
	//更换图标
	m_cursor = LoadCursorFromFileA("cursors\\1.ani"); //载入箭头光标
	SetSystemCursor(m_cursor, 32512);
#endif


}

MouseSimulation::MouseSimulation(std::string comx, int rate)
{
	serial.OpenCOM(comx);
	serial.ConfigCOM(rate);
	cout << comx << "波特率 : "<< rate << endl;
	memset(buf, 0, sizeof(buf));
}

MouseSimulation::~MouseSimulation() {

#if changeCursor
	//换回图标
	m_cursor = LoadCursorFromFileA("cursors\\default.cur"); //载入箭头光标
	SetSystemCursor(m_cursor, 32512);
#endif
	
}

void MouseSimulation::MouseMoveRelatively(int x, int y)
{
	POINT mypoint;
	GetCursorPos(&mypoint);//获取鼠标当前所在位置  
	MouseMove(mypoint.x + x, mypoint.y + y);
}

void MouseSimulation::MouseMoveRelativelyXFilp(int x, int y)
{
	MouseMoveRelatively(-x, y);
}

void MouseSimulation::MouseMove(int x, int y)
{
	double fScreenWidth = ::GetSystemMetrics(SM_CXSCREEN) - 1;//获取屏幕分辨率宽度  
	double fScreenHeight = ::GetSystemMetrics(SM_CYSCREEN) - 1;//获取屏幕分辨率高度  
	double fx = x*(65535.0f / fScreenWidth);
	double fy = y*(65535.0f / fScreenHeight);
	INPUT  Input = { 0 };
	Input.type = INPUT_MOUSE;
	Input.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE;
	Input.mi.dx = fx;
	Input.mi.dy = fy;
	SendInput(1, &Input, sizeof(INPUT));
}

void MouseSimulation::MouseLeftDown()
{
	INPUT  Input = { 0 };
	Input.type = INPUT_MOUSE;
	Input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
	SendInput(1, &Input, sizeof(INPUT));
}

void MouseSimulation::MouseLeftUp()
{
	INPUT  Input = { 0 };
	Input.type = INPUT_MOUSE;
	Input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
	SendInput(1, &Input, sizeof(INPUT));
}

void MouseSimulation::MouseRightDown()
{
	INPUT Input = { 0 };
	Input.type = INPUT_MOUSE;
	Input.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
	SendInput(1, &Input, sizeof(INPUT));
}

void MouseSimulation::MouseRightUp()
{
	INPUT  Input = { 0 };
	Input.type = INPUT_MOUSE;
	Input.mi.dwFlags = MOUSEEVENTF_RIGHTUP;
	SendInput(1, &Input, sizeof(INPUT));
}

void MouseSimulation::MouseWheel(int x)
{
	INPUT  Input = { 0 };
	Input.type = INPUT_MOUSE;
	Input.mi.dwFlags = MOUSEEVENTF_WHEEL;
	Input.mi.mouseData = x;
	SendInput(1, &Input, sizeof(INPUT));

}

inline void MouseSimulation::MouseActionSwitch(std::string cmd) {
	MouseAction temp = NoMouseAction;
	for (int i = 0; i < 10; ++i) {
		if (strcmp(cmd.c_str(), Command[i]) == 0) {
			temp = (MouseAction)i; break;
		}
	}
	switch (temp) {
	case RightSingleClick:
		MouseRightSingleClick(); break;
	case LeftSingleClick:
		MouseLeftSingleClick(); break;
	case LeftDoubleClick:
		MouseLeftDoubleClick(); break;
	case Reset:
		IsReInit = true; break;
	case LeftButtonDown:
		MouseLeftDown(); break;
	case LeftButtonUp:
		MouseLeftUp(); break;
	case RightButtonDown:
		MouseRightDown(); break;
	case RightButtonUp:
		MouseRightUp(); break;
	case DisableMouseMove:
		CanMouseMove = false; break;
	case EnableMouseMove:
		CanMouseMove = true; break;
	default:
		break;
	}
}

MouseAction MouseSimulation::MouseMonitor(void) {
	MouseAction temp = NoMouseAction;
	std::string cmd = serial.ReadCOM();
	if (!cmd.empty()) {
		int num;
		static std::string backup;
		while (num = cmd.find("\n", 0), num > 0) {
			backup += cmd.substr(0, num + 1);
			cmd.erase(0, num + 1);
			if (backup.find("\n", 0) > 0) {
				cout << "command is : " << backup << endl;
				MouseActionSwitch(backup);
				backup.clear();
			}
		}
		backup = cmd;
	}
	return temp;
}

bool MouseSimulation::waitForInit(void) {
	bool flag = false;
	std::string cmd = serial.ReadCOM();

	if (!cmd.empty()) {
		int num;
		static std::string backup;
		while (num = cmd.find("\n", 0), num > 0) {
			backup += cmd.substr(0, num + 1);
			cmd.erase(0, num + 1);
			if (backup.find("\n", 0) > 0) {
				cout << "command is : " << backup << endl;
				if (strcmp(backup.c_str(), Command[3]) == 0)
					flag = true;
				backup.clear();
			}
		}
		backup = cmd;
	}
	return flag;
}