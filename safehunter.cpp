#include "teamstyle17.h"
#include <iostream>
#include <cmath>

using namespace std;

const Position CENTER = { kMapSize / 2, kMapSize / 2, kMapSize / 2 };
const double THRES_DEVOUR = 800;
const double THRES_BOSS = 1000;
const double THRES_OPPONENT = 1200;

int last_time = 0;
int cooldown = 0;

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

void fuzzyMoveTo(int id, Position curr_pos, Position target_pos)
{
	Speed speed;
	speed.x = (target_pos.x - curr_pos.x) * (rand() % 10 + 1);
	speed.y = (target_pos.y - curr_pos.y) * (rand() % 10 + 1);
	speed.z = (target_pos.z - curr_pos.z) * (rand() % 10 + 1);
	double length = sqrt(speed.x * speed.x + speed.y * speed.y + speed.z * speed.z);
	speed.x = speed.x * kMaxMoveSpeed / length;
	speed.y = speed.y * kMaxMoveSpeed / length;
	speed.z = speed.z * kMaxMoveSpeed / length;
	Move(id, speed);
}

void moveAgainst(int id, Position curr_pos, Position obj_pos)
{
	moveTo(id, obj_pos, curr_pos);
}

void fuzzyMoveAgainst(int id, Position curr_pos, Position obj_pos)
{
	fuzzyMoveTo(id, obj_pos, curr_pos);
}

bool inEdge(Position pos, double radius, double threshold = 100) {
	if (pos.x - radius < threshold || kMapSize - pos.x - radius < threshold)
		return true;
	if (pos.y - radius < threshold || kMapSize - pos.y - radius < threshold)
		return true;
	if (pos.z - radius < threshold || kMapSize - pos.z - radius < threshold)
		return true;
	return false;
}

double Dist(Position p1, Position p2)
{
	return sqrt((p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y) + (p1.z - p2.z) * (p1.z - p2.z));
}

double cosine(Position curr_pos, Position target_pos, Speed speed)
{
	double length1 = sqrt((target_pos.x - curr_pos.x) * (target_pos.x - curr_pos.x) + (target_pos.y - curr_pos.y) * (target_pos.y - curr_pos.y) + (target_pos.z - curr_pos.z) * (target_pos.z - curr_pos.z));
	double length2 = sqrt(speed.x * speed.x + speed.y * speed.y + speed.z * speed.z);
	if (length1 == 0 || length2 == 0)
		return 1;
	double innerProduct = (target_pos.x - curr_pos.x) * speed.x + (target_pos.y - curr_pos.y) * speed.y + (target_pos.z - curr_pos.z) * speed.z;
	return innerProduct / (length1 * length2);
}

void AIMain()
{
	const Status* status = GetStatus();
	int self_id = status->objects[0].id;
	if (GetTime() - last_time > 0) {
		int curr_time = GetTime();
		const Map* map = GetMap();
		Position curr_pos = status->objects[0].pos;
		double curr_radius = status->objects[0].radius;
		Speed curr_speed = status->objects[0].speed;
		bool moved = false;
		for (int i = 0; i < map->objects_number; ++i) {
			Object object = map->objects[i];
			if (curr_time - last_time > cooldown && (object.type == ENERGY || object.type == ADVANCED_ENERGY) && !moved) {
				if (cosine(curr_pos, object.pos, curr_speed) > 0.5) {
					moveTo(self_id, curr_pos, object.pos);
					moved = true;
					last_time = curr_time;
					cooldown = rand() % 5;
				}
			}
			else if (object.type == PLAYER && object.radius > curr_radius * 1.05 && Dist(curr_pos, object.pos) - curr_radius < THRES_OPPONENT) {
				moveAgainst(self_id, curr_pos, object.pos);
				moved = true;
				last_time = curr_time;
				cooldown = 25;
				break;
			}
			else if (object.type == BOSS && object.radius > curr_radius && Dist(curr_pos, object.pos) - curr_radius < THRES_BOSS) {
				fuzzyMoveAgainst(self_id, curr_pos, object.pos);
				moved = true;
				last_time = curr_time;
				cooldown = 10;
				break;
			}
			else if (object.type == DEVOUR && Dist(curr_pos, object.pos) - curr_radius < THRES_DEVOUR) {
			   	fuzzyMoveAgainst(self_id, curr_pos, object.pos);
				moved = true;
				last_time = curr_time;
				cooldown = 10;
				break;
			}
			if (inEdge(curr_pos, curr_radius)) {
				fuzzyMoveTo(self_id, curr_pos, CENTER);
				moved = true;
				last_time = curr_time;
				cooldown = 10;
				break;
			}
		}
		if (curr_time - last_time > cooldown && !moved) {
			fuzzyMoveTo(self_id, curr_pos, CENTER);
			last_time = curr_time;
			cooldown = 5;
		}
	}
	
}