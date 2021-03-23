#ifndef __MIX_LOG_H__
#define __MIX_LOG_H__

#include<string>
#include<stdint.h>
#include<memory>
#include<list>

namespace mix {

//日志事件
class LogEvent{
public:
	typedef std::shared_ptr<LogEvent> ptr;
	LogEvent();
private:
	const char* m_file = nullptr; //文件名
	int32_t m_line = 0; 	      //行号
	uint32_t m_elapse = 0; 	      //程序启动开始到现在的毫秒数
	uint32_t m_threadId = 0;      //线程id
	uint32_t m_fiberId = 0;       //协程id
	uint64_t m_time = 0; 	      //时间戳
	std::string m_content;
};

//日志级别
class LogLevel {
public:
	enum Level {
		DEBUG = 1,
		INFO = 2, 
		WARN = 3,
		ERROR = 4,
		FATAL = 5
	};
};

//日志格式器
class LogFormatter {
public:
	typedef std::shared_ptr<LogFormatter> ptr;
	
	std::string format(LogEvent::ptr event);
private:
};

//日志输出地
class LogAppender {
public:
	typedef std::shared_ptr<LogApperder> ptr;
	virtual ~LogAppender() {}
	
	void log(LogLevel::Level level, LogEvent::ptr event);
private:
	LogLevel::Level m_level;
};

//日志器
class Logger{
public:
	typedef std::shared_ptr<Logger> ptr;
	
	Logger(const std::string& name = "root");
	void log(LogLevel::Level level, LogEvent::ptr event);
	
	void debug(LogEvent::ptr event);
	void info(LogEvent::ptr event);
	void warn(LogEvent::ptr event);
	void error(LogEvent::ptr event);
	void fatal(LogEvent::ptr event);
private:
	std::string m_name;		//日志名称
	LogLevel::Level m_level;	//日志级别
	std::list<LogAppender::ptr> m_appenders; //Appender集合
};

class StdoutLogAppender : public LogAppender{
};
class FileLogAppender : public LogAppender{
};

}


};

#endif
