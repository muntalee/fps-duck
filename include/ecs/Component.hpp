#pragma once
#include <unordered_map>
#include <typeindex>
#include <memory>

struct IComponentArray
{
  virtual ~IComponentArray() = default;
};

template <typename T>
struct ComponentArray : IComponentArray
{
  std::unordered_map<Entity, T> data;

  void Insert(Entity e, const T &component) { data[e] = component; }
  void Remove(Entity e) { data.erase(e); }
  T *Get(Entity e)
  {
    auto it = data.find(e);
    return (it != data.end()) ? &it->second : nullptr;
  }
};
