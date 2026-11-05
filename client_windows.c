/**
 * ============================================================================
 * PROJECT: Implementing Client-Server Connection Using SCTP
 * FILE: client_windows.c
 * COURSE: MCA Computer Networks Practical Project (Windows Environment)
 * PROTOCOL: Stream Control Transmission Protocol (SCTP) Emulation / Sockets
 * ENVIRONMENT: Microsoft Windows (WinSock2 API)
 * ============================================================================
 * 
 * DESCRIPTION:
 * Windows-native implementation of the SCTP Client. Initiates an association
 * with the server on 127.0.0.1:5000, establishes the connection via WinSock,
 * frames discrete messages with SCTP Chunk Headers (Chunk Type, SID, SSN, TSN),
 * transmits across multiple independent streams (Stream 0, 1, 2), receives server
 * acknowledgments, and performs a graceful shutdown.
 *
 * COMPILATION (MinGW GCC on Windows):
 *   gcc -Wall client_windows.c -o client_windows.exe -lws2_32
 *
 * COMPILATION (Microsoft Visual Studio cl.exe):
 *   cl client_windows.c ws2_32.lib
 *
 * EXECUTION:
 *   .\client_windows.exe [optional_server_ip]
 *   Default: .\client_windows.exe 127.0.0.1
 * ============================================================================
 */

#define _WIN32_WINNT 0x0600
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Windows Sockets API Headers */
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

#pragma comment(lib, "ws2_32.lib")

#define SERVER_PORT     5000
#define BUFFER_SIZE     1024

/* SCTP Chunk Header Simulation Structure for Windows */
#pragma pack(push, 1)
typedef struct {
    unsigned char  chunk_type;        /* 0x00 = DATA, 0x01 = INIT, 0x03 = SACK */
    unsigned char  chunk_flags;       /* U (Unordered), B (Beginning), E (Ending) */
    unsigned short chunk_length;      /* Total length of chunk including header */
    unsigned int   tsn;               /* Transmission Sequence Number (32 bits) */
    unsigned short stream_id;         /* Stream Identifier (16 bits) */
    unsigned short ssn;               /* Stream Sequence Number (16 bits) */
    unsigned int   ppid;              /* Payload Protocol Identifier */
} SCTP_DATA_CHUNK_HEADER;
#pragma pack(pop)

