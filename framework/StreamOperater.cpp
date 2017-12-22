#include "StreamOperater.hpp"

int IoReaderHelper::RecvGetdata(const char* iWorkBuffer, size_t size,size_t &decodeSize)
{
	size_t unDoneSize = size;
	if(!mHearderDecoded)
	{
		if(size < size(FIRST_HEADER_SIZE))
			return LACK_OF_DATA;
		memcpy(&pHeader,iWorkBuffer,FIRST_HEADER_SIZE);
		mHearderDecoded = true;
		decodeSize += FIRST_HEADER_SIZE;
		unDoneSize -= FIRST_HEADER_SIZE;
	}
	if(!mSecHeaderDecoded)
	{
		//pass
		if(unDoneSize < (size_t)(SECOND_HEADER_SIZE))
			return LACK_OF_DATA;
		mMessage = SimpleMessagePtr(new SimpleMessage());
		mMessage->set(pHeader.para);
		mMessage->HoldContent().reserve(mPackSize - FIRST_HEADER_SIZE - SECOND_HEADER_SIZE);
		mSecHeaderDecoded = true;
		decodeSize += SECOND_HEADER_SIZE;
		unDoneSize -= SECOND_HEADER_SIZE;
	}
	if(!mBodyDecoded)
	{
		size_t iCurSize = mMessage->GetContent().size();
		if(unDoneSize + iCurSize < (size_t)(pHeader.packSize - FIRST_HEADER_SIZE - SECOND_HEADER_SIZE))
		{
			mMessage->appendContent(iWorkBuffer + decodeSize,unDoneSize);
			decodeSize += unDoneSize;
			unDoneSize = 0;
			return LACK_OF_DATA;
		}
		else
		{
			size_t iNeedSize = pHeader.packSize - FIRST_HEADER_SIZE - SECOND_HEADER_SIZE - iCurSize;
			mMessage->appendContent(iWorkBuffer + decodeSize,iNeedSize);
			decodeSize += iNeedSize;
			unDoneSize -= iNeedSize;
			return SUCCESS;
		}

	}
	return SUCCESS;
}

int IoReaderHelper::appendData(char* iWorkBuffer, size_t size)
{
	int nDecodeSize = 0;
	while(true)
	{
		size_t iDecodeSize = 0;
		int ec = RecvGetdata(iWorkBuffer,size,iDecodeSize);
		switch(ec)
		{
			case LACK_OF_DATA:
				nDecodeSize += iDecodeSize;
				return nDecodeSize;
			case SUCCESS:
				mSock->submitjob(mMessage);
				this->clear();
				nDecodeSize += iDecodeSize;
				break;
			default:
				//Error
				return -1;
		}
	}

}
void IoWriterHelper::submitMsg(const SimpleMessagePtr& msg)
{
	boost::mutex::scoped_lock lock(mMutex);
	mData.push_back(msg);
}

int IoWriterHelper::GetData(char* iWorkBuffer, size_t size)
{
	boost::mutex::scoped_lock lock(mMutex);
	size_t iEncodeSize = 0;
	size_t iRemainSize = size;
	if(mSpeaiclMessage->GetUnSize() > 0)
	{
		iEncodeSize += mSpeaiclMessage->PopData(iWorkBuffer,size);
		iRemainSize -= iEncodeSize;
	}
	//为了逻辑清晰这里增加判断
	if(iRemainSize <= 0)
		return iEncodeSize;
	while(!mData.empty())
	{
		SimpleMessagePtr& ptr = mData.front();
		if(iRemainSize < ptr->GetContent().size())
		{
			if(iEncodeSize > 0)//有数据直接返回了
				return iEncodeSize;
			//超大数据
			mSpeaiclMessage.InitMessage(ptr);
			mData.pop_front();
			size_t iDecodeSize = MakeData(ptr,iRemainSize);
			mSpeaiclMessage.DecreUnDoSize(iDecodeSize);
			iRemainSize -= iDecodeSize;
			iEncodeSize += iDecodeSize;
		}
		else
		{
			size_t iDecodeSize = MakeData(ptr,iRemainSize);
			iRemainSize -= iDecodeSize;
			iEncodeSize += iDecodeSize;
		}

	}
	return iEncodeSize;
}

int IoWriterHelper::MakeData(const SimpleMessagePtr& _messageptr,char* data,size_t size)
{
	size_t iEncodeSize = 0;
	size_t itargetSize = FIRST_HEADER_SIZE + SECOND_HEADER_SIZE + _messageptr->GetContent().size();
	if(size < FIRST_HEADER_SIZE + SECOND_HEADER_SIZE)
	{
		return iEncodeSize;
	}
	_tagFirHead pHeader;
	pHeader.packSize = itargetSize;
	pHeader.magicNum = 0;
	pHeader.para = _messageptr.GetPara();
	memcpy(data,&pHeader,sizeof(pHeader));
	
	if(size - iEncodeSize < _messageptr->GetContent().size())
	{
		memcpy(data+iEncodeSize,_messageptr->GetContent()[0],size - iEncodeSize);
		iEncodeSize = size;
	}
	else
	{
		memcpy(data+iEncodeSize,_messageptr->GetContent()[0],_messageptr->GetContent().size());
		iEncodeSize += _messageptr->GetContent().size();
	}
	return iEncodeSize;
}

