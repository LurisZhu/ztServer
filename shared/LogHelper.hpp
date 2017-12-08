#pragma once


class zLogger 
{
public:
	typedef enum
	{
		LEVEL_OFF   = 2147483647,
		LEVEL_FATAL = 50000,
		LEVEL_ERROR = 40000,
		LEVEL_WARN  = 30000,
		LEVEL_INFO  = 20000,
		LEVEL_DEBUG = 10000,
		LEVEL_ALL   = 0
	}zLevel;

	const char *getLevelString(zLevel level)
	{
		switch(level)
		{
		case LEVEL_OFF:		return "OFF";
		case LEVEL_FATAL:	return "FATAL";
		case LEVEL_ERROR:	return "ERROR";
		case LEVEL_WARN:	return "WARN";
		case LEVEL_DEBUG:	return "DEBUG";
		case LEVEL_INFO:	return "INFO";
		case LEVEL_ALL:		return "ALL";
		default:			return "NO_LEVEL";
		}
	}
	zLogger& Instance(){
		static zLogger _instance;
		return _instance;
	}
}

#define ZLOG_FATAL(format, ...) \
zLogger::Instance().log(zLogger::LEVEL_FATAL, __FILE__, __LINE__, format, ##__VA_ARGS__)

#define ZLOG_ERROR(format, ...) \
zLogger::Instance().log(zLogger::LEVEL_ERROR, __FILE__, __LINE__, format, ##__VA_ARGS__)

#define ZLOG_WARN(format, ...) \
zLogger::Instance().log(zLogger::LEVEL_WARN,  __FILE__, __LINE__, format, ##__VA_ARGS__)

#define ZLOG_DEBUG(format, ...) \
zLogger::Instance().log(zLogger::LEVEL_DEBUG, __FILE__, __LINE__, format, ##__VA_ARGS__)

#define ZLOG_INFO(format, ...) \
zLogger::Instance().log(zLogger::LEVEL_INFO,  __FILE__, __LINE__, format, ##__VA_ARGS__)

#define ZLOG_DEBUG16(info, pData, Datasize) \
zLogger::Instance().log16(zLogger::LEVEL_DEBUG, __FILE__, __LINE__, info, pData, Datasize)

#define ZLOG_ERROR16(info, pData, Datasize) \
zLogger::Instance().log16(zLogger::LEVEL_ERROR, __FILE__, __LINE__, info, pData, Datasize)