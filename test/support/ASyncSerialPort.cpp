#include "ASyncSerialPort.h"
#include <iostream>

using namespace std;

ASyncSerialPort::ASyncSerialPort(void)
{
	//cout<<"串口类功能测试基于Win API"<<endl;
	comHandle=NULL;
}

ASyncSerialPort::ASyncSerialPort(string comm)
{
	// OpenCOM (comm);
	// cout<<"串口类功能测试基于Win API"<<endl;
	comHandle=NULL;
	OpenCOM (comm);
}

ASyncSerialPort::~ASyncSerialPort(void)
{
	CloseCOM();
	cout<<"串口关闭"<<endl;
}

bool ASyncSerialPort::OpenCOM(string comm) 
{
	//CloseCOM();
	comHandle = CreateFileA(
		comm.c_str(),                                   //指定要打开的串口
		GENERIC_READ | GENERIC_WRITE,                   //读写访问模式
		0,                                              //共享模式 由于串口不能共享必须为0
		NULL,                                           //引用安全性属性结构 缺省为NULL
		OPEN_EXISTING,                                  //打开串口必须这样设置
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,   //重叠操作方式又称异步操作方式 读写文件必须指定一个OVERLAPPED结构
		NULL                                            //对于串口必须为NULL
		);
	if (comHandle == INVALID_HANDLE_VALUE) {
		//cout<<"打开"<<comm.c_str ()<<"失败"<<endl;
		return false;
	}
	else
	{
		ConfigCOM(9600);
		cout<<"打开"<<comm.c_str ()<<"成功"<<endl;
		return true;
	}
}

//配置串口
void ASyncSerialPort::ConfigCOM(unsigned long rate) {
	SetupComm(comHandle,1024,1024);    //输入缓冲区和输出缓冲区的大小都是1024
	COMMTIMEOUTS timeOuts;//规定读写操作的超时
	//设定读超时
	timeOuts.ReadIntervalTimeout = 1000;
	timeOuts.ReadTotalTimeoutMultiplier = 500;
	timeOuts.ReadTotalTimeoutConstant = 500;
	//设定写超时
	timeOuts.WriteTotalTimeoutMultiplier = 500;
	timeOuts.WriteTotalTimeoutConstant = 2000;
	SetCommTimeouts(comHandle,&timeOuts);
	//获取串口的初始配置
	GetCommState(comHandle,&dcb);
	//修改串口的配置并保存
	dcb.BaudRate = rate;            //波特率
	dcb.ByteSize = 8;               //指定串口当前使用的数据位(4-8)
	dcb.fParity = false;            //是否允许奇偶校验 在为true的时候看Parity设置
	dcb.Parity = ODDPARITY;          //校验方法(EVENPARITY,MARKPARITY,NOPARITY,ODDPARITY)
	dcb.StopBits = ONESTOPBIT;     //停止位(ONESTOPBIT,ONE5STOPBITS,TWOSTOPBITS)
	// SetCommState(comHandle,&dcb);
	if(!SetCommState(comHandle,&dcb)) //设置通信端口的状态
	{
		//  MessageBox(0,"通信端口设置错误!!!","Set ERROR",MB_OK);
		cout<<"通信端口设置错误!!!"<<endl;
		CloseHandle(comHandle);
		return;
	}
	//读写串口之前必须清空缓冲区
	PurgeComm(comHandle,PURGE_TXCLEAR | PURGE_RXCLEAR);
}

//关闭串口
void ASyncSerialPort:: CloseCOM() {
	if (comHandle != NULL) {
		CloseHandle(comHandle);
		comHandle = NULL;
	}
}

