/**
 * ============================================================================
 * PROJECT: Implementing Client-Server Connection Using SCTP
 * FILE: app.js
 * COURSE: MCA Computer Networks Practical Platform Engine (Windows Edition)
 * ============================================================================
 */

document.addEventListener('DOMContentLoaded', () => {
  initNavigation();
  initHandshakeVisualizer();
});

/* ==========================================================================
   Navigation & Scroll Tracking
   ========================================================================== */

function initNavigation() {
  const menuToggle = document.getElementById('menuToggle');
  const sidebar = document.getElementById('appSidebar');

  if (menuToggle && sidebar) {
    menuToggle.addEventListener('click', () => {
      sidebar.classList.toggle('open');
    });

    const navLinks = sidebar.querySelectorAll('.nav-link');
    navLinks.forEach(link => {
      link.addEventListener('click', () => {
        if (window.innerWidth <= 1024) {
          sidebar.classList.remove('open');
        }
      });
    });
  }

  const sections = document.querySelectorAll('.content-section, .hero-section');
  const navLinks = document.querySelectorAll('.nav-link');

  window.addEventListener('scroll', () => {
    let current = '';
    const scrollPos = window.scrollY + 120;

    sections.forEach(section => {
      const top = section.offsetTop;
      const height = section.offsetHeight;
      if (scrollPos >= top && scrollPos < top + height) {
        current = section.getAttribute('id');
      }
    });

    navLinks.forEach(link => {
      link.classList.remove('active');
      if (link.getAttribute('href') === `#${current}`) {
        link.classList.add('active');
      }
    });
  });
}

/* ==========================================================================
   Toast Notification Utility
   ========================================================================== */

function showToast(message) {
  const toast = document.getElementById('toastNotice');
  const msgElem = document.getElementById('toastMessage');
  if (!toast || !msgElem) return;

  msgElem.textContent = message;
  toast.classList.add('show');
  setTimeout(() => {
    toast.classList.remove('show');
  }, 2500);
}

function copyCode(elementId, buttonElem) {
  const codeElem = document.getElementById(elementId);
  if (!codeElem) return;

  const textToCopy = codeElem.innerText;
  navigator.clipboard.writeText(textToCopy).then(() => {
    showToast('Source code copied to clipboard!');
    const originalText = buttonElem.innerHTML;
    buttonElem.innerHTML = '✓ Copied!';
    setTimeout(() => {
      buttonElem.innerHTML = originalText;
    }, 2000);
  }).catch(err => {
    console.error('Clipboard copy failed:', err);
  });
}

/* ==========================================================================
   Modal Dialog Inspector
   ========================================================================== */

const COMPONENT_DETAILS = {
  client: {
    title: 'Windows SCTP Client Node (Endpoint)',
    content: `
      <p>The <strong>Windows SCTP Client Endpoint</strong> initiates associations and transmits multi-stream messages:</p>
      <ul>
        <li><strong>WinSock Subsystem:</strong> Initialized via <code>WSAStartup(MAKEWORD(2,2), &amp;wsaData)</code>.</li>
        <li><strong>Local Address:</strong> Binds to local loopback (<code>127.0.0.1</code>) or physical adapter.</li>
        <li><strong>Stream Allocation:</strong> Negotiates up to 5 outbound and 5 inbound streams during INIT.</li>
        <li><strong>Port Number:</strong> Ephemeral Windows port assigned by WinSock (e.g. 49152).</li>
      </ul>
      <p>The client controls message multiplexing across streams (0 through 4) without Head-of-Line blocking.</p>
    `
  },
  server: {
    title: 'Windows SCTP Server Node (Passive Endpoint)',
    content: `
      <p>The <strong>Windows SCTP Server Endpoint</strong> operates in passive listening mode on port 5000:</p>
      <ul>
        <li><strong>State Cookie Defense:</strong> When an <code>INIT</code> chunk arrives, the server generates an HMAC State Cookie and sends an <code>INIT ACK</code> without allocating Transmission Control Block (TCB) memory.</li>
        <li><strong>Resource Protection:</strong> Protects Windows host memory against SYN-flood denial of service attacks.</li>
        <li><strong>Multi-Message Loop:</strong> Continuously receives messages across diverse streams and replies with matching stream acknowledgments.</li>
      </ul>
    `
  },
  network: {
    title: 'Windows Network Transport & Dual-Homed Association',
    content: `
      <p>The <strong>Transport Network</strong> carries chunk-based Protocol 132 frames across Windows network adapters:</p>
      <ul>
        <li><strong>Association:</strong> A multi-address connection binding the set of client IP addresses to server IP addresses.</li>
        <li><strong>Path Heartbeats:</strong> Periodically sends <code>HEARTBEAT</code> chunks to idle alternate paths to verify connectivity and measure RTT.</li>
        <li><strong>Packet Checksum:</strong> Employs 32-bit CRC32c for data integrity verification, far superior to TCP's 16-bit sum.</li>
      </ul>
    `
  }
};

