#pragma once

#include <vector>
#include <mutex>
#include <utility>

namespace Magnum {

/**
 * Thread-safe multi-producer / single-consumer queue.
 * Optimized for batch operations using vector swap to minimize lock contention.
 */
template<typename T>
class ConcurrentQueue {
    public:
        ConcurrentQueue() = default;

        void push(T item) {
            std::lock_guard<std::mutex> lock(_mutex);
            _queue.push_back(std::move(item));
        }

        void pushBatch(std::vector<T>&& items) {
            if(items.empty()) return;
            std::lock_guard<std::mutex> lock(_mutex);
            _queue.insert(_queue.end(),
                          std::make_move_iterator(items.begin()),
                          std::make_move_iterator(items.end()));
            items.clear();
        }

        void drain(std::vector<T>& outBatch) {
            std::lock_guard<std::mutex> lock(_mutex);
            outBatch.swap(_queue);
            _queue.clear();
        }

        bool empty() const {
            std::lock_guard<std::mutex> lock(_mutex);
            return _queue.empty();
        }

        std::size_t size() const {
            std::lock_guard<std::mutex> lock(_mutex);
            return _queue.size();
        }

    private:
        mutable std::mutex _mutex;
        std::vector<T> _queue;
};

}
