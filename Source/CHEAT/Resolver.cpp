#include "Resolver.h"
#include "Ragebot.h"
#include "Hooks.h"

enum MENUS
{
	MENU_NONE = 0,
	MENU_AIMBOT,
	MENU_TRIGGERBOT,
	MENU_ESP,
	MENU_MISC,
	MENU_PLAYERLIST,
	MENU_ABOUT,
	MENU_LEGITBOT
};

enum TYPES
{
	Float = 0,
	Double,
	Integer,
	Bool,
	Byte,
	Char
};

#define OVERLAY_OFFSETX 0
#define OVERLAY_OFFSETY 0

#define OVERLAY_OFFSETX 250
#define OVERLAY_OFFSETY 50


#define TEXTBOX_HEIGHT 16
#define TEXTBOX_SCROLL_WIDTH 15
#define TEXTBOX_SCROLL_HEIGHT 7
#define CHECKBOX_WIDTH 10
#define CHECKBOX_HEIGHT 10
#define MENU_BUTTON_WIDTH 70
#define MENU_BUTTON_HEIGHT 32

#define Centered 1
#define NotCentered 0
#define CheckedOff 0
#define CheckedOn 1
#define NotPressed 0
#define IsPressed 1
#define NotHighlighted 0
#define IsHighlighted 1
#define INDEX_ZERO 0
#define FADE_ZERO 0.0f
#define NOT_FADED 0
#define IsFaded 1
#define IsNotFaded 0
#define IsDisabled 1
#define IsNotDisabled 0


char *WDresolverstr = new char[12]{ 45, 62, 90, 40, 31, 9, 21, 22, 12, 31, 8, 0 }; /*WD Resolver*/

struct Textbox
{
	Vector2D ScreenPos;
	Vector2D IncScreenPos;
	Vector2D DecScreenPos;
	MENUS Menu;
	bool OffsetFromCenter;
	void* CallOnSet;
	int Width;
	TYPES Type;
	char* Label;
	double dbMin;
	double dbMax;
	char Value[128];
	float flValue;
	double dbValue;
	int iValue;
	BYTE bValue;
	bool Fade;
	bool Disabled;
};


Textbox WhaleDongTxt = { Vector2D(40 - OVERLAY_OFFSETX, 190 - OVERLAY_OFFSETY), Vector2D(95 - OVERLAY_OFFSETX, 185 - OVERLAY_OFFSETY), Vector2D(95 - OVERLAY_OFFSETX, 200 - OVERLAY_OFFSETY), MENU_MISC, Centered, nullptr, 50, TYPES::Integer, WDresolverstr, 0.0, 64.0, "0", 0.0f, 0.0, 0, 0, IsFaded, IsNotDisabled };


void PitchCorrection()
{

	for (int i = 0; i < Interfaces::Engine->GetMaxClients(); ++i)
	{
		IClientEntity* pLocal = hackManager.pLocal();
		IClientEntity *player = (IClientEntity*)Interfaces::EntList->GetClientEntity(i);

		if (!player || player->IsDormant() || player->GetHealth() < 1 || (DWORD)player == (DWORD)pLocal)
			continue;

		if (!player)
			continue;

		if (pLocal)
			continue;

		if (pLocal && player && pLocal->IsAlive())
		{
			if (Menu::Window.RageBotTab.AdvancedResolver.GetState())
			{
				Vector* eyeAngles = player->GetEyeAnglesXY();
				if (eyeAngles->x < -179.f) eyeAngles->x += 360.f;
				else if (eyeAngles->x > 90.0 || eyeAngles->x < -90.0) eyeAngles->x = 89.f;
				else if (eyeAngles->x > 89.0 && eyeAngles->x < 91.0) eyeAngles->x -= 90.f;
				else if (eyeAngles->x > 179.0 && eyeAngles->x < 181.0) eyeAngles->x -= 180;
				else if (eyeAngles->x > -179.0 && eyeAngles->x < -181.0) eyeAngles->x += 180;
				else if (fabs(eyeAngles->x) == 0) eyeAngles->x = std::copysign(89.0f, eyeAngles->x);
			}
		}
	}
}



float OldLowerBodyYaw[64];
float YawDelta[64];
float reset[64];
float Delta[64];
float Resolved_angles[64];
int iSmart;
static int jitter = -1;
float LatestLowerBodyYaw[64];
bool LbyUpdated[64];
float YawDifference[64];
float OldYawDifference[64];
float LatestLowerBodyYawUpdateTime[64];
float OldDeltaY;
float tolerance = 20.f;

Vector vecZero = Vector(0.0f, 0.0f, 0.0f);
QAngle angZero = QAngle(0.0f, 0.0f, 0.0f);

#define M_RADPI		57.295779513082f

