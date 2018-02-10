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

	void MouseMoveRelatively(int x, int y);  // ����ƶ���ָ��λ�ã�������꣩

	void MouseMoveRelativelyXFilp(int x, int y);  // ����ƶ���ָ��λ�ã�������꣩

	void MouseMove(int x, int y);  // ����ƶ���ָ��λ�ã��������꣩

	void MouseLeftDown();  // ����������

	void MouseLeftUp();  // �������ſ� 

	void MouseRightDown();  // ����Ҽ�����

	void MouseRightUp();  // ����Ҽ��ſ� 

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

