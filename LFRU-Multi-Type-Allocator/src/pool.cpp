#include "pool.h"

namespace pool {

Pool::Pool(const size_t size, const std::initializer_list<size_t> initializerList)
    : m_block_size(size)
    , m_storage(size * initializerList.size())
    , m_list_sizes(initializerList)
{
    std::vector<size_t> sizes_list(initializerList);
    for (size_t k = 0; k < sizes_list.size(); k++) {
        auto buffer = std::vector<size_t>(m_block_size / sizes_list[k]);
        for (size_t i = 0; i < buffer.size(); i++) {
            buffer[i] = m_block_size * k + sizes_list[k] * i;
        }
        m_free_blocks_map.insert({sizes_list[k], buffer});
    }
}

void * Pool::allocate(const size_t n)
{
    if (m_free_blocks_map[n].empty()) {
        throw std::bad_alloc{};
    }
    size_t temp = m_free_blocks_map[n].back();
    m_free_blocks_map[n].pop_back();
    return &m_storage[temp];
}

void Pool::deallocate(const void * ptr)
{
    size_t k = static_cast<const std::byte *>(ptr) - &m_storage[0];
    if (k < m_storage.size()) {
        m_free_blocks_map[m_list_sizes[k / m_block_size]].push_back(k);
    }
}

Pool * create_pool(const size_t size, const std::initializer_list<size_t> initializerList)
{
    return new Pool(size, initializerList);
}

void * allocate(Pool & pool, size_t n)
{
    return pool.allocate(n);
}

void deallocate(Pool & pool, const void * ptr)
{
    pool.deallocate(ptr);
}
} // namespace pool

PoolAllocator::~PoolAllocator()
{
    delete m_pool;
}