#include "teamstyle17.h"
#include <iostream>
#include <cmath>

using namespace std;

const int INFTY = 0x7fffffff;
const Position CENTER = { kMapSize / 2, kMapSize / 2, kMapSize / 2 };
const double THRES_DEVOUR = 800;
const double THRES_BOSS_DEFENSE = 1000;
const double THRES_BOSS_ATTACK = 2000;
const double THRES_OPPONENT_DEFENSE = 1200;
const double THRES_OPPONENT_ATTACK = 1500;
double COS_THRES_ENERGY = 0.5;
double COS_THRES_AE = 0;

int last_time = 0;
int cooldown = 0;

void moveTo(int id, Position curr_pos, Position target_pos, double maxSpeed = kMaxMoveSpeed)
{
	Speed speed;
	speed.x = target_pos.x - curr_pos.x;
	speed.y = target_pos.y - curr_pos.y;
	speed.z = target_pos.z - curr_pos.z;
	if (speed.x == 0 && speed.y == 0 && speed.z == 0) {
		speed.x = rand() % 5 + 1;
		speed.y = rand() % 5 + 1;
		speed.z = rand() % 5 + 1;
	}
	double length = sqrt(speed.x * speed.x + speed.y * speed.y + speed.z * speed.z);
	speed.x = speed.x * maxSpeed / length;
	speed.y = speed.y * maxSpeed / length;
	speed.z = speed.z * maxSpeed / length;
	Move(id, speed);
}

void fuzzyMoveTo(int id, Position curr_pos, Position target_pos, double maxSpeed = kMaxMoveSpeed)
{
	Speed speed;
	speed.x = (target_pos.x - curr_pos.x) * (rand() % 10 + 1);
	speed.y = (target_pos.y - curr_pos.y) * (rand() % 10 + 1);
	speed.z = (target_pos.z - curr_pos.z) * (rand() % 10 + 1);
	double length = sqrt(speed.x * speed.x + speed.y * speed.y + speed.z * speed.z);
	speed.x = speed.x * maxSpeed / length;
	speed.y = speed.y * maxSpeed / length;
	speed.z = speed.z * maxSpeed / length;
	Move(id, speed);
}

void moveAgainst(int id, Position curr_pos, Position obj_pos, int maxSpeed = kMaxMoveSpeed)
{
	moveTo(id, obj_pos, curr_pos, maxSpeed);
}

