#pragma once
#include <cstdlib>
#include <future>
#include <map>
#include <numeric>
#include <random>
#include <string>
#include <vector>
#include <mutex>

using namespace std;

template <typename Key, typename Value>
class ConcurrentMap {
public:
    static_assert(std::is_integral_v<Key>, "ConcurrentMap supports only integer keys"s);
    struct Access {
        std::lock_guard<std::mutex> guard_;
        Value& ref_to_value;
    };
    explicit ConcurrentMap(size_t bucket_count) :
        data_maps(bucket_count)
    {}
    Access operator[](const Key& key) {
        auto& [mutex_, map_] = data_maps[(uint64_t)key % data_maps.size()];
        return { std::lock_guard<std::mutex>(mutex_), map_[(uint64_t)key] };
    }
    std::map<Key, Value> BuildOrdinaryMap() {
        std::map<Key, Value> result;
        for (auto& [mutex_, map_] : data_maps) {
            std::lock_guard mutex_guard(mutex_);
            result.insert(map_.begin(), map_.end());
        }
        return result;
    }
private:
    std::vector<std::pair<std::mutex, std::map<Key, Value>>> data_maps;
};