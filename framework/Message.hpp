#pragma once

struct Message
{
private:
	unsigned short para; //和客户端通讯的协议号
	int indiSeid; //客户端在gamegate中的sessionid
	std::vector<char> content;
};

#define std::share_ptr<Message> SimpleMessagePtr