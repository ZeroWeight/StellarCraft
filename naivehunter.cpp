#include "teamstyle17.h"
#include <iostream>
#include <cmath>

int last_time = 0;

void moveTo(int id, Position curr_pos, Position target_pos)
{
    Speed speed;
    speed.x = target_pos.x - curr_pos.x;
    speed.y = target_pos.y - curr_pos.y;
    speed.z = target_pos.z - curr_pos.z;
    double length = sqrt(speed.x * speed.x + speed.y * speed.y + speed.z * speed.z);
    speed.x = speed.x * kMaxMoveSpeed / length;
    speed.y = speed.y * kMaxMoveSpeed / length;
    speed.z = speed.z * kMaxMoveSpeed / length;
    Move(id, speed);
}

void AIMain()
{
    const Status* status = GetStatus();
    int self_id = status->objects[0].id;
    if (GetTime() - last_time >= 1) {
        last_time = GetTime();
        const Map* map = GetMap();
        Position curr_pos = status->objects[0].pos;
        bool moved = false;
        for (int i = 0; i < map->objects_number; ++i) {
            Object object = map->objects[i];
            if (object.type == ENERGY || object.type == ADVANCED_ENERGY) {
                moveTo(self_id, curr_pos, object.pos);
                moved = true;
                break;
            }
        }
        if (!moved) // move toward center
            moveTo(self_id, curr_pos, { kMapSize / 2, kMapSize / 2, kMapSize / 2 });
    }
}
