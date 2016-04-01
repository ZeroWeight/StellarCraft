#include "teamstyle17.h"
#include <iostream>
#include <cmath>
#include <vector>

using namespace std;

typedef Position Vector;

const int safe_threshold = 20;	// devours beyond 20 rounds away are considered safe
const int border_threshold = 5;		// modify velocity 5 steps from border

int Archer;
double boss_radius;
PlayerObject me;		// currently does not support multiple player objects
int team_id;
int oppo_short_att_level = 0;
int action_clock = 0;	// the last time an action was done

int long_attack(const Object& target);
int short_attack(const Object& target);
int shield();
int dash();
void my_move(Vector v);

void update_clock()
{
	action_clock = GetTime();
}

int long_attack(const Object& target)
{
	if (me.skill_cd[LONG_ATTACK] == -1) {
		return -1;
	}
	if (me.long_attack_casting != -1) {
		return -1;
	}
	if (target.shield_time > 0) {
		return -1;
	}
	if (Distance(target.pos, me.pos) - target.radius - me.radius
			> kLongAttackRange[me.skill_level[LONG_ATTACK]]) {
		return -1;
	}
	while (GetTime() <= action_clock)
		;
	LongAttack(me.id, target.id);
	update_clock();
	return 0;
}

int short_attack(const Object& target)
{
	if (me.skill_cd[SHORT_ATTACK] == -1) {
		return -1;
	}
	if (target.shield_time > 0) {
		return -1;
	}
	if (Distance(target.pos, me.pos) - target.radius - me.radius
			> kShortAttackRange[me.skill_level[SHORT_ATTACK]]) {
		return -1;
	}
	while (GetTime() <= action_clock)
		;
	ShortAttack(me.id);
	update_clock();
	return 0;
}

int shield()
{
	if (!me.skill_level[SHIELD])
		return -1;
	if (me.skill_cd[SHIELD] == -1)
		return -1;
	while (GetTime() <= action_clock)
		;
	Shield(me.id);
	update_clock();
	return 0;
}

int dash()
{
	if (!me.skill_level[DASH])
		return -1;
	if (me.skill_cd[DASH] == -1)
		return -1;
	while (GetTime() <= action_clock)
		;
	Dash(me.id);
	update_clock();
	return 0;
}

void my_move(Vector v)
{
	unify(v);
	v = Scale(kMaxMoveSpeed, v);
	while (GetTime() <= action_clock)
		;
	Move(me.id, v);
	update_clock();
}


Vector operator + (const Vector& v1, const Vector& v2)
{
	Vector result;
	result.x = v1.x + v2.x;
	result.y = v1.y + v2.y;
	result.z = v1.z + v2.z;
	return result;
}

Vector operator - (const Vector& v1, const Vector& v2)
{
	Vector result;
	result.x = v1.x - v2.x;
	result.y = v1.y - v2.y;
	result.z = v1.z - v2.z;
	return result;
}

void unify(Vector& v)
{
	double norm = Norm(v);
	v.x /= norm;
	v.y /= norm;
	v.z /= norm;
}

void zhang_fei_reset(double& health_prev, double& att_dist)
{
	health_prev = att_dist = 0;
	cout << "Archer: 1" << endl;
	Archer = 1;		// fall back to Enshaw
}

int zhang_fei_move(const Object& target, vector<const Object&>& devour_list)
{
	Vector velocity = target.pos - me.pos;
	if (me.pos.x - me.radius - border_threshold * kMaxMoveSpeed <= 0 && velocity.x < 0 ||
		kMapSize - me.pos.x - me.radius - border_threshold * kMaxMoveSpeed <= 0 && velocity.x > 0 ||
		me.pos.y - me.radius - border_threshold * kMaxMoveSpeed <= 0 && velocity.y < 0 ||
		kMapSize - me.pos.y - me.radius - border_threshold * kMaxMoveSpeed <= 0 && velocity.y > 0 ||
		me.pos.z - me.radius - border_threshold * kMaxMoveSpeed <= 0 && velocity.z < 0 ||
		kMapSize - me.pos.z - me.radius - border_threshold * kMaxMoveSpeed <= 0 && velocity.z > 0) {
		// facing border
		return -1;
	}
	unify(velocity);
	for (auto devour : devour_list) {
		Vector relative_pos = devour.pos - me.pos;
		double product = DotProduct(relative_pos, velocity);
		if (product < 0 || product > kMaxMoveSpeed * safe_threshold) {
			// devour on the opposite side or too far, safe
			continue;
		}
		Vector projection_v = Scale(product, velocity);
		Vector projection_P = relative_pos - projection_v;
		double projection_radius = Norm(projection_P);
		if (target.radius * 0.95 < projection_radius && projection_radius < me.radius * 1.05) {
			// danger, fall back to Enshaw
			return -1;
		}
	}
	my_move(velocity);
	return 0;
}

