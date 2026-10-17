/* -----------------------------------------------------------------------
 * GGPO.net (http://ggpo.net)  -  Copyright 2009 GroundStorm Studios, LLC.
 *
 * Use of this software is governed by the MIT license that can be found
 * in the LICENSE file.
 */

#include "types.h"
#include "udp.h"

HINSTANCE hinstLib = NULL;

SOCKET
CreateSocket(int bind_port, int retries)
{
   SOCKET s;
   sockaddr_in sin;
   int port;
   int optval = 1;

   s = socket(AF_INET, SOCK_DGRAM, 0);
   setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (const char *)&optval, sizeof optval);
   setsockopt(s, SOL_SOCKET, SO_DONTLINGER, (const char *)&optval, sizeof optval);

   // non-blocking...
   u_long iMode = 1;
   ioctlsocket(s, FIONBIO, &iMode);

   sin.sin_family = AF_INET;
   sin.sin_addr.s_addr = htonl(INADDR_ANY);
   for (port = bind_port; port <= bind_port + retries; port++) {
      sin.sin_port = htons(port);
      if (bind(s, (sockaddr *)&sin, sizeof sin) != SOCKET_ERROR) {
         Log("Udp bound to port: %d.\n", port);
         return s;
      }
   }
   closesocket(s);
   return INVALID_SOCKET;
}

Udp::Udp() :
   _socket(INVALID_SOCKET),
   _callbacks(NULL)
{
}

Udp::~Udp(void)
{
   if (_socket != INVALID_SOCKET) {
      closesocket(_socket);
      _socket = INVALID_SOCKET;
   }
}

void
Udp::Init(int port, Poll *poll, Callbacks *callbacks)
{
   _callbacks = callbacks;

   _poll = poll;
   _poll->RegisterLoop(this);

   Log("binding udp socket to port %d.\n", port);
   _socket = CreateSocket(port, 0);
}

typedef int(__stdcall* SEND_TEXT_PROC)(const char*, int);
void
Udp::SendTo(const char* buffer, int len, int flags, struct sockaddr* dst, int destlen)
{
    if (hinstLib != NULL)
    {
        // Resolved once; this runs for every outbound packet.
        static SEND_TEXT_PROC PartySendProc = (SEND_TEXT_PROC)GetProcAddress(hinstLib, "PartyNet_SendNetworkMessage");

        if (NULL != PartySendProc)
        {
            (PartySendProc)(buffer, len);
            Log("sent packet length %d.\n", len);
        }
        else {
            Log("Can't find PartyNet_SendNetworkMessage method!");
        }
    }
}

void
Udp::AddRecvMsg(char const* msg, int len)
{
    if (msg == NULL || len <= 0 || len > MAX_UDP_PACKET_SIZE) {
        return;
    }

    std::lock_guard<std::mutex> lock(_recv_lock);

    // Dropping beats overrunning the ring: GGPO re-sends anything that matters.
    if (_recv_queue.size() >= 63) {
        Log("recv queue full, dropping packet of length %d.\n", len);
        return;
    }

    RecvPacket packet;
    packet.data = new char[len];
    packet.len = len;
    memcpy(packet.data, msg, len);

    _recv_queue.push(packet);
}

bool
Udp::OnLoopPoll(void *cookie)
{
    while (true) {
        RecvPacket packet;
        {
            std::lock_guard<std::mutex> lock(_recv_lock);
            if (_recv_queue.empty()) {
                break;
            }
            packet = _recv_queue.front();
            _recv_queue.pop();
        }

        sockaddr_in socketaddr{};

        Log("recvfrom returned (len:%d  from:%s:%d).\n", packet.len, inet_ntoa(socketaddr.sin_addr), ntohs(socketaddr.sin_port));
        _callbacks->OnMsg(socketaddr, (UdpMsg*)packet.data, packet.len);

        delete[] packet.data;
    }
    return true;
}


void
Udp::Log(const char *fmt, ...)
{
   char buf[1024];
   size_t offset;
   va_list args;

   strcpy(buf, "udp | ");
   offset = strlen(buf);
   va_start(args, fmt);
   vsnprintf(buf + offset, ARRAY_SIZE(buf) - offset - 1, fmt, args);
   buf[ARRAY_SIZE(buf)-1] = '\0';
   ::Log(buf);
   va_end(args);
}
