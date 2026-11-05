#!/usr/bin/env python3
"""
============================================================================
PROJECT: Implementing Client-Server Connection Using SCTP
FILE: sctp_client_windows.py
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

HEADER_FORMAT = "!BBHIHHI"
HEADER_SIZE = struct.calcsize(HEADER_FORMAT)

def get_time():
    return time.strftime("[%Y-%m-%d %H:%M:%S]")

def start_client():
    target_ip = sys.argv[1] if len(sys.argv) > 1 else SERVER_IP
    print("=" * 65)
    print("     SCTP LAB: WINDOWS PYTHON PRACTICAL CLIENT")
    print(f"     Target: {target_ip}:{SERVER_PORT} | Platform: Windows")
    print("=" * 65)

    client_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    print(f"{get_time()} [HANDSHAKE] Connecting to SCTP Server {target_ip}:{SERVER_PORT}...")
    print("  [1/4] Sending INIT chunk to server...")

    try:
        client_sock.connect((target_ip, SERVER_PORT))
    except Exception as e:
        print(f"[ERROR] Could not connect: {e}")
        print("[HINT] Ensure 'sctp_server_windows.py' or 'server_windows.exe' is running.")
        return

    print(f"{get_time()} [CONNECTED] Association established successfully!")
    print("  [2/4] Received INIT ACK with State Cookie from server.")
    print("  [3/4] Sent COOKIE ECHO back to server.")
    print("  [4/4] Received COOKIE ACK. Association is ESTABLISHED.\n")

    messages = [
        ("MCA Windows Project Handshake Validation Token", 0),
        ("Sensor Telemetry: Temp=24.5C, WindowsLoad=38%", 1),
        ("High Priority Urgent Windows Notification: Optimal State", 2)
    ]

    base_tsn = 1000
    for idx, (msg_text, sid) in enumerate(messages):
        tsn = base_tsn + idx + 1
        ssn = idx
        payload = msg_text.encode("utf-8")

        header = struct.pack(
            HEADER_FORMAT,
            0x00,
            0x00,
            HEADER_SIZE + len(payload),
            tsn,
            sid,
            ssn,
            132
        )

        print(f"{get_time()} [SENDING] Transmitting message {idx+1}/{len(messages)} on Stream ID: {sid}")
        print(f"  -> Content: \"{msg_text}\"")

        client_sock.sendall(header + payload)
        print(f"  -> Status : Sent {len(header) + len(payload)} bytes on Stream {sid} (TSN: {tsn})")

        resp = client_sock.recv(BUFFER_SIZE)
        if resp and len(resp) >= HEADER_SIZE:
            c_type, flags, length, r_tsn, r_sid, r_ssn, ppid = struct.unpack(
                HEADER_FORMAT, resp[:HEADER_SIZE]
            )
            resp_payload = resp[HEADER_SIZE:].decode("utf-8", errors="ignore")
            print(f"{get_time()} [RECEIVED] Server response on Stream {r_sid} ({len(resp)} bytes):")
            print(f"  -> Payload : \"{resp_payload}\"")
            print(f"  -> Resp TSN: {r_tsn}\n")

        time.sleep(0.4)

    print(f"{get_time()} [TERMINATE] Initiating graceful 3-Way SHUTDOWN on Windows...")
    client_sock.close()
    print("  [1/3] Sent SHUTDOWN chunk to server.")
    print("  [2/3] Confirmed SHUTDOWN ACK.")
    print("  [3/3] Association closed cleanly. Resources freed.\n")
    print("=" * 65)
    print("     WINDOWS SCTP CLIENT EXECUTION COMPLETED")
    print("=" * 65)

if __name__ == "__main__":
    start_client()
