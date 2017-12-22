#pragma once

#define WORK_BUFFER_SIZE 102400

class IOSession :public BaseSession
{
public:
	typedef SimpleMessagePtr MessagePtr;
	IOSession(boost::asio::io_service& io_service, int sid)
	: _sid(0), m_sock(io_service), is_using(false)
	{
	}
	virtual ~IOSession(){close();}

	virtual void submitjob(const MessagePtr& umessage);
	void SubmitWriteJob(const MessagePtr& message);

	virtual void write(const MessagePtr& message);
protected:
	virtual void StartGreedyRecv();
	virtual void handle_read_some2(const boost::system::error_code& error, size_t bytes_transferred);

	virtual void handle_write2(const boost::system::error_code& error, size_t bytes_transferred);

	virtual void Shutdown();
private:
	//协议部分
	char mWorkBuffer[WORK_BUFFER_SIZE];
	size_t unDoneDataSize/* = 0*/;

	char mWriteBuffer[WORK_BUFFER_SIZE];
	size_t mToWriteSize/* = 0*/;

	jobqueue<MessagePtr> &_ursjobqueue;

	IoReaderHelper mIoReaderHelper;
	IoWriterHelper mIoWriterHelper;
};

typedef boost::share_ptr<IOSession> ZtBaseSocketPtr;