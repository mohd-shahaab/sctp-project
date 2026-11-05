/**
 * ============================================================================
 * PROJECT: Implementing Client-Server Connection Using SCTP
 * FILE: server.c
 * COURSE: MCA Computer Networks Practical Project
 * PROTOCOL: Stream Control Transmission Protocol (SCTP) - RFC 4960 / RFC 9260
 * API: Linux SCTP Socket Extensions (RFC 6458) via lksctp-tools
 * ============================================================================
 * 
 * DESCRIPTION:
 * This program implements a reliable, message-oriented SCTP Server using the
 * One-to-One (SOCK_STREAM) socket style. It establishes associations with
 * incoming clients via a 4-way cookie handshake, receives messages along with
 * SCTP-specific metadata (Stream ID, SSN, TSN, Payload Protocol Identifier),
 * and transmits responses back across specified outbound streams.
 *
 * COMPILATION:
 *   gcc -Wall server.c -o server -lsctp
 *
 * EXECUTION:
 *   ./server
 * ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>

/* Networking and Socket Headers */
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/* Linux Kernel SCTP API Header */
#include <netinet/sctp.h>

#define SERVER_PORT     5000          /* Default port for SCTP association */
#define BUFFER_SIZE     1024          /* Buffer size for data transfers */
#define MAX_STREAMS     5             /* Inbound & Outbound stream capacity */
#define BACKLOG_QUEUE   5             /* Maximum pending associations */

/**
 * Utility function to print formatted timestamps for server logs
 */
void print_log_timestamp(void) {
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char time_buf[26];
    strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", tm_info);
    printf("[%s] ", time_buf);
}

