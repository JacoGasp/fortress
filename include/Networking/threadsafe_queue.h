//
// Created by Jacopo Gasparetto on 19/05/21.
//

#ifndef FORTRESS_THREADSAFE_QUEUE_H
#define FORTRESS_THREADSAFE_QUEUE_H

#include "commons.h"
#include "message.h"

namespace fortress::net {
    template<typename T>
    class threadSafeQueue {

    protected:
        std::mutex m_mutex;
        std::deque<T> m_deque;

    public:
        threadSafeQueue() = default;

        // Prevent the queue to be copied because probably it already changed in the thread
        threadSafeQueue(const threadSafeQueue<T> &) = delete;

        virtual ~threadSafeQueue() { clear(); }

        const T& front() {
            std::scoped_lock lock(m_mutex);
            return m_deque.front();
        }

        const T& back() {
            std::scoped_lock lock(m_mutex);
            return m_deque.back();
        }

        T pop_front() {
            std::scoped_lock lock(m_mutex);
            auto t = std::move(m_deque.front());     // Cache the item
            m_deque.pop_front();
            return t;
        }

        T pop_back() {
            std::scoped_lock lock(m_mutex);
            auto t = std::move(m_deque.back());     // Cache the item
            m_deque.pop_back();
            return t;
        }

        void push_front(const T& item) {
            std::scoped_lock lock(m_mutex);
            m_deque.template emplace_front(std::move(item));
        }

        void push_back(const T& item) {
            std::scoped_lock lock(m_mutex);
            m_deque.template emplace_back(std::move(item));
        }

        bool empty() {
            std::scoped_lock lock(m_mutex);
            return m_deque.empty();
        }

        size_t count() {
            std::scoped_lock lock(m_mutex);
            return m_deque.size();
        }

        void clear() {
            std::scoped_lock lock(m_mutex);
            m_deque.clear();
        }
    };
}

#endif //FORTRESS_THREADSAFE_QUEUE_H
