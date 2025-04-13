#include <atomic>
#include <condition_variable>
#include <memory>
#include <queue>

// template <typename T>
// class LockFreeQueue {
// private:
//     struct Node {
//         T data;
//         std::atomic<std::shared_ptr<Node>> next;
//         Node(T const& data) : data(data), next(nullptr) {}
//     };

//     std::atomic<std::shared_ptr<Node>> head;
//     std::atomic<std::shared_ptr<Node>> tail;

// public:
//     LockFreeQueue() : head(std::make_shared<Node>(T())), tail(head.load()) {}

//     void push(T const& value)
//     {
//         auto new_node = std::make_shared<Node>(value);
//         while (true) {
//             auto tail_node = tail.load();
//             auto next_node = tail_node->next;
//             if (tail_node == tail.load()) {
//                 if (next_node == nullptr) {
//                     if (tail_node->next.compare_exchange_strong(next_node, new_node)) {
//                         tail.compare_exchange_strong(tail_node, new_node);
//                         return;
//                     }
//                 }
//                 else {
//                     tail.compare_exchange_strong(tail_node, next_node);
//                 }
//             }
//         }
//     }

//     std::shared_ptr<T> pop()
//     {
//         while (true) {
//             auto head_node = head.load();
//             auto tail_node = tail.load();
//             auto next_node = head_node->next;

//             if (head_node == head.load()) {
//                 if (head_node == tail_node) {
//                     if (next_node == nullptr) {
//                         return nullptr;
//                     }
//                     tail.compare_exchange_strong(tail_node, next_node);
//                 }
//                 else {
//                     if (next_node == nullptr) {
//                         continue;
//                     }
//                     head.compare_exchange_strong(head_node, next_node);
//                     return std::make_shared<T>(next_node->data);
//                 }
//             }
//         }
//     }

//     bool try_pop(T& value)
//     {
//         while (true) {
//             auto head_node = head.load();
//             auto tail_node = tail.load();
//             auto next_node = head_node->next;

//             if (head_node == head.load()) {
//                 if (head_node == tail_node) {
//                     if (next_node == nullptr) {
//                         return false;
//                     }
//                     tail.compare_exchange_strong(tail_node, next_node);
//                 }
//                 else {
//                     if (head.compare_exchange_strong(head_node, next_node)) {
//                         return true;
//                     }
//                 }
//             }
//         }
//     }

//     [[nodiscard]] bool empty() const { return head.load()->next.load() == nullptr; }

//     [[nodiscard]] size_t size() const
//     {
//         size_t count = 0;
//         Node* node = head.load();
//         while (node->next.load() != nullptr) {
//             node = node->next.load();
//             count++;
//         }
//         return count;
//     }
// };

template <typename T>
class LockFreeQueue {
private:
    std::mutex m_mutex;
    std::queue<T> m_dataQueue;
    std::condition_variable m_dataCondVar;

public:
    LockFreeQueue() {}
    LockFreeQueue(LockFreeQueue const& copy)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_dataQueue = copy.m_dataQueue;
    }

    void push(T const& val)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_dataQueue.push(val);
        m_dataCondVar.notify_one();
    }

    void wait_and_pop(T& val)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_dataCondVar.wait([this]() { return !m_dataQueue.empty(); });

        val = m_dataQueue.front();
        m_dataQueue.pop();
    }

    T WaitAndPop()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_dataCondVar.wait([this]() { return !m_dataQueue.empty(); });

        T val = m_dataQueue.front();
        m_dataQueue.pop();

        return val;
    }

    bool try_pop(T& val)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_dataQueue.empty()) {
            return false;
        }

        val = m_dataQueue.front();
        m_dataQueue.pop();

        return true;
    }

    T try_pop()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_dataQueue.empty()) {
            return std::shared_ptr<T>();
        }

        T val = m_dataQueue.front();
        m_dataQueue.pop();
        return val;
    }

    bool empty()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_dataQueue.empty();
    }

    unsigned int size()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return static_cast<unsigned int>(m_dataQueue.size());
    }
};