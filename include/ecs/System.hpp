#pragma once
#include "Registry.hpp"

class System
{
public:
  virtual void Update(Registry &registry, float dt) = 0;
  virtual ~System() = default;
};
