#pragma once

class BaseSession
{
public:
	BaseSession(boost::asio::io_service& io_service, int sid)
	: _sid(0), m_sock(io_service), is_using(false)
	{
	}
	virtual ~BaseSession(){close();}
public:
	virtual void close()
	{
		if(is_using)
		{
			boost::system::error_code ignored_ec;
			m_sock.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
			m_sock.close(ignored_ec);
			is_using(false);
		}
	}
	virtual bool is_using(){return is_using;}
	virtual void set_using(bool _using){is_using = _using;}
	virtual int session_id(){return _sid;}
private:
	//网络部分
	boost::asio::ip::tcp::socket m_sock;
	int _sid;
	bool is_using;
};