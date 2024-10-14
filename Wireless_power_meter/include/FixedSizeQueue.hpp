#ifndef FixedSizeQueue_HPP
#define FixedSizeQueue_HPP



#include <queue>
template <typename T, size_t N>
class FixedSizeQueue {
private:
    std::queue<T> queue;
    size_t capacity;

public:
    FixedSizeQueue() : capacity(N) {}

    void push(const T& value) {
        if (queue.size() == capacity) {
            queue.pop(); // 移除最早的元素
        }
        queue.push(value); // 添加新元素
    }

    bool isEmpty() const {
        return queue.empty();
    }

    size_t size() const {
        return queue.size();
    }
    std::queue<T> copy(){
        return queue;
    }
    T average() const {
        T sum = 0;
        for (const T& value : queue) {
            sum += value;
        }
        return sum / queue.size();
    }

    T max() const {
        std::queue<T> copy = queue;
        T max;
        while (!copy.empty()) {
            T value = copy.front();
            copy.pop();
            if (value > max) {
                max = value;
            }
        }
        return max;
    }

    T min() const {
        std::queue<T> copy = queue;
        T min;
        while (!copy.empty()) {
            T value = copy.front();
            copy.pop();
            if (value < min) {
                min = value;
            }
        }
        return min;
    }
    T* toArray(){
        std::queue<T> copy = queue;
        size_t index = 0;
        while (!copy.empty()) {
            array[index++] = copy.front();
            copy.pop();
        }
        if (index < N){
            for (size_t i = index; i < N; i++)
            {
                array[i] = 0;
            }
        }
        return array;
    }
    private:
    T* array = new T[N];
};
#endif // FixedSizeQueue_HPP