const CHUNK_DETAILS = {
  INIT: {
    title: 'INIT Chunk (Type 0x01) - Initiation',
    content: `
      <p>Transmitted by the client to initiate an SCTP association.</p>
      <table class="tech-table">
        <tr><th>Field</th><th>Description</th></tr>
        <tr><td><strong>Initiate Tag</strong></td><td>32-bit random tag chosen by client. Server must reflect this in all subsequent packet headers.</td></tr>
        <tr><td><strong>a_rwnd</strong></td><td>Advertised Receiver Window Credit (e.g., 65535 bytes).</td></tr>
        <tr><td><strong>OS</strong></td><td>Number of Outbound Streams requested by client.</td></tr>
        <tr><td><strong>MIS</strong></td><td>Maximum Inbound Streams client allows server to send.</td></tr>
        <tr><td><strong>Initial TSN</strong></td><td>Starting 32-bit Transmission Sequence Number.</td></tr>
      </table>
    `
  },
  INIT_ACK: {
    title: 'INIT ACK Chunk (Type 0x02) - Initiation Acknowledge',
    content: `
      <p>Transmitted by the server in response to an INIT chunk.</p>
      <p>Contains the server's parameters (Initiate Tag, a_rwnd, OS, MIS) plus the critical <strong>State Cookie</strong> parameter.</p>
      <p>The State Cookie encapsulates client parameters, timestamp, and a Message Authentication Code (MAC). The server remains completely stateless.</p>
    `
  },
  COOKIE_ECHO: {
    title: 'COOKIE ECHO Chunk (Type 0x0A)',
    content: `
      <p>Sent by the client echoing back the exact State Cookie received in the INIT ACK chunk.</p>
      <p>Proves that the client actually owns the IP address it claimed during INIT, defeating IP address spoofing attacks.</p>
      <p>User DATA chunks can be bundled alongside COOKIE ECHO to reduce round-trip latency!</p>
    `
  },
  COOKIE_ACK: {
    title: 'COOKIE ACK Chunk (Type 0x0B)',
    content: `
      <p>Sent by the server to confirm successful validation of the State Cookie.</p>
      <p>The server validates the MAC signature. If authentic, the server allocates association memory (TCB) and marks the association as <strong>ESTABLISHED</strong>.</p>
    `
  },
  DATA: {
    title: 'DATA Chunk (Type 0x00) - Application Payload',
    content: `
      <p>Encapsulates user application messages.</p>
      <table class="tech-table">
        <tr><th>Field</th><th>Description</th></tr>
        <tr><td><strong>TSN</strong></td><td>32-bit Transmission Sequence Number for global ordering/retransmissions.</td></tr>
        <tr><td><strong>Stream Identifier (SID)</strong></td><td>16-bit identifier of the logical stream (0 to 65535).</td></tr>
        <tr><td><strong>Stream Sequence Number (SSN)</strong></td><td>16-bit sequence number within the specific stream.</td></tr>
        <tr><td><strong>Payload Protocol ID (PPID)</strong></td><td>32-bit application layer protocol tag.</td></tr>
        <tr><td><strong>Flags: U, B, E</strong></td><td>U=Unordered, B=Beginning fragment, E=Ending fragment.</td></tr>
      </table>
    `
  },
  SACK: {
    title: 'SACK Chunk (Type 0x03) - Selective Acknowledgment',
    content: `
      <p>Acknowledges received DATA chunks back to the peer sender.</p>
      <p>Contains <strong>Cumulative TSN Ack</strong> (highest sequential TSN received) plus <strong>Gap Ack Blocks</strong> detailing fragmented received blocks, enabling selective retransmissions without re-sending already received data.</p>
    `
  },
  HEARTBEAT: {
    title: 'HEARTBEAT & HEARTBEAT ACK Chunks (Types 0x04 & 0x05)',
    content: `
      <p>Used in multihomed associations to monitor the reachability of idle destination IP addresses.</p>
      <p>Contains Heartbeat Information (Sender timestamp and nonce). The receiver immediately echoes it in a HEARTBEAT ACK, allowing the sender to calculate Round Trip Time (RTT) and maintain path health metrics.</p>
    `
  },
  SHUTDOWN: {
    title: 'SHUTDOWN Chunk (Type 0x07) - Association Close',
    content: `
      <p>Initiates graceful association termination.</p>
      <p>Carries the <strong>Cumulative TSN Ack</strong> of the sender. Once sent, the sender will accept no new outbound user data, but will process pending inflight packets until the 3-way shutdown handshake completes.</p>
    `
  }
};

