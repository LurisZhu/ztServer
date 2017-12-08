#pragme once
#include <boost/thread.hpp>
#include <boost/asio.hpp>
#include "IoSession.hpp"
#include "config.hpp"
class ztNetBase 
{
public:
	ztNetBase(){}
	~ztNetBase(){}
	ztNetBase& GetInstance() = 0;
//---初始化模块
//---启动模块
	int RunIoService();
	int NetworkInit();
	virtual int ServerHandshaking() = 0;
//---运行模块
	virtual int RunInitilization() = 0;//模块的初始化

	void HandleAcceptSocket( ZtBaseSocketPtr sock, const boost::system::error_code &ec);
	void NetworkRunEvent();
//---关闭模块


	void Main();
	
private:
	virtual void Initilization();
	virtual void Launch();
	virtual void Run();
	virtual void ShutDown();
	boost::thread* mNetThreadPtr;	//网络线程
	boost::asio::io_service*  mIoServicePtr; //网络io
	boost::asio::io_service::work *mIoServiceWorkPtr; // io_service::work
	tcp::acceptor* mAcceptorPtr;
 
/*----------常规处理函数----------*/
public:
	void SingleKill(){ mSignalKill = true;}
	bool GetSingleKill(){ return mSignalKill;}

/*----------普通成员----------*/
private:
	ServerConfig mConfig;
	bool mSignalKill = false;
	std::list<ZtBaseSocketPtr> mSocketPool;
};

#define ztNetBase::GetInstance() sNetServerInstance 