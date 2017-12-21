#pragma once
#include "Message.hpp"
class IoReaderHelper
{
public:
	IoReaderHelper();
	~IoReaderHelper();
	int GetData(char* iWorkBuffer, size_t size);

private:
	bool mHearderDecoded;
	bool mSecHeaderDecoded;
	bool mBodyDecoded;
	Message mMessage;
};


class IoWriterHelper
{
public:
	IoWriterHelper();
	~IoWriterHelper();
	int GetData(char* iWorkBuffer, size_t size);
	
};