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

//typedef int(__cdecl* SEND_TEXT_PROC)(const std::vector<uint8_t>);
typedef int(__cdecl* SEND_TEXT_PROC)(const char*, int);
void
Udp::SendTo(const char* buffer, int len, int flags, struct sockaddr* dst, int destlen)
//Udp::SendTo(const std::vector<uint8_t> buffer, int len, int flags, struct sockaddr *dst, int destlen)
{
    if (hinstLib != NULL)
    {
        auto PartySendProc = (SEND_TEXT_PROC)GetProcAddress(hinstLib, "PartySampleApp_SendNetworkMessage");
        const char* sampleMsg = "ul\x2";
        const char* sampleMsg2 = "ul\x2";
        char* sampleMsg3 = "ul\x2";
        unsigned __int64 dataSize = len; // size of hdr plus size of u
        char* sampleMsg4 = new char[dataSize];//init this with the correct size
        char* sampleMsg5 = new char[dataSize];//init this with the correct size
        std::copy(sampleMsg3, sampleMsg3 + dataSize, sampleMsg4);
        // Insert null terminator character
        // sampleMsg4[dataSize] = '\0';
        memcpy(sampleMsg5, sampleMsg3, dataSize);
        delete[] sampleMsg4;
        /*
        const char* sampleMsg = "¼c";

        uint8 recv_buf_s[MAX_UDP_PACKET_SIZE];
        memcpy((char*)recv_buf_s, sampleMsg, MAX_UDP_PACKET_SIZE);

        UdpMsg* udpmsg1 = (UdpMsg*)recv_buf_s;

        auto PartySendProc = (SEND_TEXT_PROC)GetProcAddress(hinstLib, "PartySampleApp_SendNetworkBytes");
        */
        // If the function address is valid, call the function.


        if (NULL != PartySendProc)
        {
            //int len = sizeof(buffer);
            //(PartySendProc)(buffer);
            (PartySendProc)(sampleMsg, len);
            Log("sent packet length %d.\n", len);
        }
        else {
            Log("Can't find PartySampleApp_SendNetworkMessage method!");
        }
    }

    return;
    /*
    struct sockaddr_in *to = (struct sockaddr_in *)dst;

   int res = sendto(_socket, buffer, len, flags, dst, destlen);
   if (res == SOCKET_ERROR) {
      DWORD err = WSAGetLastError();
      DWORD e2 = WSAENOTSOCK;
      Log("unknown error in sendto (erro: %d  wsaerr: %d).\n", res, err);
      ASSERT(FALSE && "Unknown error in sendto");
   }
   Log("sent packet length %d to %s:%d (ret:%d).\n", len, inet_ntoa(to->sin_addr), ntohs(to->sin_port), res);
   */
}

bool
Udp::OnLoopPoll(void *cookie)
{
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
