#include "teamstyle17.h"
#include<cmath>
#include<iostream>
using namespace std;
enum AR_BORDER {
	NONE = 0,
	LEFT = 1 << 0,
	RIGHT = 1 << 1,
	BEHIND = 1 << 2,
	FRONT = 1 << 3,
	DOWN = 1 << 4,
	UP = 1 << 5,
};
class AR_VECTOR {
private:
	double x;
	double y;
	double z;
public:
	AR_VECTOR(Position);
	AR_VECTOR();
	AR_VECTOR(double, double, double);
	double length();
	int border(double);
	Position cast();
	void operator=(AR_VECTOR&);
	void operator=(Position);
	friend double operator^ (AR_VECTOR&, AR_VECTOR&);
	friend int operator==(const AR_VECTOR&, const AR_VECTOR&);
	friend AR_VECTOR operator*(const AR_VECTOR&, const AR_VECTOR&);
	friend AR_VECTOR operator*(const int, const AR_VECTOR&);
	friend AR_VECTOR operator*(const AR_VECTOR&, const int);
	friend AR_VECTOR operator+(const AR_VECTOR&, const AR_VECTOR&);
	friend AR_VECTOR operator-(const AR_VECTOR&, const AR_VECTOR&);
	friend double operator,(const AR_VECTOR&, const AR_VECTOR&);
	friend ostream& operator<<(ostream&, AR_VECTOR&);
};
class AR_GAME {
private:
	const Map* map;
	int myteam;
	PlayerObject my;
public:
	AR_GAME();
	int MoveInSPEED(AR_VECTOR);
	Object operator[](int);
	int cost(SkillType);
	int Update(SkillType);
	int long_attack(Object);
	int short_attack();
	int dash();
	int shield();
	int health();
	int maxhealth();
	int vision();
	double radius();
	double radius(Object);
	AR_VECTOR speed();
	AR_VECTOR position();
	AR_VECTOR position(Object);
	int long_attack_casting();
	int long_attack_casting(Object);
	int shield_time();
	int shield_time(Object);
	int dash_time();
	int ability();
	int skill_level(SkillType);
	int skill_cd(SkillType);
	int check(Object);
};


void AIMain() {
	AR_GAME G;
	G.MoveInSPEED(AR_VECTOR(500, 500, 500));
}


