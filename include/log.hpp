#ifndef LOG_HPP
#define LOG_HPP

#include <iostream>
#include <queue>
#include <vector>
#include <string>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <fstream>
#include <atomic>
#include <sstream>
#include <stdexcept>

// 将任意类型转换为字符串
template<typename T>
std::string to_string(T&& arg){
    std::ostringstream oss;
    oss << std::forward<T>(arg);
    return oss.str();
};

// 线程安全的日志消息队列
class LogQueue{
public:
    // 入队日志消息
    void push(const std::string& msg);
    // 出队日志消息(队空时阻塞)
    bool pop(std::string& msg);
    // 关闭队列
    void shutDown();

private:
    std::queue<std::string> queue_;       // 日志消息队列
    std::mutex mutex_;                    // 互斥锁
    std::condition_variable cond_var_;    // 条件变量
    bool is_shutdown_ = false;            // 关闭标志
};

// 日志系统主类
class Logger{
public:
    // 构造函数(指定日志文件)
    Logger(const std::string& filename);
    // 析构函数
    ~Logger();
    
    // 日志输出接口(支持格式化字符串)
    template<typename... Args>
    void log(const std::string& format, Args&&... args);

private:
    LogQueue log_queue_;              // 日志队列
    std::thread worker_thread_;       // 后台写入线程
    std::ofstream log_file_;          // 日志文件流
    std::atomic<bool> exti_flag_;     // 退出标志(原子变量)
    
    // 格式化日志消息(替换{})
    template<typename... Args>
    std::string formatMessage(const std::string& format, Args&&... args);
};

// 日志输出接口实现
template<typename... Args>
void Logger::log(const std::string& format, Args&&... args){
    log_queue_.push(formatMessage(format, std::forward<Args>(args)...));
};

// 日志消息格式化实现
template<typename... Args>
std::string Logger::formatMessage(const std::string& format, Args&&... args){
    std::vector<std::string> arg_strings = {to_string(std::forward<Args>(args))...};
    std::ostringstream oss;
    size_t arg_index = 0;
    size_t pos = 0;
    size_t placeholder = format.find("{}");
    
    while(placeholder != std::string::npos){
        oss << format.substr(pos, placeholder - pos);
        if(arg_index < arg_strings.size()){
            oss << arg_strings[arg_index++];
        }
        else{
            oss << "{}";
        }
    
        pos = placeholder + 2;
        placeholder = format.find("{}", pos);
    }

    oss << format.substr(pos);
    while(arg_index < arg_strings.size()){
        oss << arg_strings[arg_index++];
    }

    return oss.str();
};

#endif  
    