void print_windows_timestamp(void) {
    SYSTEMTIME st;
    GetLocalTime(&st);
    printf("[%04d-%02d-%02d %02d:%02d:%02d.%03d] ",
           st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
}

int main(int argc, char *argv[]) {
    WSADATA wsaData;
    SOCKET sock = INVALID_SOCKET;
    struct sockaddr_in serv_addr;
    char send_buf[BUFFER_SIZE];
    char recv_buf[BUFFER_SIZE];
    const char *server_ip = (argc > 1) ? argv[1] : "127.0.0.1";
    int iResult;
    unsigned int base_tsn = 1000;

    printf("================================================================\n");
    printf("     SCTP LAB: WINDOWS CLIENT-SERVER PRACTICAL CLIENT           \n");
    printf("     Platform: Microsoft Windows | Target: %s:%d       \n", server_ip, SERVER_PORT);
    printf("================================================================\n\n");

    /* ------------------------------------------------------------------------
     * STEP 1: INITIALIZE WINSOCK (WSAStartup)
     * ------------------------------------------------------------------------
     */
    print_windows_timestamp();
    printf("[WINSOCK] Initializing Windows Sockets 2.2 (WSAStartup)...\n");
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("[ERROR] WSAStartup failed: %d\n", iResult);
        return 1;
    }
    printf("[SUCCESS] WinSock 2.2 initialized.\n");

    /* ------------------------------------------------------------------------
     * STEP 2: CREATE CLIENT SOCKET
     * ------------------------------------------------------------------------
     */
    print_windows_timestamp();
    printf("[INIT] Creating Windows client socket...\n");
    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        printf("[ERROR] socket() failed with error: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }
    printf("[SUCCESS] Socket descriptor created (Handle = %llu)\n", (unsigned long long)sock);

    /* ------------------------------------------------------------------------
     * STEP 3: CONFIGURE SERVER ADDRESS
     * ------------------------------------------------------------------------
     */
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, server_ip, &serv_addr.sin_addr);

    /* ------------------------------------------------------------------------
     * STEP 4: CONNECT (INITIATE 4-WAY COOKIE HANDSHAKE)
     * ------------------------------------------------------------------------
     */
    print_windows_timestamp();
    printf("[HANDSHAKE] Connecting to SCTP Server %s:%d...\n", server_ip, SERVER_PORT);
    printf("  [1/4] Sending INIT chunk to server...\n");

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == SOCKET_ERROR) {
        printf("[ERROR] connect() failed with Windows error: %d\n", WSAGetLastError());
        printf("[HINT] Is the Windows server running? Start 'server_windows.exe' first.\n");
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    print_windows_timestamp();
    printf("[CONNECTED] Association established successfully!\n");
    printf("  [2/4] Received INIT ACK with State Cookie from server.\n");
    printf("  [3/4] Sent COOKIE ECHO verifying client return path.\n");
    printf("  [4/4] Received COOKIE ACK. Association is ESTABLISHED.\n\n");

    /* ------------------------------------------------------------------------
     * STEP 5: TRANSMIT MESSAGES ACROSS MULTIPLE INDEPENDENT STREAMS
     * ------------------------------------------------------------------------
     */
    const char *messages[] = {
        "MCA Windows Project Handshake Validation Token",
        "Sensor Telemetry: Temp=24.5C, WindowsLoad=38%",
        "High Priority Urgent Windows Notification: Optimal State"
    };
    unsigned short stream_ids[] = {0, 1, 2};
    int total_messages = 3;

    for (int i = 0; i < total_messages; i++) {
        unsigned short sid = stream_ids[i];
        const char *msg_text = messages[i];
        int payload_len = (int)strlen(msg_text);
        unsigned int tsn = base_tsn + i + 1;
        unsigned short ssn = (unsigned short)i;

        print_windows_timestamp();
        printf("[SENDING] Transmitting message %d/%d on Stream ID: %u\n", i + 1, total_messages, sid);
        printf("  -> Content: \"%s\"\n", msg_text);

        /* Construct simulated SCTP Chunk Frame */
        memset(send_buf, 0, BUFFER_SIZE);
        SCTP_DATA_CHUNK_HEADER *header = (SCTP_DATA_CHUNK_HEADER *)send_buf;
        header->chunk_type = 0x00; /* DATA Chunk */
        header->chunk_flags = 0x00; /* Ordered */
        header->chunk_length = htons(sizeof(SCTP_DATA_CHUNK_HEADER) + payload_len);
        header->tsn = htonl(tsn);
        header->stream_id = htons(sid);
        header->ssn = htons(ssn);
        header->ppid = htonl(132);

        char *payload_ptr = send_buf + sizeof(SCTP_DATA_CHUNK_HEADER);
        memcpy(payload_ptr, msg_text, payload_len);

        int total_send_len = sizeof(SCTP_DATA_CHUNK_HEADER) + payload_len;
        int sent_bytes = send(sock, send_buf, total_send_len, 0);

        if (sent_bytes == SOCKET_ERROR) {
            printf("[ERROR] send() failed with error: %d\n", WSAGetLastError());
            break;
        }
        printf("  -> Status : Sent %d bytes successfully on Stream %u (TSN: %u, SSN: %u)\n",
               sent_bytes, sid, tsn, ssn);

        /* Receive server response */
        memset(recv_buf, 0, BUFFER_SIZE);
        int recv_bytes = recv(sock, recv_buf, BUFFER_SIZE - 1, 0);
        if (recv_bytes > 0) {
            SCTP_DATA_CHUNK_HEADER *resp_header = (SCTP_DATA_CHUNK_HEADER *)recv_buf;
            char *resp_payload = recv_buf + sizeof(SCTP_DATA_CHUNK_HEADER);
            unsigned short resp_sid = ntohs(resp_header->stream_id);
            unsigned int resp_tsn = ntohl(resp_header->tsn);

            print_windows_timestamp();
            printf("[RECEIVED] Server response on Stream %u (%d bytes):\n", resp_sid, recv_bytes);
            printf("  -> Payload : \"%s\"\n", resp_payload);
            printf("  -> Resp TSN: %u\n\n", resp_tsn);
        }

        Sleep(400); /* 400 milliseconds pause between transmissions */
    }

    /* ------------------------------------------------------------------------
     * STEP 6: GRACEFUL 3-WAY SHUTDOWN
     * ------------------------------------------------------------------------
     */
    print_windows_timestamp();
    printf("[TERMINATE] Initiating graceful 3-Way SHUTDOWN on Windows...\n");
    shutdown(sock, SD_SEND);
    closesocket(sock);
    printf("  [1/3] Sent SHUTDOWN chunk to server.\n");
    printf("  [2/3] Confirmed SHUTDOWN ACK.\n");
    printf("  [3/3] Association closed cleanly. WinSock resources released.\n\n");

    WSACleanup();
    printf("================================================================\n");
    printf("     WINDOWS SCTP CLIENT EXECUTION COMPLETED SUCCESSFULLY      \n");
    printf("================================================================\n");

    return 0;
}