QAngle CalcAngle(Vector src, Vector dst)
{
	Vector delta = src - dst;
	if (delta == vecZero)
	{
		return angZero;
	}

	float len = delta.Length();

	if (delta.z == 0.0f && len == 0.0f)
		return angZero;

	if (delta.y == 0.0f && delta.x == 0.0f)
		return angZero;

	QAngle angles;
	angles.x = (asinf(delta.z / delta.Length()) * M_RADPI);
	angles.y = (atanf(delta.y / delta.x) * M_RADPI);
	angles.z = 0.0f;
	if (delta.x >= 0.0f) { angles.y += 180.0f; }



	return angles;
}

void ResoSetup::OldY(IClientEntity* pEntity)
{

	float flInterval = Interfaces::Globals->interval_per_tick;

	if (pEntity->GetLowerBodyYaw() > 130 && pEntity->GetLowerBodyYaw() < -130)
	{
		StoreThings(pEntity);
	}
	else
	{
		pEntity->GetLowerBodyYaw() - 100 - rand() % 30;

		if (Resolver::didhitHS && pEntity->GetHealth() > 60)
			pEntity->GetLowerBodyYaw() - 90;
	}


}

/// People wanted Mutiny stuff, so here it is. I will let you make the resolver yourself.
//--------------------------------------------//

float Vec2Ang(Vector Velocity)
{
	if (Velocity.x == 0 || Velocity.y == 0)
		return 0;
	float rise = Velocity.x;
	float run = Velocity.y;
	float value = rise / run;
	float theta = atan(value);
	theta = RAD2DEG(theta) + 90;
	if (Velocity.y < 0 && Velocity.x > 0 || Velocity.y < 0 && Velocity.x < 0)
		theta *= -1;
	else
		theta = 180 - theta;
	return theta;
}
void clamp(float &value)
{
	while (value > 180)
		value -= 360;
	while (value < -180)
		value += 360;
}
float clamp2(float value)
{
	while (value > 180)
		value -= 360;
	while (value < -180)
		value += 360;
	return value;
}

float difference(float first, float second)
{
	clamp(first);
	clamp(second);
	float returnval = first - second;
	if (first < -91 && second> 91 || first > 91 && second < -91)
	{
		double negativedifY = 180 - abs(first);
		double posdiffenceY = 180 - abs(second);
		returnval = negativedifY + posdiffenceY;
	}
	return returnval;
}


float mean(float real[scanstrength], int slots)
{
	float total = 0;
	for (int i = 0; i < slots; i++)
	{
		total += real[i];
	}
	return total / slots;
}

float SD(float real[scanstrength], float mean, int slots)
{
	float total = 0;
	for (int i = 0; i < slots; i++)
	{
		total += (real[i] - mean)*(real[i] - mean);
	}
	total /= slots;
	return sqrt(total);
}
float accuracy = 10;
bool checkStatic(float real[scanstrength], float meanvalue, int slots)
{
	meanvalue = mean(real, slots);
	if (SD(real, meanvalue, slots) < accuracy)
	{
		return true;
	}
	return false;
}



#define SETANGLE 180 // basically zeus resolver

bool checkJitter(float real[scanstrength], int ticks[scanstrength], float &meanvalue, int &level, int slots)
{
	float dif[scanstrength - 1];

	int ticks2[scanstrength - 1];
	float dif2[scanstrength - 2];

	for (int i = 0; i < slots - 1; i++)
	{
		ticks2[i] = abs(ticks[i + 1] - ticks[i]);
		if (ticks2[i] == 1)
			ticks2[i] = 1;

		dif[i] = clamp2(abs(difference(real[i], real[i + 1]) / (ticks2[i])));
	}
	meanvalue = mean(dif, slots - 1);
	if (SD(dif, meanvalue, slots - 1) < accuracy)
	{
		level = 1;
		return true;
	}


	for (int i = 0; i < slots - 2; i++)
	{
		int ticks = abs(ticks2[i + 1] - ticks2[i]);
		if (ticks == 0)
			ticks = 1;

		dif2[i] = clamp2(abs(difference(dif[i], dif[i + 1]) / (ticks)));
	}
	meanvalue = mean(dif2, slots - 2);
	if (SD(dif2, meanvalue, slots - 2) < accuracy)
	{
		level = 2;
		return true;
	}

	return false;
}


bool checkSpin(float real[scanstrength], int ticks[scanstrength], float &meanvalue, int &level, int slots)
{
	float dif[scanstrength - 1];

	int ticks2[scanstrength - 1];
	float dif2[scanstrength - 2];

	for (int i = 0; i < slots - 1; i++)
	{
		ticks2[i] = abs(ticks[i + 1] - ticks[i]);
		if (ticks2[i] == 1)
			ticks2[i] = 1;
		dif[i] = clamp2(difference(real[i], real[i + 1]) / (ticks2[i]));
	}
	meanvalue = mean(dif, slots - 1);
	if (SD(dif, meanvalue, slots - 1) < accuracy)
	{
		level = 1;
		return true;
	}


	for (int i = 0; i < slots - 2; i++)
	{
		int ticks = abs(ticks2[i + 1] - ticks2[i]);
		if (ticks == 0)
			ticks = 1;

		dif2[i] = clamp2(difference(dif[i], dif[i + 1]) / (ticks));
	}
	meanvalue = mean(dif2, slots - 2);
	if (SD(dif2, meanvalue, slots - 2) < accuracy)
	{
		level = 2;
		return true;
	}

	return false;
}


