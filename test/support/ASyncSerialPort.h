#pragma once

#include <windows.h>
#include <string>

class ASyncSerialPort {

public:

	ASyncSerialPort(void);

	ASyncSerialPort(std::string comm);

	virtual ~ASyncSerialPort(void);

	bool OpenCOM(std::string comm);

	void CloseCOM();

	unsigned long WriteCOM(unsigned char *buffer,unsigned long t_size);

	std::string ReadCOM(void);

	unsigned long ReadCOM(unsigned char *buffer,unsigned long t_size);

	void ConfigCOM(unsigned long rate);

protected:

	HANDLE comHandle;  //串口的句柄

	DCB dcb;                  //设备控制块

};

