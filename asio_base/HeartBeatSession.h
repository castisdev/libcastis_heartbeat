#pragma once

#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/system/error_code.hpp>

class HeartBeatSession
{
public:
	HeartBeatSession(){}
	HeartBeatSession(boost::asio::io_service& io_service, const std::string& representativeIP, const std::string& localIP, unsigned short hbPort);
	virtual ~HeartBeatSession(void);
	
	enum CiHBMessageType_t {
		CIHB_HEARTBEAT_REQUEST,
		CIHB_HEARTBEAT_RESPONSE
	};

	enum CiHBState_t
	{
		CIHB_STATE_ALIVE,
		CIHB_STATE_SW_ERROR,
		CIHB_STATE_HW_ERROR
	};

	typedef boost::shared_ptr<boost::asio::streambuf> streambuf_ptr;
	
	CiHBState_t GetProcessState();
	
	void SetProcessHWError();
	void SetProcessSWError();
	void SetProcessAlive();
	void start() { do_read_request(); }
	void clear();

protected:
	void SetProcessState(CiHBState_t state);

	void do_read_request();
	void handle_read_request(streambuf_ptr request, const boost::system::error_code& err, size_t bytes_recvd);
	void do_write_response(int seq);
	std::string remote_address();
	virtual void handle_error(int errorCode, const std::string& errorStr);

protected:
	boost::shared_ptr<boost::asio::ip::udp::socket> socket_;
	std::string representativeIP_;
	std::string localIP_;
	unsigned short hbPort_;
	CiHBState_t	processState_;
	boost::asio::ip::udp::endpoint sender_endpoint_;
};
