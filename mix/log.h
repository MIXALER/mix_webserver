#ifndef __MIX_LOG_H__
#define __MIX_LOG_H__

#include<string>
#include <iostream>
#include<cstdint>
#include<memory>
#include<list>
#include <vector>
#include <iosfwd>
#include <fstream>
#include <sstream>


namespace mix
{
    class Logger;

//  日志事件
    class LogEvent
    {

    public:
        typedef std::shared_ptr<LogEvent> ptr;

        LogEvent();

        const char *getFile() const
        {
            return m_file;
        }

        int32_t getLine() const
        {
            return m_line;
        }

        uint32_t getElapse() const
        {
            return m_elapse;
        }

        uint32_t getThreadId() const
        {
            return m_threadId;
        }

        uint32_t getFiberId() const
        {
            return m_fiberId;
        }

        uint64_t getTime() const
        {
            return m_time;
        }

        const std::string &getContent() const
        {
            return m_content;
        }

    private:
        const char *m_file = nullptr;       // 文件名
        int32_t m_line = 0;                 // 行号
        uint32_t m_elapse = 0;              // 程序启动到现在的毫秒数
        uint32_t m_threadId = 0;             // 线程 id
        uint32_t m_fiberId = 0;             // 协程 id
        std::string m_content;              // 内容
        uint64_t m_time = 0;                // 时间戳
    };

//  日志级别
    class LogLevel
    {
    public:
        enum Level
        {
            UNKNOWN = 0,
            DEBUG = 1,
            INFO = 2,
            WARN = 3,
            ERROR = 4,
            FATAL = 5
        };

        static const char *ToString(LogLevel::Level level);
    };

//  日志格式器
    class LogFormatter
    {
    public:
        typedef std::shared_ptr<LogFormatter> ptr;

        LogFormatter(const std::string &pattern);

//      %t  %m 消息 %n 换行 %thread_id
        std::string format(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event);

    public:
        class FormatItem
        {
        public:
            typedef std::shared_ptr<FormatItem> ptr;

            FormatItem(const std::string &fmt = "")
            {};

            virtual ~FormatItem()
            {

            }

            virtual void
            format(std::stringstream os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) = 0;
        };

        void init();

    private:
        std::string m_pattern;
        std::vector<FormatItem::ptr> m_items;
    };

    //    日志输出地
    class LogAppender
    {
    public:
        typedef std::shared_ptr<LogAppender> ptr;

        virtual ~LogAppender()
        {

        }

        virtual void log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) = 0;

        void setFormatter(LogFormatter::ptr val)
        {
            m_formatter = val;
        }

        LogFormatter::ptr getFormatter() const
        {
            return m_formatter;
        }

    protected:
        LogLevel::Level m_level;
        LogFormatter::ptr m_formatter;

    };


    //    日志器
    class Logger
    {
    public:
        typedef std::shared_ptr<Logger> ptr;


        Logger(const std::string &name = "root");

        void log(LogLevel::Level level, LogEvent::ptr event);

        void debug(LogEvent::ptr event);

        void warn(LogEvent::ptr event);

        void error(LogEvent::ptr event);

        void fatal(LogEvent::ptr event);

        void info(LogEvent::ptr event);

        void addAppender(LogAppender::ptr appender);

        void delAppender(LogAppender::ptr appender);

        const std::string &getName() const
        {
            return m_name;
        }

        LogLevel::Level getLevel() const
        {
            return m_level;
        }

        void setLevel(LogLevel::Level var)
        {
            m_level = var;
        }

    private:
        std::list<LogAppender::ptr> m_appenders;        // appender 集合
        std::string m_name;
        LogLevel::Level m_level;        //  满足日志级别才会被输出
    };


//    输出到控制台
    class StdoutLogAppender : public LogAppender
    {
    public:
        typedef std::shared_ptr<StdoutLogAppender> ptr;

        void log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override;

    private:


    };

//   输出到文件
    class FileLogAppender : public LogAppender
    {
    public:
        typedef std::shared_ptr<FileLogAppender> ptr;

        FileLogAppender(const std::string &filename);

        void log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override;

        bool reopen();

    private:
        std::string m_filename;
        std::ofstream m_filestream;

    };

}


#endif