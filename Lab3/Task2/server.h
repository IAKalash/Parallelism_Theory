#pragma once

#include <atomic>
#include <cstddef>
#include <condition_variable>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <thread>
#include <unordered_map>

template<typename T>
class Server {
public:
    Server() : running_(false), next_id_(1) {}

    ~Server() {
        stop();
    }

    void start() {
        bool expected = false;
        if (!running_.compare_exchange_strong(expected, true))
            return;

        worker_ = std::thread([this]() { this->worker_loop(); });
    }

    void stop() {
        bool expected = true;
        if (!running_.compare_exchange_strong(expected, false))
            return;

        cv_.notify_all();
        if (worker_.joinable())
            worker_.join();
    }

    template<typename Func>
    size_t add_task(Func func) {
        std::unique_lock<std::mutex> lk(mutex_);
        size_t id = next_id_++;

        auto prom = std::make_unique<std::promise<T>>();
        auto fut = prom->get_future().share();

        results_.emplace(id, std::move(fut));
        promises_.emplace(id, std::move(prom));

        tasks_.emplace(TaskItem{id, std::move(func)});
        lk.unlock();
        cv_.notify_one();
        return id;
    }

    T request_result(size_t id, bool block = true) {
        std::shared_future<T> fut;
        {
            std::lock_guard<std::mutex> lk(mutex_);
            auto it = results_.find(id);
            if (it == results_.end())
                throw std::invalid_argument("unknown id");
            fut = it->second;
        }

        if (!block) {
            using namespace std::chrono_literals;
            if (fut.wait_for(0s) == std::future_status::ready)
                return fut.get();
            throw std::runtime_error("result not ready");
        }

        return fut.get();
    }

private:
    struct TaskItem {
        size_t id;
        std::function<T()> task;
    };

    void worker_loop() {
        std::unique_lock<std::mutex> lk(mutex_, std::defer_lock);

        while (running_) {
            lk.lock();
            cv_.wait(lk, [this]() { return !tasks_.empty() || !running_; });

            if (!running_ && tasks_.empty()) {
                lk.unlock();
                break;
            }

            if (!tasks_.empty()) {
                auto item = std::move(tasks_.front());
                tasks_.pop();
                lk.unlock();

                try {
                    T res = item.task();
                    std::lock_guard<std::mutex> res_lock(mutex_);
                    auto it = promises_.find(item.id);
                    if (it != promises_.end()) {
                        it->second->set_value(res);
                        promises_.erase(it);
                    }
                } catch (...) {
                    std::lock_guard<std::mutex> res_lock(mutex_);
                    auto it = promises_.find(item.id);
                    if (it != promises_.end()) {
                        it->second->set_exception(std::current_exception());
                        promises_.erase(it);
                    }
                }
            }
        }

        std::lock_guard<std::mutex> res_lock(mutex_);
        while (!tasks_.empty()) {
            auto item = std::move(tasks_.front());
            tasks_.pop();
            auto it = promises_.find(item.id);
            if (it != promises_.end()) {
                it->second->set_exception(std::make_exception_ptr(std::runtime_error("server stopped")));
                promises_.erase(it);
            }
        }
    }

    std::atomic<bool> running_;
    std::thread worker_;

    std::mutex mutex_;
    std::condition_variable cv_;
    std::queue<TaskItem> tasks_;

    std::unordered_map<size_t, std::shared_future<T>> results_;
    std::unordered_map<size_t, std::unique_ptr<std::promise<T>>> promises_;

    size_t next_id_;
};
