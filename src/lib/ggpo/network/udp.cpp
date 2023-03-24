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

typedef int(__cdecl* SEND_TEXT_PROC)(const char*, int);
void
Udp::SendTo(const char* buffer, int len, int flags, struct sockaddr* dst, int destlen)
{
    if (hinstLib != NULL)
    {
        auto PartySendProc = (SEND_TEXT_PROC)GetProcAddress(hinstLib, "PartySampleApp_SendNetworkMessage");

        // If the function address is valid, call the function.
        if (NULL != PartySendProc)
        {
            (PartySendProc)(buffer, len);
            Log("sent packet length %d.\n", len);
        }
        else {
            Log("Can't find PartySampleApp_SendNetworkMessage method!");
        }
    }
}

bool
Udp::OnLoopPoll(void *cookie)
{
    // TODO: Fix loop here!!!!!!  OnMsg needs to occur here, as originally intended
    return true;
   uint8          recv_buf[MAX_UDP_PACKET_SIZE];
   sockaddr_in    recv_addr;
   int            recv_addr_len;

   for (;;) {
      recv_addr_len = sizeof(recv_addr);
      int len = recvfrom(_socket, (char *)recv_buf, MAX_UDP_PACKET_SIZE, 0, (struct sockaddr *)&recv_addr, &recv_addr_len);

      // TODO: handle len == 0... indicates a disconnect.

      if (len == -1) {
         int error = WSAGetLastError();
         if (error != WSAEWOULDBLOCK) {
            Log("recvfrom WSAGetLastError returned %d (%x).\n", error, error);
         }
         break;
      } else if (len > 0) {
         Log("recvfrom returned (len:%d  from:%s:%d).\n", len,inet_ntoa(recv_addr.sin_addr), ntohs(recv_addr.sin_port) );
         UdpMsg *msg = (UdpMsg *)recv_buf;
         _callbacks->OnMsg(recv_addr, msg, len);
      } 
   }
   return true;
}

bool
Udp::OnMessageReceived(const char* buffer)
{
    int len = sizeof(buffer);
    Log("recieved message of length %d.\n", len);
    UdpMsg* msg = (UdpMsg*)buffer;
    sockaddr_in socketaddr{};
    _callbacks->OnMsg(socketaddr, msg, len);
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
