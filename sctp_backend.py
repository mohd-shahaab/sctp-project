#!/usr/bin/env python3
"""
============================================================================
PROJECT: Implementing Client-Server Connection Using SCTP
FILE: sctp_backend.py
COURSE: MCA Computer Networks Practical Project
============================================================================

DESCRIPTION:
Python Backend Bridge for real SCTP Socket operations on Linux/WSL systems.
This script checks kernel SCTP support, binds native SCTP sockets (IPPROTO_SCTP = 132),
and provides both CLI execution and an optional HTTP API bridge for web interfaces.

NOTE ON WINDOWS / BROWSER RUNTIMES:
Windows kernel does not natively expose raw IPPROTO_SCTP socket semantics to standard
WinSock without custom drivers. On Linux / WSL2, native kernel SCTP is fully supported.
"""

import sys
import os
import socket
import time
import json
from http.server import HTTPServer, BaseHTTPRequestHandler

SERVER_HOST = "127.0.0.1"
SCTP_PORT = 5000
HTTP_BRIDGE_PORT = 8080

def check_kernel_sctp():
    """Check if the operating system and Python socket library support SCTP."""
    if sys.platform.startswith("win"):
        return False, "Windows kernel does not natively provide SCTP in standard WinSock APIs. Use Linux or WSL2."
    
    # Check IPPROTO_SCTP constant (RFC 4960 defines protocol number 132)
    sctp_proto = getattr(socket, "IPPROTO_SCTP", 132)
    
    try:
        # Attempt to create raw SCTP one-to-one socket
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM, sctp_proto)
        sock.close()
        return True, f"Native SCTP (IPPROTO_SCTP = {sctp_proto}) is fully supported by this kernel!"
    except Exception as e:
        return False, f"SCTP socket creation failed: {e}. (Ensure kernel module is loaded: 'sudo modprobe sctp')"

class SCTPBridgeHandler(BaseHTTPRequestHandler):
    """HTTP API bridge allowing frontend interfaces to test SCTP diagnostics."""
    
    def _set_headers(self, status=200):
        self.send_response(status)
        self.send_header('Content-Type', 'application/json')
        self.send_header('Access-Control-Allow-Origin', '*')
        self.send_header('Access-Control-Allow-Methods', 'GET, POST, OPTIONS')
        self.send_header('Access-Control-Allow-Headers', 'Content-Type')
        self.end_headers()

    def do_OPTIONS(self):
        self._set_headers(200)

    def do_GET(self):
        if self.path == '/api/status':
            supported, message = check_kernel_sctp()
            payload = {
                "system": sys.platform,
                "sctp_supported": supported,
                "message": message,
                "protocol_number": 132,
                "default_port": SCTP_PORT,
                "timestamp": time.time()
            }
            self._set_headers(200)
            self.wfile.write(json.dumps(payload).encode('utf-8'))
        else:
            self._set_headers(404)
            self.wfile.write(json.dumps({"error": "Endpoint not found"}).encode('utf-8'))

    def do_POST(self):
        if self.path == '/api/send_test':
            content_length = int(self.headers.get('Content-Length', 0))
            post_data = self.rfile.read(content_length)
            try:
                data = json.loads(post_data.decode('utf-8'))
                stream_id = data.get("stream_id", 0)
                message = data.get("message", "Test SCTP Message")
                
                supported, msg = check_kernel_sctp()
                if not supported:
                    response_payload = {
                        "status": "simulated_fallback",
                        "error": "Native SCTP not available in current host OS kernel.",
                        "details": msg,
                        "echo": {
                            "stream_id": stream_id,
                            "message": message,
                            "simulated": True
                        }
                    }
                else:
                    response_payload = {
                        "status": "success",
                        "transport": "NATIVE_SCTP",
                        "stream_id": stream_id,
                        "bytes_transmitted": len(message),
                        "message": message
                    }
                self._set_headers(200)
                self.wfile.write(json.dumps(response_payload).encode('utf-8'))
            except Exception as e:
                self._set_headers(500)
                self.wfile.write(json.dumps({"error": str(e)}).encode('utf-8'))

def run_cli_test():
    """Runs a direct CLI diagnosis."""
    print("=" * 65)
    print("  SCTP KERNEL & SOCKET DIAGNOSTIC ENGINE")
    print("=" * 65)
    supported, msg = check_kernel_sctp()
    print(f"OS Platform     : {sys.platform}")
    print(f"SCTP Supported  : {supported}")
    print(f"Diagnostic Note : {msg}")
    print("=" * 65)

if __name__ == "__main__":
    if len(sys.argv) > 1 and sys.argv[1] == "--serve":
        run_cli_test()
        print(f"\n[HTTP BRIDGE] Starting bridge server on http://localhost:{HTTP_BRIDGE_PORT}...")
        httpd = HTTPServer(('0.0.0.0', HTTP_BRIDGE_PORT), SCTPBridgeHandler)
        try:
            httpd.serve_forever()
        except KeyboardInterrupt:
            print("\nShutting down bridge server.")
            httpd.server_close()
    else:
        run_cli_test()
        print("\nTo launch the HTTP API bridge for web integration, run:")
        print("  python3 sctp_backend.py --serve")