struct BruteforceInfo
{
	enum BruteforceStep : unsigned int {
		BF_STEP_YAW_STANDING,
		BF_STEP_YAW_FAKEWALK,
		BF_STEP_YAW_AIR,
		BF_STEP_YAW_DUCKED,
		BF_STEP_YAW_PITCH,
		BF_STEP_COUNT,
	};

	unsigned char step[BF_STEP_COUNT];
	bool changeStep[BF_STEP_COUNT];
	bool missedBySpread;
	int missedCount;
	int spentBullets;
};

int GetEstimatedServerTickCount(float latency)
{
	return (int)floorf((float)((float)(latency) / (float)((uintptr_t)&Interfaces::Globals->interval_per_tick)) + 0.5) + 1 + (int)((uintptr_t)&Interfaces::Globals->tickcount);
}
inline float RandomFloat(float min, float max)
{
	static auto fn = (decltype(&RandomFloat))(GetProcAddress(GetModuleHandle("vstdlib.dll"), "RandomFloat"));
	return fn(min, max);
}

bool HasFakeHead(IClientEntity* pEntity)
{
	//lby should update if distance from lby to eye angles exceeds 35 degrees
	return abs(pEntity->GetEyeAnglesXY()->y - pEntity->GetLowerBodyYaw()) > 35;
}
bool Lbywithin35(IClientEntity* pEntity) {
	//lby should update if distance from lby to eye angles less than 35 degrees
	return abs(pEntity->GetEyeAnglesXY()->y - pEntity->GetLowerBodyYaw()) < 35;
}
bool IsMovingOnGround(IClientEntity* pEntity) {
	//Check if player has a velocity greater than 0 (moving) and if they are onground.
	return pEntity->GetVelocity().Length2D() > 45.f && pEntity->GetFlags() & FL_ONGROUND;
}
bool IsMovingOnInAir(IClientEntity* pEntity) {
	//Check if player has a velocity greater than 0 (moving) and if they are onground.
	return !(pEntity->GetFlags() & FL_ONGROUND);
}
bool OnGround(IClientEntity* pEntity) {
	//Check if player has a velocity greater than 0 (moving) and if they are onground.
	return pEntity->GetFlags() & FL_ONGROUND;
}
bool IsFakeWalking(IClientEntity* pEntity) {
	//Check if a player is moving, but at below a velocity of 36
	return IsMovingOnGround(pEntity) && pEntity->GetVelocity().Length2D() < 36.0f;
}

//--------------------------------------------//

