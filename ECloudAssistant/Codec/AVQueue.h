#ifndef AVQUEUE_H
#define AVQUEUE_H

#include <mutex>
#include <memory>
template<typename T>
class CAVList {
public:
    CAVList() : m_pHead(nullptr), m_pTail(nullptr), m_nSize(0) {}
    ~CAVList() { clear(); }
    void push(const T& value) {
        std::unique_lock<std::mutex> lock(m_mutex);
        Node* node = new Node(value);
        if (m_pTail) {
            m_pTail->next = node;
            node->prev = m_pTail;
            m_pTail = node;
        } else {
            m_pHead = m_pTail = node;
        }
        ++m_nSize;
    }

    bool bPop(T& value) {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (!m_pHead) {
            return false;
        }
        Node* node = m_pHead;
        value = node->value;
        m_pHead = m_pHead->next;
        if (m_pHead) {
            m_pHead->prev = nullptr;
        } else {
            m_pTail = nullptr;
        }
        delete node;
        --m_nSize;
        return true;
    }

    void clear() {
        std::unique_lock<std::mutex> lock(m_mutex);
        Node* node = m_pHead;
        while (node) {
            Node* next = node->next;
            delete node;
            node = next;
        }
        m_pHead = m_pTail = nullptr;
        m_nSize = 0;
    }
    bool bEmpty() const { return m_nSize == 0; }
    int nSize() const { return m_nSize; }

private:
    struct Node {
        T value;
        Node* prev;
        Node* next;
        Node(const T& v) : value(v), prev(nullptr), next(nullptr) {}
    };

private:
    Node* m_pHead;
    Node* m_pTail;
    int m_nSize;
    mutable std::mutex m_mutex;
};

template<typename T>
class CAVQueue
{
public:
    CAVQueue() {}
    void push(T packet) {
        m_queue.push(packet);
    }
    bool bPop(T& packet) {
        return m_queue.bPop(packet);
    }
    bool bEmpty() const { return m_queue.bEmpty(); }
    int nSize() const { return m_queue.nSize(); }
    void clear() {
        m_queue.clear();
    }

protected:
    CAVList<T> m_queue;
};

#endif // AVQUEUE_H
