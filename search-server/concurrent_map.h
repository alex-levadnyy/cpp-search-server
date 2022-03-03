#pragma once

#include "log_duration.h"

#include <mutex>
#include <map>

template <typename Key, typename Value>
class ConcurrentMap {
private:
    struct Part {
        std::mutex mutex;
        std::map<Key, Value> map;
    };

    uint64_t Position(const Key& key) const {
        const uint64_t position = key;
        return position % parts_.size();
    }

    Part& FindPart(const Key& key) {
        return parts_[Position(key)];
    }
 
public:
    static_assert(std::is_integral_v<Key>, "ConcurrentMap supports only integer keys"s);
 
    struct Access {
        std::lock_guard<std::mutex> guard;
        Value& ref_to_value;
 
        Access(const Key& key, Part& part)
            : guard(part.mutex)
            , ref_to_value(part.map[key]) {
        }
    };
 
    explicit ConcurrentMap(size_t part_count)
        : parts_(part_count) {
    }
 
    Access operator[](const Key& key) {
        auto& part = parts_[static_cast<uint64_t>(key) % parts_.size()];
        return {key, part};
    }
 
    std::map<Key, Value> BuildOrdinaryMap() {
        std::map<Key, Value> result;
        for (auto& [mutex, map] : parts_) {
            std::lock_guard g(mutex);
            result.insert(map.begin(), map.end());
        }
        return result;
    }

    void erase(const Key& key) {
        Part& part = FindPart(key);
        std::lock_guard guard(part.mutex);
        part.map.erase(key);
    }
 
private:
    std::vector<Part> parts_;
};