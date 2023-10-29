//
//  ThreadsPool.hpp
//  ThreadPool
//
//  Created by Yiping Wang on 2023/10/29.
//

#ifndef ThreadsPool_hpp
#define ThreadsPool_hpp

#include <mutex>
#include <shared_mutex>
#include <list>
#include <queue>
#include <functional>
#include <future>
#include <atomic>
#include <semaphore>
#include <sys/time.h>
#include <cstdlib>
#include <csignal>

#define THREAD_PRIORITY 20 // 工作线程优先级
#define GUADE_THREAD_PRIORITY 10 // 守护线程优先级
#define MAX_THREAD_NUM 4000 // 允许最大线程数
#define HINT_CLOCK_SEC 30 // 守护线程提示时钟
#define DEAD_CLOCK_MICROSEC 8000 // 回收时钟


class ThreadsPool {
public:
    ThreadsPool(const ThreadsPool &) = delete;
    ThreadsPool(ThreadsPool &&) = delete;
    explicit ThreadsPool(unsigned long maxThreadNum = 10, unsigned long appending = 0): _over(false), _daemonOver(false), _del(false), _dead(0), _working(0) {
        _addThreads(maxThreadNum); // 导入预设
        // 创建守护线程
        _daemon = std::thread([this](){
            { // 更新线程优先级
                struct sched_param sched_param;
                sched_param.sched_priority = GUADE_THREAD_PRIORITY;
                pthread_setschedparam(pthread_self(), SCHED_RR, &sched_param);
            }
            for(;!_daemonOver;) {
                {
                    std::unique_lock<std::mutex> lk(_m3);
                    _hint.wait_for(lk, std::chrono::seconds(HINT_CLOCK_SEC)); // 定时器
                }
                if(_tooMany()) _delThreads(_delThreadNum()); // 线程过多处理逻辑（考虑新开线程）
                if(_dead > 0) _delThreadsSlow(); // 列表移除
            }
            _over = true;
            _cv.notify_all();
            for(;!_threadsEmpty();) { // 结束等待所有工作线程结束
                std::this_thread::sleep_for(std::chrono::microseconds(DEAD_CLOCK_MICROSEC));
                if(_dead > 0) _delThreadsSlow();
            }
        });
    }
    virtual ~ThreadsPool() {
        _daemonOver = true;
        _hint.notify_one();
        _daemon.join();
    };
public:
    template<class _Func, class ... _Args>
    auto submit(_Func&& _func, _Args&& ... _args) -> std::future<decltype(_func(std::forward<_Args>(_args) ...))> {
        auto taskPtr = std::make_shared<
                std::packaged_task<decltype(_func(std::forward<_Args>(_args) ...))(_Args&& ...)>
            > (std::forward<_Func>(_func));
        {
            
            std::unique_lock<std::mutex> lk(_m);
            if(_tooFew()) _addThreads(_addThreadNum()); // 线程过少，扩充
            if(_over) throw std::runtime_error("submit on stopped ThreadPool");
            _tasks.push_back([taskPtr, ... _args = std::forward<_Args>(_args)]() mutable {
                (*taskPtr)(std::forward<_Args>(_args) ...);
            });
        }
        _cv.notify_one();
        return taskPtr -> get_future();
    }
private:
    unsigned long _addThreadNum() { // 安全，返回扩容增加线程的数量
        std::shared_lock<std::shared_timed_mutex> lk(_m2);
        return _threads.size();
    }
    unsigned long _delThreadNum() { // 安全，返回删除线程数量
        std::shared_lock<std::shared_timed_mutex> lk(_m2);
        return _threads.size() / 2;
    }
    bool _tooFew() { // 安全，调用这个的时候_tasks已加锁，判断系统中线程数是否过少
        std::shared_lock<std::shared_timed_mutex> lk(_m2);
        return _tasks.size()  > (_threads.size() - _working) * 4/5 && _threads.size() < MAX_THREAD_NUM;
    }
    bool _tooMany() { // 安全，判断系统中线程数是否过多
        std::unique_lock<std::mutex> lk(_m);
        std::shared_lock<std::shared_timed_mutex> lk2(_m2);
        return _tasks.size() == 0 && _threads.size() * 4 > MAX_THREAD_NUM;
    }
    bool _threadsEmpty() { // 安全，判断线程池是否为空
        std::shared_lock<std::shared_timed_mutex> lk(_m2);
        return _threads.empty();
    }
    void _addThreads(unsigned long num) { // 增加线程
        std::unique_lock<std::shared_timed_mutex> lk(_m2);
        unsigned long base = _threads.size();
        if(base + num > MAX_THREAD_NUM) {
            num = MAX_THREAD_NUM - base;
        }
        if(num < 0) throw std::runtime_error("illigal number of threads");
        if(num == 0) return;
        for(unsigned long i = base; i < num + base; ++ i) {
            auto alivePtr = std::make_shared<std::atomic<bool>>(true);
            std::thread thread = std::thread([this, tid = i](std::shared_ptr<std::atomic<bool>> alivePtr) mutable {
                {
                    struct sched_param sched_param;
                    sched_param.sched_priority = THREAD_PRIORITY;
                    pthread_setschedparam(pthread_self(), SCHED_RR, &sched_param);
                }
                for(;;) {
                    std::function<void()> f;
                    {
                        std::unique_lock<std::mutex> lk(_m);
                        _cv.wait(lk, [this]() {
                            return !_tasks.empty() || _over || _del;
                        });
                        if((_over || _del) && _tasks.empty()) break;
                        f = std::move(_tasks.front());
                        _tasks.pop_front();
                    }
                    ++ _working;
                    f();
                    -- _working;
                }
                // 线程结束后处理
                ++ _dead;
                *alivePtr = false; // 存活标识
            }, alivePtr);
            _threads.push_back({std::move(thread), std::move(alivePtr)});
        }
    }
    void _delThreads(unsigned long num) { // 删除线程
        _del = true;
        for(unsigned long i = 0; i < num; i ++) _cv.notify_one();
        std::this_thread::sleep_for(std::chrono::microseconds(DEAD_CLOCK_MICROSEC)); //回收信号停留DEAD_CLOCK_MICROSEC，之后恢复
        _del = false;
    }
    void _delThreadsSlow() { // 将死亡的线程真正删除，列表清空
        std::unique_lock<std::shared_timed_mutex> lk(_m2);
        for (auto i = _threads.begin(); i != _threads.end();) {
            if(*((*i).second) == false) {
                (*i).first.join();
                i = _threads.erase(i);
                -- _dead;
            }else ++ i;
        }
    }
private:
    std::atomic<bool> _over; // 结束信号
    std::mutex _m3; // 提示信号锁
    std::atomic<bool> _del; // 缩容信号
    
    std::list<std::function<void()>> _tasks;
    std::atomic<unsigned long> _working; // 正在工作的节点数量
    std::atomic<unsigned long> _dead; // 死亡节点数量
    std::mutex _m; // _tasks互斥锁
    std::condition_variable _cv; // _tasks信号量
    
    std::list<std::pair<std::thread, std::shared_ptr<std::atomic<bool>>>> _threads; // 线程池
    std::shared_timed_mutex _m2; // _threads读写锁
    
    std::thread _daemon; // 守护线程
    std::atomic<bool> _daemonOver; // 守护线程结束信号
    std::condition_variable _hint; // 守护线程提示信号
};

#endif /* ThreadsPool_hpp */