AR_VECTOR::AR_VECTOR() {
	x = (kMapSize >> 1);
	y = (kMapSize >> 1);
	z = (kMapSize >> 1);
}
AR_VECTOR::AR_VECTOR(double a, double b, double c) {
	x = a;
	y = b;
	z = c;
}
double AR_VECTOR::length()
{
	return sqrt((this->x)*(this->x)+(this->y)*(this->y)+(this->z)*(this->z));
}
int AR_VECTOR::border(double d)
{
	int R = NONE;
	if (this->x < d) R |= LEFT;
	else if (kMapSize - this->x < d) R |= RIGHT;
	if (this->y < d) R |= BEHIND;
	else if (kMapSize - this->y < d) R |= FRONT;
	if (this->z < d) R |= DOWN;
	else if (kMapSize - this->z < d) R |= UP;
	return R;
}
Position AR_VECTOR::cast()
{
	return{ this->x, this->y, this->z };
}
void AR_VECTOR::operator=(AR_VECTOR &A)
{
	this->x = A.x;
	this->y = A.y;
	this->z = A.z;
}
void AR_VECTOR::operator=(Position P)
{
	x = P.x;
	y = P.y;
	z = P.z;
}
AR_VECTOR::AR_VECTOR(Position p) {
	x=p.x;
	y=p.y;
	z=p.z;
}
AR_VECTOR operator+(const AR_VECTOR&A, const AR_VECTOR&B) {
	return AR_VECTOR(A.x + B.x, A.y + B.y, A.z + B.z);
}
AR_VECTOR operator-(const AR_VECTOR&A, const AR_VECTOR&B) {
	return AR_VECTOR(A.x - B.x, A.y - B.y, A.z - B.z);
}
double operator,(const AR_VECTOR & A, const AR_VECTOR & B)
{
	return (A.x*B.x + A.y*B.y + A.z*B.z);
}
ostream & operator<<(ostream & os, AR_VECTOR & p)
{
	os << p.x << '\t' << p.y << '\t' << p.z << '\t' << p.length() << endl;
	return os;
}
AR_VECTOR operator*(const AR_VECTOR&A, const int k) {
	return AR_VECTOR(A.x*k, A.y*k, A.z*k);
}
AR_VECTOR operator*(const int k, const AR_VECTOR& A) {
	return (A*k);
}
double operator^(AR_VECTOR &A, AR_VECTOR &B)
{
	return (A, B) / (A.length()*B.length());
}
int operator==(const AR_VECTOR &A, const AR_VECTOR &B)
{
	return (A - B).length() < 0.01;
}
AR_VECTOR operator*(const AR_VECTOR& A, const AR_VECTOR& B) {
	return AR_VECTOR(A.y*B.z - A.z*B.y, A.z*B.x - A.x*B.z, A.x*B.y - A.y * B.x);
}
AR_GAME::AR_GAME()
{
	map = GetMap();
	myteam = GetStatus()->team_id;
	my = GetStatus()->objects[0];
}
int AR_GAME::MoveInSPEED(AR_VECTOR P)
{
	if (!(P == my.pos)) {
		Move(this->my.id, P.cast());
		return 0;
	}
	else return -1;
}
Object  AR_GAME::operator[](int n)
{
	return map->objects[n];
}
int AR_GAME::cost(SkillType skill) {
	if (my.skill_level[skill]) {
		return (kBasicSkillPrice[skill] << my.skill_level[skill]);
	}
	else {
		int i, count;
		for (i = 0, count = 0;i < kSkillTypes;i++) {
			if (my.skill_level[i]) count++;
		}
		return(kBasicSkillPrice[skill] << count);
	}
}
int AR_GAME::Update(SkillType skill)
{
	if (my.ability >= cost(skill)) {
		UpgradeSkill(my.id, skill);
		return 0;
	}
	else return -1;
}
int AR_GAME::long_attack(Object aim) {
	if (my.skill_level[LONG_ATTACK] && my.skill_cd[LONG_ATTACK] == 0
		&& (AR_VECTOR(my.pos)-AR_VECTOR(aim.pos)).length()<kLongAttackRange[my.skill_level[LONG_ATTACK]]) {
		LongAttack(my.id, aim.id);
		printf("LONG ATTACK AT\t%d\tLEVEL\t%d\n", GetTime(), my.skill_level[LONG_ATTACK]);
		return 0;
	}
	else return -1;
}
int AR_GAME::short_attack() {
	if (my.skill_level[SHORT_ATTACK] && (my.skill_cd[SHORT_ATTACK] == 0)) {
		ShortAttack(my.id);
		printf("SHORT ATTACK AT\t%d\tLEVEL\t%d\n", GetTime(), my.skill_level[SHORT_ATTACK]);
		return 0;
	}
	else {
		return -1;
	}
}
int AR_GAME::shield() {
	if (my.skill_level[SHIELD] && my.skill_cd[SHIELD] == 0) {
		Shield(my.id);
		printf("SHIELD AT\t%d\tLEVEL\t%d\n", GetTime(), my.skill_level[SHIELD]);
		return 0;
	}
	else return -1;
}
int AR_GAME::health()
{
	return my.health;
}
int AR_GAME::maxhealth()
{
	return my.max_health;
}
int AR_GAME::vision()
{
	return my.vision;
}
double AR_GAME::radius()
{
	return my.radius;
}
double AR_GAME::radius(Object aim)
{
	return aim.radius;
}
AR_VECTOR AR_GAME::speed()
{
	return AR_VECTOR(my.speed);
}
AR_VECTOR AR_GAME::position()
{
	return AR_VECTOR(my.pos);
}
AR_VECTOR AR_GAME::position(Object aim)
{
	return AR_VECTOR(aim.pos);
}
int AR_GAME::long_attack_casting()
{
	return my.long_attack_casting;
}
int AR_GAME::long_attack_casting(Object aim)
{
	return aim.long_attack_casting;
}
int AR_GAME::shield_time()
{
	return my.shield_time;
}
int AR_GAME::shield_time(Object aim)
{
	return aim.shield_time;
}
int AR_GAME::dash_time()
{
	return my.dash_time;
}
int AR_GAME::ability()
{
	return my.ability;
}
int AR_GAME::skill_level(SkillType skill)
{
	return my.skill_level[skill];
}
int AR_GAME::skill_cd(SkillType skill)
{
	return my.skill_cd[skill];
}
int AR_GAME::check(Object aim)
{
	if (aim.type != PLAYER) return aim.type;
	else {
		if (aim.team_id == myteam) return -1;
		else return 100;
	}
}
int AR_GAME::dash() {
	if (my.skill_level[DASH] && my.skill_cd[DASH] == 0) {
		Dash(my.id);
		printf("%d\tDASH AT\t\t%d\tLEVEL\t%d\n", my.id, GetTime(), my.skill_level[DASH]);
		return 0;
	}
	else return -1;
}