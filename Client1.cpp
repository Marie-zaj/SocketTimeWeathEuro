#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

SOCKET ConnectSocket = INVALID_SOCKET;


DWORD WINAPI Receiver(LPVOID)
{
    char recvbuf[DEFAULT_BUFLEN];

    while (true)
    {
        int iResult = recv(ConnectSocket, recvbuf, DEFAULT_BUFLEN, 0);

        if (iResult > 0)
        {
            recvbuf[iResult] = '\0';
            printf("%s", recvbuf);
        }
        else
            break;
    }

    return 0;
}


DWORD WINAPI Sender(LPVOID)
{
    char sendbuf[DEFAULT_BUFLEN];

    while (true)
    {
        printf("Enter command: ");
        gets_s(sendbuf);

        send(ConnectSocket, sendbuf, strlen(sendbuf), 0);
    }

    return 0;
}

int main()
{
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    struct addrinfo hints, * result = NULL;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    getaddrinfo("127.0.0.1", DEFAULT_PORT, &hints, &result);

    ConnectSocket = socket(result->ai_family,
        result->ai_socktype,
        result->ai_protocol);

    connect(ConnectSocket, result->ai_addr, (int)result->ai_addrlen);

    printf("Connected to server.\n");

    CreateThread(NULL, 0, Receiver, NULL, 0, NULL);
    CreateThread(NULL, 0, Sender, NULL, 0, NULL);

    Sleep(INFINITE);

    closesocket(ConnectSocket);
    WSACleanup();

    return 0;
}