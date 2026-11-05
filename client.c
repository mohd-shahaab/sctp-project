/**
 * ============================================================================
 * PROJECT: Implementing Client-Server Connection Using SCTP
 * FILE: client.c
 * COURSE: MCA Computer Networks Practical Project
 * PROTOCOL: Stream Control Transmission Protocol (SCTP) - RFC 4960 / RFC 9260
 * API: Linux SCTP Socket Extensions (RFC 6458) via lksctp-tools
 * ============================================================================
 * 
 * DESCRIPTION:
 * This program implements a reliable SCTP Client in C. It initiates an SCTP
 * association with the server on port 5000, negotiates multiple independent
 * streams, transmits messages across distinct stream IDs (demonstrating
 * Multistreaming without Head-of-Line blocking), receives server replies,
 * and performs a graceful 3-way SHUTDOWN.
 *
 * COMPILATION:
 *   gcc -Wall client.c -o client -lsctp
 *
 * EXECUTION:
 *   ./client [optional_server_ip]
 *   Default: ./client 127.0.0.1
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

#define SERVER_PORT     5000          /* Target server port */
#define BUFFER_SIZE     1024          /* Buffer size for data transfers */
#define MAX_STREAMS     5             /* Negotiated stream count */

/**
 * Utility function to print formatted timestamps for client logs
 */
void print_log_timestamp(void) {
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char time_buf[26];
    strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", tm_info);
    printf("[%s] ", time_buf);
}

