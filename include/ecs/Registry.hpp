#pragma once
#include "Entity.hpp"
#include "Component.hpp"
#include <unordered_map>
#include <typeindex>
#include <memory>
#include <vector>

class Registry
{
  Entity nextEntity = 1;
  std::unordered_map<std::type_index, std::shared_ptr<IComponentArray>> components;

public:
  Entity CreateEntity() { return nextEntity++; }

  template <typename T>
  void AddComponent(Entity e, const T &c)
  {
    auto type = std::type_index(typeid(T));
    if (components.find(type) == components.end())
      components[type] = std::make_shared<ComponentArray<T>>();
    std::static_pointer_cast<ComponentArray<T>>(components[type])->Insert(e, c);
  }

  template <typename T>
  T *GetComponent(Entity e)
  {
    auto type = std::type_index(typeid(T));
    auto it = components.find(type);
    if (it == components.end())
      return nullptr;
    return std::static_pointer_cast<ComponentArray<T>>(it->second)->Get(e);
  }

  template <typename T>
  std::vector<std::pair<Entity, T *>> View()
  {
    std::vector<std::pair<Entity, T *>> out;
    auto type = std::type_index(typeid(T));
    auto it = components.find(type);
    if (it == components.end())
      return out;
    auto arr = std::static_pointer_cast<ComponentArray<T>>(it->second);
    for (auto &[e, c] : arr->data)
      out.push_back({e, &c});
    return out;
  }
};
