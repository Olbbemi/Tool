#ifndef HTTP_REQUEST_INFO
#define HTTP_REQUEST_INFO

/*
 *
 *
 */

#define IP_ADDRESS_LEN 16
#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <WinSock2.h>
#include <string>
#pragma comment(lib,"ws2_32.lib")

using namespace std;

class HTTP_REQUEST
{
private:

	BYTE m_major_version, m_minor_version;
	TCHAR m_ip[IP_ADDRESS_LEN];
	WSADATA m_wsadata;

	SOCKET m_http_socket;
	SOCKADDR_IN m_sockaddr;
	
	string m_send_http, m_recv_http;

public:
	HTTP_REQUEST(bool &p_flag);
	~HTTP_REQUEST();

	void AssemblePacket(string p_path);
	void DisassemblePacket();

	bool SendPost();
	bool RecvPost();
	void Disconnect();
};

#endif