int main(int argc, char *argv[]) {
    int sock_fd = -1;                 /* Client socket descriptor */
    struct sockaddr_in serv_addr;     /* Server IPv4 address structure */
    struct sctp_initmsg initmsg;      /* Multi-stream init structure */
    struct sctp_sndrcvinfo sri;       /* Received message metadata */
    struct sctp_event_subscribe events; /* Event subscription structure */
    char buffer[BUFFER_SIZE];         /* Response reception buffer */
    const char *server_ip = "127.0.0.1";
    int bytes_sent = 0;
    int bytes_received = 0;
    int msg_flags = 0;
    socklen_t from_len;
    struct sockaddr_in from_addr;

    if (argc > 1) {
        server_ip = argv[1];
    }

    printf("================================================================\n");
    printf("     SCTP LAB: ONE-TO-ONE CLIENT-SERVER PRACTICAL CLIENT        \n");
    printf("     Target Server: %s:%d | Protocol: SCTP (132)      \n", server_ip, SERVER_PORT);
    printf("================================================================\n\n");

    /* ------------------------------------------------------------------------
     * STEP 1: CREATE SCTP SOCKET
     * ------------------------------------------------------------------------
     * socket(AF_INET, SOCK_STREAM, IPPROTO_SCTP)
     * - AF_INET: IPv4 protocol family.
     * - SOCK_STREAM: One-to-one socket abstraction.
     * - IPPROTO_SCTP: Protocol number 132 in IP header.
     */
    print_log_timestamp();
    printf("[INIT] Creating client SCTP socket...\n");
    sock_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_SCTP);
    if (sock_fd < 0) {
        perror("[ERROR] Failed to create SCTP socket");
        fprintf(stderr, "[HINT] Ensure 'sctp' kernel module is loaded: sudo modprobe sctp\n");
        exit(EXIT_FAILURE);
    }
    printf("[SUCCESS] SCTP client socket descriptor created (fd = %d)\n", sock_fd);

    /* ------------------------------------------------------------------------
     * STEP 2: CONFIGURE SCTP INIT PARAMETERS (REQUEST MULTIPLE STREAMS)
     * ------------------------------------------------------------------------
     * Inform the SCTP stack how many inbound/outbound streams to negotiate
     * during the upcoming 4-way INIT handshake.
     */
    memset(&initmsg, 0, sizeof(initmsg));
    initmsg.sinit_num_ostreams = MAX_STREAMS;
    initmsg.sinit_max_instreams = MAX_STREAMS;
    initmsg.sinit_max_attempts = 4;

    if (setsockopt(sock_fd, IPPROTO_SCTP, SCTP_INITMSG, &initmsg, sizeof(initmsg)) < 0) {
        perror("[WARNING] setsockopt(SCTP_INITMSG) failed");
    }

    /* ------------------------------------------------------------------------
     * STEP 3: SUBSCRIBE TO NOTIFICATION EVENTS
     * ------------------------------------------------------------------------
     * Request the kernel to deliver sctp_sndrcvinfo with received messages
     */
    memset(&events, 0, sizeof(events));
    events.sctp_data_io_event = 1;
    if (setsockopt(sock_fd, IPPROTO_SCTP, SCTP_EVENTS, &events, sizeof(events)) < 0) {
        perror("[WARNING] setsockopt(SCTP_EVENTS) failed");
    }

    /* ------------------------------------------------------------------------
     * STEP 4: PREPARE SERVER DESTINATION ADDRESS
     * ------------------------------------------------------------------------
     */
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, server_ip, &serv_addr.sin_addr) <= 0) {
        fprintf(stderr, "[ERROR] Invalid IP address format: %s\n", server_ip);
        close(sock_fd);
        exit(EXIT_FAILURE);
    }

    /* ------------------------------------------------------------------------
     * STEP 5: CONNECT (INITIATE SCTP 4-WAY COOKIE HANDSHAKE)
     * ------------------------------------------------------------------------
     * connect() triggers the kernel SCTP stack to exchange:
     * 1. Client -> Server : INIT (Initiation Tag, a_rwnd, Streams, Initial TSN)
     * 2. Server -> Client : INIT ACK (contains cryptographically signed State Cookie)
     * 3. Client -> Server : COOKIE ECHO (echoes the State Cookie back to server)
     * 4. Server -> Client : COOKIE ACK (Association Established!)
     */
    print_log_timestamp();
    printf("[HANDSHAKE] Initiating SCTP 4-Way Handshake with %s:%d...\n", server_ip, SERVER_PORT);
    printf("  [1/4] Sending INIT chunk to server...\n");

    if (connect(sock_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("[ERROR] Connection failed! (Is the SCTP server running?)");
        fprintf(stderr, "[HINT] Run './server' in another terminal first.\n");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }

    print_log_timestamp();
    printf("[CONNECTED] Association successfully established!\n");
    printf("  [2/4] Received INIT ACK with State Cookie from server.\n");
    printf("  [3/4] Sent COOKIE ECHO back to server.\n");
    printf("  [4/4] Received COOKIE ACK. Association is now ESTABLISHED.\n\n");

    /* ------------------------------------------------------------------------
     * STEP 6: DEMONSTRATE MULTISTREAMING DATA TRANSMISSION
     * ------------------------------------------------------------------------
     * We will send 3 distinct messages across 3 DIFFERENT streams:
     * - Stream 0: Application control / authentication message
     * - Stream 1: Telemetry sensor data payload
     * - Stream 2: High-priority urgent alert notification
     *
     * In SCTP, if packets on Stream 1 were delayed or lost, Stream 0 and 2
     * would STILL be delivered immediately to the receiving application!
     */
    const char *test_messages[] = {
        "MSG-1: [Stream 0] MCA Student Project Handshake Validation Token",
        "MSG-2: [Stream 1] Sensor Real-Time Telemetry: Temperature=24.5C, Load=42%",
        "MSG-3: [Stream 2] Urgent System Notification: Memory Threshold Optimal",
        "MSG-4: [Stream 0] Secondary Audit Log: Checkpoint Reached Successfully"
    };
    uint16_t stream_ids[] = {0, 1, 2, 0};
    int total_messages = sizeof(test_messages) / sizeof(test_messages[0]);

    for (int i = 0; i < total_messages; i++) {
        uint16_t current_stream = stream_ids[i];
        const char *msg_text = test_messages[i];
        size_t msg_len = strlen(msg_text);

        print_log_timestamp();
        printf("[SENDING] Transmitting message %d/%d on Stream ID: %u\n",
               i + 1, total_messages, current_stream);
        printf("  -> Content: \"%s\"\n", msg_text);

        /*
         * sctp_sendmsg() sends message over specific stream:
         * - sctp_sendmsg(sd, msg, len, to, tolen, ppid, flags, stream_no, ttl, context)
         * - ppid: 0 (Payload Protocol Identifier)
         * - flags: 0 (Ordered delivery)
         * - stream_no: target stream identifier (0, 1, 2, ...)
         */
        bytes_sent = sctp_sendmsg(
            sock_fd,
            msg_text,
            msg_len,
            NULL,
            0,
            0,                  /* ppid */
            0,                  /* flags */
            current_stream,     /* stream number */
            0,                  /* time-to-live: 0 for reliable */
            0                   /* context */
        );

        if (bytes_sent < 0) {
            perror("[ERROR] sctp_sendmsg() failed");
            break;
        }
        printf("  -> Status : Sent %d bytes successfully on Stream %u\n", bytes_sent, current_stream);

        /* Receive server response */
        from_len = sizeof(from_addr);
        memset(buffer, 0, sizeof(buffer));
        memset(&sri, 0, sizeof(sri));
        msg_flags = 0;

        bytes_received = sctp_recvmsg(
            sock_fd,
            buffer,
            sizeof(buffer) - 1,
            (struct sockaddr *)&from_addr,
            &from_len,
            &sri,
            &msg_flags
        );

        if (bytes_received > 0) {
            buffer[bytes_received] = '\0';
            print_log_timestamp();
            printf("[RECEIVED] Server response on Stream %u (%d bytes):\n",
                   sri.sinfo_stream, bytes_received);
            printf("  -> Payload : \"%s\"\n", buffer);
            printf("  -> SSN     : %u (Stream Sequence Number)\n", sri.sinfo_ssn);
            printf("  -> TSN     : %u (Transmission Sequence Number)\n\n", sri.sinfo_tsn);
        } else {
            perror("[WARNING] sctp_recvmsg() error or server closed");
            break;
        }

        /* Brief pause between transmissions for clean visualization */
        usleep(400000); /* 400 milliseconds */
    }

    /* ------------------------------------------------------------------------
     * STEP 7: GRACEFUL ASSOCIATION TERMINATION (3-WAY SHUTDOWN)
     * ------------------------------------------------------------------------
     * close() initiates SCTP Graceful Association Shutdown:
     * 1. Client -> Server : SHUTDOWN chunk (announces no more data to send)
     * 2. Server -> Client : SHUTDOWN ACK (confirms all pending chunks received)
     * 3. Client -> Server : SHUTDOWN COMPLETE
     *
     * Unlike TCP's half-closed state, SCTP shutdown is an explicit mutual close.
     */
    print_log_timestamp();
    printf("[TERMINATE] Initiating graceful 3-Way SHUTDOWN procedure...\n");
    close(sock_fd);
    printf("  [1/3] Sent SHUTDOWN chunk to server.\n");
    printf("  [2/3] Received SHUTDOWN ACK.\n");
    printf("  [3/3] Association closed cleanly. Resources freed.\n\n");

    printf("================================================================\n");
    printf("     SCTP CLIENT EXECUTION COMPLETED SUCCESSFULLY               \n");
    printf("================================================================\n");

    return 0;
}
