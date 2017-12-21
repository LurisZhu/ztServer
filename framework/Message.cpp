#include "Message.hpp"
#include <cstring>

void Message::SetContent(const std::vector<char>& iContent)
{
	if(content.size() < iContent.size())
		content.resize(iContent.size());
	memcpy(content[0],iContent[0],iContent.size());
}
void Message::SetContent(const char* iContent, size_t size)
{
	if(content.size() < size)
		content.resize(size);
	memcpy(content[0],iContent,size);
}
