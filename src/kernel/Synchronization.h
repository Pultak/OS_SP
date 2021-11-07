#pragma once
#include <atomic>    
#include <thread>         
#include <vector> 
#include <cassert>
#include <mutex>

struct spinlock {
    std::atomic<bool> lock = { 0 };

    void lock() noexcept {
        for (;;) {
            // Optimistically assume the lock is free on the first try
            if (!lock.exchange(true, std::memory_order_acquire)) {
                return;
            }
            // Wait for lock to be released without generating cache misses
            while (lock.load(std::memory_order_relaxed)) {
                // Issue X86 PAUSE or ARM YIELD instruction to reduce contention between
                // hyper-threads

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
        // First do a relaxed load to check if lock is free in order to prevent
        // unnecessary cache misses if someone does while(!try_lock())
        return !lock.load(std::memory_order_relaxed) &&
            !lock.exchange(true, std::memory_order_acquire);
    }

    void unlock() noexcept {
        lock.store(false, std::memory_order_release);
    }
};



class Synchronization{

private:
#if defined(USE_SPINLOCK)

#else


#endif


public:


};

