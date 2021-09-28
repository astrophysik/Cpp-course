#pragma once

#include <algorithm>
#include <cstddef>
#include <deque>
#include <iostream>
#include <list>
#include <new>
#include <ostream>

template <class Key, class KeyProvider, class Allocator>
class Cache
{
public:
    template <class... AllocArgs>
    Cache(const std::size_t cache_size, AllocArgs &&... alloc_args)
        : m_max_top_size(cache_size)
        , m_max_low_size(cache_size)
        , m_alloc(std::forward<AllocArgs>(alloc_args)...)
        , m_top(0)
        , m_low(0)

    {
    }

    std::size_t size() const
    {
        return m_low.size() + m_top.size();
    }

    bool empty() const
    {
        return m_low.empty() && m_top.empty();
    }

    template <class T>
    T & get(const Key & key);

    std::ostream & print(std::ostream & strm) const;

    friend std::ostream & operator<<(std::ostream & strm, const Cache & cache)
    {
        return cache.print(strm);
    }

private:
    const std::size_t m_max_top_size;
    const std::size_t m_max_low_size;
    Allocator m_alloc;
    std::deque<KeyProvider *> m_top;
    std::deque<KeyProvider *> m_low;
    static constexpr size_t npos = static_cast<size_t>(-1);
    size_t finding_number_in(const Key & key, std::deque<KeyProvider *>) const;
};

template <class Key, class KeyProvider, class Allocator>
size_t Cache<Key, KeyProvider, Allocator>::finding_number_in(const Key & key, const std::deque<KeyProvider *> lst) const
{
    for (size_t k = 0; k < lst.size(); k++) {
        if (*lst[k] == key) {
            return k;
        }
    }
    return npos;
}

template <class Key, class KeyProvider, class Allocator>
template <class T>
inline T & Cache<Key, KeyProvider, Allocator>::get(const Key & key)
{
    size_t it = finding_number_in(key, m_top);
    if (it != npos) {
        m_top.push_front(m_top[it]);
        m_top.erase(m_top.begin() + it + 1);
        return *static_cast<T *>(m_top.front());
    }
    else {
        it = finding_number_in(key, m_low);
        if (it == npos) {
            if (m_low.size() == m_max_low_size) {
                m_alloc.template destroy<KeyProvider>(m_low.back());
                m_low.pop_back();
            }
            m_low.push_front(m_alloc.template create<T>(key));
            return *static_cast<T *>(m_low.front());
        }
        else {
            m_top.push_front(m_low[it]);
            m_low.erase(m_low.begin() + it);
            if (m_top.size() > m_max_top_size) {
                m_low.push_front(m_top.back());
                m_top.pop_back();
            }
            if (m_low.size() > m_max_low_size) {
                m_low.pop_back();
            }
            return *static_cast<T *>(m_top.front());
        }
    }
}

template <class Key, class KeyProvider, class Allocator>
inline std::ostream & Cache<Key, KeyProvider, Allocator>::print(std::ostream & strm) const
{
    return strm << "Priority: <empty>"
                << "\nRegular: <empty>"
                << "\n";
}
