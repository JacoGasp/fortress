#ifndef ASYNC_SYNCHRONIZATION
#define ASYNC_SYNCHRONIZATION

#include <Arduino.h>
// #include <freertos/FreeRTOS.h>

class AsyncPlainLock {
   private:
    SemaphoreHandle_t _lock;

   public:
    AsyncPlainLock() {
        _lock = xSemaphoreCreateBinary();
        // In this fails, the system is likely that much out of memory that
        // we should abort anyways. If assertions are disabled, nothing is lost..
        assert(_lock);
        xSemaphoreGive(_lock);
    }

    ~AsyncPlainLock() { 
        unlock();
        vSemaphoreDelete(_lock); }

    bool lock() const {
        xSemaphoreTake(_lock, portMAX_DELAY);
        return true;
    }

    void unlock() const { xSemaphoreGive(_lock); }
};

using AsyncLock = AsyncPlainLock;
class AsyncLockGuard {
   private:
    const AsyncLock *_lock;

   public:
    AsyncLockGuard(const AsyncLock &l) {
        if (l.lock()) {
            _lock = &l;
        } else {
            _lock = nullptr;
        }
    }

    ~AsyncLockGuard() {
        if (_lock) {
            _lock->unlock();
        }
    }
};

#endif