function inspectComponent(nodeKey) {
  const data = COMPONENT_DETAILS[nodeKey];
  if (!data) return;
  openModal(data.title, data.content);
}

function showChunkModal(chunkKey) {
  const data = CHUNK_DETAILS[chunkKey];
  if (!data) return;
  openModal(data.title, data.content);
}

function openModal(title, htmlContent) {
  const modal = document.getElementById('infoModal');
  const titleElem = document.getElementById('modalTitle');
  const bodyElem = document.getElementById('modalBody');

  if (modal && titleElem && bodyElem) {
    titleElem.textContent = title;
    bodyElem.innerHTML = htmlContent;
    modal.classList.add('active');
  }
}

function closeModal() {
  const modal = document.getElementById('infoModal');
  if (modal) modal.classList.remove('active');
}

/* ==========================================================================
   Handshake Interactive Player
   ========================================================================== */

let currentHsStep = 0;
let hsInterval = null;

function initHandshakeVisualizer() {
  updateHandshakeUI();
}

function updateHandshakeUI() {
  const stepNum = document.getElementById('handshakeStepNum');
  const clientBadge = document.getElementById('clientStateBadge');
  const serverBadge = document.getElementById('serverStateBadge');

  if (stepNum) stepNum.textContent = currentHsStep;

  for (let i = 1; i <= 4; i++) {
    const stepElem = document.getElementById(`hsStep${i}`);
    if (stepElem) {
      stepElem.classList.remove('active', 'completed');
      if (i < currentHsStep) {
        stepElem.classList.add('completed');
      } else if (i === currentHsStep) {
        stepElem.classList.add('active');
      }
    }
  }

  if (currentHsStep === 0) {
    clientBadge.textContent = 'STATE: CLOSED';
    serverBadge.textContent = 'STATE: LISTEN';
  } else if (currentHsStep === 1) {
    clientBadge.textContent = 'STATE: COOKIE-WAIT';
    serverBadge.textContent = 'STATE: LISTEN (STATELESS)';
  } else if (currentHsStep === 2) {
    clientBadge.textContent = 'STATE: COOKIE-WAIT';
    serverBadge.textContent = 'STATE: COOKIE SENT';
  } else if (currentHsStep === 3) {
    clientBadge.textContent = 'STATE: COOKIE-ECHOED';
    serverBadge.textContent = 'STATE: VALIDATING COOKIE';
  } else if (currentHsStep === 4) {
    clientBadge.textContent = 'STATE: ESTABLISHED';
    serverBadge.textContent = 'STATE: ESTABLISHED';
  }
}

function nextHandshakeStep() {
  if (currentHsStep < 4) {
    currentHsStep++;
    updateHandshakeUI();
  } else {
    stopHandshakePlay();
  }
}

function prevHandshakeStep() {
  if (currentHsStep > 0) {
    currentHsStep--;
    updateHandshakeUI();
  }
}

function resetHandshake() {
  stopHandshakePlay();
  currentHsStep = 0;
  updateHandshakeUI();
}

function toggleHandshakePlay() {
  const btn = document.getElementById('btnPlayHandshake');
  if (hsInterval) {
    stopHandshakePlay();
  } else {
    if (currentHsStep >= 4) currentHsStep = 0;
    updateHandshakeUI();
    btn.textContent = 'Pause';
    hsInterval = setInterval(() => {
      if (currentHsStep < 4) {
        currentHsStep++;
        updateHandshakeUI();
      } else {
        stopHandshakePlay();
      }
    }, 1500);
  }
}

