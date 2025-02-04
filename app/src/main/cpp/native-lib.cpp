#include <jni.h>
#include <string>
#include <exception>
#include <deque>
#include <thread>
#include <memory>
#include <condition_variable>
#include <boost/coroutine/all.hpp>
#include <iostream>
#include <fstream>
#include <pthread.h>

using namespace boost::coroutines;

class Task {
public:
    Task(std::function<void(coroutine<void>::push_type&)> f)
    : f_(std::move(f))
    {
    }

    Task(Task && other) {
        this->resume_ = std::move(other.resume_);
        this->f_ = std::move(other.f_);
    }

    void operator()() {
        if (!resume_) {
            resume_.reset(new coroutine<void>::pull_type(f_));
        } else {
            resume_->operator()();
        }
    }

    operator bool() const {
        return resume_->operator bool();
    }
private:
    std::unique_ptr<coroutine<void>::pull_type> resume_;
    std::function<void(coroutine<void>::push_type&)> f_;
};

class TaskQueue {
public:

    void push(Task task)
    {
        {
            std::lock_guard<std::mutex> guard(mutex_);
            queue_.push_back(std::move(task));
        }
        cv_.notify_one();
    }

    Task pop()
    {
        std::unique_lock<std::mutex> lock(mutex_);

        for (;;) {
            if (stop_) {
                throw std::exception();
            }

            if (!queue_.empty()) {
                auto&& task = queue_.front();
                queue_.pop_front();
                return std::move(task);
            } else {
                cv_.wait_for(lock, std::chrono::milliseconds(1000), [&] { return !queue_.empty(); });
            }
        }
    }

    void stop() {
        stop_ = true;
    }

private:
    bool stop_ = false;
    std::condition_variable cv_;
    mutable std::mutex mutex_;
    std::deque<Task> queue_;
};

class Worker {
public:
    Worker()
    {
        thread_ = std::thread([&]() {
            pthread_setname_np(pthread_self(), "CoroutineThread");
            for (;;) {
                try {
                    auto&& task = tasks_.pop();
                    task.operator()();
                    if (task) {
                        tasks_.push(std::move(task));
                    }
                } catch (std::exception& e) {
                    return;
                }
            }
        });
    }

    ~Worker() {
        thread_.join();
    }

    void push(Task task) {
        tasks_.push(std::move(task));
    }

    void stop() {
        tasks_.stop();
    }
private:
    TaskQueue tasks_;
    std::thread thread_;
};

std::unique_ptr<Worker> g_worker;

extern "C" JNIEXPORT jstring JNICALL
Java_com_example_myapplication_MainActivity_stringFromJNI(
        JNIEnv* env,
        jobject /* this */) {
    g_worker = std::make_unique<Worker>();
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}

int foo(int j)
{
    int result = 0;
    for (int i = 0; i <= j; ++i) {
        result += i;
    }

    return result;
}

void doSomeWork(const std::string& filename) {
    std::ofstream f{filename};
    for (int i = 0; i < 10; ++i) {
        f << foo(i);
    }
}

void coro1(coroutine<void>::push_type &yield)
{
    for (int i = 0; i < 100; ++i) {
        doSomeWork("coro1.txt");
        yield();
    }
}

void coro2(coroutine<void>::push_type &yield)
{
    for (int i = 0; i < 100; ++i) {
        doSomeWork("coro2.txt");
        yield();
    }
}

std::thread g_thread;

extern "C" JNIEXPORT void JNICALL
Java_com_example_myapplication_MainActivity_startTask(
        JNIEnv* /* env */,
        jobject /* this */) {

    g_worker->push(Task(coro1));
    g_worker->push(Task(coro2));
}


