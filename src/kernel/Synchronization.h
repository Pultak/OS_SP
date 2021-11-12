#pragma once
#include <atomic>    
#include <thread>         
#include <vector> 
#include <cassert>
#include <mutex>

namespace Synchronization{

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

}

