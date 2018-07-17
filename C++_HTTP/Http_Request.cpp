#include "Precompile.h"
#include "Http_Request.h"


#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
using namespace rapidjson;

#include <string.h>

HTTP_REQUEST::HTTP_REQUEST(bool &p_flag)
{
	int check, timeout_value = 2000;
	u_long flag = 1; // 1 �̸� �ͺ��, 0�̸� ���
	timeval tv = { 0,200 };

	WSAStartup(MAKEWORD(2, 2), &m_wsadata);
	m_http_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	ioctlsocket(m_http_socket, FIONBIO, &flag);

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

	fd_set wset, eset;

	wset.fd_count = 0;
	eset.fd_count = 0;

	check = connect(m_http_socket, (SOCKADDR*)&m_sockaddr, sizeof(m_sockaddr));
	if (check == SOCKET_ERROR)
	{
		int error = WSAGetLastError();
		if (error != WSAEWOULDBLOCK)
		{
			// log
			p_flag = false;
			_tprintf(_TEXT("connect error : %d\n"), WSAGetLastError());
			return;
		}
	}

	FD_SET(m_http_socket, &wset);
	FD_SET(m_http_socket, &eset);

	check = select(0, NULL, &wset, &eset, &tv);
	if (check == SOCKET_ERROR)
	{
		// log
		return;
	}

	if (eset.fd_count == 1)
	{
		// log -> �������
		return;
	}

	flag = !flag;
	ioctlsocket(m_http_socket, FIONBIO, &flag);
	m_major_version = 1; 	m_minor_version = 1;

	p_flag = true;
}

HTTP_REQUEST::~HTTP_REQUEST()
{
	closesocket(m_http_socket);
	WSACleanup();
}

// php ������ �����ϴ� ��θ� �Է¹���
void HTTP_REQUEST::AssemblePacket(string p_path)
{
	int accountNo = 0, contenst_length = 9; // \r\n ���� ������ ����
	StringBuffer stringJSON;
	Writer< StringBuffer, UTF16<> > writer(stringJSON);

	/*
		�ʿ��� ������ json �������� ����
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

	//	http ��� ����
	wstring via_value(SERVER_IP); // wchar ������ string Ÿ������ ��ȯ�ϴ� ���
	string t_http_header(via_value.begin(), via_value.end()), ip_value(via_value.begin(), via_value.end());

	t_http_header = "POST http://" + t_http_header + "/" + p_path + " HTTP/1.1\r\n";
	t_http_header += "Host: " + ip_value + "\r\n";
	t_http_header += "Content-Length: "+ to_string(contenst_length) + "\r\n";
	t_http_header += "Connection:Close\r\n\r\n";
	t_http_header += to_string(m_major_version) + "\r\n";
	t_http_header += to_string(m_minor_version) + "\r\n";
	t_http_header += to_string(accountNo) + "\r\n";
	t_http_header += body;

	m_send_http = t_http_header;
}

// ���Ŀ� ������ �۾��� ���Ե� �� ������
void HTTP_REQUEST::DisassemblePacket()
{
	//m_recv_http.find;

	

	/*	string 
		char* buffer = new char(m_recvQ->GetUseSize());
		TCHAR* \str = new TCHAR( m_recvQ->GetUseSize());

		memcpy_s(buffer, m_recvQ->GetUseSize(), m_recvQ->GetRearPtr(), m_recvQ->GetUseSize());
		MultiByteToWideChar(CP_UTF8, 0, buffer, strlen(buffer), str, m_recvQ->GetUseSize());
	*/
}

bool HTTP_REQUEST::SendPost()
{
	int check = send(m_http_socket, (char*)m_send_http.c_str(), m_send_http.size() , 0);
	if (check == SOCKET_ERROR)
	{
		// log -> wsagetlasterror
		Disconnect();
		return false;
	}

	return true;
}

bool HTTP_REQUEST::RecvPost()
{
	char utf8_buffer[1000];
	TCHAR utf16_buffer[1000];

	ZeroMemory(utf8_buffer, 1000);	ZeroMemory(utf16_buffer, 1000);

	while (1)
	{
		int check = recv(m_http_socket, utf8_buffer, 1000, 0);
		if (check == SOCKET_ERROR)
		{
			int a = WSAGetLastError();
			// log -> wsagetlasterror
			Disconnect();
			return false;
		}
		else if (check == 0)
			break;
	}
	
	MultiByteToWideChar(CP_UTF8, 0, utf8_buffer, strlen(utf8_buffer), utf16_buffer, strlen(utf8_buffer));

	wstring via16_buffer(utf16_buffer);
	string via8_buffer(via16_buffer.begin(), via16_buffer.end());

	m_recv_http = via8_buffer;
	return true;
}

// ������ �߻��� ��쿡�� ȣ���ϴ� �Լ�
void HTTP_REQUEST::Disconnect()
{
	closesocket(m_http_socket);
}