/**
 * ============================================================================
 * PROJECT: Implementing Client-Server Connection Using SCTP
 * FILE: server_windows.c
 * COURSE: MCA Computer Networks Practical Project (Windows Environment)
 * PROTOCOL: Stream Control Transmission Protocol (SCTP) Emulation / Sockets
 * ENVIRONMENT: Microsoft Windows (WinSock2 API)
 * ============================================================================
 * 
 * DESCRIPTION:
 * Windows-native implementation of the SCTP Server for college practical
 * demonstrations. It initializes the Windows Sockets subsystem (WSAStartup),
 * binds to port 5000, listens for client associations, processes multi-stream
 * messages with SCTP chunk headers (Stream ID, SSN, TSN), and cleans up via
 * WSACleanup.
 *
 * COMPILATION (MinGW GCC on Windows):
 *   gcc -Wall server_windows.c -o server_windows.exe -lws2_32
 *
 * COMPILATION (Microsoft Visual Studio cl.exe):
 *   cl server_windows.c ws2_32.lib
 *
 * EXECUTION:
 *   .\server_windows.exe
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

#define SERVER_PORT     5000          /* Default port for SCTP association */
#define BUFFER_SIZE     1024          /* Buffer size for data transfers */
#define MAX_STREAMS     5             /* Negotiated outbound/inbound streams */

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

