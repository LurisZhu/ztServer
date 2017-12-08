#include "LogFile.hpp"
#include <assert.h>
#include <stdio.h>
#include <time.h>
#include <sys/stat.h>

#define  MAX_FILE_SUFFIX 1000

// not thread safe
class LogFile::File : boost::noncopyable
{
 public:
  explicit File(const string& filename)
    : fp_(::fopen(filename.data(), "a")),
      writtenBytes_(0)
  {
    assert(fp_);
    ::setbuffer(fp_, buffer_, sizeof buffer_);

	//计算当前日志文件大小
	struct stat curr_stat = { 0 };
	int32_t ret = stat(filename.c_str(), &curr_stat);
	if (0 != ret )
	{
		return;
	}
	writtenBytes_ = curr_stat.st_size;
    // posix_fadvise POSIX_FADV_DONTNEED ?
  }

  ~File()
  {
    ::fclose(fp_);
  }

  void append(const char* logline, const size_t len)
  {
    size_t n = write(logline, len);
    size_t remain = len - n;
    while (remain > 0)
    {
      size_t x = write(logline + n, remain);
      if (x == 0)
      {
        int err = ferror(fp_);
        if (err)
        {
          fprintf(stderr, "LogFile::File::append() failed %d\n", (err));
        }
        break;
      }
      n += x;
      remain = len - n; // remain -= x
    }

    writtenBytes_ += len;
  }

  void flush()
  {
    ::fflush(fp_);
  }

  size_t writtenBytes() const { return writtenBytes_; }

 private:

  size_t write(const char* logline, size_t len)
  {
#undef fwrite_unlocked
    return ::fwrite_unlocked(logline, 1, len, fp_);
  }

  FILE* fp_;
  char buffer_[64*1024];
  size_t writtenBytes_;
};

LogFile::LogFile(const string& basename,
                 size_t rollSize,
                 bool threadSafe,
                 int kCheckTimeRoll,
                 int flushInterval)
  : basename_(basename),
    rollSize_(rollSize),
    flushInterval_(flushInterval),
    count_(0),
    mutex_(threadSafe ? new boost::mutex : NULL),
    startOfPeriod_(0),
    lastRoll_(0),
    lastFlush_(0),
    kCheckTimeRoll_(kCheckTimeRoll)
{
  //assert(basename.find('/') == string::npos);
  //计算当前时区
  time_t t_time0 = 0;
  struct tm tm_time;
  ::localtime_r(&t_time0, &tm_time); 
  timezone = tm_time.tm_mday == 1?(tm_time.tm_hour):(tm_time.tm_hour - 23); //计算时区
  
  rollFile();
}

LogFile::~LogFile()
{
	std::cout << " delete logFile " << std::endl;
}

void LogFile::append(const char* logline, int len)
{
  if (mutex_)
  {
    boost::mutex::scoped_lock lock(*mutex_);
    append_unlocked(logline, len);
  }
  else
  {
    append_unlocked(logline, len);
  }
}

void LogFile::flush()
{
  if (mutex_)
  {
    boost::mutex::scoped_lock lock(*mutex_);
    file_->flush();
  }
  else
  {
    file_->flush();
  }
}

void LogFile::append_unlocked(const char* logline, int len)
{
  file_->append(logline, len);

  if (file_->writtenBytes() > rollSize_)
  {
    rollFile();
  }
  else
  {
    if (count_ > kCheckTimeRoll_)
    {
      count_ = 0;
      time_t now = ::time(NULL) + timezone*60*60;
      time_t thisPeriod_ = now / kRollPerSeconds_ * kRollPerSeconds_;
      if (thisPeriod_ != startOfPeriod_)
      {
        rollFile();
      }
      else if (now - lastFlush_ > flushInterval_)
      {
        lastFlush_ = now;
        file_->flush();
      }
    }
    else
    {
      ++count_;
    }
  }
}

void LogFile::rollFile()
{
  time_t now = 0;
  string filename = getLogFileName(basename_, &now);
  //修改文件名
  renameLogFile(filename.c_str());

  now += timezone*60*60;
  time_t start = now / kRollPerSeconds_ * kRollPerSeconds_;

  if (now > lastRoll_)
  {
    lastRoll_ = now;
    lastFlush_ = now;
    startOfPeriod_ = start;
    file_.reset(new File(filename));
  }
}

 void LogFile::renameLogFile(const char* file_name)
 {
	 char new_name[1024] = { 0 };

	 //获取合适的文件扩展名
	 int32_t num = 1;
	 for (num = 1; num < MAX_FILE_SUFFIX; ++num)
	 {
		 sprintf(new_name, "%s.%d", file_name, num);
		 if (access(new_name, F_OK) < 0)
		 {
			 if(errno == ENOENT)
			 {
				 break;
			 }
		 }
	 }

	 //日志文件数量达到上线
	 if (num == MAX_FILE_SUFFIX)
	 {
		 return;
	 }
	 //更改当前文件名
	 std::cout << "rename " << file_name << " to " << new_name <<std::endl;
	 rename(file_name, new_name);
 }


string LogFile::getLogFileName(const string& basename, time_t* now)
{
  string filename;
  filename.reserve(basename.size() + 64);
  filename = basename;

  char timebuf[32];
  //char pidbuf[32];
  struct tm tm;
  *now = time(NULL);
  localtime_r(now, &tm); // FIXME: localtime_r ?
  strftime(timebuf, sizeof timebuf, "_%Y-%m-%d", &tm);
  filename += timebuf;
  //filename += ProcessInfo::hostname();
  //snprintf(pidbuf, sizeof pidbuf, ".%d", ProcessInfo::pid());
  //filename += pidbuf;
  filename += ".log";

  return filename;
}