void fuzzyMoveAgainst(int id, Position curr_pos, Position obj_pos, int maxSpeed = kMaxMoveSpeed)
{
	fuzzyMoveTo(id, obj_pos, curr_pos, maxSpeed);
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

const int powerof2[] = { 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024 };

void tryUpgradeSkills(const PlayerObject& self)
{
	int ability = self.ability;
	int cost = INFTY;
	SkillType skill;
	while (true) {
		if (self.skill_level[HEALTH_UP] < 3 ||
			(self.skill_level[SHORT_ATTACK] == kMaxSkillLevel && self.skill_level[DASH] == kMaxSkillLevel && self.skill_level[HEALTH_UP] < kMaxSkillLevel)) {
			cost = powerof2[self.skill_level[HEALTH_UP]];
			skill = HEALTH_UP;
		}
		else if (self.skill_level[SHORT_ATTACK] < kMaxSkillLevel) {
			if (self.skill_level[SHORT_ATTACK] == 0)
				cost = powerof2[1];
			else
				cost = powerof2[self.skill_level[SHORT_ATTACK]];
			skill = SHORT_ATTACK;
		}
		else if (self.skill_level[DASH] < kMaxSkillLevel) {
			if (self.skill_level[DASH] == 0)
				cost = powerof2[2];
			else
				cost = powerof2[self.skill_level[DASH]];
			skill = DASH;
		}
		else if (self.skill_level[VISION_UP] < kMaxSkillLevel) {
			COS_THRES_ENERGY = 0.3;
			COS_THRES_AE = 0.6;
			if (self.skill_level[VISION_UP] == 0)
				cost = powerof2[4];
			else
				cost = powerof2[self.skill_level[VISION_UP] + 1];
			skill = VISION_UP;
		}
		else {
			COS_THRES_ENERGY = 0.15;
			COS_THRES_AE = 1;
		}
		if (ability >= cost) {
			UpgradeSkill(self.id, skill);
			cost = INFTY;
			ability -= cost;
		}
		else {
			break;
		}
	}
}

void AIMain()
{
	const Status* status = GetStatus();
	PlayerObject self = status->objects[0];
	int curr_time = GetTime();
	if (curr_time - last_time > 0) {
		if (curr_time % 50 == 0)
			tryUpgradeSkills(self);
		const Map* map = GetMap();
		Position curr_pos = status->objects[0].pos;
		double curr_radius = status->objects[0].radius;
		Speed curr_speed = status->objects[0].speed;
		bool moved = false;
		bool attacked = false;
		for (int i = 0; i < map->objects_number; ++i) {
			Object object = map->objects[i];
			if (curr_time - last_time > cooldown && object.type == ENERGY && !moved) {
				if (cosine(curr_pos, object.pos, curr_speed) > COS_THRES_ENERGY) {
					moveTo(self.id, curr_pos, object.pos);
					moved = true;
					last_time = curr_time;
					cooldown = rand() % 3;
				}
			}
			else if (object.type == ADVANCED_ENERGY) {
				if (cosine(curr_pos, object.pos, curr_speed) > COS_THRES_AE) {
					moveTo(self.id, curr_pos, object.pos);
					moved = true;
					last_time = curr_time;
					cooldown = rand() % 5;
				}
			}
			else if (object.type == PLAYER && object.id != self.id) {
				if (!attacked && Dist(curr_pos, object.pos) - curr_radius - object.radius < kShortAttackRange[self.skill_level[SHORT_ATTACK]] && self.skill_cd[SHORT_ATTACK] == 0) {
					attacked = true;
					ShortAttack(self.id);
				}
				if ((self.health < 0.7 * self.max_health) || (object.radius > curr_radius * 1.05 && Dist(curr_pos, object.pos) - curr_radius < THRES_OPPONENT_DEFENSE)) {
					if (!attacked && self.skill_level[DASH] > 0 && self.skill_cd[DASH] == 0)
						Dash(self.id);
					cooldown = 25;
					moveAgainst(self.id, curr_pos, object.pos, kMaxMoveSpeed + kDashSpeed[self.skill_level[DASH]]);
					moved = true;
					last_time = curr_time;
					break;
				}
				if (object.radius < curr_radius * 0.95 && Dist(curr_pos, object.pos) - curr_radius < THRES_OPPONENT_ATTACK) {
					if (!attacked && self.skill_level[DASH] > 0 && self.skill_cd[DASH] == 0)
						Dash(self.id);
					cooldown = 15;
					moveTo(self.id, curr_pos, object.pos, kMaxMoveSpeed + kDashSpeed[self.skill_level[DASH]]);
					moved = true;
					last_time = curr_time;
					break;
				}
			}
			else if (object.type == BOSS) {
				if (!attacked && Dist(curr_pos, object.pos) - curr_radius - object.radius < kShortAttackRange[self.skill_level[SHORT_ATTACK]] && self.skill_cd[SHORT_ATTACK] == 0) {
					ShortAttack(self.id);
					attacked = true;
				}
				if (object.radius > curr_radius && Dist(curr_pos, object.pos) - curr_radius < THRES_BOSS_DEFENSE) {
					fuzzyMoveAgainst(self.id, curr_pos, object.pos);
					moved = true;
					last_time = curr_time;
					cooldown = 10;
					break;
				}
				if (object.radius < curr_radius * 0.8 && Dist(curr_pos, object.pos) - curr_radius < THRES_BOSS_ATTACK) {
					moveTo(self.id, curr_pos, object.pos);
					moved = true;
					last_time = curr_time;
					cooldown = 15;
					break;
				}
			}
			else if (object.type == DEVOUR && Dist(curr_pos, object.pos) - curr_radius < THRES_DEVOUR) {
			   	fuzzyMoveAgainst(self.id, curr_pos, object.pos);
				moved = true;
				last_time = curr_time;
				cooldown = 10;
				break;
			}
			if (inEdge(curr_pos, curr_radius)) {
				fuzzyMoveTo(self.id, curr_pos, CENTER);
				moved = true;
				last_time = curr_time;
				cooldown = 20;
				break;
			}
		}
		if (curr_time - last_time > cooldown && !moved) {
			fuzzyMoveTo(self.id, curr_pos, CENTER);
			last_time = curr_time;
			cooldown = 5;
		}
	}
	
}