double get_health(const Object& obj)
{
	return pow(obj.radius / 100, 3);
}

void infer_oppo_att_level(double health_loss, double att_dist)
{
	int new_level = -1;
	for (int i = 1; i <= kMaxSkillLevel; ++i) {
		if (health_loss <= kShortAttackDamage[i] && att_dist <= kShortAttackRange[i]) {
			new_level = i;
		}
	}
	if (new_level > oppo_short_att_level) {
		oppo_short_att_level = new_level;
		cout << "level raised to " << new_level << endl;
	}
}

void zhang_fei() {
	const Map* map = GetMap();
	me = GetStatus()->objects[0];
	int boss_i = -1, oppo_i = -1;
	vector<const Object&> devour_list;
	static double last_time = GetTime(), health_prev = 0, att_dist = 0;

	// scan map
	for (int i = 0; i < map->objects_number; ++i) {
		switch (map->objects[i].type) {
		case PLAYER:
			if (map->objects[i].team_id != team_id) {
				oppo_i = i;
			}
			break;
		case BOSS:
			boss_i = i;
			boss_radius = map->objects[i].radius;
			break;
		case DEVOUR:
			devour_list.push_back(map->objects[i]);
			break;
		}
	}

	if (boss_i == -1 && oppo_i == -1) {
		// no attackable object, reset
		return zhang_fei_reset(health_prev, att_dist);
	}

	if (oppo_i == -1) {
		// try attacking boss and reset
		short_attack(map->objects[boss_i]);
		return zhang_fei_reset(health_prev, att_dist);
	}


	// deal with opponent
	const Object& oppo = map->objects[oppo_i];

	if (GetTime() - last_time > 0) {
		double health_loss;
		if (health_prev != 0) {
			health_loss = me.health - health_prev;
		}
		else {
			health_loss = 0;
		}
		health_prev = me.health;
		infer_oppo_att_level(health_loss, att_dist);
		att_dist = Distance(me.pos, oppo.pos) - me.radius - oppo.radius;
		last_time = GetTime();
	}

	if (short_attack(oppo) == -1) {
		if (long_attack(oppo) == -1) {
			dash();
		}
	}
	double k = me.radius / oppo.radius;
	if (k < 0.9) {
		return zhang_fei_reset(health_prev, att_dist);
	}
	else if (0.9 <= k && k < 1) {
		if (me.skill_level[SHORT_ATTACK] == kMaxSkillLevel) {
			// equipped with full SHORT_ATTACK, safe to move,
			// but stop when health is getting dangerous
			if (me.health / me.max_health < 0.5 || zhang_fei_move(oppo, devour_list) == -1) {
				return zhang_fei_reset(health_prev,att_dist);
			}
		}
		else {
			return zhang_fei_reset(health_prev, att_dist);
		}
	}
	else if (1 <= k && k < 1.2) {
		if (me.health / me.max_health < 0.5) {
			// TODO: tell Enshaw not to launch attack when health is too low
			return zhang_fei_reset(health_prev, att_dist);
		}
		// flee if short attack is weaker than opponent
		if (me.skill_level[SHORT_ATTACK] < oppo_short_att_level) {
			return zhang_fei_reset(health_prev, att_dist);
		}
		else {
			// try chasing opponent
			if (zhang_fei_move(oppo, devour_list) == -1) {
				return zhang_fei_reset(health_prev, att_dist);
			}
		}
	}
	else {
		// large enough, attack except when the health level is too dangerous
		if (me.health / me.max_health < 0.4 || zhang_fei_move(oppo, devour_list) == -1) {
			return zhang_fei_reset(health_prev, att_dist);
		}
	}
}
