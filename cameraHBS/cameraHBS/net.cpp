// net.cpp
// auther : ppppdm

#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>
#include "net.h"

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Kernel32.lib")

/**
 * ö�ٱ�����OPERATOIN
 * ���ܣ�ָ��IO��������
 **/
enum OPERATOIN{
	RECV_POSTED,
	SEND_POSTED
};

/**
 * �ṹ�����ƣ�PER_IO_DATA
 * �ṹ�幦�ܣ��ص�I/O��Ҫ�õ��Ľṹ�壬��ʱ��¼IO����
 **/
const int DataBuffSize  = 1024; //* 2
typedef struct
{
	OVERLAPPED overlapped;
	WSABUF databuff;
	char buffer[ DataBuffSize ];
	int BufferLen;
	int operationType;
}PER_IO_OPERATEION_DATA, *LPPER_IO_OPERATION_DATA, *LPPER_IO_DATA, PER_IO_DATA;

/**
 * �ṹ�����ƣ�PER_HANDLE_DATA
 * �ṹ��洢����¼�����׽��ֵ����ݣ��������׽��ֵı������׽��ֵĶ�Ӧ�Ŀͻ��˵ĵ�ַ��
 * �ṹ�����ã��������������Ͽͻ���ʱ����Ϣ�洢���ýṹ���У�֪���ͻ��˵ĵ�ַ�Ա��ڻطá�
 **/
typedef struct
{
	SOCKET socket;
}PER_HANDLE_DATA, *LPPER_HANDLE_DATA;


// ��������
int LoadSocketLib(void);
int createThreadHandleIOCP(HANDLE ioCompletePort);
int createServerSocketAndWorkWithIOCP(HANDLE ioCompletePort);
DWORD WINAPI ServerWorkThread(LPVOID IpParam);

// ȫ�ֱ���
int g_totalWorkingThreads = 0;
int DefaultPort = 6000;
HANDLE hMutex = CreateMutex(NULL, FALSE, NULL);


// The enter of server net work
void startNetWork(void)
{
	int err;

	// ����socket��̬���ӿ�
	err = LoadSocketLib();
	if (err != 0){
		fprintf(stderr, "Can't load socket dll, will exit!\n");
		system("pause");
		exit(-1);
	}

	// ������ɶ˿�IOCP
	HANDLE completionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE,NULL,0,0);
	if(completionPort == NULL)
	{
		fprintf(stderr, "CreateIoCompletionPort failed. Error:%d\n", GetLastError());
		system("pause");
		exit(-1);
	}

	// �������̴߳���IOCP����
	err = createThreadHandleIOCP(completionPort);
	if (err != 0){
		fprintf(stderr, "Create threads for handle IOCP error��\n");
		system("pause");
		exit(-1);
	}


	// ���������׽��֣�����������׽�������ɶ˿�IOCP����
	err = createServerSocketAndWorkWithIOCP(completionPort);
	if (err != 0){
		fprintf(stderr, "ServerSocket work error, will exit!\n");
		system("pause");
		exit(-1);
	}

	// ���ٿͻ����׽��ֺͷ����׽���

	// ����IOCP�߳�

	// ������ɶ˿�IOCP
	
	//exit_pefect:

	return;
}

// ����socket��̬���ӿ�
int LoadSocketLib(void)
{
	WORD wVersionRequested = MAKEWORD(2, 2); // ����2.2�汾��WinSock��
	WSADATA wsaData;	// ����Windows Socket�Ľṹ��Ϣ
	DWORD err = WSAStartup(wVersionRequested, &wsaData);

	if (0 != err){	// ����׽��ֿ��Ƿ�����ɹ�
		fprintf(stderr,"Request Windows Socket Library Error!\n");
		system("pause");
		return -1;
	}
	if(LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2){// ����Ƿ�����������汾���׽��ֿ�
		WSACleanup();
		fprintf(stderr,"Request Windows Socket Version 2.2 Error!\n");
		system("pause");
		return -1;
	}

	return 0;
}

