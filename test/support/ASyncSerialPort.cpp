#include "ASyncSerialPort.h"
#include <iostream>

using namespace std;

ASyncSerialPort::ASyncSerialPort(void)
{
	//cout<<"�����๦�ܲ��Ի���Win API"<<endl;
	comHandle=NULL;
}

ASyncSerialPort::ASyncSerialPort(string comm)
{
	// OpenCOM (comm);
	// cout<<"�����๦�ܲ��Ի���Win API"<<endl;
	comHandle=NULL;
	OpenCOM (comm);
}

ASyncSerialPort::~ASyncSerialPort(void)
{
	CloseCOM();
	cout<<"���ڹر�"<<endl;
}

bool ASyncSerialPort::OpenCOM(string comm) 
{
	//CloseCOM();
	comHandle = CreateFileA(
		comm.c_str(),                                   //ָ��Ҫ�򿪵Ĵ���
		GENERIC_READ | GENERIC_WRITE,                   //��д����ģʽ
		0,                                              //����ģʽ ���ڴ��ڲ��ܹ������Ϊ0
		NULL,                                           //���ð�ȫ�����Խṹ ȱʡΪNULL
		OPEN_EXISTING,                                  //�򿪴��ڱ�����������
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,   //�ص�������ʽ�ֳ��첽������ʽ ��д�ļ�����ָ��һ��OVERLAPPED�ṹ
		NULL                                            //���ڴ��ڱ���ΪNULL
		);
	if (comHandle == INVALID_HANDLE_VALUE) {
		//cout<<"��"<<comm.c_str ()<<"ʧ��"<<endl;
		return false;
	}
	else
	{
		ConfigCOM(9600);
		cout<<"��"<<comm.c_str ()<<"�ɹ�"<<endl;
		return true;
	}
}

//���ô���
void ASyncSerialPort::ConfigCOM(unsigned long rate) {
	SetupComm(comHandle,1024,1024);    //���뻺����������������Ĵ�С����1024
	COMMTIMEOUTS timeOuts;//�涨��д�����ĳ�ʱ
	//�趨����ʱ
	timeOuts.ReadIntervalTimeout = 1000;
	timeOuts.ReadTotalTimeoutMultiplier = 500;
	timeOuts.ReadTotalTimeoutConstant = 500;
	//�趨д��ʱ
	timeOuts.WriteTotalTimeoutMultiplier = 500;
	timeOuts.WriteTotalTimeoutConstant = 2000;
	SetCommTimeouts(comHandle,&timeOuts);
	//��ȡ���ڵĳ�ʼ����
	GetCommState(comHandle,&dcb);
	//�޸Ĵ��ڵ����ò�����
	dcb.BaudRate = rate;            //������
	dcb.ByteSize = 8;               //ָ�����ڵ�ǰʹ�õ�����λ(4-8)
	dcb.fParity = false;            //�Ƿ�������żУ�� ��Ϊtrue��ʱ��Parity����
	dcb.Parity = ODDPARITY;          //У�鷽��(EVENPARITY,MARKPARITY,NOPARITY,ODDPARITY)
	dcb.StopBits = ONESTOPBIT;     //ֹͣλ(ONESTOPBIT,ONE5STOPBITS,TWOSTOPBITS)
	// SetCommState(comHandle,&dcb);
	if(!SetCommState(comHandle,&dcb)) //����ͨ�Ŷ˿ڵ�״̬
	{
		//  MessageBox(0,"ͨ�Ŷ˿����ô���!!!","Set ERROR",MB_OK);
		cout<<"ͨ�Ŷ˿����ô���!!!"<<endl;
		CloseHandle(comHandle);
		return;
	}
	//��д����֮ǰ������ջ�����
	PurgeComm(comHandle,PURGE_TXCLEAR | PURGE_RXCLEAR);
}

//�رմ���
void ASyncSerialPort:: CloseCOM() {
	if (comHandle != NULL) {
		CloseHandle(comHandle);
		comHandle = NULL;
	}
}

//---------------------------------------------------------------------------
//�첽���������� ����ʵ�ʶ������ֽ���
unsigned long ASyncSerialPort::ReadCOM(unsigned char *buffer,unsigned long t_size)
{
	DWORD readBytes = t_size + 1;
	DWORD error;
	COMSTAT comStat;
	OVERLAPPED lapped;
	memset(&lapped,0,sizeof(OVERLAPPED));

	//���������߻�����������
	if (ClearCommError(comHandle, &error, &comStat) && error > 0) {
		PurgeComm(comHandle, PURGE_RXABORT | PURGE_RXCLEAR); /*������뻺����*/
		return 0;
	}
	if (comStat.cbInQue==0) 
		return 0;

	//lapped.hEvent�¼�����Ϊ���ź�״̬
	lapped.hEvent = CreateEvent(NULL,TRUE,FALSE,NULL);
	//���������Ϣ
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
		//��ʾ�������ڽ��ж�ȡ����
		if(GetLastError() == ERROR_IO_PENDING)
		{
			//�ȴ�2s �ö��������
			WaitForSingleObject(lapped.hEvent,2000);
			//WaitForSingleObject(lapped.hEvent, INFINITE);
			//�����ڶ�������ɺ� lapped.hEvent�¼����Ϊ���ź�״̬
			PurgeComm(comHandle,PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);
			return readBytes;
		}
		return 0;
	}
	buffer[readBytes] = '\0';
	PurgeComm(comHandle,PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);
	return readBytes;
}

//�첽д��������
unsigned long ASyncSerialPort:: WriteCOM(unsigned char *buffer,unsigned long t_size)
{
	DWORD writeBytes = t_size + 1;
	DWORD error;
	COMSTAT comStat;
	OVERLAPPED lapped;
	memset(&lapped,0,sizeof(OVERLAPPED));
	lapped.Offset = 0;
	lapped.OffsetHigh = 0;
	//lapped.hEvent�¼�����Ϊ���ź�״̬
	lapped.hEvent = CreateEvent(NULL,TRUE,FALSE,NULL);
	//���������Ϣ
	ClearCommError(comHandle,&error,&comStat);
	bool result = WriteFile(
		comHandle,
		buffer,
		writeBytes,
		&writeBytes,
		&lapped);
	if(!result)
	{
		//��ʾ�������ڽ���дȡ����
		if(GetLastError() == ERROR_IO_PENDING)
		{
			//�ȴ�2s �ö��������
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

