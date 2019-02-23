#pragma once

#include <vector>
#include <atomic>
#include <array>

#if LINUX > 0
    #define _relax_ __asm__ __volatile("pause" ::: "memory")
#else
    #define _relax_ _mm_pause()
#endif

struct SPSCQueueParams
{
    enum CallMode
    {
        BLOCKING,
        NON_BLOCKING
    };
};

template <typename T, size_t N>
class SPSCQueue final : SPSCQueueParams
{
public:
    SPSCQueue() = default;    
    SPSCQueue(const SPSCQueue&) = delete;
    SPSCQueue(SPSCQueue&&) = delete;
    
    // only called by producer - blocking way!
    T& getForWrite()
    {
        while (1)
        {
            bool available = !queue[writePos].locked()->load(
                std::memory_order_acquire);           
            if (available)
            {                
                return reinterpret_cast<T&>(
                    queue[writePos].raw);
            }
            else
            {
                _relax_;
            }
        }
    }

    // only called by producer 
    template <CallMode m = CallMode::BLOCKING, 
              typename F>
    bool produce(F p)
    {
        while (1)
        {
            bool available = !queue[writePos].locked()->load(
                std::memory_order_acquire);
            if (available)
            {
                p(reinterpret_cast<T&>(
                    queue[writePos].raw));
                notifyPush();
                return true;
            }

            if constexpr (m == CallMode::BLOCKING)  _relax_;
               else return false;
            
        }
    }

   /* Producer must first call getForWrite in order
    * to retrieve a free item for editing and only
    * then call the notifyPush method
    */
    void notifyPush()
    {        
        queue[writePos].locked()->store(true, std::memory_order_release);
        ++writePos; writePos &= (N - 1);
    }

    // only called by consumer - blocking way!
    const T& getForRead()
    {
        while (1)
        {
            bool available = queue[readPos].locked()->load(
                std::memory_order_acquire);            
            if (available)
            {
                return reinterpret_cast<const T&>(
                    queue[readPos].raw);
            }
            else
            {
                _relax_;                
            }
        }
    }

    // only called by consumer - non blocking way!
    template <CallMode m = CallMode::BLOCKING,
              typename F>
    bool consume(F c)
    {
        while (1)
        {
            bool available = queue[readPos].locked()->load(
                std::memory_order_acquire);
            if (available)
            {
                c(reinterpret_cast<const T&>(
                    queue[readPos].raw));
                notifyPop();
                return true;
            }

            if constexpr (m == CallMode::BLOCKING)  _relax_;
               else return false;
        }
    }

    // only called by consumer - non blocking way!
    using Buffer = std::array<T, N>;
    size_t drainByCopy(Buffer& buffer)
    {
        size_t index = 0;
        while (1)
        {
            bool available = queue[readPos].locked()->load(
                std::memory_order_acquire);
            if (available && index < N)
            {
                buffer[index++] = std::move_if_noexcept(reinterpret_cast<T&>(
                    queue[readPos].raw));                
                notifyPop();
            }
            else
            {
                break;
            }                       
        }

        return index;
    }

   /* Consumer must first call getForWrite in order
    * to retrieve a free item for editing and only
    * then call the push method
    */

    void notifyPop()
    {        
        queue[readPos].locked()->store(false, std::memory_order_release);
        ++readPos; readPos &= (N - 1);            
    } 

private:  
    struct Item
    {
        bool lock = 0;  
        char raw[sizeof(T)];

        std::atomic<bool>* locked() noexcept
        {
            return reinterpret_cast<std::atomic<bool>*>(
                &this->lock);
        }
    };
    
    std::array<Item, N> queue;
    alignas(64) size_t readPos = 0;
    alignas(64) size_t writePos = 0;    
};



