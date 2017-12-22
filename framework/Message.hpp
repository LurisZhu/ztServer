#pragma once

struct Message
{
public:
	void SetPara(int iPara){ para = iPara;}
	int GetPara(){ return para;}
	void SetSeid(int iSeid){ para = iSeid;}
	int GetSeid(){ return para;}
	const std::vector<char>& GetContent(){ return content;}
	//用memcpy提速，使用前确保空间充足
	void SetContent(const std::vector<char>& iContent);
	void SetContent(const char* iContent);
	//用memcpy提速，使用前确保空间充足
	void appendContent(const char* iContent, size_t size);

	int toProtoclBuffer()
private:
	unsigned short para; //和客户端通讯的协议号
	int indiSeid; //客户端在gamegate中的sessionid
	std::vector<char> content;
};

struct SpeacialMessage
{
	SpeacialMessage(){}
	~SpeacialMessage(){}
	void clear(){
		mMessagePtr.reset();
		unDoSize = 0;
	}
	void InitMessage(const boost::share_ptr<Message>& _messageptr)
	{
		unDoSize = _messageptr->GetContent().size();
		mMessagePtr = _messageptr;
	}
	int GetUnSize(){
		return mMessagePtr.get()?unDoSize:0;
	}
	int PopData(char* data,size_t size);
	void DecreUnDoSize(size_t size){unDoSize -= size;}
private:
	int unDoSize;
	boost::share_ptr<Message> mMessagePtr;
}


#define std::share_ptr<Message> SimpleMessagePtr