class LogFileGiant::RecordFile : boost::noncopyable
{
 public:
  explicit RecordFile(const string& filename)
    : fp_(::fopen(filename.data(), "a")),
      writtenBytes_(0)
  {
    assert(fp_);
    ::setbuffer(fp_, buffer_, sizeof buffer_);

	//计算当前日志文件大小
	struct stat curr_stat = { 0 };
	int32_t ret = stat(filename.c_str(), &curr_stat);
	if (0 != ret )
	{
		return;
	}
	writtenBytes_ = curr_stat.st_size;
    // posix_fadvise POSIX_FADV_DONTNEED ?
  }

  ~RecordFile()
  {
    ::fclose(fp_);
  }

  void append(const char* logline, const size_t len)
  {
    size_t n = write(logline, len);
    size_t remain = len - n;
    while (remain > 0)
    {
      size_t x = write(logline + n, remain);
      if (x == 0)
      {
        int err = ferror(fp_);
        if (err)
        {
          fprintf(stderr, "LogFile::File::append() failed %d\n", (err));
        }
        break;
      }
      n += x;
      remain = len - n; // remain -= x
    }

    writtenBytes_ += len;
  }

  void flush()
  {
    ::fflush(fp_);
  }

  size_t writtenBytes() const { return writtenBytes_; }

 private:

  size_t write(const char* logline, size_t len)
  {
#undef fwrite_unlocked
    return ::fwrite_unlocked(logline, 1, len, fp_);
  }

  FILE* fp_;
  char buffer_[64*1024];
  size_t writtenBytes_;
};


LogFileGiant::LogFileGiant(const string& basename,
                 size_t rollSize,
                 bool threadSafe,
                 int kCheckTimeRoll,
                 int flushInterval)
  : basename_(basename),
    rollSize_(rollSize),
    flushInterval_(flushInterval),
    count_(0),
    mutex_(threadSafe ? new boost::mutex : NULL),
    startOfPeriod_(0),
    lastRoll_(0),
    lastFlush_(0),
    kCheckTimeRoll_(kCheckTimeRoll)
{
  //assert(basename.find('/') == string::npos);
  //计算当前时区
  time_t t_time0 = 0;
  struct tm tm_time;
  ::localtime_r(&t_time0, &tm_time); 
  timezone = tm_time.tm_mday == 1?(tm_time.tm_hour):(tm_time.tm_hour - 23); //计算时区
  
  rollFile();
}

LogFileGiant::~LogFileGiant()
{
	std::cout << " delete LogFileGiant " << std::endl;
}

void LogFileGiant::append(const char* logline, int len)
{
  if (mutex_)
  {
    boost::mutex::scoped_lock lock(*mutex_);
    append_unlocked(logline, len);
  }
  else
  {
    append_unlocked(logline, len);
  }
}

void LogFileGiant::flush()
{
  if (mutex_)
  {
    boost::mutex::scoped_lock lock(*mutex_);
    file_->flush();
  }
  else
  {
    file_->flush();
  }
}

void LogFileGiant::append_unlocked(const char* logline, int len)
{
  file_->append(logline, len);
  if (file_->writtenBytes() > rollSize_)
  {
    rollFile();
  }
  else
  {
    //if (count_ > kCheckTimeRoll_)
    //{
      count_ = 0;
      time_t now = ::time(NULL) + timezone*60*60;
      time_t thisPeriod_ = now / kRollPerSeconds_ * kRollPerSeconds_;
      if (thisPeriod_ != startOfPeriod_)
      {
        rollFile();
      }
      else if (now - lastFlush_ > flushInterval_)
      {
        lastFlush_ = now;
        file_->flush();
      }
    //}
    //else
    //{
    //  ++count_;
    //}
  }
}

void LogFileGiant::rollFile()
{
  time_t now = 0;
  string filename = getLogFileName(basename_, &now);
  //修改文件名
  renameLogFile(filename.c_str());

  now += timezone*60*60;
  time_t start = now / kRollPerSeconds_ * kRollPerSeconds_;
  if (now > lastRoll_)
  {
    lastRoll_ = now;
    lastFlush_ = now;
    startOfPeriod_ = start;
    file_.reset(new RecordFile(filename));
  }
}

 void LogFileGiant::renameLogFile(const char* file_name)
 {
	 char new_name[1024] = { 0 };

	 //获取合适的文件扩展名
	 int32_t num = 1;
	 for (num = 1; num < MAX_FILE_SUFFIX; ++num)
	 {
		 sprintf(new_name, "%s.%d", file_name, num);
		 if (access(new_name, F_OK) < 0)
		 {
			 if(errno == ENOENT)
			 {
				 break;
			 }
		 }
	 }

	 //日志文件数量达到上线
	 if (num == MAX_FILE_SUFFIX)
	 {
		 return;
	 }
	 //更改当前文件名
	 std::cout << "rename " << file_name << " to " << new_name <<std::endl;
	 rename(file_name, new_name);
 }


string LogFileGiant::getLogFileName(const string& basename, time_t* now)
{
  string filename;
  filename.reserve(basename.size() + 64);
  filename = basename;

  char timebuf[32];
  //char pidbuf[32];
  struct tm tm;
  *now = time(NULL);
  localtime_r(now, &tm); // FIXME: localtime_r ?
  strftime(timebuf, sizeof timebuf, "_%Y-%m-%d", &tm);
  filename += timebuf;
  //filename += ProcessInfo::hostname();
  //snprintf(pidbuf, sizeof pidbuf, ".%d", ProcessInfo::pid());
  //filename += pidbuf;
  filename += ".log";

  return filename;
}


