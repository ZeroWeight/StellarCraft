#include"teamstyle17.h"
#include<stdio.h>
#include<stdlib.h>
#include<math.h>
/**************V1.1.0***********************/
/*	底层封装与方法 BY Zero Weight 2016/3/15 11:30	*/
/*	所有底层变量以AR为字头，请注意命名重复		*/
/*	结构体封装方式参见使用手册								*/
/*	假定双方各自只控制一个球，只有两方竞争		*/
/*	场上道具：E(nergy)能量源，
	A(dvancedEnergy）光之隧道，D(evour)吞噬者		*/
/*********************************************/





/******************底层对象*******************/
//算法执行判据，用来记录当前回合数与同步矫正	样例：AR_GO=GetTime（）	同步时间
int AR_GO=-1;
//本方操作对象，用来提供本方各项属性     样例：AR_my.pos； 得到本方位置
PlayerObject AR_my;	
//对方操作对象，用来提供对方各项属性     样例：AR_opposite.pos； 得到对方位置
Object AR_opposite;	
//本方队旗，一般不会用到，队伍选择分配已在底层完成
int AR_MYTeam;		
//对方是否在视野内，只有为true的时候 AR_opposite才有意义
bool AR_OppositeIN;		
//boss对象，用来提供boss各项属性     样例：AR_boss.pos 得到boss位置
Object AR_boss;					
//boss是否在视野内，只有为true的时候 AR_boss才有意义
bool AR_BossIN;			
//初始值为零，用来记录视野内的E，A和D     样例：用来提供循环节
int AR_NUME,AR_NUMA,AR_NUMD;	
//场上E,A,D对象数组，已在底层对地图对象分类	样例：AR_E[0].pos	得到一个能量源位置
Object AR_E[kMaxObjectNumber],	AR_A[kMaxObjectNumber],AR_D[kMaxObjectNumber];
//地图，一般不会用到，已经在底层对地图彻底解析	样例：AR_map->objectnumber 得到对象总数
const Map* AR_map;
//调试向量，用于无方向移动的一个暂用方向，为地图中心
const Position AR_Center={kMapSize/2,kMapSize/2,kMapSize/2};
//
Position AR_speed = AR_Center;
/******************初始化函数*******************/
//初始化函数，必须在AIMain最开始调用
void initialize(){		
	AR_BossIN=AR_OppositeIN=AR_NUMA=AR_NUMD=AR_NUME=0;
	AR_map=GetMap();
	AR_my=GetStatus()->objects[0];
	AR_MYTeam=GetStatus()->team_id;
	for(int i=0;i<AR_map->objects_number;i++){
		switch(AR_map->objects[i].type){
		case BOSS:{
			AR_BossIN=1;
			AR_boss=AR_map->objects[i];
			break;
				  }
		case ENERGY:{
			AR_E[AR_NUME]=AR_map->objects[i];
			AR_NUME++;
			break;
					}
		case ADVANCED_ENERGY:{
			AR_A[AR_NUMA]=AR_map->objects[i];
			AR_NUMA++;
			break;
					}
		case DEVOUR:{
			AR_D[AR_NUMD]=AR_map->objects[i];
			AR_NUMD++;
			break;
					}
		default:{
			if(AR_map->objects[i].team_id!=AR_MYTeam&&AR_map->objects[i].team_id!=-2) {
				AR_OppositeIN=1;
				AR_opposite=AR_map->objects[i];
			}
				}
		}
	}
}
void wait() {
	int i;
	for (i = 0;i < 10000000000;i++);
}




/******************向量函数*******************/
//求一个向量的模长     样例：length(A)==||A||;
double length(Position a){							
	return sqrt(a.x*a.x+a.y*a.y+a.z*a.z);
}
//求两个向量之间的方向向量    样例：PointTo(A,B)==AB;从A到B
Position PointTo(Position src,Position des){
	Position temp={des.x-src.x,des.y-src.y,des.z-src.z};
	return(temp);
}
//求两个点之间的距离     样例：distance(A,B)==||AB||;
double distance(Position src,Position des){
	return length(PointTo(src,des));
}
//比较两向量是否相等,误差为0.001
bool equal(Position A, Position B) {
	return distance(A, B) < 0.001;
}
//向量数乘   样例：multiple(k,A)==kA;
Position multiple(double times,Position a){
	Position temp={a.x*times,a.y*times,a.z*times};
	return temp;
}





