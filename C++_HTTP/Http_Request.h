#ifndef HTTP_REQUEST_INFO
#define HTTP_REQUEST_INFO

#define SERVER_IP _TEXT("127.0.0.1")
#define PORT 80
#define IP_ADDRESS_LEN 16
#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <WinSock2.h>
#include <string>
#pragma comment(lib,"ws2_32.lib")

using namespace std;

class RINGBUFFER;

class HTTP_REQUEST
{
private:

	BYTE m_major_version, m_minor_version; // 추후에 DB에서 가져오는거???

	TCHAR m_ip[IP_ADDRESS_LEN];
	WSADATA m_wsadata;

	SOCKET m_http_socket;
	SOCKADDR_IN m_sockaddr;
	
	RINGBUFFER *m_sendQ, *m_recvQ;

	string m_http_header;

public:
	HTTP_REQUEST(bool &p_flag);
	~HTTP_REQUEST();

	void MakePacket(string p_path);
	bool SendPost();
	bool RecvPost();
	void Disconnect();
};

#endif