// �������̴߳���IOCP����
int createThreadHandleIOCP(HANDLE ioCompletionPort)
{
	// ȷ���������ĺ�������
	SYSTEM_INFO mySysInfo;
	GetSystemInfo(&mySysInfo);
	printf("Number of Process : %d\n", mySysInfo.dwNumberOfProcessors);

	// ���ڴ������ĺ������������߳�
	for(DWORD i = 0; i < (mySysInfo.dwNumberOfProcessors * 2); i++){
		// �����������������̣߳�������ɶ˿ڴ��ݵ����߳�
		HANDLE ThreadHandle = CreateThread(NULL, 0, ServerWorkThread, ioCompletionPort, 0, NULL);
		if(NULL == ThreadHandle){
			fprintf(stderr, "Create Thread Handle failed. Error:%d\n",GetLastError());
		}
		else {
			g_totalWorkingThreads += 1;
		}
		// ������ٵ��ôι����̣߳���������ر�����
		// ���ǵ��������ر�ʱҪȷ����Щ�̵߳Ĺرգ������Ҫ����Щ���������
		CloseHandle(ThreadHandle);
	}

	if (g_totalWorkingThreads == 0){
		fprintf(stderr, "No work thread be created!\n");
		return -1;
	}
	return 0;
}

//
int createServerSocketAndWorkWithIOCP(HANDLE ioCompletionPort)
{
	// ������ʽ�׽���
	SOCKET srvSocket = socket(AF_INET, SOCK_STREAM, 0);

	// ��SOCKET������
	SOCKADDR_IN srvAddr;
	srvAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	srvAddr.sin_family = AF_INET;
	srvAddr.sin_port = htons(DefaultPort);
	int bindResult = bind(srvSocket, (SOCKADDR*)&srvAddr, sizeof(SOCKADDR));
	if(SOCKET_ERROR == bindResult){
		fprintf(stderr, "Bind failed. Error:%d\n",GetLastError());
		system("pause");
		return -1;
	}

	// ��SOCKET����Ϊ����ģʽ
	int listenResult = listen(srvSocket, 10);
	if(SOCKET_ERROR == listenResult){
		fprintf(stderr, "Listen failed. Error:%d\n",GetLastError());
		system("pause");
		return -1;
	}

	fprintf(stdout,"Server start, wait for client connect\n");
	while(true){
		PER_HANDLE_DATA * PerHandleData = NULL;
		SOCKADDR_IN saRemote;
		int RemoteLen;
		SOCKET acceptSocket;

		// �������ӣ���������ɶˣ����������AcceptEx()
		RemoteLen = sizeof(saRemote);
		acceptSocket = accept(srvSocket, (SOCKADDR*)&saRemote, &RemoteLen);
		if(SOCKET_ERROR == acceptSocket){	// ���տͻ���ʧ��
			fprintf(stderr, "Accept Socket Error: %d\n",GetLastError());
			continue;
		}

		// �����������׽��ֹ����ĵ����������Ϣ�ṹ
		PerHandleData = (LPPER_HANDLE_DATA)GlobalAlloc(GPTR, sizeof(PER_HANDLE_DATA));	// �ڶ���Ϊ���PerHandleData����ָ����С���ڴ�
		PerHandleData -> socket = acceptSocket;

		// �������׽��ֺ���ɶ˿ڹ���
		CreateIoCompletionPort((HANDLE)(PerHandleData -> socket), ioCompletionPort, (DWORD)PerHandleData, 0);

		// ��ʼ�ڽ����׽����ϴ���I/Oʹ���ص�I/O����
		// ���½����׽�����Ͷ��һ�������첽
		// WSARecv��WSASend������ЩI/O������ɺ󣬹������̻߳�ΪI/O�����ṩ����	
		LPPER_IO_OPERATION_DATA PerIoData = NULL;
		PerIoData = (LPPER_IO_OPERATION_DATA)GlobalAlloc(GPTR, sizeof(PER_IO_OPERATEION_DATA));
		ZeroMemory(&(PerIoData -> overlapped), sizeof(OVERLAPPED));
		PerIoData->databuff.len = 1024;
		PerIoData->databuff.buf = PerIoData->buffer;
		PerIoData->operationType = RECV_POSTED;

		DWORD RecvBytes;
		DWORD Flags = 0;
		WSARecv(PerHandleData->socket, &(PerIoData->databuff), 1, &RecvBytes, &Flags, &(PerIoData->overlapped), NULL);
	}

	return 0;
}