//---------------------------------------------------------------------------
//异步读串口数据 返回实际读到的字节数
unsigned long ASyncSerialPort::ReadCOM(unsigned char *buffer,unsigned long t_size)
{
	DWORD readBytes = t_size + 1;
	DWORD error;
	COMSTAT comStat;
	OVERLAPPED lapped;
	memset(&lapped,0,sizeof(OVERLAPPED));

	//清除错误或者缓冲区无数据
	if (ClearCommError(comHandle, &error, &comStat) && error > 0) {
		PurgeComm(comHandle, PURGE_RXABORT | PURGE_RXCLEAR); /*清除输入缓冲区*/
		return 0;
	}
	if (comStat.cbInQue==0) 
		return 0;

	//lapped.hEvent事件设置为无信号状态
	lapped.hEvent = CreateEvent(NULL,TRUE,FALSE,NULL);
	//清除错误信息
	//ClearCommError(comHandle,&error,&comStat);
	readBytes = min(readBytes, (DWORD)comStat.cbInQue);
	bool result = ReadFile(
		comHandle,
		buffer,
		readBytes,
		&readBytes,
		&lapped
		);
	if(!result)
	{
		//表示串口正在进行读取数据
		if(GetLastError() == ERROR_IO_PENDING)
		{
			//等待2s 让读操作完成
			WaitForSingleObject(lapped.hEvent,2000);
			//WaitForSingleObject(lapped.hEvent, INFINITE);
			//当串口读操作完成后 lapped.hEvent事件会变为有信号状态
			PurgeComm(comHandle,PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);
			return readBytes;
		}
		return 0;
	}
	buffer[readBytes] = '\0';
	PurgeComm(comHandle,PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);
	return readBytes;
}

//异步写串口数据
unsigned long ASyncSerialPort:: WriteCOM(unsigned char *buffer,unsigned long t_size)
{
	DWORD writeBytes = t_size + 1;
	DWORD error;
	COMSTAT comStat;
	OVERLAPPED lapped;
	memset(&lapped,0,sizeof(OVERLAPPED));
	lapped.Offset = 0;
	lapped.OffsetHigh = 0;
	//lapped.hEvent事件设置为无信号状态
	lapped.hEvent = CreateEvent(NULL,TRUE,FALSE,NULL);
	//清除错误信息
	ClearCommError(comHandle,&error,&comStat);
	bool result = WriteFile(
		comHandle,
		buffer,
		writeBytes,
		&writeBytes,
		&lapped);
	if(!result)
	{
		//表示串口正在进行写取数据
		if(GetLastError() == ERROR_IO_PENDING)
		{
			//等待2s 让读操作完成
			WaitForSingleObject(lapped.hEvent,2000);
			
			return writeBytes;
		}
		return 0;
	}
	return writeBytes;
}

std::string ASyncSerialPort::ReadCOM(void) {
	DWORD error;
	COMSTAT comStat;
	OVERLAPPED lapped;
	memset(&lapped, 0, sizeof(OVERLAPPED));
	DWORD BytesRead = 0;
	BOOL  bRead = TRUE;
	BOOL  bResult = TRUE;
	unsigned char RXBuff;
	std::string tmp;
	for (int i = 0; i < 15; ++i) {
		bResult = ClearCommError(comHandle, &error, &comStat);
		if (comStat.cbInQue == 0)
			break;
		if (bRead)
		{
			bResult = ReadFile(comHandle,		// Handle to COMM port 
				&RXBuff,				// RX Buffer Pointer
				1,					// Read one byte
				&BytesRead,			// Stores number of bytes read
				&lapped);		// pointer to the m_ov structure
									// deal with the error code 
			if (!bResult) {
				if (ERROR_IO_PENDING == GetLastError())
					// asynchronous i/o is still in progress 
					// Proceed on to GetOverlappedResults();
					bRead = FALSE;

			}
			else {
				// ReadFile() returned complete. It is not necessary to call GetOverlappedResults()
				bRead = TRUE;
			}
		}  // close if (bRead)
		if (!bRead) {
			bRead = TRUE;
			bResult = GetOverlappedResult(comHandle,	// Handle to COMM port 
				&lapped,		// Overlapped structure
				&BytesRead,		// Stores number of bytes read
				TRUE); 			// Wait flag
								// deal with the error code 
			if (!bResult) {
				printf("GetOverlappedResults() in ReadFile()\n");
			}
		}  // close if (!bRead)
		
		if(RXBuff)
			tmp += RXBuff;
	}
	return tmp;
}

