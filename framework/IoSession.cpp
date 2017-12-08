#include "IoSession.hpp"


void IoSession::handle_read_some2( const boost::system::error_code& ec, size_t bytes_transferred )
{
	if(ec)
	{
		if(ec != boost::asio::error::operation_aborted)
		{
			ZLOG_WARN("socket recv fail sid(%d) system msg: %s", sid(), ec.message().c_str());
			close();
			return;
		}
	}
	if(bytes_transferred == 0)
	{
		ZLOG_FATAL("0000000000000000000000000000000000000000000000000000000000000");
		return;
	}

	unDoneDataSize += bytes_transferred;
	int dealDataSize = mStreamHandler->AppendData(mWorkBuffer, unDoneDataSize);
	if(dealDataSize < 0)
	{
		ZLOG_WARN("invalid stream handler return(%d)", dealDataSize);
		OnSocketOpreateFail();
		close();
		return;
	}
	else if(dealDataSize == 0)
	{
		ZLOG_FATAL("dealDataSize = 0, exists error");
		OnSocketOpreateFail();
		close();
		return;
	}
	else
	{
		memcpy(mWorkBuffer, mWorkBuffer + dealDataSize, unDoneDataSize - dealDataSize);
		unDoneDataSize -= dealDataSize;
	}

	this->_socket.async_read_some(boost::asio::buffer(mWorkBuffer + unDoneDataSize, WORK_BUFFER_SIZE - unDoneDataSize),
		boost::bind(&IoSession::handle_read_some2, this, _1, _2));
}

void IoSession::StartGreedyRecv()
{
	is_using(true);
	unDoneDataSize = 0;
	mStreamHandler->clear();
	this->_socket.async_read_some(boost::asio::buffer(mWorkBuffer + unDoneDataSize, WORK_BUFFER_SIZE - unDoneDataSize),
		boost::bind(&IoSession::handle_read_some2, this, _1, _2));
}

void IoSession::Shutdown()
{
	boost::system::error_code ec;
	this->_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_receive, ec);
	if(ec)
	{
		// ZLOG_FATAL("socket id(%d) shutdown receive fail(%s) errorCode(%d)",
		// 	sid(), ec.message().c_str(), ec.value());
	}

}