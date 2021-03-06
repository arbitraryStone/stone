#ifndef __STONE_LOG_HPP__
#define __STONE_LOG_HPP__
#include <stdint.h>
#include <memory>
#include <vector>
#include <string>
#include <map>
#include <functional>
#include <sstream>
#include <fstream>
#include <stdarg.h>
#include <map>
#include "util.hpp"
#include "thread.hpp"
#include "singleton.hpp"
#include "mutex.hpp"
#define STONE_LOG_LEVEL(logger,level) \
    if(logger->getLevel()<=level) \
        stone::LogEventWrap(stone::LogEvent::ptr(new stone::LogEvent(logger,level,__FILE__,__LINE__,0 \
                           ,stone::GetThreadId(),stone::GetFiberId(),time(0),stone::Thread::GetName()))).getSS()

#define STONE_LOG_DEBUG(logger) STONE_LOG_LEVEL(logger,stone::LogLevel::DEBUG)
#define STONE_LOG_INFO(logger) STONE_LOG_LEVEL(logger,stone::LogLevel::INFO)
#define STONE_LOG_WARNING(logger) STONE_LOG_LEVEL(logger,stone::LogLevel::WARNING)
#define STONE_LOG_ERROR(logger) STONE_LOG_LEVEL(logger,stone::LogLevel::ERROR)
#define STONE_LOG_FATAL(logger) STONE_LOG_LEVEL(logger,stone::LogLevel::FATAL)

#define STONE_LOG_FMT_LEVEL(logger,level,fmt,...) \
    if(logger->getLevel()<=level) \
        stone::LogEventWrap(stone::LogEvent::ptr(new stone::LogEvent(logger,level,\
        __FILE__,__LINE__,0,stone::GetThreadId(),stone::GetFiberId(),time(0),stone::Thread::GetName()))).getEvent()->format(fmt,__VA_ARGS__)

#define STONE_LOG_FMT_DEBUG(logger,fmt,...) STONE_LOG_FMT_LEVEL(logger,stone::LogLevel::DEBUG,fmt,__VA_ARGS__)
#define STONE_LOG_FMT_INFO(logger,fmt,...) STONE_LOG_FMT_LEVEL(logger,stone::LogLevel::INFO,fmt,__VA_ARGS__)
#define STONE_LOG_FMT_WARNING(logger,fmt,...) STONE_LOG_FMT_LEVEL(logger,stone::LogLevel::WARNING,fmt,__VA_ARGS__)
#define STONE_LOG_FMT_ERROR(logger,fmt,...) STONE_LOG_FMT_LEVEL(logger,stone::LogLevel::ERROR,fmt,__VA_ARGS__)
#define STONE_LOG_FMT_FATAL(logger,fmt,...) STONE_LOG_FMT_LEVEL(logger,stone::LogLevel::FATAL,fmt,__VA_ARGS__)

#define STONE_LOG_ROOT() stone::LoggerMgr::GetInstance()->getRoot()
#define STONE_LOG_NAME(name) stone::LoggerMgr::GetInstance()->getLogger(name)
namespace stone{

class Logger;
class LoggerManager;
//LogLevel ???????????????
class LogLevel
{
public:
    enum Level {
        UNKOWN = 0,
        DEBUG = 1,
        INFO= 2,
        WARNING = 3,
        ERROR = 4,
        FATAL = 5
    };
    static const std::string toStr(LogLevel::Level level);
    static const LogLevel::Level fromStr(const std::string& str);
};
//logEvent???????????????
class LogEvent
{
public:
    typedef std::shared_ptr<LogEvent> ptr;
public:
    LogEvent(std::shared_ptr<Logger> logger,LogLevel::Level level, const char* filename, uint32_t line, uint32_t elapse, uint32_t threadId, uint32_t fiberId, uint64_t time,const std::string& thread_name);
    const char*  getFileName() const { return m_filename; }
    uint32_t getLine() const { return m_line; }
    uint32_t getElapse() const { return m_elapse; }
    uint32_t getThreadId() const { return m_threadId; }
    const std::string& getThreadName() const { return m_threadName; }
    uint32_t getFiberId() const { return m_fiberId; }
    uint64_t getTime() const { return m_time; }
    const char* getName() const { return m_filename; }
    std::string getContent() const { return m_ss.str(); }
    std::shared_ptr<Logger> getLogger() const { return m_logger; }
    LogLevel::Level getLevel() const { return m_level; }
    std::stringstream& getSS() { return m_ss; }
    void format(const char* fmt ,...);
    void format(const char* fmt,va_list  al);
private:
    const char*  m_filename;//?????????
    uint32_t m_line;        //??????
    uint32_t m_elapse;      //?????????????????????????????????
    uint32_t m_threadId;    //??????id
    uint32_t m_fiberId;     //??????id
    uint64_t m_time;        //?????????
    std::string m_threadName;
    std::stringstream m_ss; //string??? (??????????????????)
    std::shared_ptr<Logger> m_logger;
    LogLevel::Level m_level;
};

class LogEventWrap{
public:
    LogEventWrap(LogEvent::ptr e);
    ~LogEventWrap();
    std::stringstream& getSS();
    LogEvent::ptr getEvent() const { return m_event; }
private:
    LogEvent::ptr m_event;
};
/**
 * @brief ???????????????
 */
class LogFormatter
{
public:
    typedef std::shared_ptr<LogFormatter> ptr;
public:
    /**
     *  @brief ????????????????????????
     */
    class FormatterItem{
    public:
        typedef std::shared_ptr<FormatterItem> ptr;
    public:
        virtual ~FormatterItem(){}
    public:
      virtual void format(std::ostream& os,std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event)= 0;
    };
public:
    void init();
    std::string format(std::shared_ptr<Logger> logger,LogLevel::Level level , LogEvent::ptr event);
public:
    LogFormatter(const std::string& pattern = "");
    ~LogFormatter(){};
    bool isError() const { return m_error; }
    const std::string getPattern() const { return m_pattern; }
private:
    std::string m_pattern;
    std::vector<FormatterItem::ptr> m_items;
    bool m_error = false;
};
/**
 * @brief ???????????????
 */
class LogAppender{
friend class Logger;
public:
    typedef std::shared_ptr<LogAppender> ptr;
    typedef Mutex MutexType;
public:
    
