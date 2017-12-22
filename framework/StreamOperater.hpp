#pragma once
#include "Message.hpp"

enum ErrCode{
	LACK_OF_DATA = 1,
	SUCCESS = 0,

	INVALID_PACK_SIZE = -1,
	INVALID_MAGIC_NUM = -2,
	INVALID_CONNECT_TYPE = -3
};

typedef struct _tagFirHead
{
	 unsigned int packSize;  	//代表整个包的长度,如果有packSecHead包，包含packSecHead的长度
	 unsigned char magicNum;	//810代表有packSecHead包的部分
	 unsigned short para;		//客户端协议编号
} *packFirHeadPtr, packFirHead;

#define FIRST_HEADER_SIZE (sizeof(struct _tagFirHead))
#define SECOND_HEADER_SIZE 0
class IoReaderHelper
{
public:
	IoReaderHelper():mSock(_sock_ptr)
	{clear();}
	~IoReaderHelper(){}
	int RecvGetdata(char* iWorkBuffer, size_t size,size_t &decodeSize);
private:
	int appendData(char* iWorkBuffer, size_t size);
	int clear(){
		mHearderDecoded = false;
		mSecHeaderDecoded = false;
		mBodyDecoded = false;
		memset(&pHeader,0,sizeof(pHeader));
		mMessage->reset();
	}
	bool mHearderDecoded;
	bool mSecHeaderDecoded;
	bool mBodyDecoded;

	packFirHead pHeader;
	SimpleMessagePtr mMessage;

	boost::weak_ptr<IOSession> mSock;
};


class IoWriterHelper
{
public:
	IoWriterHelper(ZtBaseSocketPtr _sock_ptr):mSock(_sock_ptr),nUndoSpeacialSize(0)
	{}
	~IoWriterHelper(){}

	int GetData(char* iWorkBuffer, size_t size);
	void submitMsg(const SimpleMessagePtr& msg);	
private:
	int MakeData(const SimpleMessagePtr& _messageptr,char* data,size_t size);
	boost::weak_ptr<IOSession> mSock;

	std::list<SimpleMessagePtr> mData;
	SpeacialMessage mSpeaiclMessage;

	boost::mutex mMutex;
};