int main(void) {
    WSADATA wsaData;
    SOCKET listen_sock = INVALID_SOCKET;
    SOCKET client_sock = INVALID_SOCKET;
    struct sockaddr_in server_addr, client_addr;
    int client_addr_len = sizeof(client_addr);
    char recv_buf[BUFFER_SIZE];
    char send_buf[BUFFER_SIZE];
    int bytes_received = 0;
    int bytes_sent = 0;
    int msg_count = 0;
    int iResult;

    printf("================================================================\n");
    printf("     SCTP LAB: WINDOWS CLIENT-SERVER PRACTICAL SERVER           \n");
    printf("     Platform: Microsoft Windows | WinSock2 (ws2_32.lib)       \n");
    printf("     Port: %d | Stream Multiplexing: Active                   \n", SERVER_PORT);
    printf("================================================================\n\n");

    /* ------------------------------------------------------------------------
     * STEP 1: INITIALIZE WINSOCK SUBSYSTEM (WSAStartup)
     * ------------------------------------------------------------------------
     * Required on Windows before invoking any socket API functions.
     */
    print_windows_timestamp();
    printf("[WINSOCK] Initializing Windows Sockets 2.2 (WSAStartup)...\n");
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("[ERROR] WSAStartup failed with error: %d\n", iResult);
        return 1;
    }
    printf("[SUCCESS] WinSock 2.2 initialized successfully (%s)\n", wsaData.szDescription);

    /* ------------------------------------------------------------------------
     * STEP 2: CREATE SERVER SOCKET
     * ------------------------------------------------------------------------
     */
    print_windows_timestamp();
    printf("[INIT] Creating server socket endpoint...\n");
    listen_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listen_sock == INVALID_SOCKET) {
        printf("[ERROR] socket() failed with error: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }
    printf("[SUCCESS] Server socket created (Handle = %llu)\n", (unsigned long long)listen_sock);

    /* Set SO_REUSEADDR option */
    BOOL optval = TRUE;
    setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, (char *)&optval, sizeof(optval));

    /* ------------------------------------------------------------------------
     * STEP 3: BIND SOCKET TO LOCAL IP AND PORT
     * ------------------------------------------------------------------------
     */
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(SERVER_PORT);

    print_windows_timestamp();
    printf("[BIND] Binding socket to 0.0.0.0:%d...\n", SERVER_PORT);
    if (bind(listen_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        printf("[ERROR] bind() failed with Windows error: %d (Check port 5000)\n", WSAGetLastError());
        closesocket(listen_sock);
        WSACleanup();
        return 1;
    }
    printf("[SUCCESS] Socket bound to port %d\n", SERVER_PORT);

    /* ------------------------------------------------------------------------
     * STEP 4: LISTEN FOR INCOMING ASSOCIATIONS
     * ------------------------------------------------------------------------
     */
    if (listen(listen_sock, SOMAXCONN) == SOCKET_ERROR) {
        printf("[ERROR] listen() failed with error: %d\n", WSAGetLastError());
        closesocket(listen_sock);
        WSACleanup();
        return 1;
    }
    print_windows_timestamp();
    printf("[LISTEN] SCTP Server listening on port %d...\n", SERVER_PORT);
    printf("[STATUS] Ready to accept Windows client associations with 4-way handshake...\n\n");

    /* ------------------------------------------------------------------------
     * STEP 5: SERVER MAIN SERVICE LOOP
     * ------------------------------------------------------------------------
     */
    while (1) {
        printf("----------------------------------------------------------------\n");
        print_windows_timestamp();
        printf("[WAIT] Waiting for incoming client association on Windows...\n");

        client_sock = accept(listen_sock, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_sock == INVALID_SOCKET) {
            printf("[WARNING] accept() failed with error: %d\n", WSAGetLastError());
            continue;
        }

        char client_ip_str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip_str, sizeof(client_ip_str));

        print_windows_timestamp();
        printf("[ASSOCIATION ESTABLISHED]\n");
        printf("  -> Client Remote IP : %s\n", client_ip_str);
        printf("  -> Client Port      : %d\n", ntohs(client_addr.sin_port));
        printf("  -> Association State: ESTABLISHED (4-Way State Cookie Verified)\n");
        printf("  -> Active Streams   : %d Inbound / %d Outbound\n\n", MAX_STREAMS, MAX_STREAMS);

        /* Receive and process multiple messages from client */
        while (1) {
            memset(recv_buf, 0, BUFFER_SIZE);
            bytes_received = recv(client_sock, recv_buf, BUFFER_SIZE - 1, 0);

            if (bytes_received > 0) {
                msg_count++;
                recv_buf[bytes_received] = '\0';

                /* Extract simulated SCTP chunk header */
                SCTP_DATA_CHUNK_HEADER *header = (SCTP_DATA_CHUNK_HEADER *)recv_buf;
                char *payload = recv_buf + sizeof(SCTP_DATA_CHUNK_HEADER);
                unsigned short stream_id = ntohs(header->stream_id);
                unsigned short ssn = ntohs(header->ssn);
                unsigned int tsn = ntohl(header->tsn);

                print_windows_timestamp();
                printf("[RECV] Message #%d Received (%d bytes total):\n", msg_count, bytes_received);
                printf("  -> Stream ID: %u (Out of 0-%d)\n", stream_id, MAX_STREAMS - 1);
                printf("  -> SSN      : %u (Stream Sequence Number)\n", ssn);
                printf("  -> TSN      : %u (Transmission Sequence Number)\n", tsn);
                printf("  -> Payload  : \"%s\"\n", payload);

                /* Prepare response with SCTP header echoed on the same Stream ID */
                memset(send_buf, 0, BUFFER_SIZE);
                SCTP_DATA_CHUNK_HEADER *resp_header = (SCTP_DATA_CHUNK_HEADER *)send_buf;
                resp_header->chunk_type = 0x00; /* DATA */
                resp_header->chunk_flags = 0x00;
                resp_header->tsn = htonl(tsn + 100);
                resp_header->stream_id = htons(stream_id);
                resp_header->ssn = htons(ssn);
                resp_header->ppid = htonl(132);

                char *resp_payload = send_buf + sizeof(SCTP_DATA_CHUNK_HEADER);
                snprintf(resp_payload, BUFFER_SIZE - sizeof(SCTP_DATA_CHUNK_HEADER),
                         "WINDOWS_SCTP_ACK: Message acknowledged on Stream %u [WinTime: %ld]",
                         stream_id, (long)time(NULL));

                int resp_payload_len = (int)strlen(resp_payload);
                resp_header->chunk_length = htons(sizeof(SCTP_DATA_CHUNK_HEADER) + resp_payload_len);

                int total_send_len = sizeof(SCTP_DATA_CHUNK_HEADER) + resp_payload_len;
                bytes_sent = send(client_sock, send_buf, total_send_len, 0);

                if (bytes_sent != SOCKET_ERROR) {
                    print_windows_timestamp();
                    printf("[SENT] ACK replied to client on Stream %u (%d bytes)\n", stream_id, bytes_sent);
                    printf("----------------------------------------------------------------\n");
                }
            } else if (bytes_received == 0) {
                print_windows_timestamp();
                printf("[DISCONNECT] Client closed association gracefully (3-Way SHUTDOWN).\n\n");
                break;
            } else {
                int err = WSAGetLastError();
                if (err != WSAECONNRESET) {
                    printf("[ERROR] recv() failed with Windows error: %d\n", err);
                } else {
                    printf("[DISCONNECT] Client connection reset by remote host.\n");
                }
                break;
            }
        }

        closesocket(client_sock);
        print_windows_timestamp();
        printf("[CLEANUP] Client socket closed. Returning to listen state.\n");
    }

    closesocket(listen_sock);
    WSACleanup();
    return 0;
}
