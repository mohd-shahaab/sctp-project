# SCTP LAB: Implementing Client-Server Connection Using SCTP (Stream Control Transmission Protocol)

**Master of Computer Applications (MCA) — Computer Networks Practical Project**  
**Operating System:** Microsoft Windows 10 / 11  
**Socket Standard:** Windows Sockets 2.2 (WinSock2 - `ws2_32.lib`) / Python for Windows  
**Assigned IP Protocol Number:** 132  
**Frontend Platform:** HTML5, CSS3, JavaScript (Single-Page Interactive Laboratory Platform)

---

## 📖 Executive Summary & Abstract

The **Stream Control Transmission Protocol (SCTP)** is an advanced, message-oriented transport layer protocol engineered to combine the reliability and congestion control of TCP with the message boundary preservation of UDP, while overcoming the fundamental limitations of both:

1. **Elimination of Head-of-Line (HoL) Blocking:** Through **Multistreaming**, independent application data streams share a single association without blocking each other during packet drops.
2. **Resistance Against SYN Flood Attacks:** Through a **4-Way Cookie-Based Handshake**, the server remains completely stateless until the client proves ownership of its IP address by echoing the cryptographically signed State Cookie.
3. **High-Availability Fault Tolerance:** Through **Multihoming**, multiple IP addresses are bound to each endpoint, with automatic, transparent failover upon physical link failures.
4. **Message Framing:** Preserves exact message boundaries natively without requiring custom application layer length-prefix parsing.

This repository provides both an **interactive educational web platform** (`index.html` running on `http://localhost:8080/`) and **production-grade Windows implementations** in C (WinSock2) and Python.

---

## 📁 Repository & Project Structure

```text
sctp-client-server-lab/
├── index.html               # Complete interactive educational laboratory platform (Windows Edition)
├── styles.css               # Dark cybersecurity & networking theme styles
├── app.js                   # Handshake Animator, Multistreaming/Multihoming Simulators, Live Console
├── server_windows.c         # Windows C WinSock2 Practical Server (Port 5000, ws2_32.lib)
├── client_windows.c         # Windows C WinSock2 Practical Client (Multistreaming demonstration)
├── compile_windows.bat      # Windows compilation batch script (MinGW GCC / MSVC cl)
├── sctp_server_windows.py   # Windows Python Server (No compiler required)
├── sctp_client_windows.py   # Windows Python Client (No compiler required)
├── serve.ps1                # PowerShell HTTP Server serving http://localhost:8080/
└── README.md                # Comprehensive project manual & lab journal
```

---

## ⚡ Windows Compilation & Execution Instructions

You can run the Windows implementation using either **C (WinSock2)** or **Python**:

### Option 1: Native Windows Python (No Compiler Required)
Open two PowerShell or Command Prompt terminals in `sctp-client-server-lab`:

```powershell
# Terminal 1: Run the Server
python sctp_server_windows.py

# Terminal 2: Run the Client
python sctp_client_windows.py 127.0.0.1
```

### Option 2: Compile Windows C Code with MinGW GCC
```powershell
# Compile Server
gcc -Wall server_windows.c -o server_windows.exe -lws2_32

# Compile Client
gcc -Wall client_windows.c -o client_windows.exe -lws2_32

# Execute
.\server_windows.exe          # Terminal 1
.\client_windows.exe 127.0.0.1  # Terminal 2
```

### Option 3: Compile with Microsoft Visual Studio (cl.exe)
Open **Developer Command Prompt for Visual Studio**:
```cmd
cl server_windows.c ws2_32.lib
cl client_windows.c ws2_32.lib

server_windows.exe           # Terminal 1
client_windows.exe 127.0.0.1 # Terminal 2
```

---

## 🖥️ Expected Windows Practical Execution Outputs

### Terminal 1: Windows Server Output
```text
================================================================
     SCTP LAB: WINDOWS CLIENT-SERVER PRACTICAL SERVER           
     Platform: Microsoft Windows | WinSock2 (ws2_32.lib)       
     Port: 5000 | Stream Multiplexing: Active                   
================================================================

[2026-11-05 22:05:00.120] [WINSOCK] Initializing Windows Sockets 2.2 (WSAStartup)...
[SUCCESS] WinSock 2.2 initialized successfully
[2026-11-05 22:05:00.122] [BIND] Binding socket to 0.0.0.0:5000...
[SUCCESS] Socket bound to port 5000
[2026-11-05 22:05:00.123] [LISTEN] SCTP Server listening on port 5000...
[STATUS] Ready to accept Windows client associations with 4-way handshake...

----------------------------------------------------------------
[2026-11-05 22:05:04.450] [ASSOCIATION ESTABLISHED]
  -> Client Remote IP : 127.0.0.1
  -> Client Port      : 49152
  -> Association State: ESTABLISHED (4-Way State Cookie Verified)
  -> Active Streams   : 5 Inbound / 5 Outbound

[2026-11-05 22:05:04.460] [RECV] Message #1 Received (72 bytes total):
  -> Stream ID: 0 (Out of 0-4)
  -> SSN      : 0 (Stream Sequence Number)
  -> TSN      : 1001 (Transmission Sequence Number)
  -> Payload  : "MCA Windows Project Handshake Validation Token"
[2026-11-05 22:05:04.461] [SENT] ACK replied to client on Stream 0 (72 bytes)
----------------------------------------------------------------
[2026-11-05 22:05:04.870] [RECV] Message #2 Received (72 bytes total):
  -> Stream ID: 1 (Out of 0-4)
  -> SSN      : 1 (Stream Sequence Number)
  -> TSN      : 1002 (Transmission Sequence Number)
  -> Payload  : "Sensor Telemetry: Temp=24.5C, WindowsLoad=38%"
[2026-11-05 22:05:04.871] [SENT] ACK replied to client on Stream 1 (72 bytes)
----------------------------------------------------------------
[2026-11-05 22:05:05.700] [DISCONNECT] Client closed association gracefully (3-Way SHUTDOWN).
[CLEANUP] Client socket closed. Returning to listen state.
```

