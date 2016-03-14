#include "teamstyle17.h"
#include <iostream>

void AIMain() {
	const Status* status = GetStatus();
	int self_id = status->objects[0].id;
	std::cout << "id: " << self_id << std::endl;
	Move(self_id, { 100, 0, 0 });
}
