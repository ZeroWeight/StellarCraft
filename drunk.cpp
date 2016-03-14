#include "teamstyle17.h"
#include <iostream>

int last_time = 0, duration = 0;

int randint(int start, int end)
{
    return start + std::rand() % (end - start);
}

void AIMain()
{
    const Status* status = GetStatus();
    int self_id = status->objects[0].id;
    if (GetTime() - last_time >= duration) {
	last_time = GetTime();
	duration = randint(0, 10);
	Speed speed = { randint(0, 1000), randint(0, 1000), randint(0, 1000) };
	Move(self_id, speed);
	std::cout << self_id << ":Moving at (" << speed.x << ", " << speed.y << ", " << speed.z << ") "
		  << "for " << duration << std::endl;
    }
}
