// net.cpp
// auther : ppppdm

#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>
#include "net.h"

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Kernel32.lib")

// Function Define
int LoadSocketLib(void);
int createThreadHandleIOCP(HANDLE ioCompletePort);
int createServerSocketAndWorkWithIOCP(HANDLE ioCompletePort);
DWORD WINAPI ServerWorkThread(LPVOID IpParam);

// Global Value
int g_totalWorkingThreads = 0;
int DefaultPort = 6000;

//The enter of server net work
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
	
	exit_pefect:

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
int createThreadHandleIOCP(HANDLE ioCompletePort)
{
	// ȷ���������ĺ�������
	SYSTEM_INFO mySysInfo;
	GetSystemInfo(&mySysInfo);
	printf("Number of Process : %d\n", mySysInfo.dwNumberOfProcessors);

	// ���ڴ������ĺ������������߳�
	for(DWORD i = 0; i < (mySysInfo.dwNumberOfProcessors * 2); i++){
		// �����������������̣߳�������ɶ˿ڴ��ݵ����߳�
		HANDLE ThreadHandle = CreateThread(NULL, 0, ServerWorkThread, ioCompletePort, 0, NULL);
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
int createServerSocketAndWorkWithIOCP(HANDLE ioCompletePort)
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

	return 0;
}