function stopHandshakePlay() {
  if (hsInterval) {
    clearInterval(hsInterval);
    hsInterval = null;
  }
  const btn = document.getElementById('btnPlayHandshake');
  if (btn) btn.textContent = 'Play Animation';
}

/* ==========================================================================
   Multistreaming Interactive Demonstration
   ========================================================================== */

function resetMultistreamSim() {
  ['s0', 's1', 's2'].forEach(s => {
    const prog = document.getElementById(`${s}Progress`);
    const status = document.getElementById(`${s}Status`);
    if (prog) prog.style.width = '0%';
    if (status) {
      status.textContent = 'READY';
      status.className = 'badge';
    }
  });
  const res = document.getElementById('multistreamResult');
  if (res) res.textContent = 'Status: Ready to trigger multistreaming simulation.';
}

function runMultistreamSim(simulateLoss) {
  resetMultistreamSim();
  const res = document.getElementById('multistreamResult');

  if (!simulateLoss) {
    res.textContent = '[TRANSMITTING] Sending 3 messages simultaneously across Stream 0, 1, and 2 on Windows...';

    setTimeout(() => {
      document.getElementById('s0Progress').style.width = '100%';
      document.getElementById('s0Status').textContent = 'DELIVERED (200ms)';
      document.getElementById('s0Status').className = 'badge badge-green';
    }, 200);

    setTimeout(() => {
      document.getElementById('s1Progress').style.width = '100%';
      document.getElementById('s1Status').textContent = 'DELIVERED (250ms)';
      document.getElementById('s1Status').className = 'badge badge-green';
    }, 250);

    setTimeout(() => {
      document.getElementById('s2Progress').style.width = '100%';
      document.getElementById('s2Status').textContent = 'DELIVERED (300ms)';
      document.getElementById('s2Status').className = 'badge badge-green';
      res.textContent = '[SUCCESS] All 3 streams delivered concurrently with independent Stream Sequence Numbers (SSN).';
    }, 300);

  } else {
    res.textContent = '[SIMULATION: LOSS ON STREAM 1] Transmitting 3 messages. Dropping packet on Stream 1...';

    setTimeout(() => {
      document.getElementById('s0Progress').style.width = '100%';
      document.getElementById('s0Status').textContent = 'DELIVERED (200ms)';
      document.getElementById('s0Status').className = 'badge badge-green';
    }, 200);

    setTimeout(() => {
      document.getElementById('s2Progress').style.width = '100%';
      document.getElementById('s2Status').textContent = 'DELIVERED (280ms)';
      document.getElementById('s2Status').className = 'badge badge-green';
      res.innerHTML = '<span class="text-green">✓ Stream 0 and Stream 2 delivered immediately! Zero Head-of-Line Blocking!</span><br><span class="text-yellow">Stream 1 packet lost. SACK indicates missing TSN. Retransmitting...</span>';
    }, 280);

    document.getElementById('s1Progress').style.width = '40%';
    document.getElementById('s1Status').textContent = 'DROPPED (RETRANSMITTING)';
    document.getElementById('s1Status').className = 'badge badge-yellow';

    setTimeout(() => {
      document.getElementById('s1Progress').style.width = '100%';
      document.getElementById('s1Status').textContent = 'RETRANSMITTED & DELIVERED (1200ms)';
      document.getElementById('s1Status').className = 'badge badge-green';
      res.innerHTML = '<strong class="text-cyan">CONCLUSION:</strong> In TCP, the entire connection would have paused for 1200ms waiting for packet 1. In SCTP, Streams 0 & 2 delivered in under 300ms without delay!';
    }, 1200);
  }
}

/* ==========================================================================
   Multihoming Dual-Homed Path Simulator
   ========================================================================== */

let isPathAActive = true;