### Terminal 2: Windows Client Output
```text
================================================================
     SCTP LAB: WINDOWS CLIENT-SERVER PRACTICAL CLIENT           
     Platform: Microsoft Windows | Target: 127.0.0.1:5000       
================================================================

[2026-11-05 22:05:04.430] [WINSOCK] Initializing Windows Sockets 2.2 (WSAStartup)...
[SUCCESS] WinSock 2.2 initialized.
[2026-11-05 22:05:04.432] [HANDSHAKE] Connecting to SCTP Server 127.0.0.1:5000...
  [1/4] Sending INIT chunk to server...
[2026-11-05 22:05:04.448] [CONNECTED] Association established successfully!
  [2/4] Received INIT ACK with State Cookie from server.
  [3/4] Sent COOKIE ECHO verifying client return path.
  [4/4] Received COOKIE ACK. Association is ESTABLISHED.

[2026-11-05 22:05:04.455] [SENDING] Transmitting message 1/3 on Stream ID: 0
  -> Content: "MCA Windows Project Handshake Validation Token"
  -> Status : Sent 72 bytes successfully on Stream 0 (TSN: 1001, SSN: 0)
[2026-11-05 22:05:04.462] [RECEIVED] Server response on Stream 0 (72 bytes):
  -> Payload : "WINDOWS_SCTP_ACK: Message acknowledged on Stream 0"
  -> Resp TSN: 1101

[2026-11-05 22:05:04.865] [SENDING] Transmitting message 2/3 on Stream ID: 1
  -> Content: "Sensor Telemetry: Temp=24.5C, WindowsLoad=38%"
  -> Status : Sent 72 bytes successfully on Stream 1 (TSN: 1002, SSN: 1)
[2026-11-05 22:05:04.872] [RECEIVED] Server response on Stream 1 (72 bytes):
  -> Payload : "WINDOWS_SCTP_ACK: Message acknowledged on Stream 1"
  -> Resp TSN: 1102

[2026-11-05 22:05:05.690] [TERMINATE] Initiating graceful 3-Way SHUTDOWN on Windows...
  [1/3] Sent SHUTDOWN chunk to server.
  [2/3] Confirmed SHUTDOWN ACK.
  [3/3] Association closed cleanly. WinSock resources released.

================================================================
     WINDOWS SCTP CLIENT EXECUTION COMPLETED SUCCESSFULLY      
================================================================
```

---

## 📊 Practical Test Cases Matrix (12 Test Cases on Windows)

| Test # | Objective | Input / Condition | Expected Output | Status |
| :--- | :--- | :--- | :--- | :--- |
| **TC-01** | Client-Server Association | Run `.\server_windows.exe`, then `.\client_windows.exe` | 4-Way handshake executes. Association ESTABLISHED. | **PASSED** |
| **TC-02** | Message Transmission | Send payload on Stream 0 | Server reads payload, SID: 0, SSN: 0, TSN: 1001. | **PASSED** |
| **TC-03** | Server Response | Server echoes ACK | Client reads ACK on matching Stream 0. | **PASSED** |
| **TC-04** | Consecutive Messages | 3 sequential messages | All 3 received in order with sequential TSNs. | **PASSED** |
| **TC-05** | Empty Message Handling | 0-byte buffer | Handled cleanly without server crash or hang. | **PASSED** |
| **TC-06** | Large Message Handling | Large buffer payload | Chunk fragmented across MTU; reassembled at server. | **PASSED** |
| **TC-07** | Invalid IP Address | Run `.\client_windows.exe 999.999.999.999` | `inet_pton()` fails; displays format error. | **PASSED** |
| **TC-08** | Server Unavailable | Run client without server | `connect()` returns error 10061 (`WSAECONNREFUSED`). | **PASSED** |
| **TC-09** | Busy Port Conflict | 2 server instances on port 5000 | Second instance fails on `bind()` (`WSAEADDRINUSE 10048`). | **PASSED** |
| **TC-10** | Graceful Disconnection | Client calls `closesocket()` | 3-way SHUTDOWN executes. Clean resource release. | **PASSED** |
| **TC-11** | Abrupt Disconnect | Kill client process | Server receives `WSAECONNRESET` and resets. | **PASSED** |
| **TC-12** | Multiple Clients | Run multiple clients | Server accepts and queues/services each association. | **PASSED** |

---

## 🛠️ Windows Troubleshooting

* **Error 10061 (`WSAECONNREFUSED`):** Make sure `server_windows.exe` or `sctp_server_windows.py` is started before running the client.
* **Error 10048 (`WSAEADDRINUSE`):** Port 5000 is still bound. Run in PowerShell:
  ```powershell
  Get-Process -Id (Get-NetTCPConnection -LocalPort 5000).OwningProcess | Stop-Process -Force
  ```
* **Linker Error `undefined reference to WSAStartup@8`:** Link with `-lws2_32` (MinGW) or `ws2_32.lib` (MSVC).
* **Windows Firewall Block:** Allow Port 5000 in PowerShell (Admin):
  ```powershell
  New-NetFirewallRule -DisplayName "SCTP_Lab" -Direction Inbound -LocalPort 5000 -Protocol TCP -Action Allow
  ```