// �������̺߳���
DWORD WINAPI ServerWorkThread(LPVOID IpParam)
{
	HANDLE CompletionPort = (HANDLE)IpParam;
	DWORD BytesTransferred;
	LPOVERLAPPED IpOverlapped;
	LPPER_HANDLE_DATA PerHandleData = NULL;
	LPPER_IO_DATA PerIoData = NULL;
	BOOL bRet = false;
	DWORD RecvBytes;
	DWORD Flags = 0;

	while(true){
		bRet = GetQueuedCompletionStatus(CompletionPort, &BytesTransferred, (PULONG_PTR)&PerHandleData, (LPOVERLAPPED*)&IpOverlapped, INFINITE);

		// 
		if(bRet == false){
			fprintf(stderr, "GetQueuedCompletionStatus Error:%d\n",GetLastError());
			fprintf(stderr, "BytesTransferred %d, &IpOverlapped %d\n", BytesTransferred, &IpOverlapped);
			if (IpOverlapped == NULL){
				fprintf(stderr, "WorkThread %d, will quit!\n", 1);
				break;
			}
			else {
				continue;
			}
		}
		
		PerIoData = (LPPER_IO_DATA)CONTAINING_RECORD(IpOverlapped, PER_IO_DATA, overlapped);

		// ������׽������Ƿ��д�����
		if(0 == BytesTransferred){
			fprintf(stdout, "operationType %d\n", PerIoData->operationType);
			fprintf(stdout, "client close! Thread %d\n", GetCurrentThreadId());
			closesocket(PerHandleData->socket);
			GlobalFree(PerHandleData);
			GlobalFree(PerIoData);		
			continue;
		}

		// �������û�з�������
		if(PerIoData->operationType == RECV_POSTED){
			WaitForSingleObject(hMutex,INFINITE);
			fprintf(stdout, "A Client says: %s\n", PerIoData->databuff.buf);
			ReleaseMutex(hMutex);

			// Ϊ��һ���ص����ý�����I/O��������
			ZeroMemory(&(PerIoData->overlapped), sizeof(OVERLAPPED)); // ����ڴ�
			PerIoData->databuff.len = 1024;
			//ZeroMemory(&(PerIoData->buffer), 1024); // ����ڴ�
			//PerIoData->databuff.buf = PerIoData->buffer;
			//PerIoData->operationType = SEND_POSTED;
			WSASend(PerHandleData->socket, &(PerIoData->databuff), 1, NULL, Flags, &(PerIoData->overlapped), NULL);
		}
		if(PerIoData->operationType == SEND_POSTED){
			WaitForSingleObject(hMutex,INFINITE);
			fprintf(stdout, "Operate type send_posted, thread %d, bytes trans %d\n", GetCurrentThreadId(),BytesTransferred);
			ReleaseMutex(hMutex);

			// Ϊ��һ���ص����ý�����I/O��������
			ZeroMemory(&(PerIoData->overlapped), sizeof(OVERLAPPED)); // ����ڴ�
			PerIoData->databuff.len = 1024;
			PerIoData->databuff.buf = PerIoData->buffer;
			PerIoData->operationType = RECV_POSTED;
			WSARecv(PerHandleData->socket, &(PerIoData->databuff), 1, &RecvBytes, &Flags, &(PerIoData->overlapped), NULL);
		}
	}

	return 0;
}