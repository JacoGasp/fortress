#ifndef TASK_SAFE_QUEUE_H
#define TASK_SAFE_QUEUE_H

#include <Arduino.h>
#include "AsyncSyncronization.h"

// #include "FreeRTOS.h"

namespace fortress {
namespace esp32 {
template <typename T>
class TaskSafeQueue {
   private:
    // std::mutex m_mutex;
    // SemaphoreHandle_t m_mutex;
    std::deque<T> m_deque;

    T m_front{};
    T m_back{};

    AsyncPlainLock m_lock;

   public:
    TaskSafeQueue() : m_lock{} {}

    virtual ~TaskSafeQueue() { clear(); }

    T& front() {
        // const std::lock_guard<std::mutex> lock(m_mutex);
        // xSemaphoreTake(m_mutex, portMAX_DELAY);
        m_lock.lock();
        m_front = m_deque.front();
        // xSemaphoreGive(m_mutex);
        m_lock.unlock();
        return m_front;
    }

    T& back() {
        // const std::lock_guard<std::mutex> lock(m_mutex);
        // xSemaphoreTake(m_mutex, portMAX_DELAY);
        m_lock.lock();
        m_back = m_deque.back();
        // xSemaphoreGive(m_mutex);
        m_lock.unlock();
        return m_back;
    }

    T pop_front() {
        // const std::lock_guard<std::mutex> lock(m_mutex);
        // xSemaphoreTake(m_mutex, portMAX_DELAY);
        m_lock.lock();
        T t = std::move(m_deque.front());
        m_deque.pop_front();
        // xSemaphoreGive(m_mutex);
        m_lock.unlock();
        return t;
    }

    T pop_back() {
        // const std::lock_guard<std::mutex> lock(m_mutex);
        // xSemaphoreTake(m_mutex, portMAX_DELAY);
        m_lock.lock();
        T t = std::move(m_deque.back());
        m_deque.pop_back();
        // xSemaphoreGive(m_mutex);
        m_lock.unlock();
        return t;
    }

    void push_front(const T& item) {
        // const std::lock_guard<std::mutex> lock(m_mutex);
        // xSemaphoreTake(m_mutex, portMAX_DELAY);
        m_lock.lock();
        m_deque.template emplace_front(std::move(item));
        // xSemaphoreGive(m_mutex);
        m_lock.unlock();
    }

    void push_back(const T& item) {
        // const std::lock_guard<std::mutex> lock(m_mutex);
        // xSemaphoreTake(m_mutex, portMAX_DELAY);
        m_lock.lock();
        m_deque.template emplace_back(std::move(item));
        // xSemaphoreGive(m_mutex);
        m_lock.unlock();
    }

    bool empty() {
        // const std::lock_guard<std::mutex> lock(m_mutex);
        // xSemaphoreTake(m_mutex, portMAX_DELAY);
        m_lock.lock();

        bool isEmpty{m_deque.empty()};
        // xSemaphoreGive(m_mutex);
        m_lock.unlock();
        return isEmpty;
    }

    void clear() {
        // const std::lock_guard<std::mutex> lock(m_mutex);
        // xSemaphoreTake(m_mutex, portMAX_DELAY);
        m_lock.lock();
        m_deque.clear();
        // xSemaphoreGive(m_mutex);
        m_lock.unlock();
    }
};
}  // namespace esp32
}  // namespace fortress
#endif