#pragma once
#include <iostream>
#include <list>
#include <mutex>
#include <unordered_map>
#include <utility>
#include <vector>

template <typename K, typename V> class Lru {
public:
  Lru(int cap, V nan_value = V()) : cap(cap), nan_value(nan_value) {}

  std::pair<bool, V> Insert(const K &key, const V &value) {
    V victim;
    bool is_evict = false;
    evictList.push_front({key, value});
    items[key] = evictList.begin();

    if (needEvict()) {
      is_evict = true;
      victim = evictList.back().second;
      items.erase(evictList.back().first);
      evictList.pop_back();
    }

    return std::make_pair(is_evict, victim);
  }

  bool Has(const K &key) { return items.find(key) != items.end(); }

  V Get(const K &key) {
    auto it = items.find(key);
    if (it != items.end()) {
      evictList.splice(evictList.begin(), evictList, it->second);
      return it->second->second;
    }

    return nan_value;
  }

  int Len() { return evictList.size(); }

  std::vector<V> GetAll() {
    std::vector<V> result;
    result.reserve(items.size());

    for (const auto &pair : items) {
      result.push_back(pair.second->second);
    }

    return result;
  }

private:
  bool needEvict() { return evictList.size() > cap; }

  int cap;
  V nan_value;
  std::list<std::pair<K, V>> evictList;
  std::unordered_map<K, typename std::list<std::pair<K, V>>::iterator> items;
};
