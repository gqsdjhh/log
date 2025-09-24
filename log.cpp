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

//将单个参数转化为字符串
template<typename T>
std::string to_string(T&& arg) {
    std::ostringstream oss;
    oss << std::forward<T>(arg);
    return oss.str();
}

class LogQueue{
public:
    void push(const std::string& msg);
    bool pop(std::string& msg);
    void shutDown();
private:
    std::queue<std::string> queue_;
    std::mutex mutex_;
    std::condition_variable cond_var_;
    bool is_shutdown_ = false;
};

void LogQueue::push(const std::string& msg) {
    std::lock_guard<std::mutex> lock(mutex_);
    queue_.push(msg);
    if(queue_.size() == 1) {
        cond_var_.notify_one();
    }
}

bool LogQueue::pop(std::string& mag){
    std::unique_lock<std::mutex> lock(mutex_);
    cond_var_.wait(lock, [this](){
        return !queue_.empty() || is_shutdown_;
    });

    if(is_shutdown_ && queue_.empty()) {
        return false;
    }
}

int main() {
    



    return 0;
}