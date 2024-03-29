#include <map>
#include <vector>
#include <random>

#pragma once

template<typename K, typename V>
class cache
{
    private:
        struct value_entry
        {
            uint64_t time;
            size_t index;
            V value;
        };

        const size_t max_capacity;
        std::default_random_engine engine;
        uint64_t clock;
        std::map<K, value_entry> storage;
        std::vector<typename std::map<K, value_entry>::iterator> iters;

        void evict(typename std::map<K, value_entry>::iterator it);

    public:
        cache(size_t size):
            max_capacity(size),
            clock(0)
        {
            iters.reserve(size);
        }

        void insert(const K &key, const V &value);
        const V* find(const K &key);
        void evict();
        size_t evict(const K &key, V *value = nullptr);
};

template<typename K, typename V>
void cache<K, V>::insert(const K &key, const V &value)
{
    auto it = storage.lower_bound(key);
    if (it != storage.end() && it->first == key)
        it->second.time = (clock++);
    else {
        if (storage.size() >= max_capacity)
            evict();
        it = storage.emplace_hint(it, key, value_entry{ clock++, iters.size(), value});
        iters.push_back(it);
    }
}

template<typename K, typename V>
const V* cache<K, V>::find(const K &key)
{
    auto it = storage.lower_bound(key);
    if (it == storage.end() || it->first != key)
        return nullptr;
    else {
        it->second.time = (clock++);
        return &it->second.value;
    }
}

template<typename K, typename V>
void cache<K, V>::evict(typename std::map<K, value_entry>::iterator it)
{
    const size_t index = it->second.index;
    storage.erase(it);
    iters[index] = std::move(iters.back());
    iters.pop_back();
    iters[index]->second.index = index;
}

template<typename K, typename V>
void cache<K, V>::evict()
{
    if (iters.size() > 0) {
        std::uniform_int_distribution<size_t> distribution(0, iters.size() - 1);
        auto p = iters[distribution(engine)];
        auto q = iters[distribution(engine)];
        if (p->second.time <= q->second.time)
            evict(p);
        else
            evict(q);
    }
}

template<typename K, typename V>
size_t cache<K, V>::evict(const K &key, V *value)
{
    auto it = storage.lower_bound(key);
    if (it == storage.end() || it->first != key)
        return 0;
    else {
        if (value != nullptr)
            *value = std::move(it->second.value);

        evict(it);
        return 1;
    }
}
