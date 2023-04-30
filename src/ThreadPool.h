//
// Created by user on 26.03.2023.
//

#ifndef SEARCHENGINE_TEST_THREADPOOL_H
#define SEARCHENGINE_TEST_THREADPOOL_H


#include <iostream>
#include <queue>
#include <thread>
#include <mutex>
#include <future>
#include <unordered_set>
#include <atomic>
#include <vector>
#include <chrono>
#include <functional>

class thread_pool {

    void run();

    std::queue<std::pair<std::future<void>, int64_t>> q; // очередь задач - хранит функцию(задачу), которую нужно исполнить и номер данной задачи
    std::mutex q_mtx;
    std::condition_variable q_cv;
    std::unordered_set<int64_t> completed_task_ids;  // помещаем в данный контейнер исполненные задачи
    std::condition_variable completed_task_ids_cv;
    std::mutex completed_task_ids_mtx;
    std::vector<std::thread> threads;
    std::atomic<bool> quite{ false };  // флаг завершения работы thread_pool
    std::atomic<int64_t> last_idx; // переменная хранящая id который будет выдан следующей задаче

public:
    thread_pool(uint32_t num_threads);

    template <typename Func, typename ...Args>
    int64_t add_task(const Func& task_func, Args&&... args) {
        int64_t task_idx = last_idx++;

        std::lock_guard<std::mutex> q_lock(q_mtx);
        q.emplace(std::async(std::launch::deferred, task_func, args...), task_idx);
        q_cv.notify_one();
        return task_idx;
    }

    void wait(int64_t task_id);

    void wait_all();

    bool calculated(int64_t task_id);

    ~thread_pool();


};


#endif //SEARCHENGINE_TEST_THREADPOOL_H
