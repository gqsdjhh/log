#include "log.hpp"

// 将任意类型转换为字符串
template<typename T>
std::string to_string(T&& arg){
    std::ostringstream oss;
    oss << std::forward<T>(arg);
    return oss.str();
}

// 向日志队列添加消息
void LogQueue::push(const std::string& msg){
    std::lock_guard<std::mutex> lock(mutex_);
    queue_.push(msg);
    if(queue_.size() == 1){
        cond_var_.notify_one();
    }
}

// 从日志队列取出消息
bool LogQueue::pop(std::string& msg){
    std::unique_lock<std::mutex> lock(mutex_);
    // 等待队列非空或已关闭
    cond_var_.wait(lock, [this](){
        return !queue_.empty() || is_shutdown_;
    });

    if(is_shutdown_ && queue_.empty()){
        return false;
    }

    msg = queue_.front();
    queue_.pop();
    return true;
}

// 关闭日志队列
void LogQueue::shutDown(){
    std::lock_guard<std::mutex> lock(mutex_);
    is_shutdown_ = true;
    cond_var_.notify_all();
}

// 日志类构造函数
Logger::Logger(const std::string& filename) : log_file_(filename, std::ios::out | std::ios::app)
, exti_flag_(false){
    if(!log_file_.is_open()){
        throw std::runtime_error("Failed to open log file: " + filename);
    }

    // 启动后台工作线程
    worker_thread_ = std::thread([this](){
        std::string msg;
        while(log_queue_.pop(msg)){
                log_file_ << msg << '\n';
        }
    });
};

// 日志类析构函数
Logger::~Logger(){
    exti_flag_ = true;
    log_queue_.shutDown();
    if(worker_thread_.joinable()){
        worker_thread_.join();
    }

    if(log_file_.is_open()){
        log_file_.close();
    }
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
    