/******************运动方法*******************/
//以最大速度向指定目标移动     样例：MoveTo(A);以最大速度向A移动
//封装时保证若A为当前矢量返回-1，不执行任何操作，否则返回0
int MoveTo(Position des){
	if ((!equal(des,AR_my.pos))&&(!equal(des,AR_speed))) {
		Move(AR_my.id, multiple(kMaxMoveSpeed / distance(AR_my.pos, des), PointTo(AR_my.pos, des)));
		AR_speed = des;
		return 0;
	}
	else return -1;
}





/******************升级科技(不完整）*******************/
//升级科技函数（测试），成功升级返回0，失败不执行，返回-1
//技能见手册，不能升级HEALTH
//在技能点数的计算上存疑
int cost(SkillType skill) {
	if (AR_my.skill_level[skill]) {
		return (kBasicSkillPrice[skill] << AR_my.skill_level[skill]);
	}
	else {
		int i, count;
		for (i = 0, count = 0;i < kSkillTypes;i++) {
			if (AR_my.skill_level[i]) count++;
		}
		return(kBasicSkillPrice[skill] << count);
	}
}
int update(SkillType skill) {
	if (AR_my.ability >= cost(skill))
	{
		UpgradeSkill(AR_my.id, skill);
		return 0;
	}
	else return -1;
}



/******************技能使用*******************/
//以下技能使用完毕注意检查使用是否成功，0为成功，-1失败
int Lattack(Object aim) {
	if (AR_my.skill_level[LONG_ATTACK] && AR_my.skill_cd[LONG_ATTACK] == 0
		&&distance(AR_my.pos,aim.pos)<kLongAttackRange[AR_my.skill_level[LONG_ATTACK]] ){
		LongAttack(AR_my.id, aim.id);
		printf("LONG ATTACK AT\t%d\tLEVEL\t%d\n", GetTime(), AR_my.skill_level[LONG_ATTACK]);
		return 0;
	}
	else return -1;
}
int Sattack() {
	if (AR_my.skill_level[SHORT_ATTACK] && (AR_my.skill_cd[SHORT_ATTACK] == 0)) {
		ShortAttack(AR_my.id);
		printf("SHORT ATTACK AT\t%d\tLEVEL\t%d\n", GetTime(), AR_my.skill_level[SHORT_ATTACK]);
		return 0;
	}
	else {
		return -1;
	}
}
int shield() {
	if (AR_my.skill_level[SHIELD] && AR_my.skill_cd[SHIELD] == 0) {
		Shield(AR_my.id);
		printf("SHIELD AT\t%d\tLEVEL\t%d\n", GetTime(), AR_my.skill_level[SHIELD]);
		return 0;
	}
	else return -1;
}
int dash() {
	if (AR_my.skill_level[DASH] && AR_my.skill_cd[DASH] == 0) {
		Dash(AR_my.id);
		printf("DASH AT\t\t%d\tLEVEL\t%d\n", GetTime(), AR_my.skill_level[DASH]);
		return 0;
	}
	else return -1;
}
/**********底层封装结束**********/
//核心函数
void AIMain(){
	initialize();
	update(SHIELD);
	update(SHORT_ATTACK);
	update(DASH);
	dash();
	Sattack();
	shield();
	if (AR_BossIN&&AR_my.radius > 1.5*AR_boss.radius) MoveTo(AR_boss.pos);
	if(AR_NUMA)MoveTo(AR_A[0].pos);
	else if(AR_NUME)MoveTo(AR_E[0].pos);
	else MoveTo(AR_Center);
}
