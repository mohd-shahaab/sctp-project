/**
 * ============================================================================
 * PROJECT: Implementing Client-Server Connection Using SCTP
 * FILE: server_onetoMany.c
 * COURSE: MCA Computer Networks Practical Project
 * PROTOCOL: Stream Control Transmission Protocol (SCTP) - RFC 4960 / RFC 6458
 * ============================================================================
 * 
 * DESCRIPTION:
 * Demonstrates SCTP One-to-Many socket style (SOCK_SEQPACKET).
 * In this style, a single socket descriptor handles multiple associations
 * simultaneously without needing accept(). The application can communicate
 * with multiple endpoints concurrently using sctp_sendmsg() and sctp_recvmsg().
 *
 * COMPILATION:
 *   gcc -Wall server_onetoMany.c -o server_onetoMany -lsctp
 * ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/sctp.h>

#define SERVER_PORT 5001
#define BUFFER_SIZE 1024

int main(void) {
    int sock_fd;
    struct sockaddr_in serv_addr, client_addr;
    socklen_t client_len;
    struct sctp_sndrcvinfo sri;
    struct sctp_event_subscribe events;
    char buffer[BUFFER_SIZE];
    char reply[BUFFER_SIZE];
    int msg_flags = 0;
    int bytes;

    printf("[SCTP LAB] Starting One-to-Many (SOCK_SEQPACKET) SCTP Server on port %d...\n", SERVER_PORT);

    /* Create SOCK_SEQPACKET SCTP socket */
    sock_fd = socket(AF_INET, SOCK_SEQPACKET, IPPROTO_SCTP);
    if (sock_fd < 0) {
        perror("socket() failed");
        exit(EXIT_FAILURE);
    }

    /* Subscribe to SCTP data IO events */
    memset(&events, 0, sizeof(events));
    events.sctp_data_io_event = 1;
    setsockopt(sock_fd, IPPROTO_SCTP, SCTP_EVENTS, &events, sizeof(events));

    /* Bind socket */
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(SERVER_PORT);

    if (bind(sock_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("bind() failed");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }

    /* In one-to-many style, listen sets the maximum association queue backlog */
    if (listen(sock_fd, 10) < 0) {
        perror("listen() failed");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }

    printf("[SCTP LAB] One-to-Many server ready. Listening for associations on multiple endpoints...\n");

    while (1) {
        client_len = sizeof(client_addr);
        memset(&sri, 0, sizeof(sri));
        memset(buffer, 0, sizeof(buffer));

        /* In one-to-many style, sctp_recvmsg directly receives data from any client */
        bytes = sctp_recvmsg(sock_fd, buffer, sizeof(buffer) - 1,
                            (struct sockaddr *)&client_addr, &client_len,
                            &sri, &msg_flags);
        if (bytes <= 0) continue;

        buffer[bytes] = '\0';
        printf("[ONE-TO-MANY RECV] From %s:%d | Assoc ID: %d | Stream: %u | TSN: %u\n  Payload: \"%s\"\n",
               inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port),
               sri.sinfo_assoc_id, sri.sinfo_stream, sri.sinfo_tsn, buffer);

        /* Reply using the client_addr destination or assoc_id */
        snprintf(reply, sizeof(reply), "ACK: Received on Assoc %d, Stream %u",
                 sri.sinfo_assoc_id, sri.sinfo_stream);

        sctp_sendmsg(sock_fd, reply, strlen(reply),
                    (struct sockaddr *)&client_addr, client_len,
                    0, 0, sri.sinfo_stream, 0, 0);
    }

    close(sock_fd);
    return 0;
}
