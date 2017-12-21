#pragma once

struct Message
{
public:
	void SetPara(int iPara){ para = iPara;}
	int GetPara(){ return para;}
	void SetSeid(int iSeid){ para = iSeid;}
	int GetSeid(){ return para;}
	const std::vector<char>& GetContent(){ return content;}
	void SetContent(const std::vector<char>& iContent);
	void SetContent(const char* iContent);
private:
	unsigned short para; //和客户端通讯的协议号
	int indiSeid; //客户端在gamegate中的sessionid
	std::vector<char> content;
};

#define std::share_ptr<Message> SimpleMessagePtr