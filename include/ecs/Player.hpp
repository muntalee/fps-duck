#pragma once
#include "Entity.hpp"

struct Player
{
    Entity gun = 0;
    bool wantFire = false;
    float cooldown = 0.0f;
    float fireRate = 8.0f;
};
