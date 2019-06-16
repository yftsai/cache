#include <unordered_map>
#include <vector>
#include <random>

#pragma once

template<typename K, typename V>
class unordered_cache
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
        std::unordered_map<K, value_entry> storage;
        std::vector<K> keys;

        void evict(typename std::unordered_map<K, value_entry>::iterator it);

    public:
        unordered_cache(size_t size):
            max_capacity(size),
            clock(0)
        {
            keys.reserve(size);
        }

        void insert(const K &key, const V &value);
        const V* find(const K &key);
        void evict();
        size_t evict(const K &key, V *value = nullptr);
};

template<typename K, typename V>
void unordered_cache<K, V>::insert(const K &key, const V &value)
{
    auto it = storage.find(key);
    if (it != storage.end())
        it->second.time = (clock++);
    else {
        if (storage.size() >= max_capacity)
            evict();
        storage.emplace(key, value_entry{ clock++, keys.size(), value});
        keys.push_back(key);
    }
}

template<typename K, typename V>
const V* unordered_cache<K, V>::find(const K &key)
{
    auto it = storage.find(key);
    if (it == storage.end())
        return nullptr;
    else {
        it->second.time = (clock++);
        return &it->second.value;
    }
}

template<typename K, typename V>
void unordered_cache<K, V>::evict(typename std::unordered_map<K, value_entry>::iterator it)
{
    const size_t index = it->second.index;
    storage.erase(it);
    keys[index] = std::move(keys.back());
    keys.pop_back();
    storage.find(keys[index])->second.index = index;
}

template<typename K, typename V>
void unordered_cache<K, V>::evict()
{
    if (keys.size() > 0) {
        std::uniform_int_distribution<size_t> distribution(0, keys.size() - 1u);
        auto p = storage.find(keys[distribution(engine)]);
        auto q = storage.find(keys[distribution(engine)]);

        if (p->second.time <= q->second.time)
            evict(p);
        else
            evict(q);
    }
}

template<typename K, typename V>
size_t unordered_cache<K, V>::evict(const K &key, V *value)
{
    auto it = storage.find(key);
    if (it == storage.end())
        return 0;
    else {
        if (value != nullptr)
            *value = std::move(it->second.value);

        evict(it);
        return 1;
    }
}
