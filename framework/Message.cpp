#include "Message.hpp"
#include <cstring>

void Message::SetContent(const std::vector<char>& iContent)
{
	memcpy(content[0],iContent[0],iContent.size());
}
void Message::SetContent(const char* iContent, size_t size)
{
	memcpy(content[0],iContent,size);
}

void Message::appendContent(const char* iContent, size_t size)
{
	size_t iCurSize = content.size();
	memcpy(content[iCurSize],iContent,size);
}

int SpeacialMessage::PopData(char* data,size_t size)
{
	int iEncodeSize = 0;
	if(unDoSize <= 0)
	{
		return iEncodeSize;
	}
	else if(size >= unDoSize)
	{
		iEncodeSize = unDoSize;
		int iIndex = mMessagePtr->GetContent().size() - unDoSize;
		memcpy(data,mMessagePtr->GetContent()[iIndex],unDoSize);
		this->clear();
		return iEncodeSize;
	}
	else
	{
		iEncodeSize = size;
		int iIndex = mMessagePtr->GetContent().size() - unDoSize;
		memcpy(data,mMessagePtr->GetContent()[iIndex],iEncodeSize);
		unDoSize -= iEncodeSize;
		return iEncodeSize;
	}
	return iEncodeSize;
}