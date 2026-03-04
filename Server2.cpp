#undef UNICODE
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <time.h>

#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

SOCKET ClientSocket = INVALID_SOCKET;

CRITICAL_SECTION cs;

void send_response(const char* text)
{
    EnterCriticalSection(&cs);
    send(ClientSocket, text, strlen(text), 0);
    LeaveCriticalSection(&cs);
}

DWORD WINAPI handle_time(LPVOID)
{
    time_t now = time(NULL);
    struct tm t;
    localtime_s(&t, &now);

    char buffer[128];
    strftime(buffer, sizeof(buffer), "Current time: %H:%M:%S\n", &t);

    send_response(buffer);
    return 0;
}

DWORD WINAPI handle_date(LPVOID)
{
    time_t now = time(NULL);
    struct tm t;
    localtime_s(&t, &now);

    char buffer[128];
    strftime(buffer, sizeof(buffer), "Current date: %Y-%m-%d\n", &t);

    send_response(buffer);
    return 0;
}

DWORD WINAPI handle_weather(LPVOID param)
{
    char* city = (char*)param;

    char buffer[256];
    sprintf_s(buffer, "Weather in %s: +18 C, sunny\n", city);

    send_response(buffer);
    delete[] city;
    return 0;
}

DWORD WINAPI handle_euro(LPVOID)
{
    send_response("Euro rate: 42.30 UAH\n");
    return 0;
}

DWORD WINAPI handle_bitcoin(LPVOID)
{
    send_response("Bitcoin rate: 62000 USD\n");
    return 0;
}

DWORD WINAPI Receiver(LPVOID)
{
    char recvbuf[DEFAULT_BUFLEN];

    while (true)
    {
        int iResult = recv(ClientSocket, recvbuf, DEFAULT_BUFLEN, 0);

        if (iResult > 0)
        {
            recvbuf[iResult] = '\0';

            if (strcmp(recvbuf, "time") == 0)
                CreateThread(NULL, 0, handle_time, NULL, 0, NULL);

            else if (strcmp(recvbuf, "date") == 0)
                CreateThread(NULL, 0, handle_date, NULL, 0, NULL);

            else if (strncmp(recvbuf, "weather ", 8) == 0)
            {
                char* city = new char[100];
                strcpy_s(city, 100, recvbuf + 8);
                CreateThread(NULL, 0, handle_weather, city, 0, NULL);
            }

            else if (strcmp(recvbuf, "euro") == 0)
                CreateThread(NULL, 0, handle_euro, NULL, 0, NULL);

            else if (strcmp(recvbuf, "bitcoin") == 0)
                CreateThread(NULL, 0, handle_bitcoin, NULL, 0, NULL);
            else
                send_response("Unknown command\n");
        }
        else
            break;
    }

    return 0;
}

int main()
{
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    InitializeCriticalSection(&cs);

    struct addrinfo hints, * result = NULL;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);

    SOCKET ListenSocket = socket(result->ai_family,
        result->ai_socktype,
        result->ai_protocol);

    bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    listen(ListenSocket, SOMAXCONN);

    printf("Server started...\n");

    ClientSocket = accept(ListenSocket, NULL, NULL);
    closesocket(ListenSocket);

    printf("Client connected.\n");

    CreateThread(NULL, 0, Receiver, NULL, 0, NULL);

    Sleep(INFINITE);

    DeleteCriticalSection(&cs);
    closesocket(ClientSocket);
    WSACleanup();

    return 0;
}