int main(int argc, char *argv[]) {
    int listen_fd = -1;              /* Server listening socket descriptor */
    int conn_fd = -1;                /* Accepted association socket descriptor */
    struct sockaddr_in serv_addr;    /* Server IPv4 address structure */
    struct sockaddr_in client_addr;  /* Client IPv4 address structure */
    socklen_t client_len;            /* Length of client address structure */
    struct sctp_initmsg initmsg;     /* Association initialization parameters */
    struct sctp_sndrcvinfo sri;      /* SCTP Send/Receive Information structure */
    struct sctp_event_subscribe events; /* SCTP notification event subscription */
    int msg_flags = 0;               /* Message reception flags */
    char buffer[BUFFER_SIZE];        /* Message read buffer */
    char response[BUFFER_SIZE];      /* Message response buffer */
    int bytes_received = 0;          /* Number of bytes received */
    int bytes_sent = 0;              /* Number of bytes sent */
    int message_counter = 0;         /* Counter for processed messages */
    int optval = 1;

    printf("================================================================\n");
    printf("     SCTP LAB: ONE-TO-ONE CLIENT-SERVER PRACTICAL SERVER        \n");
    printf("     Protocol: SCTP (IP Protocol 132) | Socket: SOCK_STREAM     \n");
    printf("================================================================\n\n");

    /* ------------------------------------------------------------------------
     * STEP 1: CREATE SCTP SOCKET
     * ------------------------------------------------------------------------
     * socket(domain, type, protocol)
     * - AF_INET: IPv4 Internet protocols.
     * - SOCK_STREAM: One-to-one socket style (similar interface to TCP).
     * - IPPROTO_SCTP: Stream Control Transmission Protocol (Protocol #132).
     */
    print_log_timestamp();
    printf("[INIT] Creating SCTP socket (AF_INET, SOCK_STREAM, IPPROTO_SCTP)...\n");
    
    listen_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_SCTP);
    if (listen_fd < 0) {
        perror("[ERROR] Socket creation failed! (Make sure SCTP module is loaded: sudo modprobe sctp)");
        fprintf(stderr, "[HINT] Error code %d: %s\n", errno, strerror(errno));
        exit(EXIT_FAILURE);
    }
    printf("[SUCCESS] SCTP listening socket descriptor created successfully (fd = %d)\n", listen_fd);

    /* Allow socket address reuse to prevent 'Address already in use' during quick restarts */
    if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) {
        perror("[WARNING] setsockopt(SO_REUSEADDR) failed");
    }

    /* ------------------------------------------------------------------------
     * STEP 2: CONFIGURE SCTP INIT PARAMETERS (MULTISTREAMING)
     * ------------------------------------------------------------------------
     * struct sctp_initmsg defines:
     * - sinit_num_ostreams: Number of outbound streams negotiated during INIT.
     * - sinit_max_instreams: Maximum number of inbound streams server accepts.
     * - sinit_max_attempts: Maximum INIT retransmission attempts.
     * - sinit_max_init_timeo: Maximum INIT retransmission timeout.
     */
    memset(&initmsg, 0, sizeof(initmsg));
    initmsg.sinit_num_ostreams = MAX_STREAMS;   /* Request 5 outbound streams */
    initmsg.sinit_max_instreams = MAX_STREAMS;  /* Accept up to 5 inbound streams */
    initmsg.sinit_max_attempts = 4;             /* Max INIT retransmissions */

    if (setsockopt(listen_fd, IPPROTO_SCTP, SCTP_INITMSG, &initmsg, sizeof(initmsg)) < 0) {
        perror("[ERROR] setsockopt(SCTP_INITMSG) configuration failed");
        close(listen_fd);
        exit(EXIT_FAILURE);
    }
    print_log_timestamp();
    printf("[CONFIG] Multistreaming parameters configured: %d Outbound / %d Inbound streams.\n",
           initmsg.sinit_num_ostreams, initmsg.sinit_max_instreams);

    /* ------------------------------------------------------------------------
     * STEP 3: SUBSCRIBE TO SCTP EVENTS & METADATA
     * ------------------------------------------------------------------------
     * Subscribing to sctp_data_io_event enables the kernel to populate the
     * struct sctp_sndrcvinfo when sctp_recvmsg() is invoked.
     */
    memset(&events, 0, sizeof(events));
    events.sctp_data_io_event = 1;              /* Receive stream ID and TSN info */
    events.sctp_association_event = 1;        /* Receive association state notifications */
    if (setsockopt(listen_fd, IPPROTO_SCTP, SCTP_EVENTS, &events, sizeof(events)) < 0) {
        perror("[WARNING] setsockopt(SCTP_EVENTS) failed");
    }

    /* ------------------------------------------------------------------------
     * STEP 4: BIND SOCKET TO LOCAL IP AND PORT
     * ------------------------------------------------------------------------
     * bind() assigns local IP (INADDR_ANY) and port 5000 to listen_fd.
     * Note: For multi-homing with multiple IPs, sctp_bindx() can be used.
     */
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); /* Listen on all available interfaces */
    serv_addr.sin_port = htons(SERVER_PORT);       /* Port 5000 */

    print_log_timestamp();
    printf("[BIND] Binding socket to 0.0.0.0:%d...\n", SERVER_PORT);
    if (bind(listen_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("[ERROR] Socket bind failed! (Port might already be in use)");
        close(listen_fd);
        exit(EXIT_FAILURE);
    }
    printf("[SUCCESS] Socket bound successfully to port %d\n", SERVER_PORT);

    /* ------------------------------------------------------------------------
     * STEP 5: LISTEN FOR INCOMING ASSOCIATIONS
     * ------------------------------------------------------------------------
     * listen() marks the socket as passive to accept incoming association
     * connection requests via the SCTP 4-Way Cookie Handshake.
     */
    if (listen(listen_fd, BACKLOG_QUEUE) < 0) {
        perror("[ERROR] Socket listen failed");
        close(listen_fd);
        exit(EXIT_FAILURE);
    }
    print_log_timestamp();
    printf("[LISTEN] SCTP Server is actively listening on port %d...\n", SERVER_PORT);
    printf("[STATUS] Ready to handle SCTP 4-Way Handshake (INIT -> INIT ACK -> COOKIE ECHO -> COOKIE ACK)\n\n");

    /* ------------------------------------------------------------------------
     * STEP 6: SERVER MAIN SERVICE LOOP (HANDLE MULTIPLE ASSOCIATIONS)
     * ------------------------------------------------------------------------
     */
    while (1) {
        printf("----------------------------------------------------------------\n");
        print_log_timestamp();
        printf("[WAIT] Waiting for incoming client association...\n");

        client_len = sizeof(client_addr);
        memset(&client_addr, 0, sizeof(client_addr));

        /* accept() blocks until an association's 4-way handshake completes */
        conn_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &client_len);
        if (conn_fd < 0) {
            perror("[ERROR] accept() failed");
            continue;
        }

        print_log_timestamp();
        printf("[ASSOCIATION ESTABLISHED]\n");
        printf("  -> Client Address : %s\n", inet_ntoa(client_addr.sin_addr));
        printf("  -> Client Port    : %d\n", ntohs(client_addr.sin_port));
        printf("  -> Connected FD   : %d\n", conn_fd);
        printf("  -> Handshake      : 4-Way Cookie Exchange Verified Successfully!\n\n");

        /* Handle multiple consecutive messages from this connected client */
        while (1) {
            memset(buffer, 0, BUFFER_SIZE);
            memset(&sri, 0, sizeof(sri));
            msg_flags = 0;

            /*
             * sctp_recvmsg() receives a complete message chunk from the client
             * along with SCTP-specific ancillary data stored in 'sri':
             * - sinfo_stream: The stream number on which data arrived
             * - sinfo_ssn: Stream Sequence Number
             * - sinfo_tsn: Transmission Sequence Number
             * - sinfo_assoc_id: Unique Association ID
             * - sinfo_ppid: Application-defined Payload Protocol Identifier
             */
            bytes_received = sctp_recvmsg(
                conn_fd,
                buffer,
                sizeof(buffer) - 1,
                (struct sockaddr *)&client_addr,
                &client_len,
                &sri,
                &msg_flags
            );

            if (bytes_received < 0) {
                perror("[ERROR] sctp_recvmsg() error");
                break;
            } else if (bytes_received == 0) {
                /* Client gracefully closed the association via SHUTDOWN */
                print_log_timestamp();
                printf("[DISCONNECT] Client closed association gracefully (SHUTDOWN received).\n\n");
                break;
            }

            buffer[bytes_received] = '\0';
            message_counter++;

            print_log_timestamp();
            printf("[RECV] Message #%d Received (%d bytes):\n", message_counter, bytes_received);
            printf("  -> Content  : \"%s\"\n", buffer);
            printf("  -> Stream ID: %u (Out of 0-%d)\n", sri.sinfo_stream, MAX_STREAMS - 1);
            printf("  -> SSN      : %u (Stream Sequence Number)\n", sri.sinfo_ssn);
            printf("  -> TSN      : %u (Transmission Sequence Number)\n", sri.sinfo_tsn);
            printf("  -> Assoc ID : %d\n", sri.sinfo_assoc_id);
            printf("  -> Flags    : 0x%x (MSG_EOR=%d)\n", msg_flags, (msg_flags & MSG_EOR) ? 1 : 0);

            /* Prepare a structured acknowledgement response */
            snprintf(response, sizeof(response),
                     "SERVER_ACK: Message received on Stream %u [Server Time: %ld]",
                     sri.sinfo_stream, (long)time(NULL));

            /*
             * sctp_sendmsg() sends response back to the client.
             * Notice: We echo the response back on the EXACT SAME Stream ID!
             * - stream_no = sri.sinfo_stream (demonstrates multistream routing)
             * - ppid = 0 (Payload Protocol Identifier)
             * - flags = 0 (Ordered delivery)
             */
            bytes_sent = sctp_sendmsg(
                conn_fd,
                response,
                (size_t)strlen(response),
                NULL,
                0,
                0,                      /* ppid */
                0,                      /* flags: 0 for ordered */
                sri.sinfo_stream,       /* stream number: echo on same stream */
                0,                      /* timetolive: 0 = infinite / reliable */
                0                       /* context */
            );

            if (bytes_sent < 0) {
                perror("[ERROR] sctp_sendmsg() response failed");
                break;
            }

            print_log_timestamp();
            printf("[SENT] Response dispatched to client on Stream %u (%d bytes)\n",
                   sri.sinfo_stream, bytes_sent);
            printf("----------------------------------------------------------------\n");
        }

        /* Close association socket descriptor and prepare for next client */
        close(conn_fd);
        print_log_timestamp();
        printf("[CLEANUP] Association socket %d closed. Returning to listen state.\n", conn_fd);
    }

    /* Close listening socket (unreachable in infinite loop, kept for completeness) */
    close(listen_fd);
    return 0;
}
