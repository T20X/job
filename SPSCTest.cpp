#pragma once

#include <thread>
#include <chrono>
#include "SPSCQueue.h"
#include <iostream>
#include <functional>

using namespace std;

    constexpr size_t N = 1000000;
    constexpr size_t Q_SIZE = 10;

    class WrapInt
    {
    public:
        WrapInt() = delete;
       // WrapInt(int input) { v = input;  }
        //WrapInt(const WrapInt& other) { v = other.v; }
       // void setValue(int input) { v = input; }
      //  int getValue() { return v;  }

        operator int() const { return v;  }
        
       // operator int& () { return v; }
        operator int* () { return &v; }

        WrapInt& operator= (int other) { v = other; return *this; }
       // WrapInt& operator= (const WrapInt& other) { v = other.v; return *this; }

    private:
        int v;
    };

    void producerSPSC(reference_wrapper<SPSCQueue<WrapInt, 8>>& q)
    {
        //WrapInt* item = nullptr;
        for (int i = 0; i < N; i++)
        {            
            q.get().update([](WrapInt& w) { *w = 1; });
        }
    }

    void consumerSPSC(reference_wrapper<SPSCQueue<WrapInt, 8>>& q)
    {
        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

        int count = 0;
        WrapInt* item = nullptr;
        for (int i = 0; i < N; i++)
        {
            q.get().scan([&count](const WrapInt& w) { count += w.operator int(); });
        }

        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        cout << "Took ->" << (double) std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() / N
             << " microseconds per item" << std::endl;

        cout << "Consumer result -> " << count << endl;
    }

    
    /*void producerSPSC(reference_wrapper<SPSCQueue<WrapInt, 8>>& q)
    {
        //WrapInt* item = nullptr;
        for (int i = 0; i < N; i++)
        {
            WrapInt& item = q.get().getForWrite();
            item = 2;
            q.get().notifyPush();
            //q.get().update([](WrapInt& w) { *w = 1; });
        }
    }

    void consumerSPSC(reference_wrapper<SPSCQueue<WrapInt, 8>>& q)
    {
        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

        int count = 0;
        WrapInt* item = nullptr;
        for (int i = 0; i < N; i++)
        {
            const WrapInt& w = q.get().getForRead();
            //([&count](const WrapInt& w) { count += w.operator int(); });
            count += w.operator int();
            q.get().notifyPop();
        }

        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        cout << "Took ->" << (double) std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() / N
             << " microseconds per item" << std::endl;

        cout << "Consumer result -> " << count << endl;
    }*/

    int main()
    {
        {
            SPSCQueue<WrapInt, 8> q;

            thread p(producerSPSC, ref(q));
            thread c(consumerSPSC, ref(q));

            p.join();
            c.join();
        }

        /*{
            SPSCQueue<OrderBook, 10> q;
            constexpr static int LEN = 10;

            thread p([&q]() {
               // OrderBook* item = nullptr;
                for (int i = 0; i < LEN; i++)
                {
                    q.update([i](OrderBook& item) {

                        item.price = i;
                        item.side = 'B';
                        item.volume = 2 * i;

                        string instrument("GOOOOOOOOOOOOOOOOOOO00000000000000000000000000000000000000000000000000000000000000OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOGLE");
                        for (int j = 0; j < i % 20; j++)
                            instrument += (char)(65 + j);

                        item.instrument = instrument;
                    });
                }
            });

            thread c([&q]() {
               // OrderBook* item = nullptr;
                for (int i = 0; i < LEN; i++)
                {
                    q.scan([](const OrderBook& item) {});                   
                    
                }
            });

            p.join();
            c.join();*/
     //   }
     //
    return 0;
    }
