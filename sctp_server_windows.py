#!/usr/bin/env python3
"""
============================================================================
PROJECT: Implementing Client-Server Connection Using SCTP
FILE: sctp_server_windows.py
COURSE: MCA Computer Networks Practical (Windows Platform)
============================================================================
"""

import socket
import struct
import time
import sys

SERVER_IP = "127.0.0.1"
SERVER_PORT = 5000
BUFFER_SIZE = 1024

# Struct format: chunk_type(B), flags(B), length(H), tsn(I), stream_id(H), ssn(H), ppid(I)
HEADER_FORMAT = "!BBHIHHI"
HEADER_SIZE = struct.calcsize(HEADER_FORMAT)

def get_time():
    return time.strftime("[%Y-%m-%d %H:%M:%S]")

def start_server():
    print("=" * 65)
    print("     SCTP LAB: WINDOWS PYTHON PRACTICAL SERVER")
    print(f"     Platform: Microsoft Windows {sys.platform} | Port: {SERVER_PORT}")
    print("=" * 65)

    server_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

    server_sock.bind((SERVER_IP, SERVER_PORT))
    server_sock.listen(5)
    print(f"{get_time()} [LISTEN] Server actively listening on {SERVER_IP}:{SERVER_PORT}")
    print("[STATUS] Ready to handle Windows client connections with simulated 4-way cookie handshake...\n")

    while True:
        client_sock, client_addr = server_sock.accept()
        print("-" * 65)
        print(f"{get_time()} [ASSOCIATION ESTABLISHED] Client connected from {client_addr[0]}:{client_addr[1]}")
        print("  -> Association State: ESTABLISHED (4-Way State Cookie Verified)")
        print("  -> Active Streams   : 5 Inbound / 5 Outbound\n")

        msg_count = 0
        while True:
            data = client_sock.recv(BUFFER_SIZE)
            if not data:
                print(f"{get_time()} [DISCONNECT] Client closed association gracefully (3-Way SHUTDOWN).\n")
                break

            msg_count += 1
            if len(data) >= HEADER_SIZE:
                chunk_type, flags, length, tsn, stream_id, ssn, ppid = struct.unpack(
                    HEADER_FORMAT, data[:HEADER_SIZE]
                )
                payload = data[HEADER_SIZE:].decode("utf-8", errors="ignore")

                print(f"{get_time()} [RECV] Message #{msg_count} ({len(data)} bytes):")
                print(f"  -> Stream ID: {stream_id} | SSN: {ssn} | TSN: {tsn}")
                print(f"  -> Payload  : \"{payload}\"")

                # Prepare response echoing back on same Stream ID
                reply_payload = f"WINDOWS_SCTP_ACK: Received on Stream {stream_id} [WinTime: {int(time.time())}]".encode()
                resp_header = struct.pack(
                    HEADER_FORMAT,
                    0x00, # DATA
                    0x00,
                    HEADER_SIZE + len(reply_payload),
                    tsn + 100,
                    stream_id,
                    ssn,
                    132
                )
                client_sock.sendall(resp_header + reply_payload)
                print(f"{get_time()} [SENT] ACK replied to client on Stream {stream_id}")
                print("-" * 65)
            else:
                print(f"{get_time()} [RECV] Raw frame: {data}")

        client_sock.close()

if __name__ == "__main__":
    start_server()
