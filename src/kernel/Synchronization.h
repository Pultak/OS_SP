#pragma once
#include <atomic>    
#include <thread>         
#include <vector> 
#include <cassert>
#include <mutex>
#include "../api/api.h"

namespace Synchronization {

    class Spinlock {

    private:
        std::atomic<bool> lockVal = { 0 };
    public:
        Spinlock(bool locked) {
            lockVal = locked;
        }

        void lock() noexcept {
            for (;;) {
                if (!lockVal.exchange(true, std::memory_order_acquire)) {
                    return;
                }
                /*while (lockVal.load(std::memory_order_relaxed)) {
#if defined(USE_mm_pause)
                    //platform specific pauses that should be faster
                    __asm__("pause;");
                    //_mm_pause();
                    //__builtin_ia32_pause();
#else
                    std::this_thread::sleep_for(std::chrono::seconds(0));
#endif

                }*/
            }
        }

        bool try_lock() noexcept {
            return !lockVal.load(std::memory_order_relaxed) &&
                !lockVal.exchange(true, std::memory_order_acquire);
        }

        void unlock() noexcept {
            lockVal.store(false, std::memory_order_release);
        }
    };


    class IntSpinlock {

    public:
        std::atomic<size_t> lockVal = { 0 };
    public:
        explicit IntSpinlock(size_t val) {
            lockVal = val;
        }

        void lock() noexcept {
            for (;;) {
                if (lockVal.load(std::memory_order_relaxed)) {
                    lockVal.fetch_sub(1, std::memory_order_acquire);
                    return;
                }
                /*while (lockVal.load(std::memory_order_relaxed)) {
                    std::this_thread::sleep_for(std::chrono::seconds(0));
                }*/
            }
        }

        bool try_lock() noexcept {
            return !lockVal.load(std::memory_order_relaxed) &&
                !lockVal.exchange(true, std::memory_order_acquire);
        }

        void unlock(size_t val) noexcept {
            lockVal.fetch_add(val, std::memory_order_release);
        }
    };

    class Semaphore {
    public:
        explicit Semaphore(kiv_os::THandle sleepHandle) {
            sleeperHandle = sleepHandle;
            count = 0;
        }

        inline void notify()
        {
            std::unique_lock<std::mutex> lock(mtx);
            count++;
            cv.notify_one();
        }

        inline void wait()
        {
            std::unique_lock<std::mutex> lock(mtx);
            while (count == 0) {
                cv.wait(lock);
            }
            count--;
        }

        std::mutex mtx; 


        /// <summary>
        /// Handle of the locked thread
        /// </summary>
        kiv_os::THandle sleeperHandle;

        /// <summary>
        /// Handle of the thread that notified the locked thread
        /// </summary>
        kiv_os::THandle notifierHandle = kiv_os::Invalid_Handle;
        /// <summary>
        /// Flag if the listener was already notified
        /// </summary>
        bool notified = false;
    private:

        std::condition_variable cv;
        size_t count;
    };


};