function togglePrimaryLink() {
  const btn = document.getElementById('btnToggleLink');
  const pathAStatus = document.getElementById('pathAStatus');
  const pathBStatus = document.getElementById('pathBStatus');
  const logElem = document.getElementById('multihomeLog');
  const pathAContainer = document.getElementById('pathAContainer');
  const pathBContainer = document.getElementById('pathBContainer');

  if (isPathAActive) {
    isPathAActive = false;
    btn.textContent = 'Restore Primary Link (Path A)';
    btn.className = 'btn-secondary btn-sm';

    pathAStatus.textContent = 'DISCONNECTED (LINK DOWN)';
    pathAStatus.className = 'badge badge-yellow';
    pathAContainer.style.borderColor = 'var(--accent-red)';
    pathAContainer.style.background = 'rgba(239, 68, 68, 0.08)';

    pathBStatus.textContent = 'PRIMARY ACTIVE (FAILOVER SUCCESS)';
    pathBStatus.className = 'badge badge-green';
    pathBContainer.style.borderColor = 'var(--accent-cyan)';
    pathBContainer.style.borderStyle = 'solid';
    pathBContainer.style.background = 'rgba(0, 242, 254, 0.08)';

    logElem.innerHTML = `
      <span class="text-red">[WINDOWS NETWORKING ALERT] Primary Path A (192.168.1.10 &rarr; 192.168.1.100) failed! Heartbeat threshold exceeded.</span><br>
      <span class="text-green">[FAILOVER SUCCESS] Windows WinSock transparently redirected DATA traffic to Alternate Path B (10.0.0.10 &rarr; 10.0.0.100). Association stayed connected with ZERO socket disconnects!</span>
    `;
    showToast('Failover triggered: Routed to Path B!');
  } else {
    isPathAActive = true;
    btn.textContent = 'Disconnect Primary Link (Path A)';
    btn.className = 'btn-primary btn-sm';

    pathAStatus.textContent = 'ACTIVE (RTT: 12ms)';
    pathAStatus.className = 'badge badge-green';
    pathAContainer.style.borderColor = 'var(--accent-cyan)';
    pathAContainer.style.background = 'rgba(0, 242, 254, 0.05)';

    pathBStatus.textContent = 'STANDBY (HEARTBEAT OK)';
    pathBStatus.className = 'badge';
    pathBContainer.style.borderColor = 'var(--border-color)';
    pathBContainer.style.borderStyle = 'dashed';
    pathBContainer.style.background = 'rgba(255, 255, 255, 0.02)';

    logElem.innerHTML = `
      <span class="text-green">[RESTORED] Primary Path A link back online. Heartbeat probe confirmed health on Windows network interface.</span>
    `;
    showToast('Path A restored!');
  }
}

function sendHeartbeatSim() {
  const logElem = document.getElementById('multihomeLog');
  const now = new Date().toLocaleTimeString();
  logElem.innerHTML = `
    <span class="text-cyan">[HEARTBEAT @ ${now}]</span> Transmitted HEARTBEAT chunk (Type 0x04) to alternate IP.<br>
    Received HEARTBEAT ACK (Type 0x05) in 11.4 ms. Windows interface health verified optimal.
  `;
  showToast('HEARTBEAT probe acknowledged!');
}

/* ==========================================================================
   Live Interactive SCTP Demonstration Lab Engine (Windows)
   ========================================================================== */

let demoState = {
  serverRunning: false,
  clientConnected: false,
  assocId: 0,
  vTag: '0x00000000',
  currentTsn: 1000,
  packetCount: 0
};

function formatTimestamp() {
  const d = new Date();
  const pad = n => n.toString().padStart(2, '0');
  return `[${pad(d.getHours())}:${pad(d.getMinutes())}:${pad(d.getSeconds())}]`;
}

function appendDemoLog(type, message) {
  const terminal = document.getElementById('demoTerminalOutput');
  if (!terminal) return;

  const line = document.createElement('div');
  line.className = 'terminal-line';

  let tagClass = 'log-sctp';
  if (type === 'SERVER') tagClass = 'log-server';
  if (type === 'CLIENT') tagClass = 'log-client';
  if (type === 'ERROR')  tagClass = 'log-err';
  if (type === 'WARN')   tagClass = 'log-warn';

  line.innerHTML = `<span class="log-time">${formatTimestamp()}</span> <span class="${tagClass}">[${type}]</span> ${message}`;
  terminal.appendChild(line);
  terminal.scrollTop = terminal.scrollHeight;
}

