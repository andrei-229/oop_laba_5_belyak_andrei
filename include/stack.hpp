// stack.hpp
// Simple single-linked stack container using std::pmr::polymorphic_allocator
// Each node is allocated separately using the provided memory_resource via
// polymorphic_allocator. Iterator models std::forward_iterator_tag.

#pragma once

#include <memory_resource>
#include <memory>
#include <cstddef>
#include <iterator>
#include <type_traits>

template <typename T>
class PmrStack
{
public:
    using value_type = T;

private:
    struct Node
    {
        T value;
        Node *next = nullptr;
        template <typename... Args>
        Node(Args &&...args) : value(std::forward<Args>(args)...) {}
    };

    using NodeAlloc = std::pmr::polymorphic_allocator<Node>;
    NodeAlloc alloc_;
    Node *head_ = nullptr;
    std::size_t sz_ = 0;

public:
    explicit PmrStack(std::pmr::memory_resource *mr = std::pmr::get_default_resource()) noexcept
        : alloc_(mr), head_(nullptr), sz_(0) {}

    ~PmrStack() noexcept
    {
        clear();
    }

    // non-copyable for simplicity (can be implemented later)
    PmrStack(const PmrStack &) = delete;
    PmrStack &operator=(const PmrStack &) = delete;

    PmrStack(PmrStack &&other) noexcept
        : alloc_(other.alloc_), head_(other.head_), sz_(other.sz_)
    {
        other.head_ = nullptr;
        other.sz_ = 0;
    }

    PmrStack &operator=(PmrStack &&other) noexcept
    {
        if (this != &other)
        {
            clear();
            alloc_ = other.alloc_;
            head_ = other.head_;
            sz_ = other.sz_;
            other.head_ = nullptr;
            other.sz_ = 0;
        }
        return *this;
    }

    // push copies
    void push(const T &value)
    {
        emplace(value);
    }

    void push(T &&value)
    {
        emplace(std::move(value));
    }

    template <typename... Args>
    void emplace(Args &&...args)
    {
        Node *node = std::allocator_traits<NodeAlloc>::allocate(alloc_, 1);
        try
        {
            std::allocator_traits<NodeAlloc>::construct(alloc_, node, std::forward<Args>(args)...);
        }
        catch (...)
        {
            std::allocator_traits<NodeAlloc>::deallocate(alloc_, node, 1);
            throw;
        }
        node->next = head_;
        head_ = node;
        ++sz_;
    }

    void pop()
    {
        if (!head_)
            return;
        Node *node = head_;
        head_ = head_->next;
        --sz_;
        std::allocator_traits<NodeAlloc>::destroy(alloc_, node);
        std::allocator_traits<NodeAlloc>::deallocate(alloc_, node, 1);
    }

    T &top()
    {
        return head_->value;
    }

    const T &top() const
    {
        return head_->value;
    }

    bool empty() const noexcept { return sz_ == 0; }
    std::size_t size() const noexcept { return sz_; }

    void clear() noexcept
    {
        while (head_)
            pop();
    }

    // iterator
    class iterator
    {
        Node *cur_ = nullptr;

    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = T *;
        using reference = T &;

        iterator() noexcept = default;
        explicit iterator(Node *p) noexcept : cur_(p) {}

        reference operator*() const noexcept { return cur_->value; }
        pointer operator->() const noexcept { return std::addressof(cur_->value); }

        iterator &operator++() noexcept
        {
            cur_ = cur_->next;
            return *this;
        }
        iterator operator++(int) noexcept
        {
            iterator tmp = *this;
            ++*this;
            return tmp;
        }

        friend bool operator==(const iterator &a, const iterator &b) noexcept { return a.cur_ == b.cur_; }
        friend bool operator!=(const iterator &a, const iterator &b) noexcept { return a.cur_ != b.cur_; }
    };

    iterator begin() noexcept { return iterator(head_); }
    iterator end() noexcept { return iterator(nullptr); }
};
