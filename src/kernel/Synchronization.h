#pragma once
#include <atomic>    
#include <thread>         
#include <vector> 
#include <cassert>
#include <mutex>

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
                while (lockVal.load(std::memory_order_relaxed)) {
#if defined(USE_mm_pause)
                    //platform specific pauses that should be faster
                    __asm__("pause;");
                    //_mm_pause();
                    //__builtin_ia32_pause();
#else
                    std::this_thread::sleep_for(std::chrono::seconds(0));
#endif

                }
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

    private:
        std::atomic<uint16_t> lockVal = { 0 };
    public:
        explicit IntSpinlock(uint16_t val) {
            lockVal = val;
        }

        void lock() noexcept {
            for (;;) {
                if (lockVal.fetch_sub(1, std::memory_order_acquire)) {
                    //lockVal--;
                    return;
                }
                while (lockVal.load(std::memory_order_relaxed)) {
                    std::this_thread::sleep_for(std::chrono::seconds(0));
                }
            }
        }

        bool try_lock() noexcept {
            return !lockVal.load(std::memory_order_relaxed) &&
                !lockVal.exchange(true, std::memory_order_acquire);
        }

        void unlock(uint16_t val) noexcept {
            lockVal.fetch_add(val, std::memory_order_release);
        }
    };


};

