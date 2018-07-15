#include "Precompile.h"
#include "Http_Request.h"
#include "RingBuffer.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
using namespace rapidjson;

#include <string.h>

HTTP_REQUEST::HTTP_REQUEST(bool &p_flag)
{
	int check, timeout_value = 200;
	WSAStartup(MAKEWORD(2, 2), &m_wsadata);
	m_http_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	check = setsockopt(m_http_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout_value, sizeof(timeout_value));
	if (check == SOCKET_ERROR)
	{
		// log
		p_flag = false;
		return;
	}

	check = setsockopt(m_http_socket, SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeout_value, sizeof(timeout_value));
	if (check == SOCKET_ERROR)
	{
		// log
		p_flag = false;
		return;
	}

	int len = sizeof(m_sockaddr);
	_tcscpy_s(m_ip, IP_ADDRESS_LEN, SERVER_IP);

	ZeroMemory(&m_sockaddr, sizeof(m_sockaddr));
	m_sockaddr.sin_family = AF_INET;
	WSAStringToAddress(m_ip, AF_INET, NULL, (SOCKADDR*)&m_sockaddr, &len);
	WSAHtons(m_http_socket, PORT, &m_sockaddr.sin_port);

	check = connect(m_http_socket, (SOCKADDR*)&m_sockaddr, sizeof(m_sockaddr));
	if (check == SOCKET_ERROR)
	{
		// log
		p_flag = false;
		_tprintf(_TEXT("connect error : %d\n"), WSAGetLastError());
		return;
	}
	
	m_sendQ = new RINGBUFFER;	m_recvQ = new RINGBUFFER;
	m_major_version = 1; 	m_minor_version = 1;

	p_flag = true;
}

HTTP_REQUEST::~HTTP_REQUEST()
{
	closesocket(m_http_socket);
	WSACleanup();
}

// php 파일이 존재하는 경로를 입력받음
void HTTP_REQUEST::MakePacket(string p_path)
{
	int accountNo = 0, contenst_length = 9; // \r\n 까지 포함한 길이
	StringBuffer stringJSON;
	Writer< StringBuffer, UTF16<> > writer(stringJSON);

	/*
		필요한 정보를 json 형식으로 제작
	*/
	writer.StartObject();

	writer.String(_TEXT("id"));
	writer.String(_TEXT("zzzz"));

	writer.String(_TEXT("pass"));
	writer.String(_TEXT("eee"));

	writer.String(_TEXT("nickname"));
	writer.String(_TEXT("ddd"));

	writer.EndObject();

	string body(stringJSON.GetString());
	contenst_length += stringJSON.GetLength();

	/*
		http 헤더 제작
	*/
	wstring via_value(SERVER_IP); // wchar 변수를 string 타입으로 변환하는 방법
	string t_http_header(via_value.begin(), via_value.end()), ip_value(via_value.begin(), via_value.end());

	t_http_header = "POST http://" + t_http_header + "/" + p_path + " HTTP/1.1\r\n";
	t_http_header += "Host: " + ip_value + "\r\n";
	t_http_header += "Content-Length: "+ to_string(contenst_length) + "\r\n";
	t_http_header += "Connection:Close\r\n\r\n";
	t_http_header += to_string(m_major_version) + "\r\n";
	t_http_header += to_string(m_minor_version) + "\r\n";
	t_http_header += to_string(accountNo) + "\r\n";
	t_http_header += body;

	m_sendQ->Enqueue((char*)t_http_header.c_str(), t_http_header.size());
}


bool HTTP_REQUEST::SendPost()
{
	int check = send(m_http_socket, m_sendQ->GetFrontPtr(), m_sendQ->LinearRemainFrontSize(), 0);
	if (check == SOCKET_ERROR)
	{
		// log -> wsagetlasterror
		Disconnect();
		return false;
	}

	m_sendQ->MoveFront(check);
	return true;
}

bool HTTP_REQUEST::RecvPost()
{
	while (1)
	{
		int check = recv(m_http_socket, m_recvQ->GetRearPtr(), m_recvQ->LinearRemainRearSize(), 0);
		if (check == SOCKET_ERROR)
		{
			// log -> wsagetlasterror
			Disconnect();
			return false;
		}
		else if (check == 0)
			break;
	}
	
	return true;
}

// 에러가 발생한 경우에만 호출하는 함수
void HTTP_REQUEST::Disconnect()
{
	closesocket(m_http_socket);
}