void ResoSetup::Resolve(IClientEntity* pEntity, int CurrentTarget) // int + currenttarget = bad
{
	IClientEntity* pLocal = hackManager.pLocal();
	bool isAlive;

	float angletolerance;
	angletolerance = pEntity->GetEyeAnglesXY()->y + 180.0;
	float consider; 
	float v24;
	double v20;

	int rnd = rand() % 360;
	if (rnd < 30)
		rnd = 30;

	if (Interfaces::Engine->IsConnected() & pLocal->IsPlayer())
	{
		if (pLocal->IsAlive())
		{
#define RandomInt(nMin, nMax) (rand() % (nMax - nMin + 1) + nMin);
			std::string aa_info[64];

			
			//------bool------//

			std::vector<int> HitBoxesToScan;
		
		
		
			bool Prediction; //Func: Prediction
			bool MeetsLBYReq;
			bool maybefakehead = 0;
			bool shouldpredict;
			//------bool------//

			//------float------//
			float org_yaw;
			float server_time = pLocal->GetTickBase() * Interfaces::Globals->interval_per_tick;

			//------float------//

	
			//------Statics------//
			static Vector Skip[65];
			static float StaticHeadAngle = 0;

			static bool GetEyeAngles[65]; //Resolve: Frame EyeAngle
			static bool GetLowerBodyYawTarget[65]; //Resolve: LowerBody
			static bool isLBYPredictited[65];
			static bool switch2;

			static float OldLowerBodyYaws[65];
			static float OldYawDeltas[65];
			static float oldTimer[65];

			bool badresolverlmao;

			if (Globals::Shots >= 4)
			{
				if (pEntity->GetHealth() >= ResoSetup::storedhealth[pEntity->GetIndex()] -10) // lets compensate for nades n shit ghetto style
					badresolverlmao = true;
			}

		

			auto new_yaw = org_yaw;
			ResoSetup::storedhp[pEntity->GetIndex()] = pEntity->GetHealth();
			ResoSetup::NewANgles[pEntity->GetIndex()] = *pEntity->GetEyeAnglesXY();
			ResoSetup::newlby[pEntity->GetIndex()] = pEntity->GetLowerBodyYaw();
			ResoSetup::newsimtime = pEntity->GetSimulationTime();
			ResoSetup::newdelta[pEntity->GetIndex()] = pEntity->GetEyeAnglesXY()->y;
			ResoSetup::newlbydelta[pEntity->GetIndex()] = pEntity->GetLowerBodyYaw();
			ResoSetup::finaldelta[pEntity->GetIndex()] = ResoSetup::newdelta[pEntity->GetIndex()] - ResoSetup::storeddelta[pEntity->GetIndex()];
			ResoSetup::finallbydelta[pEntity->GetIndex()] = ResoSetup::newlbydelta[pEntity->GetIndex()] - ResoSetup::storedlbydelta[pEntity->GetIndex()];
			if (newlby == storedlby)
				ResoSetup::lbyupdated = false;
			else
				ResoSetup::lbyupdated = true;
			StoreThings(pEntity);

			IClientEntity* player = (IClientEntity*)pLocal;
			IClientEntity* pLocal = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
			INetChannelInfo *nci = Interfaces::Engine->GetNetChannelInfo();
			Vector* eyeAngles = player->GetEyeAnglesXY();

			if (pEntity->GetFlags() & FL_ONGROUND)
				MeetsLBYReq = true;
			else
				MeetsLBYReq = false;

			StoreFGE(pEntity);

			float Delta[64];
			float OldLowerBodyYaw;
			float Resolved_angles[64];
			static Vector vLast[65];
			static bool bShotLastTime[65];
			static bool bJitterFix[65];

			float prevlby = 0.f;
			int avg = 1;
			int count = 1;
			static float LatestLowerBodyYawUpdateTime[55];
			static float LatestLowerBodyYawUpdateTime1[55];

			static float time_at_update[65];
			float kevin[64];
			static bool bLowerBodyIsUpdated = false;

			if (pEntity->GetLowerBodyYaw() != kevin[pEntity->GetIndex()])
				bLowerBodyIsUpdated = true;
			else
				bLowerBodyIsUpdated = false;

			if (pEntity->GetVelocity().Length2D() > 0.1)
			{
				kevin[pEntity->GetIndex()] = pEntity->GetLowerBodyYaw();//storing their moving lby for later
				LatestLowerBodyYawUpdateTime[pEntity->GetIndex()] = pEntity->GetSimulationTime() + 0.22;

			}
			bool bHasAA;
			bool bSpinbot;


			bool IsMoving;

			if (pEntity->GetVelocity().Length2D() >= 1)
				IsMoving = true;
			else
				IsMoving = false;

			bool tolerantLBY;
			if (!MeetsLBYReq)
				if (pEntity->GetLowerBodyYawTarget() + 35)
					tolerantLBY = true;
				else
					tolerantLBY = false;

			bool IsBreathing;
			{
				bool idk = 0;
				{
					if (pEntity->GetEyePosition().y)
						idk = true;
					else
						idk = false;
				}

				server_time >= idk;
				if (server_time >= idk + 0.705f)
				{
					IsBreathing = true;
				}
				else
					IsBreathing = false;

			}

			float breakcomp = pEntity->GetEyeAnglesXY()->y - (pEntity->GetLowerBodyYaw() - 35);

			float healthbase = pEntity->GetHealth();
			float rnde = rand() % 160;
			if (rnde < 30)
				rnde = 30;


			bool IsStatic;
			{
				bool breathing = false;
				if (IsBreathing)
				{
					pEntity->GetEyePosition();
					IsStatic = true;
				}
				else
					IsStatic = false;
			}

			bool wrongsideleft;

			bool wrongsideright;

			float panicflip = ResoSetup::panicangle[pEntity->GetIndex()] + 30;
			bool shouldpanic;

			pEntity->GetChokedTicks();

			/*
			static float LastLBYUpdateTime = 0;
			IClientEntity* pLocal = hackManager.pLocal();
			float server_time = pLocal->GetTickBase() * Interfaces::Globals->interval_per_tick;
			if (server_time >= LastLBYUpdateTime)
			{
			LastLBYUpdateTime = server_time + 1.125f;

			*/


			bool faketolerance;
			{
				if (!IsMoving)
				{
					if (MeetsLBYReq)
					{
						pEntity->GetLowerBodyYaw();
					}
					StoreFGE(pEntity);
					if (!lbyupdated)
					{
						if (server_time >= StaticHeadAngle + 1.805f)
						{
							ResoSetup::storedlby[pEntity->GetIndex()] = maybefakehead;
						}
						if (maybefakehead = OldLowerBodyYaw)
						{
							if (tolerantLBY = maybefakehead & Resolver::didhitHS)
							{
								if (Resolver::didhitHS & pEntity->GetHealth() > ResoSetup::storedhealth[pEntity->GetIndex()])
									faketolerance = true;
								else
									faketolerance = false;
							}
						}

					}
				}


				float yaw = pEntity->GetLowerBodyYaw();





				bool IsFast;

				if (pEntity->GetFlags() & FL_ONGROUND & pEntity->GetVelocity().Length2D() >= 170.5)
					IsFast = true;
				else
					IsFast = false;


				int flip = (int)floorf(Interfaces::Globals->curtime / 0.34) % 3;
				static bool bFlipYaw;
				float flInterval = Interfaces::Globals->interval_per_tick;

				if (ResoSetup::storedlby[pEntity->GetIndex()] > yaw + 150)
					lbyupdated = true;



				if (Menu::Window.RageBotTab.AimbotResolver.GetIndex() == 0)
				{

				}
				else if (Menu::Window.RageBotTab.AimbotResolver.GetIndex() == 1)
				{
					int baim = rand() % 130;
					if (baim < 30)
						baim = 30;

					pEntity->GetEyeAnglesXY()->y - 45;

				

					if (pEntity->IsMoving()) pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw();
					else
					{
						switch (Globals::Shots % 6)
						{
						case 0: pEntity->GetEyeAnglesXY()->y -= 170; break; case 1: pEntity->GetEyeAnglesXY()->y += 170; break;
						case 2: pEntity->GetEyeAnglesXY()->y += 145; break; case 3: pEntity->GetEyeAnglesXY()->y -= 145; break;
						case 4: pEntity->GetEyeAnglesXY()->y -= 115; break; case 5: pEntity->GetEyeAnglesXY()->y += baim; break;
						}
					}
				}
				else if (Menu::Window.RageBotTab.AimbotResolver.GetIndex() == 2)
				{



					pEntity->GetEyeAnglesXY()->y + 35.f;

					if (pEntity->IsAlive() & pEntity->GetHealth())
						StoreFGE(pEntity);

					CUserCmd* pCmd;
					for (int i = 0; i < Interfaces::Engine->GetMaxClients(); ++i)
					{
						IClientEntity* pLocal = hackManager.pLocal();
						IClientEntity *player = (IClientEntity*)Interfaces::EntList->GetClientEntity(i);

						if (!player || player->IsDormant() || player->GetHealth() < 1 || (DWORD)player == (DWORD)pLocal)
							continue;

						if (!player)
							continue;

						if (pLocal)
							continue;

						if (pLocal && player && pLocal->IsAlive())
						{

							Vector* eyeAngles = player->GetEyeAnglesXY();

							if (eyeAngles->x > 90.0 || eyeAngles->x < 60.0) eyeAngles->x = -88.9f;
							else if (eyeAngles->x > 60.0 && eyeAngles->x < 20.0) eyeAngles->x = 50.f;
							else if (eyeAngles->x > 20.0 && eyeAngles->x < -30.0) eyeAngles->x = 0.f;
							else if (eyeAngles->x < -30.0 && eyeAngles->x < 60.0) eyeAngles->x = 55.f;



						}
						else
						{
							eyeAngles->x = 88;
							pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw();
						}
					}
					if (LatestLowerBodyYawUpdateTime[pEntity->GetIndex()] < pEntity->GetSimulationTime() || bLowerBodyIsUpdated)
					{
						pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw();
						LatestLowerBodyYawUpdateTime[pEntity->GetIndex()] = pEntity->GetSimulationTime() + 1.1;
					}
					else
					{
						if (IsMoving && MeetsLBYReq)
						{
							pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw();
							switch (Globals::Shots % 4)
							{
							case 0: pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw(); break;
							case 1: pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() - 30; break;
							case 2: pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() + 30; break;
							case 3: pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw(); break;
							}
						}
						else if (Lbywithin35(pEntity) && LatestLowerBodyYawUpdateTime[pEntity->GetIndex()] != pEntity->GetSimulationTime() + 1.1 && pEntity->GetVelocity().Length2D() > 1 && pEntity->GetVelocity().Length2D() <= 110)
						{
							switch (Globals::missedshots % 5)
							{
							case 0:
								pEntity->GetEyeAnglesXY()->y = OldLowerBodyYaw;
								break;
							case 1:
								pEntity->GetEyeAnglesXY()->y + OldLowerBodyYaw;
								break;
							case 2:
								pEntity->GetEyeAnglesXY()->y = *pEntity->GetLowerBodyYawTarget();
								break;
							case 3:
								pEntity->GetEyeAnglesXY()->y = breakcomp + 180;
								break;
							case 4:
								pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() + 130;
								break;
							case 5:
								pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() - 130;
								break;
							}
						}
						else if (MeetsLBYReq && IsFakeWalking(pEntity))
						{
							HitBoxesToScan.clear();
							HitBoxesToScan.push_back((int)CSGOHitboxID::Chest);
							HitBoxesToScan.push_back((int)CSGOHitboxID::Stomach);
							HitBoxesToScan.push_back((int)CSGOHitboxID::Pelvis);
							HitBoxesToScan.push_back((int)CSGOHitboxID::LowerChest);
							HitBoxesToScan.push_back((int)CSGOHitboxID::UpperChest);

						}
						else if (IsMoving && MeetsLBYReq)
						{
							switch (Globals::Shots % 7)
							{
							case 0: pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw(); break;
							case 1: pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() - 40; break;
							case 2: pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() + 40;
							case 3: pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw(); break;
							case 4: pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() - 75; break;
							case 5: pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() + 75; break;
							case 6: pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() + (160 + rand() % 40); break;
							}
						}
						else if (faketolerance && !IsMoving && MeetsLBYReq)
						{
							breakcomp;

							if (LatestLowerBodyYawUpdateTime[pEntity->GetIndex()] = pEntity->GetSimulationTime() + 1.1)
							{
								if (Lbywithin35(pEntity))
								{
									switch (Globals::missedshots % 6)
									{
									case 0:
										pEntity->GetEyeAnglesXY()->y = *pEntity->GetLowerBodyYawTarget() - 160;
										break;
									case 1:
										pEntity->GetEyeAnglesXY()->y = *pEntity->GetLowerBodyYawTarget() + 160;
										break;
									case 2:
										pEntity->GetEyeAnglesXY()->y + breakcomp;
										break;
									case 3:
										pEntity->GetEyeAnglesXY()->y = breakcomp - 60;
										break;
									case 4:
										pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() - 110;
										break;
									case 5:
										pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() + 110;
										break;
									}
								}
								else
								{
									switch (Globals::missedshots % 6)
									{
									case 0:
										pEntity->GetEyeAnglesXY()->y = *pEntity->GetLowerBodyYawTarget() - 50;
										break;
									case 1:
										pEntity->GetEyeAnglesXY()->y = *pEntity->GetLowerBodyYawTarget() + 50;
										break;
									case 2:
										pEntity->GetEyeAnglesXY()->y = breakcomp;
										break;
									case 3:
										pEntity->GetEyeAnglesXY()->y = (breakcomp - 55);
										break;
									case 4:
										pEntity->GetEyeAnglesXY()->y = (breakcomp + 55);
										break;
									case 5:
										pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() + (160 + rand() % 40);
										break;
									}
								}
							}
						}
						else if (HasFakeHead(pEntity))
						{
							pEntity->GetEyeAnglesXY()->y = pEntity->GetEyeAnglesXY()->y - pEntity->GetLowerBodyYaw();
							switch (Globals::missedshots % 4)
							{
							case 0:
								pEntity->GetEyeAnglesXY()->y = pEntity->GetEyeAnglesXY()->y - pEntity->GetLowerBodyYaw();
								break;
							case 1:
								pEntity->GetEyeAnglesXY()->y = pEntity->GetEyeAnglesXY()->y - pEntity->GetLowerBodyYaw();
								break;
							case 2:
								pEntity->GetEyeAnglesXY()->y = breakcomp;
								break;
							case 3:
								pEntity->GetEyeAnglesXY()->y = breakcomp - 60;
								break;
							}
						}
						else if (badresolverlmao)
						{
							pEntity->GetEyeAnglesXY()->y = *pEntity->GetLowerBodyYawTarget() - 155;
						}

					}
				}

				
				
				else if (Menu::Window.RageBotTab.AimbotResolver.GetIndex() == 3)
				{
				
					if (LatestLowerBodyYawUpdateTime[pEntity->GetIndex()] < pEntity->GetSimulationTime() || bLowerBodyIsUpdated)
					{
						pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw();
						LatestLowerBodyYawUpdateTime[pEntity->GetIndex()] = pEntity->GetSimulationTime() + 1.1;
					}
					else {

						if (IsMovingOnGround(pEntity))
						{
							if (IsFakeWalking(pEntity))
							{
								HitBoxesToScan.clear();
								HitBoxesToScan.push_back((int)CSGOHitboxID::Chest);
								HitBoxesToScan.push_back((int)CSGOHitboxID::Stomach);
								HitBoxesToScan.push_back((int)CSGOHitboxID::Pelvis);
								HitBoxesToScan.push_back((int)CSGOHitboxID::LowerChest);
								HitBoxesToScan.push_back((int)CSGOHitboxID::UpperChest);
							}
							pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw();
						}
						else if (IsMovingOnInAir(pEntity))
						{
							switch (Globals::Shots % 6)
							{
							case 0:
								pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() - 40;
								break;
							case 1:
								pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() + 40;
								break;
							case 2:
								pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() + 80;
								break;
							case 3:
								pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() + 110;
								break;
							case 4:
								pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() - 110;
								break;
							case 5:
								pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() - 180;
								break;

							}
						}
						else
						{
							if (HasFakeHead(pEntity))
							{
								pEntity->GetEyeAnglesXY()->y = pEntity->GetEyeAnglesXY()->y - pEntity->GetLowerBodyYaw();
							}
						
							if (IsMovingOnGround(pEntity))
								pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw();
							else if (MeetsLBYReq && !IsMoving && pEntity->GetHealth() >= ResoSetup::storedhealth[pEntity->GetIndex()])
							{
								if ((pEntity->GetEyeAnglesXY()->y + 180.0) <= 180.0)
								{
									if (angletolerance < -180.0)
										angletolerance = angletolerance + 360.0;
								}
								else
								{
									angletolerance = angletolerance - 360.0;
								}

								consider = angletolerance - pEntity->GetLowerBodyYaw();

								if (consider <= 180.0)
								{
									if (consider < -180.0)
										consider = consider + 360.0;
								}
								else
								{
									consider = consider - 360.0;
								}
								if (consider >= 0.0)
									v24 = RandomFloat(0.0, consider / 2);
								else
									v24 = RandomFloat(consider / 2, 0.0);
								v20 = v24 + pEntity->GetEyeAnglesXY()->y;
								pEntity->GetEyeAnglesXY()->y = v20;
							}

							else
							{
								if (Lbywithin35(pEntity))
								{
									switch (Globals::missedshots % 5)
									{
									case 0:
										pEntity->GetEyeAnglesXY()->y = *pEntity->GetLowerBodyYawTarget() - 175;
										break;
									case 1:
										pEntity->GetEyeAnglesXY()->y = *pEntity->GetLowerBodyYawTarget() + 175;
										break;
									case 2:
										pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() - 50;
										break;
									case 3:
										pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() + 50;
										break;
									case 4:
										pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() + 110;
										break;
									case 5:
										pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() - 110;
										break;
									}
								}
							}
						}
					}
				}
				else if (Menu::Window.MiscTab.OtherSafeMode.GetState() == false && Menu::Window.RageBotTab.AimbotResolver.GetIndex() == 4)
				{
					float pitch = pEntity->GetEyeAnglesXY()->x;
					float yaw = pEntity->GetEyeAnglesXY()->y;
					float lby = pEntity->GetLowerBodyYaw();
					float z_ang = pEntity->GetEyeAnglesXY()->z;
					float health = pEntity->GetHealth();
					StoreThings(pEntity);
					StoreExtra(pEntity);
					StoreFGE(pEntity);

					bool isfake;

					CUserCmd* pCmd;
					for (int i = 0; i < Interfaces::Engine->GetMaxClients(); ++i)
					{
						IClientEntity* pLocal = hackManager.pLocal();
						IClientEntity *player = (IClientEntity*)Interfaces::EntList->GetClientEntity(i);

						if (!player || player->IsDormant() || player->GetHealth() < 1 || (DWORD)player == (DWORD)pLocal)
							continue;

						if (!player)
							continue;

						if (pLocal)
							continue;

						if (pLocal && player && pLocal->IsAlive())
						{
							if (pitch < -40)
								pitch = 160;
							else if (pitch <= 40 && pitch >= -40)
								pitch = 91;
							else if (pitch > 40)
								pitch = -160;
							else pitch = -91;
							if (z_ang > 20)
							{
								z_ang = 0;
							}
							else if (z_ang < -20)
							{
								z_ang = 0;
							}
							else
								z_ang = 0;

							if (Globals::Shots % 3 && pEntity->GetHealth() > ResoSetup::storedhealth[pEntity->GetIndex()])
								isfake = true;
							else isfake = false;

							if (isfake)
								yaw - 120;
						}
						else
						{
							pitch = 0;
						}
					}
			

					if (MeetsLBYReq && !IsMoving)
					{
				
						if (health > 45)
						{
							switch (Globals::Shots % 14)
							{
							case 0: yaw - 180; break;
							case 1: lby = OldLowerBodyYaw; break;
							case 2: yaw = 90; break;
							case 3: yaw = -90; break;
							case 4: lby - 50; break;
							case 5: lby - 70; break;
							case 6: lby = -155; break;
							case 7: lby -= 0; break;
							case 8: lby = 160; break;
							case 9: lby = 50; break;
							case 10: lby = -55; break;
							case 11: (yaw - 45) - (lby - 30); break;
							case 12: (yaw + 45) + (lby + 30); break;
							case 13: (yaw = lby) + rand() % 360; break;
							}
						}
						else
						{
							switch (Globals::Shots % 14)
							{
							case 0: yaw - 110; break;
							case 1: yaw + 110; break;
							case 2: yaw = 180; break;
							case 3: yaw = -90; break;
							case 4: yaw = 90; break;
							case 5: (yaw = lby) - 50; break;
							case 6: (yaw = lby) + 100; break;
							case 7: (lby = 0); yaw = 35; break;
							case 8: (lby = 0); yaw = -35; break;
							case 9: (lby = 180); yaw = -155; break;
							case 10: (lby = 180); yaw = 155; break;
							case 11: (yaw - 65) - (lby - 50); break;
							case 12: (yaw + 95) + (lby + 60); break;
							case 13: (yaw = lby) - rand() % 360; break;
							}
						}
					}
					else
					{
					
						switch (Globals::Shots % 14)
						{
						case 0: yaw = lby; break;
						case 1: lby = OldLowerBodyYaw; break;
						case 2: (yaw = lby) - 35; break;
						case 3: (yaw = lby) - 75; break;
						case 4: (yaw = lby) + 115; break;
						case 5: (yaw = lby) + 115; break;
						case 6: (yaw = lby) + 65; break;
						case 7: (yaw = lby) - 180; break;
						case 8: lby = 160; break;
						case 9: (yaw = lby) + 30 + (rand() % 360); break;
						case 10: lby = -180; break;
						case 11: (yaw - 85) - (lby - 100); break;
						case 12: (yaw + 45) + (lby + 30); break;
						case 13: (yaw = lby) + rand() % 360; break;
						}
						
					}
				}
				else
				{
					CUserCmd* pCmd;
					for (int i = 0; i < Interfaces::Engine->GetMaxClients(); ++i)
					{
						IClientEntity* pLocal = hackManager.pLocal();
						IClientEntity *player = (IClientEntity*)Interfaces::EntList->GetClientEntity(i);

						if (!player || player->IsDormant() || player->GetHealth() < 1 || (DWORD)player == (DWORD)pLocal)
							continue;

						if (!player)
							continue;

						if (pLocal)
							continue;

						if (pLocal && player && pLocal->IsAlive())
						{
							pEntity->GetEyeAnglesXY()->x = 88;
						}
						pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() - 90;
					}

				}

			}
		}
	}
}


