#include "HeartBeatSession.h"

#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>

HeartBeatSession::HeartBeatSession(boost::asio::io_service& io_service
								   , const std::string& representativeIP
								   , const std::string& localIP
								   , unsigned short hbPort)
: socket_(new boost::asio::ip::udp::socket(io_service))
, representativeIP_(representativeIP)
, localIP_(localIP)
, hbPort_(hbPort)
, processState_(CIHB_STATE_ALIVE)
{
}

HeartBeatSession::~HeartBeatSession(void)
{
}

void HeartBeatSession::start() {
  socket_->open(boost::asio::ip::udp::v4());
  socket_->set_option(boost::asio::ip::udp::socket::reuse_address(true));
  socket_->bind(boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(),
                                               hbPort_));

  do_read_request();
}

void HeartBeatSession::clear()
{
	if ( socket_ ) {
	  boost::system::error_code ec;
		socket_->close(ec);
		if (ec)
		  handle_error(ec.value(), ec.message());
	}
	socket_.reset();
}

HeartBeatSession::CiHBState_t HeartBeatSession::GetProcessState()
{
	return processState_;
}

void HeartBeatSession::SetProcessState(CiHBState_t state)
{
	processState_ = state;
}

void HeartBeatSession::SetProcessHWError()
{
	SetProcessState(CIHB_STATE_HW_ERROR);
}

void HeartBeatSession::SetProcessSWError()
{
	SetProcessState(CIHB_STATE_SW_ERROR);
}

void HeartBeatSession::SetProcessAlive()
{
	SetProcessState(CIHB_STATE_ALIVE);
}

std::string HeartBeatSession::remote_address()
{
	try
	{
		return sender_endpoint_.address().to_string();
	}
	catch (std::exception& )
	{
		return "";
	}
}

void HeartBeatSession::do_read_request()
{
	streambuf_ptr request(new boost::asio::streambuf);
	boost::asio::streambuf::mutable_buffers_type buffer = request->prepare(8);
	socket_->async_receive_from(buffer, sender_endpoint_
		, boost::bind(&HeartBeatSession::handle_read_request, this, request, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
}

void HeartBeatSession::handle_read_request(streambuf_ptr request, const boost::system::error_code& err, size_t bytes_recvd)
{
	if ( err )
	{
		std::ostringstream oss;
		oss << err.message();
		oss << " from ";
		oss << remote_address();
		handle_error(err.value(), oss.str());
	}
	else
	{
		request->commit(bytes_recvd);
		std::istream is(request.get());
		boost::int32_t msg = 0, seq = 0;
		is.read(reinterpret_cast<char*>(&msg), sizeof(msg));
		msg = ntohl(msg);
		is.read(reinterpret_cast<char*>(&seq), sizeof(seq));
		seq = ntohl(seq);

		if ( msg != CIHB_HEARTBEAT_REQUEST )
		{
			std::ostringstream oss;
			oss << "invalid msg from ";
			oss << remote_address();
			handle_error(msg, oss.str());
		}
		else
			do_write_response(seq);

		do_read_request();
	}
}

void HeartBeatSession::do_write_response(int seq)
{
	boost::asio::streambuf response;
	std::ostream os(&response);

	boost::int32_t temp = htonl(CIHB_HEARTBEAT_RESPONSE);
	os.write(reinterpret_cast<const char*>(&temp), sizeof(temp));
	temp = htonl(seq);
	os.write(reinterpret_cast<const char*>(&temp), sizeof(temp));
	temp = htonl(representativeIP_.length());
	os.write(reinterpret_cast<const char*>(&temp), sizeof(temp));
	os << representativeIP_;
	temp = htonl(processState_);
	os.write(reinterpret_cast<const char*>(&temp), sizeof(temp));
	temp = htonl(localIP_.length());
	os.write(reinterpret_cast<const char*>(&temp), sizeof(temp));
	os << localIP_;

	boost::system::error_code ec;
	std::size_t n = socket_->send_to(response.data(), sender_endpoint_, 0, ec);

	if ( ec )
	{
		std::ostringstream oss;
		oss << ec.message();
		oss << " from ";
		oss << remote_address();
		handle_error(ec.value(), oss.str());
	}
	else if ( response.size() != n )
	{
		std::ostringstream oss;
		oss << "invalid send size from ";
		oss << remote_address();
		handle_error(n, oss.str());
	}
}

void HeartBeatSession::handle_error(int errorCode, const std::string& errorStr)
{
	std::cerr << "errorCode = " << errorCode << ", errorStr = " << errorStr << "\n";
}
