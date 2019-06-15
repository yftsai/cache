#include <map>
#include <vector>
#include <random>

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
        void evict(const K &key, V *value = nullptr);
};

template<typename K, typename V>
void cache<K, V>::insert(const K &key, const V &value)
{
    auto it = storage.lower_bound(key);
    if (it != storage.end() && it->first == key) {
        it->second.time = (clock++);
        it->second.value = value;
    }
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
void cache<K, V>::evict()
{
    if (iters.size() > 0) {
        std::uniform_int_distribution<size_t> distribution(0, iters.size() - 1);
        size_t i = distribution(engine);
        do {
            const size_t j = distribution(engine);
            if (iters[i]->second.time > iters[j]->second.time)
                i = j;
        } while (false);
        std::swap(iters[i], iters.back());
        iters[i]->second.index = i;

        storage.erase(iters.back());
        iters.pop_back();
    }
}

template<typename K, typename V>
void cache<K, V>::evict(const K &key, V *value)
{
    auto it = storage.lower_bound(key);
    if (it != storage.end() && it->first == key) {
        if (value != nullptr)
            *value = std::move(it->second.value);

        size_t i = it->second.index;
        std::swap(iters[i], iters.back());
        iters[i]->second.index = i;

        storage.erase(iters.back());
        iters.pop_back();
    }
}
