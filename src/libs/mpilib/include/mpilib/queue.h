#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

namespace mpilib {

    template<
            class T,
            class Container = std::vector<T>,
            class Compare = std::less<typename Container::value_type>
    >
    class PriorityQueue : public std::priority_queue<T, Container, Compare> {
    public:
        PriorityQueue() : std::priority_queue<T, Container, Compare>(Compare(), Container()) { }
        explicit PriorityQueue(const Compare &compare) : std::priority_queue<T, Container, Compare>(compare, Container()) {}

        /**
         * When modifying the internal container, be very careful
         * not to modify the values used in the comparison function.
         * @return The internal container.
         */
        Container &get_container() {
            return this->c;
        }
    };

    template<typename T>
    class Queue {
    public:
        T pop();

        void pop(T &item);

        void push(const T &item);

        void push(T &&item);

    private:
        std::queue<T> queue_;
        std::mutex mutex_;
        std::condition_variable cond_;
    };

    template<typename T>
    T Queue<T>::pop() {
        std::unique_lock<std::mutex> mlock(mutex_);

        while (queue_.empty()) {
            cond_.wait(mlock);
        }

        auto item = std::move(queue_.front());
        queue_.pop();
        return item;
    }

    template<typename T>
    void Queue<T>::pop(T &item) {
        std::unique_lock<std::mutex> mlock(mutex_);

        while (queue_.empty()) {
            cond_.wait(mlock);
        }

        item = std::move(queue_.front());
        queue_.pop();
        return item;
    }

    template<typename T>
    void Queue<T>::push(const T &item) {
        std::unique_lock<std::mutex> mlock(mutex_);
        queue_.push(std::move(item));
        mlock.unlock();
        cond_.notify_one();
    }

    template<typename T>
    void Queue<T>::push(T &&item) {
        std::unique_lock<std::mutex> mlock(mutex_);
        queue_.push(std::move(item));
        mlock.unlock();
        cond_.notify_one();
    }

}
