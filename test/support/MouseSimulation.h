#pragma once
#include "ASyncSerialPort.h"

typedef enum MouseAction {
	RightSingleClick,
	LeftSingleClick,
	LeftDoubleClick,
	Reset,
	LeftButtonDown, 
	LeftButtonUp,
	RightButtonDown, 
	RightButtonUp,
	DisableMouseMove,
	EnableMouseMove,
	NoMouseAction
}MouseAction;

class MouseSimulation {

public:

	MouseSimulation();

	MouseSimulation(std::string comx, int rate);

	~MouseSimulation();

	void MouseMoveRelatively(int x, int y);  // 鼠标移动到指定位置（相对坐标）

	void MouseMoveRelativelyXFilp(int x, int y);  // 鼠标移动到指定位置（相对坐标）

	void MouseMove(int x, int y);  // 鼠标移动到指定位置（绝对坐标）

	void MouseLeftDown();  // 鼠标左键按下

	void MouseLeftUp();  // 鼠标左键放开 

	void MouseRightDown();  // 鼠标右键按下

	void MouseRightUp();  // 鼠标右键放开 

	void MouseWheel(int x);

	void MouseActionSwitch(std::string cmd);

	MouseAction MouseMonitor(void);

	bool waitForInit(void);

	ASyncSerialPort serial;

	bool CanMouseMove;

protected:

	inline void MouseLeftSingleClick() {
		MouseLeftDown(); MouseLeftUp();
	}

	inline void MouseRightSingleClick() {
		MouseRightDown(); MouseRightUp();
	}

	inline void MouseLeftDoubleClick() {
		MouseLeftSingleClick(); MouseLeftSingleClick();
	}

	const char Command[11][9] = {
		"rclick\n", 
		"lclick\n", 
		"dclick\n",
		"Areset\n",
		"llongd\n",
		"llongu\n",
		"rlongd\n",
		"rlongu\n",
		"dmmove\n",
		"emmove\n",
		"SiDown\n"
	};

	unsigned char buf[10];

	std::string COMX;

	HCURSOR  m_cursor;

};

