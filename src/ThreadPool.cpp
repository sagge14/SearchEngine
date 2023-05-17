//
// Created by sage on 26.04.2023.
//

#include "ThreadPool.h"

thread_pool::thread_pool(uint32_t num_threads) {
    last_idx = 0;
    threads.reserve(num_threads);
    for (uint32_t i = 0; i < num_threads; ++i) {
        threads.emplace_back(&thread_pool::run, this);
    }
}

void thread_pool::wait(int64_t task_id) {
    std::unique_lock<std::mutex> lock(completed_task_ids_mtx);
    completed_task_ids_cv.wait(lock, [this, task_id]()->bool {
        return completed_task_ids.find(task_id) != completed_task_ids.end();
    });
}

void thread_pool::wait_all() {
    std::unique_lock<std::mutex> lock(q_mtx);
    completed_task_ids_cv.wait(lock, [this]()->bool {
        std::lock_guard<std::mutex> task_lock(completed_task_ids_mtx);
        return q.empty() && last_idx == completed_task_ids.size();
    });
}

bool thread_pool::calculated(int64_t task_id) {
    std::lock_guard<std::mutex> lock(completed_task_ids_mtx);
    if (completed_task_ids.find(task_id) != completed_task_ids.end()) {
        return true;
    }
    return false;
}

thread_pool::~thread_pool() {
    quite = true;
    for (auto & thread : threads) {
        q_cv.notify_all();
        thread.join();
    }
}

void thread_pool::run() {
    while (!quite) {
        std::unique_lock<std::mutex> lock1(q_mtx);
        q_cv.wait(lock1, [this]()->bool { return !q.empty() || quite; });

        if (!q.empty()) {
            auto elem = std::move(q.front());
            q.pop();
            lock1.unlock();

            elem.first.get();

            std::lock_guard<std::mutex> lock2(completed_task_ids_mtx);
            completed_task_ids.insert(elem.second);

            completed_task_ids_cv.notify_all();
        }
    }
}
