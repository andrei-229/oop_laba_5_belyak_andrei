#pragma once

#include <memory_resource>
#include <vector>
#include <cstddef>
#include <new>
#include <algorithm>
#include <cassert>

class ReusingMemoryResource : public std::pmr::memory_resource
{
private:
    struct Block
    {
        void *ptr = nullptr;
        std::size_t size = 0;
        std::size_t alignment = alignof(std::max_align_t);
    };

    std::vector<Block> in_use_;
    std::vector<Block> free_blocks_;

    static void *raw_alloc(std::size_t bytes, std::size_t alignment)
    {
        if (alignment > alignof(std::max_align_t))
        {
            return ::operator new(bytes, std::align_val_t(alignment));
        }
        return ::operator new(bytes);
    }

    static void raw_dealloc(void *p, std::size_t bytes, std::size_t alignment) noexcept
    {
        (void)bytes;
        if (!p)
            return;
        if (alignment > alignof(std::max_align_t))
        {
            ::operator delete(p, std::align_val_t(alignment));
        }
        else
        {
            ::operator delete(p);
        }
    }

protected:
    void *do_allocate(std::size_t bytes, std::size_t alignment) override
    {
        for (auto it = free_blocks_.begin(); it != free_blocks_.end(); ++it)
        {
            if (it->size >= bytes && it->alignment >= alignment)
            {
                Block b = *it;
                free_blocks_.erase(it);
                in_use_.push_back(b);
                return b.ptr;
            }
        }

        void *p = raw_alloc(bytes, alignment);
        Block nb;
        nb.ptr = p;
        nb.size = bytes;
        nb.alignment = alignment;
        in_use_.push_back(nb);
        return p;
    }

    void do_deallocate(void *p, std::size_t bytes, std::size_t alignment) override
    {
        auto it = std::find_if(in_use_.begin(), in_use_.end(),
                               [p](const Block &b)
                               { return b.ptr == p; });
        if (it != in_use_.end())
        {
            Block b = *it;
            in_use_.erase(it);
            if (b.size == 0)
                b.size = bytes;
            if (b.alignment == 0)
                b.alignment = alignment;
            free_blocks_.push_back(b);
        }
        else
        {
            raw_dealloc(p, bytes, alignment);
        }
    }

    bool do_is_equal(const std::pmr::memory_resource &other) const noexcept override
    {
        return this == &other;
    }

public:
    ReusingMemoryResource() = default;

    ~ReusingMemoryResource() override
    {
        for (auto &b : in_use_)
        {
            raw_dealloc(b.ptr, b.size, b.alignment);
        }
        for (auto &b : free_blocks_)
        {
            raw_dealloc(b.ptr, b.size, b.alignment);
        }
    }

    std::size_t in_use_count() const noexcept { return in_use_.size(); }
    std::size_t free_count() const noexcept { return free_blocks_.size(); }
};
