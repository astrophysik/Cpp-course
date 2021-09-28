#pragma once

#include <cassert>
#include <cstddef>
#include <cstdio>
#include <functional>
#include <map>
#include <vector>

namespace pool {

class Pool
{
    const size_t m_block_size;
    std::vector<std::byte> m_storage;
    std::vector<size_t> m_list_sizes;
    std::unordered_map<size_t, std::vector<size_t>> m_free_blocks_map;

public:
    Pool(const size_t size, const std::initializer_list<size_t> initializerList);

    void deallocate(const void * ptr);

    void * allocate(size_t n);
};

Pool * create_pool(size_t size, std::initializer_list<size_t> initializerList);

void * allocate(Pool & pool, size_t n);

void deallocate(Pool & pool, const void * ptr);
} // namespace pool

class PoolAllocator
{
public:
    PoolAllocator(const std::size_t size, const std::initializer_list<std::size_t> sizes)
        : m_pool(pool::create_pool(size, sizes))
    {
    }

    ~PoolAllocator();

    void * allocate(std::size_t n);

    void deallocate(const void * ptr);

private:
    pool::Pool * m_pool;
};

inline void * PoolAllocator::allocate(const size_t n)
{
    return pool::allocate(*m_pool, n);
}

inline void PoolAllocator::deallocate(const void * ptr)
{
    pool::deallocate(*m_pool, ptr);
}
