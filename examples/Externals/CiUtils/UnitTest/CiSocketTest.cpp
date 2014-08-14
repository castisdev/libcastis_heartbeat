#include "gtest/gtest.h"
#include "CiGlobals.h"
#include "common_CiUtils.h"

class TestableCiSocket : public CCiSocket
{
public:
	TestableCiSocket(int iAcceptSocket,
				CCiSocketType_t socketType = CI_SOCKET_TCP,
				int iSendBufferSize = CI_SOCKET_SEND_BUFFER_SIZE,
				int iRecvBufferSize = CI_SOCKET_RECV_BUFFER_SIZE)
		: CCiSocket(iAcceptSocket, socketType, iSendBufferSize, iRecvBufferSize) {}
	using CCiSocket::WriteNToBuffer;
	using CCiSocket::m_iSendStartPtr;
	using CCiSocket::m_iSendEndPtr;
	using CCiSocket::m_pSendBuffer;
	using CCiSocket::m_iSendBufferSize;
	using CCiSocket::m_iRecvStartPtr;
	using CCiSocket::m_iRecvEndPtr;
	using CCiSocket::m_pRecvBuffer;
	using CCiSocket::m_iRecvBufferSize;
};

class CiSocketTest : public testing::Test
{
protected:
	int _fd_listen_socket;
	int _fd_receive_socket;
	int _fd_send_socket;
	void SetUp()
	{
		if (!nu_create_listen_socket(&_fd_listen_socket, 0, 10))
		{
			FAIL() << "failed to create listen socket";
		}
		unsigned short listen_port;
		if (!nu_get_local_port_number(_fd_listen_socket, &listen_port))
		{
			close(_fd_listen_socket);
			FAIL() << "failed to get port number";
			return;
		}
		_fd_send_socket = socket(AF_INET, SOCK_STREAM, 0);
		linger ling = {1,0};
		setsockopt(_fd_send_socket, SOL_SOCKET, SO_LINGER, (char*)&ling, sizeof(linger));
		struct sockaddr_in server_address;
		memset((void *)&server_address, 0, sizeof(server_address));
		server_address.sin_family = AF_INET;
		server_address.sin_port = htons(listen_port);
#ifdef _WIN32
		server_address.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
#else
		if (inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr) != 1)
		{
			FAIL() << "failed to inet_pton";
			return;
		}
#endif
		if (connect(_fd_send_socket, (const struct sockaddr *)&server_address, sizeof(server_address)) != 0)
		{
			FAIL() << "failed to connect";
			return;
		}
		if (!nu_accept(_fd_listen_socket, &_fd_receive_socket, NULL, NULL))
		{
			FAIL() << "failed to accept";
			return;
		}
	}
	void TearDown()
	{
		close(_fd_listen_socket);
	}
};

TEST_F(CiSocketTest, Realloc)
{
	int buffer_size = 100 * 1024;
	int message_size = buffer_size * 10;  // message is 10 times bigger than socket buffer
	CCiSocket receiver(_fd_receive_socket, CI_SOCKET_TCP, buffer_size, buffer_size);
	CCiSocket sender(_fd_send_socket, CI_SOCKET_TCP, buffer_size, buffer_size);

	int written_bytes = 0;
	unsigned char message[message_size];
	ASSERT_TRUE(sender.WriteN(message, message_size, &written_bytes));
	EXPECT_EQ(message_size, written_bytes);
	ASSERT_TRUE(receiver.ReadFromNetworkToBuffer(message_size - receiver.GetCurrentRecvDataSize()));
}