    virtual ~LogAppender(){};
    virtual void log(std::shared_ptr<Logger> logger,LogLevel::Level level , LogEvent::ptr event) = 0;
    virtual std::string toYamlString() = 0;
    LogFormatter::ptr getFormatter();
    void setFormatter(LogFormatter::ptr val);
    LogLevel::Level getLevel() const { return m_level; }
    void setLevel(LogLevel::Level val) { m_level = val; }
protected:
    LogLevel::Level m_level;
    bool m_hasFormatter = false;
    LogFormatter::ptr m_formatter;
    MutexType m_mutex;
};

/**
 * @brief ???????????????(??????????????????)
 */
class StdoutLogAppender: public LogAppender{
public:
    typedef std::shared_ptr<StdoutLogAppender> ptr;
public:
    void log(std::shared_ptr<Logger> logger,LogLevel::Level level, LogEvent::ptr event) override;
    std::string toYamlString() override;
};

/**
 * @brief ?????????????????????(???????????????)
 */ 
class FileLogAppender: public LogAppender{
public:
    typedef std::shared_ptr<FileLogAppender> ptr;
public:
    FileLogAppender(const std::string& fileName);
    bool reopen();
public:
    void log(std::shared_ptr<Logger> logger,LogLevel::Level level, LogEvent::ptr event) override;
    std::string toYamlString() override;
private:
    std::string m_filename;     //?????????
    std::ofstream m_filestream; //?????????
};

/**
 * @brief ?????????
 */
class Logger:public std::enable_shared_from_this<Logger>{
friend class LoggerManager;

public:
   typedef std::shared_ptr<Logger> ptr;
   typedef Mutex MutexType;
public:
   Logger(const std::string& name = "root");
   ~Logger(){};
public:
   void log(LogEvent::ptr event,LogLevel::Level level);
   void debug(LogEvent::ptr event);
   void info(LogEvent::ptr event);
   void warning(LogEvent::ptr event);
   void error(LogEvent::ptr event);
   void fatal(LogEvent::ptr event);
public:
   void addAppender(LogAppender::ptr appender);
   void delAppender(LogAppender::ptr appender);
   void clearAppender();
public:
   LogLevel::Level getLevel() const { return m_level; }
   void setLevel(LogLevel::Level level) { m_level = level; }
   const std::string& getName(){ return m_name; }
   void setFormatter(LogFormatter::ptr val);
   void setFormatter(const std::string& val);
   LogFormatter::ptr getFormatter();
    std::string toYamlString();
private:
   std::string m_name;                       //????????????
   LogLevel::Level m_level;                  //????????????
   MutexType m_mutex;
   std::vector<LogAppender::ptr> m_appenders;//?????????????????????
   LogFormatter::ptr m_formatter;            //???????????????
   Logger::ptr m_root;
};

class LoggerManager{
public:
    typedef Mutex MutexType;
    LoggerManager();
    Logger::ptr getLogger(const std::string& name); 
    Logger::ptr getRoot() const { return m_root; }
    void init();
    std::string toYamlString();
private:
    MutexType m_mutex;
    std::map<std::string,Logger::ptr>  m_loggers;
    Logger::ptr m_root;
};
typedef stone::Singleton<LoggerManager> LoggerMgr;
};
#endif
