#pragma once

class IoReaderHelper
{
public:
	IoReaderHelper();
	~IoReaderHelper();
	int GetData(char* iWorkBuffer, size_t size);
};


class IoWriterHelper
{
public:
	IoWriterHelper();
	~IoWriterHelper();
	int GetData(char* iWorkBuffer, size_t size);
};