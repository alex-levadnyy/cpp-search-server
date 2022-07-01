#pragma once


#include <chrono>
#include <iostream>
#include <mutex>
#include <map>

using namespace std::literals;

#define PROFILE_CONCAT_INTERNAL(X, Y) X##Y
#define PROFILE_CONCAT(X, Y) PROFILE_CONCAT_INTERNAL(X, Y)
#define UNIQUE_VAR_NAME_PROFILE PROFILE_CONCAT(profileGuard, __LINE__)
#define LOG_DURATION(x) LogDuration UNIQUE_VAR_NAME_PROFILE(x)

class LogDuration {
public:
    
    using Clock = std::chrono::steady_clock;

    LogDuration(const std::string_view& id) : id_(id) {
    }

    ~LogDuration() {
        using namespace std::chrono;
        using namespace std::literals;

        const auto end_time = Clock::now();
        const auto dur = end_time - start_time_;
        std::cerr << id_ << ": "s << duration_cast<milliseconds>(dur).count() << " ms"s << std::endl;
    }

private:
    const std::string id_;
    const Clock::time_point start_time_ = Clock::now();
};

template <typename Key, typename Value>
class ConcurrentMap {
private:
    struct Part {
        std::mutex mutex;
        std::map<Key, Value> map;
    };

    uint64_t CalculatePosition(const Key& key) const {
        const uint64_t position = key;

        return position % parts_.size();
    }

    Part& FindPart(const Key& key) {
        return parts_[CalculatePosition(key)];
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