function appendPacketRow(src, dst, chunkType, vtag, tsn, stream, info) {
  const tbody = document.getElementById('packetCaptureTable');
  if (!tbody) return;

  if (demoState.packetCount === 0) {
    tbody.innerHTML = '';
  }
  demoState.packetCount++;

  const tr = document.createElement('tr');
  tr.innerHTML = `
    <td>${demoState.packetCount}</td>
    <td>${src}</td>
    <td>${dst}</td>
    <td><strong class="text-cyan">${chunkType}</strong></td>
    <td><code>${vtag}</code></td>
    <td>${tsn}</td>
    <td>${stream}</td>
    <td>${info}</td>
  `;
  tbody.appendChild(tr);
}

function demoStartServer() {
  if (demoState.serverRunning) return;

  demoState.serverRunning = true;
  document.getElementById('serverLed').className = 'status-dot listening';
  document.getElementById('serverLedText').textContent = 'Listening on 0.0.0.0:5000 (WinSock)';
  document.getElementById('btnStartServer').disabled = true;
  document.getElementById('btnConnectClient').disabled = false;

  appendDemoLog('SERVER', 'WSAStartup(MAKEWORD(2,2)) loaded ws2_32.lib subsystem.');
  appendDemoLog('SERVER', 'socket(AF_INET, SOCK_STREAM, IPPROTO_TCP) created Windows handle (0x104).');
  appendDemoLog('SERVER', 'Configured 5 Outbound and 5 Inbound logical streams.');
  appendDemoLog('SERVER', 'bind() bound to local address 0.0.0.0:5000.');
  appendDemoLog('SERVER', 'listen(): Waiting for incoming client associations on Windows...');
  showToast('Windows SCTP Server started on port 5000!');
}

function demoConnectClient() {
  if (!demoState.serverRunning || demoState.clientConnected) return;

  demoState.assocId = Math.floor(Math.random() * 8999) + 1000;
  demoState.vTag = '0x' + Math.floor(Math.random() * 0xFFFFFFFF).toString(16).toUpperCase().padStart(8, '0');

  appendDemoLog('CLIENT', 'WSAStartup() initialized client WinSock context.');
  appendDemoLog('CLIENT', 'socket() created client descriptor handle (0x108).');
  appendDemoLog('CLIENT', `Connecting to server 127.0.0.1:5000 via SCTP 4-Way Cookie Handshake...`);

  // 1. INIT
  setTimeout(() => {
    appendDemoLog('CLIENT', 'Sending INIT chunk (Initiate Tag: ' + demoState.vTag + ', OS: 5, MIS: 5, Initial TSN: 1000)...');
    appendPacketRow('127.0.0.1:49152', '127.0.0.1:5000', 'INIT', '0x00000000', '1000', '-', 'OS: 5, MIS: 5, a_rwnd: 65535');
  }, 200);

  // 2. INIT ACK
  setTimeout(() => {
    appendDemoLog('SERVER', 'Received INIT. Server remains stateless and generates HMAC State Cookie...');
    appendDemoLog('SERVER', 'Sending INIT ACK chunk with State Cookie to client...');
    appendPacketRow('127.0.0.1:5000', '127.0.0.1:49152', 'INIT ACK', demoState.vTag, '-', '-', 'State Cookie HMAC-SHA256, OS: 5');
  }, 500);

  // 3. COOKIE ECHO
  setTimeout(() => {
    appendDemoLog('CLIENT', 'Received INIT ACK. Echoing State Cookie back in COOKIE ECHO chunk...');
    appendPacketRow('127.0.0.1:49152', '127.0.0.1:5000', 'COOKIE ECHO', demoState.vTag, '-', '-', 'Echo State Cookie (Validating Return Path)');
  }, 800);

  // 4. COOKIE ACK
  setTimeout(() => {
    appendDemoLog('SERVER', 'Verified State Cookie signature! Allocating TCB state and sending COOKIE ACK...');
    appendPacketRow('127.0.0.1:5000', '127.0.0.1:49152', 'COOKIE ACK', demoState.vTag, '-', '-', 'Association Setup Acknowledged');

    demoState.clientConnected = true;
    document.getElementById('clientLed').className = 'status-dot active';
    document.getElementById('clientLedText').textContent = 'Connected (Assoc #' + demoState.assocId + ')';
    document.getElementById('demoAssocId').textContent = '#' + demoState.assocId;

    document.getElementById('btnConnectClient').disabled = true;
    document.getElementById('demoStreamSelect').disabled = false;
    document.getElementById('demoMsgInput').disabled = false;
    document.getElementById('btnSendMessage').disabled = false;
    document.getElementById('btnDisconnect').disabled = false;

    appendDemoLog('SCTP', 'Association successfully ESTABLISHED between Client (49152) and Server (5000)!');
    showToast('Association established via 4-Way Cookie Handshake!');
  }, 1100);
}