void ResoSetup::StoreFGE(IClientEntity* pEntity)
{
	ResoSetup::storedanglesFGE = pEntity->GetEyeAnglesXY()->y;
	ResoSetup::storedlbyFGE = pEntity->GetLowerBodyYaw();
	ResoSetup::storedsimtimeFGE = pEntity->GetSimulationTime();
}

void ResoSetup::StoreThings(IClientEntity* pEntity)
{
	ResoSetup::StoredAngles[pEntity->GetIndex()] = *pEntity->GetEyeAnglesXY();
	ResoSetup::storedlby[pEntity->GetIndex()] = pEntity->GetLowerBodyYaw();
	ResoSetup::storedsimtime = pEntity->GetSimulationTime();
	ResoSetup::storeddelta[pEntity->GetIndex()] = pEntity->GetEyeAnglesXY()->y;
	ResoSetup::storedlby[pEntity->GetIndex()] = pEntity->GetLowerBodyYaw();
}

void ResoSetup::anglestore(IClientEntity * pEntity)
{
	ResoSetup::badangle[pEntity->GetIndex()] = pEntity->GetLowerBodyYaw();

}

void ResoSetup::StoreExtra(IClientEntity * pEntity)
{
	ResoSetup::storedpanic[pEntity->GetIndex()] = pEntity->GetLowerBodyYaw();
	ResoSetup::panicangle[pEntity->GetIndex()] = pEntity->GetEyeAnglesXY()->y - 30;
	ResoSetup::storedhp[pEntity->GetIndex()] = pEntity->GetHealth();
}

void ResoSetup::CM(IClientEntity* pEntity)
{
	for (int x = 1; x < Interfaces::Engine->GetMaxClients(); x++)
	{

		pEntity = (IClientEntity*)Interfaces::EntList->GetClientEntity(x);

		if (!pEntity
			|| pEntity == hackManager.pLocal()
			|| pEntity->IsDormant()
			|| !pEntity->IsAlive())
			continue;

		ResoSetup::StoreThings(pEntity);
	}
}

void ResoSetup::FSN(IClientEntity* pEntity, ClientFrameStage_t stage)
{
	if (stage == ClientFrameStage_t::FRAME_NET_UPDATE_POSTDATAUPDATE_START)
	{
		for (int i = 1; i < Interfaces::Engine->GetMaxClients(); i++)
		{

			pEntity = (IClientEntity*)Interfaces::EntList->GetClientEntity(i);

			if (!pEntity
				|| pEntity == hackManager.pLocal()
				|| pEntity->IsDormant()
				|| !pEntity->IsAlive())
				continue;

			ResoSetup::Resolve(pEntity, stage);
		}
	}
}

