#include "NetBase.h"

/*
* 设置新号处理函数
*/
static void ctrlcHandler(int signum)
{
	switch(signum)
	{
	case SIGINT:
		std::cout << "SIGINT" << std::endl;
		GetZtBaseServerInstance().SignalKill();
		break;
	case SIGTERM:
		std::cout << "SIGTERM" << std::endl;
		GetZtBaseServerInstance().SignalKill();
		break;
	default:
		;
	}
}

void ztNetBase::Initilization()
{
	mIoServicePtr = new boost::asio::io_service();
	mNetThreadPtr = new boost::thread();
/*
* 由于io_service并不会主动常见调度线程，需要我们手动分配，常见的方式是给其分配一个线程，然后执行run函数。但run函数在io事件完成后会退出，
* 线程会终止，后续基于该对象的异步io任务无法得到调度。
* 解决这个问题的方法是通过一个asio::io_service::work对象来守护io_service。这样，即使所有io任务都执行完成，也不会退出，继续等待新的io任务。
*/
	mIoServiceWorkPtr = new boost::asio::io_service::work(*mIoServicePtr);

	//加载启动配置
	{
		mConfig.mSocketPoolSize = 1024;

	}

	//创建socket池
	for(int i = 0; i < mConfig.mSocketPoolSize; ++ i)
	{
		ZtBaseSocketPtr newSocket = CreateSocketPtr(*mIoServicePtr, *mJobQueuePtr, i);//队列引用,所有io读写共享队列
		mSocketPool.push_back(newSocket);
	}
	mAcceptorPtr = NULL;
}
//启动模块
//---启动网络线程，处理网络io--
void ztNetBase::NetworkInit()
{
	mNetThreadPtr = new boost::thread(boost::bind(&ztNetBase::RunIoService,this));
}

void ztNetBase::RunIoService()
{
	boost::system::error_code ec;
	mIoServicePtr->run(ec);
	if(ec)
	{
		printf("%s\n", ec.message().c_str());
	}
}

void ztNetBase::Launch()
{
	NetworkInit();
	ServerHandshaking();
}

void ztNetBase::HandleAcceptSocket( ZtBaseSocketPtr sock, const boost::system::error_code &ec)
{
	if(mSignalKill)
	{
		// 关闭状态不再新增socket
		sock->close();
	}
	int sockId = sock->sid();
	if(ec)
	{
		ZLOG_FATAL("handle accept on socket id(%d) fail(%s)", sockId, ec.message().c_str());
		return;
	}
	sock->is_using(true);

	int i = sockId + 1;
	for(; i < int(mSocketPool.size()); ++ i)
	{
		if(!mSocketPool[i]->is_using())
		{
			break;
		}
	}
	if(i == int(mSocketPool.size()))
	{
		for(i = 0; i < sockId; ++ i)
		{
			if(!mSocketPool[i]->is_using())
			{
				break;
			}
		}
	}
	if(i == sockId)
	{
		sock->close();
	}
	else
	{
		sock->StartGreedyRecv();
	}

	
	ZtBaseSocketPtr tmpSocket = mSocketPool[i];

	mAcceptorPtr->async_accept(tmpSocket->socket(),
		boost::bind(&ZtBaseServer::HandleAcceptSocket, this, tmpSocket, boost::asio::placeholders::error));
}

void ztNetBase::NetworkRunEvent()
{
	assert(!mSocketPool.empty());
	ZtBaseSocketPtr tmpSocket = mSocketPool[0];
	mAcceptorPtr = new tcp::acceptor(*mIoServicePtr, tcp::endpoint(tcp::v4(), mConfig.mListenPort));
	mAcceptorPtr->async_accept(tmpSocket->socket(),
		boost::bind(&ztNetBase::HandleAcceptSocket, this, tmpSocket, boost::asio::placeholders::error));
}



void ztNetBase::Run()
{
	NetworkRunEvent();
	if(single(SIGINT,ctrlcHandler) == SIG_ERR || single(SIGTERM,ctrlcHandler) == SIG_ERR)
	{
		std::cout << "新号设置失败" << std::endl;
		exit(1);
	}
}
void ztNetBase::ShutDown()
{
	boost::system::error_code ec;
	mAcceptorPtr->close(ec);
	if(ec)
	{
		ZLOG_ERROR("accpet cancel fail(%d)", ec.message().c_str());
	}
	while(mSocketPool.size() > 0)
	{
		ZtBaseSocketPtr tmpSocket = mSocketPool[mSocketPool.size()-1];
		tmpSocket->ShutDown();
		mSocketPool.pop_back();
	}

}

void ztNetBase::Main()
{
	Initilization();
	Launch();
	Run();
	ShutDown();
}