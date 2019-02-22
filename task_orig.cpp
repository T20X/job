#include <array>
#include <chrono>
#include <cstdint>
#include <iostream>

// Test config
static const uint64_t INTERVAL = 1000; // delay between produced messages (ns)
static const uint64_t MESSAGE_COUNT = 50000000;   // test size
static const uint64_t STATS_SAMPLE_SIZE = 100000; // sample size for stats
static const int MAX_INPUT = 10000;

// Sample Implementation
template <typename Input, typename Output>
void stream_processor(Input &rx, Output &tx) {
  // This is the state we'd like to persist
  std::array<int, MAX_INPUT> state = {0};
  int tmp;

  while (rx(&tmp, 1) > 0) {
    state[tmp] += 1 + state[(tmp + 1) % MAX_INPUT];
    tx(&state[tmp], 1);
  }
}

// High Resolution Clock
using hrc = std::chrono::time_point<std::chrono::high_resolution_clock>;

inline auto hrc_now() { return std::chrono::high_resolution_clock::now(); }

inline auto hrc_elapsed(hrc start) {
  return std::max(
      std::chrono::duration_cast<std::chrono::nanoseconds>(hrc_now() - start)
          .count(),
      0L);
}

// Benchmark Framework
class generator {
  hrc start_;
  uint64_t sent_ = 0;
  uint64_t value_ = 0xf00533d;

public:
  generator(hrc start) : start_(start) {}

  int operator()(int *buffer, int count) {
    if (sent_ == MESSAGE_COUNT) {
      return 0; // eof
    }
    int num_rx = 0;
    do { // busy-wait
      int avail = (hrc_elapsed(start_) / INTERVAL) + 1 - sent_;
      num_rx = std::min(avail, count);
    } while (num_rx <= 0);
    // fill data
    for (int i = 0; i < num_rx; ++i) {
      value_ += (++sent_) + 1;
      buffer[i] = abs(value_) % MAX_INPUT;
    }
    return num_rx;
  }
};

class sink {
  hrc start_;
  uint64_t count_ = 0;

  hrc stats_start_;
  double avg_latency = 0;
  double stats_count_ = 0;

public:
  sink(hrc start) : start_(start), stats_start_(start) {}

  void operator()(const int * /*send_buffer*/, int count) {

    double latency =
        hrc_elapsed(start_ + std::chrono::nanoseconds(INTERVAL) * count_);

    stats_count_ += count;
    avg_latency = avg_latency  * 
	         ((stats_count_ - count)/ stats_count_) + 
		 latency / stats_count_; 

      uint64_t avg_latency = avg_latency;
      auto throughput =
          (stats_count_ * 1'000'000'000) / (hrc_elapsed(stats_start_));

      std::cout << stats_count_ << " messages: avg latency: " << avg_latency
                << "ns; throughput: " << throughput << " msgs/sec;"
                << " last ack size: " << count << "\n";
      // reset
      stats_start_ = hrc_now();
      avg_latency = 0;
      stats_count_ = 0;
    }
	
    count_ += count;
  }
};

int main() {
  // Test
  auto start_time = hrc_now() + std::chrono::seconds(1);
  auto input = generator(start_time);
  auto output = sink(start_time);
  stream_processor(input, output);
}