TEST_F(CiSocketTest, ReallocSendBufferWrapAround)
{
	int buffer_size = 10;
	TestableCiSocket sender(_fd_send_socket, CI_SOCKET_TCP, buffer_size, buffer_size);

	sender.m_iSendStartPtr = 8;
	memset(sender.m_pSendBuffer + sender.m_iSendStartPtr, '1', buffer_size - sender.m_iSendStartPtr);
	sender.m_iSendEndPtr = 6;
	memset(sender.m_pSendBuffer, '1', sender.m_iSendEndPtr);
	// now, sender's buffer look like
	// offset: 0 1 2 3 4 5 6 7 8 9
	// buffer: 1 1 1 1 1 1 ? ? 1 1
	// ptr   :             E   S
	ASSERT_EQ(8, sender.GetRemainSendDataSize());

	int message_size = 4;
	unsigned char message[message_size];
	memset(message, '2', message_size);
	int written_bytes = 0;
	ASSERT_TRUE(sender.WriteNToBuffer(message, message_size, &written_bytes));

	// now, sender's buffer look like
	// offset: 0 1 2 3 4 5 6 7 8 9 0 1 2
	// buffer: 1 1 1 1 1 1 2 2 2 2 ? 1 1
	// ptr   :                     E S
	EXPECT_EQ(message_size, written_bytes);
	EXPECT_EQ(13, sender.m_iSendBufferSize);
	EXPECT_EQ('2', sender.m_pSendBuffer[6]);
	EXPECT_EQ('2', sender.m_pSendBuffer[7]);
	EXPECT_EQ('2', sender.m_pSendBuffer[8]);
	EXPECT_EQ('2', sender.m_pSendBuffer[9]);
	EXPECT_EQ('1', sender.m_pSendBuffer[11]);
	EXPECT_EQ('1', sender.m_pSendBuffer[12]);
	EXPECT_EQ(11, sender.m_iSendStartPtr);
	EXPECT_EQ(10, sender.m_iSendEndPtr);
}

TEST_F(CiSocketTest, ReallocRecvBufferWrapAround)
{
	int buffer_size = 10;
	TestableCiSocket receiver(_fd_receive_socket, CI_SOCKET_TCP, buffer_size, buffer_size);

	receiver.m_iRecvStartPtr = 8;
	memset(receiver.m_pRecvBuffer + receiver.m_iRecvStartPtr, '1', buffer_size - receiver.m_iRecvStartPtr);
	receiver.m_iRecvEndPtr = 6;
	memset(receiver.m_pRecvBuffer, '1', receiver.m_iRecvEndPtr);
	// now, receiver's buffer look like
	// offset: 0 1 2 3 4 5 6 7 8 9
	// buffer: 1 1 1 1 1 1 ? ? 1 1
	// ptr   :             E   S
	ASSERT_EQ(8, receiver.GetCurrentRecvDataSize());

	int message_size = 4;
	unsigned char message[message_size];
	memset(message, '2', message_size);
	int written_bytes = 0;
	CCiSocket sender(_fd_send_socket, CI_SOCKET_TCP, buffer_size, buffer_size);
	ASSERT_TRUE(sender.WriteNWithTimeOut(message, message_size, &written_bytes));
	ASSERT_TRUE(receiver.ReadFromNetworkToBuffer(message_size));

	// now, receiver's buffer look like
	// offset: 0 1 2 3 4 5 6 7 8 9 0 1 2
	// buffer: 1 1 1 1 1 1 2 2 2 2 ? 1 1
	// ptr   :                     E S
	EXPECT_EQ(message_size, written_bytes);
	EXPECT_EQ(13, receiver.m_iRecvBufferSize);
	EXPECT_EQ('2', receiver.m_pRecvBuffer[6]);
	EXPECT_EQ('2', receiver.m_pRecvBuffer[7]);
	EXPECT_EQ('2', receiver.m_pRecvBuffer[8]);
	EXPECT_EQ('2', receiver.m_pRecvBuffer[9]);
	// EXPECT_EQ('0', receiver.m_pRecvBuffer[10]);
	EXPECT_EQ('1', receiver.m_pRecvBuffer[11]);
	EXPECT_EQ('1', receiver.m_pRecvBuffer[12]);
	EXPECT_EQ(11, receiver.m_iRecvStartPtr);
	EXPECT_EQ(10, receiver.m_iRecvEndPtr);
}