function demoSendMessage() {
  if (!demoState.clientConnected) return;

  const streamSelect = document.getElementById('demoStreamSelect');
  const msgInput = document.getElementById('demoMsgInput');

  const streamId = parseInt(streamSelect.value, 10);
  const msgText = msgInput.value.trim() || 'Windows SCTP Test Payload';
  const tsn = ++demoState.currentTsn;
  const ssn = Math.floor(Math.random() * 20);

  appendDemoLog('CLIENT', `send() dispatched: "${msgText}" on Stream ID ${streamId} (TSN: ${tsn})`);
  appendPacketRow('127.0.0.1:49152', '127.0.0.1:5000', 'DATA', demoState.vTag, tsn, streamId, `"${msgText}" (SSN: ${ssn})`);

  setTimeout(() => {
    appendDemoLog('SERVER', `recv() received on Stream ${streamId} | SSN: ${ssn} | TSN: ${tsn} | Payload: "${msgText}"`);
    appendPacketRow('127.0.0.1:5000', '127.0.0.1:49152', 'SACK', demoState.vTag, `CumAck: ${tsn}`, '-', 'Selective Ack, a_rwnd: 65535');

    const reply = `WINDOWS_ACK: Received "${msgText}" on Stream ${streamId}`;
    appendDemoLog('SERVER', `send() replying response back to client on Stream ${streamId}...`);
    appendPacketRow('127.0.0.1:5000', '127.0.0.1:49152', 'DATA', demoState.vTag, ++demoState.currentTsn, streamId, `"${reply}"`);

    setTimeout(() => {
      appendDemoLog('CLIENT', `Received response on Stream ${streamId}: "${reply}"`);
    }, 200);
  }, 300);
}

function demoDisconnect() {
  if (!demoState.clientConnected) return;

  appendDemoLog('CLIENT', 'closesocket() invoked. Initiating graceful 3-Way SHUTDOWN on Windows...');
  appendPacketRow('127.0.0.1:49152', '127.0.0.1:5000', 'SHUTDOWN', demoState.vTag, `CumAck: ${demoState.currentTsn}`, '-', 'No more data will be sent');

  setTimeout(() => {
    appendDemoLog('SERVER', 'Received SHUTDOWN. Flushing queues and replying with SHUTDOWN ACK...');
    appendPacketRow('127.0.0.1:5000', '127.0.0.1:49152', 'SHUTDOWN ACK', demoState.vTag, '-', '-', 'All TSNs acknowledged');

    setTimeout(() => {
      appendDemoLog('CLIENT', 'Received SHUTDOWN ACK. Sending SHUTDOWN COMPLETE...');
      appendPacketRow('127.0.0.1:49152', '127.0.0.1:5000', 'SHUTDOWN COMP', demoState.vTag, '-', '-', 'Association resources freed');

      demoState.clientConnected = false;
      document.getElementById('clientLed').className = 'status-dot';
      document.getElementById('clientLedText').textContent = 'Client Closed';
      document.getElementById('demoAssocId').textContent = 'NONE';

      document.getElementById('btnConnectClient').disabled = false;
      document.getElementById('demoStreamSelect').disabled = true;
      document.getElementById('demoMsgInput').disabled = true;
      document.getElementById('btnSendMessage').disabled = true;
      document.getElementById('btnDisconnect').disabled = true;

      appendDemoLog('SCTP', 'Association terminated cleanly. WSACleanup() called.');
      showToast('Association terminated via 3-Way SHUTDOWN!');
    }, 300);
  }, 300);
}

function demoClearLogs() {
  const terminal = document.getElementById('demoTerminalOutput');
  const tbody = document.getElementById('packetCaptureTable');
  if (terminal) terminal.innerHTML = '<div class="terminal-line"><span class="log-time">[00:00:00]</span> <span class="log-sctp">[SYSTEM]</span> Console logs cleared.</div>';
  if (tbody) tbody.innerHTML = '<tr><td colspan="8" style="text-align: center; color: var(--text-dim);">Packet capture buffer cleared.</td></tr>';
  demoState.packetCount = 0;
  showToast('Logs and packet captures cleared!');
}
