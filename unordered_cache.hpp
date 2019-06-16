#include <unordered_map>
#include <random>

#pragma once

template<typename K, typename V>
class unordered_cache
{
    private:
        struct value_entry
        {
            uint64_t time;
            V value;
        };

        const size_t max_capacity;
        std::default_random_engine engine;
        uint64_t clock;
        std::unordered_map<K, value_entry> storage;

    public:
        unordered_cache(size_t size):
            max_capacity(size),
            clock(0)
        {}

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
        storage.emplace(key, value_entry{ clock++, value});
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
void unordered_cache<K, V>::evict()
{
    if (storage.size() > 0) {
        std::uniform_int_distribution<size_t> distribution(0, storage.bucket_count() - 1u);
        size_t i = distribution(engine);
        while (storage.begin(i) == storage.end(i))
            (++i) %= storage.bucket_count();

        do {
            size_t j = distribution(engine);
            while (storage.begin(j) == storage.end(j))
                (++j) %= storage.bucket_count();
            if (storage.begin(i)->second.time > storage.begin(j)->second.time)
                i = j;
        } while (false);

        storage.erase(storage.begin(i)->first);
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

        storage.erase(it);
        return 1;
    }
}
