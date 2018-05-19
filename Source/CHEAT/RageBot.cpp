#include "RageBot.h"
#include "RenderManager.h"
#include "Resolver.h"
#include "Autowall.h"
#include <iostream>
#include "UTIL Functions.h"
#include "fakelaghelper.h"
#include "interfaces.h"
#include "quickmaths.h" // 2+2 = selfharm
#include "movedata.h"
#include "nospreadhelper.h"

void CRageBot::Init()
{

	IsAimStepping = false;
	IsLocked = false;
	TargetID = -1;
}

#define BEGIN_NAMESPACE( x ) namespace x {
#define END_NAMESPACE }

BEGIN_NAMESPACE(XorCompileTime)
constexpr auto time = __TIME__;
constexpr auto seed = static_cast<int>(time[7]) + static_cast<int>(time[6]) * 10 + static_cast<int>(time[4]) * 60 + static_cast<int>(time[3]) * 600 + static_cast<int>(time[1]) * 3600 + static_cast<int>(time[0]) * 36000;

// 1988, Stephen Park and Keith Miller
// "Random Number Generators: Good Ones Are Hard To Find", considered as "minimal standard"
// Park-Miller 31 bit pseudo-random number generator, implemented with G. Carta's optimisation:
// with 32-bit math and without division

template <int N>
struct RandomGenerator
{
private:
	static constexpr unsigned a = 16807; // 7^5
	static constexpr unsigned m = 2147483647; // 2^31 - 1

	static constexpr unsigned s = RandomGenerator<N - 1>::value;
	static constexpr unsigned lo = a * (s & 0xFFFF); // Multiply lower 16 bits by 16807
	static constexpr unsigned hi = a * (s >> 16); // Multiply higher 16 bits by 16807
	static constexpr unsigned lo2 = lo + ((hi & 0x7FFF) << 16); // Combine lower 15 bits of hi with lo's upper bits
	static constexpr unsigned hi2 = hi >> 15; // Discard lower 15 bits of hi
	static constexpr unsigned lo3 = lo2 + hi;

public:
	static constexpr unsigned max = m;
	static constexpr unsigned value = lo3 > m ? lo3 - m : lo3;
};

template <>
struct RandomGenerator<0>
{
	static constexpr unsigned value = seed;
};

template <int N, int M>
struct RandomInt
{
	static constexpr auto value = RandomGenerator<N + 1>::value % M;
};

template <int N>
struct RandomChar
{
	static const char value = static_cast<char>(1 + RandomInt<N, 0x7F - 1>::value);
};

template <size_t N, int K>
struct XorString
{
private:
	const char _key;
	std::array<char, N + 1> _encrypted;

	constexpr char enc(char c) const
	{
		return c ^ _key;
	}

	char dec(char c) const
	{
		return c ^ _key;
	}

public:
	template <size_t... Is>
	constexpr __forceinline XorString(const char* str, std::index_sequence<Is...>) : _key(RandomChar<K>::value), _encrypted{ enc(str[Is])... }
	{
	}

	__forceinline decltype(auto) decrypt(void)
	{
		for (size_t i = 0; i < N; ++i)
		{
			_encrypted[i] = dec(_encrypted[i]);
		}
		_encrypted[N] = '\0';
		return _encrypted.data();
	}
};

//--------------------------------------------------------------------------------
//-- Note: XorStr will __NOT__ work directly with functions like printf.
//         To work with them you need a wrapper function that takes a const char*
//         as parameter and passes it to printf and alike.
//
//         The Microsoft Compiler/Linker is not working correctly with variadic 
//         templates!
//  
//         Use the functions below or use std::cout (and similar)!
//--------------------------------------------------------------------------------

static auto w_printf = [](const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vprintf_s(fmt, args);
	va_end(args);
};

static auto w_printf_s = [](const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vprintf_s(fmt, args);
	va_end(args);
};

static auto w_sprintf = [](char* buf, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	int len = _vscprintf(fmt, args) // _vscprintf doesn't count  
		+ 1; // terminating '\0' 
	vsprintf_s(buf, len, fmt, args);
	va_end(args);
};

static auto w_sprintf_s = [](char* buf, size_t buf_size, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vsprintf_s(buf, buf_size, fmt, args);
	va_end(args);
};
static bool w_strcmp(const char* str1, const char* str2)
{
	return strcmp(str1, str2);
};
//#define XorStr( s ) ( XorCompileTime::XorString< sizeof( s ) - 1, __COUNTER__ >( s, std::make_index_sequence< sizeof( s ) - 1>() ).decrypt() )
/*#ifdef NDEBUG //nDEBUG
#define XorStr( s ) ( XorCompileTime::XorString< sizeof( s ) - 1, __COUNTER__ >( s, std::make_index_sequence< sizeof( s ) - 1>() ).decrypt() )
#elif*/
#define XorStr( s ) ( s )
//#endif

END_NAMESPACE

void CRageBot::Prediction(CUserCmd* pCmd, IClientEntity* LocalPlayer)
{
	if (Interfaces::MoveHelper && Menu::Window.RageBotTab.AimbotEnable.GetState() && Menu::Window.RageBotTab.posadjust.GetState() && LocalPlayer->IsAlive())
	{
		float curtime = Interfaces::Globals->curtime;
		float frametime = Interfaces::Globals->frametime;
		int iFlags = LocalPlayer->GetFlags();

		Interfaces::Globals->curtime = (float)LocalPlayer->GetTickBase() * Interfaces::Globals->interval_per_tick;
		Interfaces::Globals->frametime = Interfaces::Globals->interval_per_tick;

		Interfaces::MoveHelper->SetHost(LocalPlayer);

		Interfaces::Prediction1->SetupMove(LocalPlayer, pCmd, nullptr, bMoveData);
		Interfaces::GameMovement->ProcessMovement(LocalPlayer, bMoveData);
		Interfaces::Prediction1->FinishMove(LocalPlayer, pCmd, bMoveData);

		Interfaces::MoveHelper->SetHost(0);

		Interfaces::Globals->curtime = curtime;
		Interfaces::Globals->frametime = frametime;
	}
}



float InterpFix()
{

		static ConVar* cvar_cl_interp = Interfaces::CVar->FindVar("cl_interp");
		static ConVar* cvar_cl_updaterate = Interfaces::CVar->FindVar("cl_updaterate");
		static ConVar* cvar_sv_maxupdaterate = Interfaces::CVar->FindVar("sv_maxupdaterate");
		static ConVar* cvar_sv_minupdaterate = Interfaces::CVar->FindVar("sv_minupdaterate");
		static ConVar* cvar_cl_interp_ratio = Interfaces::CVar->FindVar("cl_interp_ratio");

		IClientEntity* pLocal = hackManager.pLocal();
		CBaseCombatWeapon* pWeapon = (CBaseCombatWeapon*)Interfaces::EntList->GetClientEntityFromHandle(hackManager.pLocal()->GetActiveWeaponHandle());

		float cl_interp = cvar_cl_interp->GetFloat();
		int cl_updaterate = cvar_cl_updaterate->GetInt();
		int sv_maxupdaterate = cvar_sv_maxupdaterate->GetInt();
		int sv_minupdaterate = cvar_sv_minupdaterate->GetInt();
		int cl_interp_ratio = cvar_cl_interp_ratio->GetInt();

		if (sv_maxupdaterate <= cl_updaterate)
			cl_updaterate = sv_maxupdaterate;

		if (sv_minupdaterate > cl_updaterate)
			cl_updaterate = sv_minupdaterate;

		float new_interp = (float)cl_interp_ratio / (float)cl_updaterate;

		if (new_interp > cl_interp)
			cl_interp = new_interp;

		return max(cl_interp, cl_interp_ratio / cl_updaterate);
	
	// 
}

void FakeWalk(CUserCmd * pCmd, bool & bSendPacket)
{

	IClientEntity* pLocal = hackManager.pLocal();
	if (GetAsyncKeyState(VK_SHIFT))
	{

		static int iChoked = -1;
		iChoked++;

		if (iChoked < 1)
		{
			bSendPacket = false;



			pCmd->tick_count += 5.95; // 10.95
			pCmd->command_number += 3.07 + pCmd->tick_count % 2 ? 0 : 1; // 5
	
			pCmd->buttons |= pLocal->GetMoveType() == IN_BACK;
			pCmd->forwardmove = pCmd->sidemove = 0.f;
		}
		else
		{
			bSendPacket = true;
			iChoked = -1;

			Interfaces::Globals->frametime *= (pLocal->GetVelocity().Length2D()) / 6; // 10
			pCmd->buttons |= pLocal->GetMoveType() == IN_FORWARD;
		}
	}
}

void CRageBot::Draw()
{

}

bool IsAbleToShoot(IClientEntity* pLocal)
{
	CBaseCombatWeapon* pWeapon = (CBaseCombatWeapon*)Interfaces::EntList->GetClientEntityFromHandle(pLocal->GetActiveWeaponHandle());
	if (!pLocal)return false;
	if (!pWeapon)return false;
	float flServerTime = pLocal->GetTickBase() * Interfaces::Globals->interval_per_tick;
	return (!(pWeapon->GetNextPrimaryAttack() > flServerTime));
}

float hitchance(IClientEntity* pLocal, CBaseCombatWeapon* pWeapon)
{
	float hitchance = 101;
	if (!pWeapon) return 0;
	if (Menu::Window.RageBotTab.AccuracyHitchance.GetValue() > 1)
	{
		float inaccuracy = pWeapon->GetInaccuracy();
		if (inaccuracy == 0) inaccuracy = 0.000001;
		inaccuracy = 1 / inaccuracy;
		hitchance = inaccuracy;
	}
	return hitchance;
}

bool CanOpenFire() 
{
	IClientEntity* pLocalEntity = (IClientEntity*)Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
	if (!pLocalEntity)
		return false;

	CBaseCombatWeapon* entwep = (CBaseCombatWeapon*)Interfaces::EntList->GetClientEntityFromHandle(pLocalEntity->GetActiveWeaponHandle());

	float flServerTime = (float)pLocalEntity->GetTickBase() * Interfaces::Globals->interval_per_tick;
	float flNextPrimaryAttack = entwep->GetNextPrimaryAttack();

	std::cout << flServerTime << " " << flNextPrimaryAttack << std::endl;

	return !(flNextPrimaryAttack > flServerTime);
}

void CRageBot::Move(CUserCmd *pCmd, bool &bSendPacket)
{

	IClientEntity* pLocalEntity = (IClientEntity*)Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());

	if (!pLocalEntity || !Menu::Window.RageBotTab.Active.GetState() || !Interfaces::Engine->IsConnected() || !Interfaces::Engine->IsInGame())
		return;

	if (Menu::Window.RageBotTab.AntiAimEnable.GetState())
	{
		static int ChokedPackets = -1;

		CBaseCombatWeapon* pWeapon = (CBaseCombatWeapon*)Interfaces::EntList->GetClientEntityFromHandle(hackManager.pLocal()->GetActiveWeaponHandle());
		if (!pWeapon)
			return;

		if (ChokedPackets < 1 && pLocalEntity->GetLifeState() == LIFE_ALIVE && pCmd->buttons & IN_ATTACK && CanOpenFire() && GameUtils::IsBallisticWeapon(pWeapon))
		{
			bSendPacket = false;
		}
		else
		{
			if (pLocalEntity->GetLifeState() == LIFE_ALIVE)
			{
				DoAntiAim(pCmd, bSendPacket);

			}
			ChokedPackets = 1;
		}
	}

	if (Menu::Window.RageBotTab.AimbotEnable.GetState())
		DoAimbot(pCmd, bSendPacket);

	if (Menu::Window.RageBotTab.AccuracyRecoil.GetIndex() == 0)
	{

	}
	else if (Menu::Window.RageBotTab.AccuracyRecoil.GetIndex() == 1)
	{
		DoNoRecoil(pCmd);
	}

	if (Menu::Window.RageBotTab.positioncorrect.GetState())
		pCmd->tick_count = TIME_TO_TICKS(InterpFix());

	if (Menu::Window.RageBotTab.AimbotAimStep.GetState())
	{
		Vector AddAngs = pCmd->viewangles - LastAngle;
		if (AddAngs.Length2D() > 25.f)
		{
			Normalize(AddAngs, AddAngs);
			AddAngs *= 23;
			pCmd->viewangles = LastAngle + AddAngs;
			GameUtils::NormaliseViewAngle(pCmd->viewangles);
		}
	}

	LastAngle = pCmd->viewangles;
}

Vector BestPoint(IClientEntity *targetPlayer, Vector &final)
{
	IClientEntity* pLocal = hackManager.pLocal();
	CBaseCombatWeapon* pWeapon = (CBaseCombatWeapon*)Interfaces::EntList->GetClientEntityFromHandle(hackManager.pLocal()->GetActiveWeaponHandle());


	trace_t tr;
	Ray_t ray;
	CTraceFilter filter;

	filter.pSkip = targetPlayer;
	ray.Init(final + Vector(0, 0, 0), final); // all 0 or 0,0,10, either way, should hit the top of the head, but its weird af.
	Interfaces::Trace->TraceRay(ray, MASK_SHOT, &filter, &tr);

	// if you're actually one to look through this stuff, do tag me @ discord and tell me.

	final = tr.endpos;
	return final;
}

bool CRageBot::extra(bool &bSendPacket)
{
	static int ChokedPackets = -1;
	ChokedPackets++;
	bool canchoke;


	if (Menu::Window.RageBotTab.chokepackets.GetState())
		canchoke = true;
	else
		canchoke = false;

	if (canchoke == true)
		return ChokedPackets = 2;
	else
		return ChokedPackets = 0;
}

void CRageBot::DoAimbot(CUserCmd *pCmd, bool &bSendPacket)
{


	static int ChokedPackets = -1;
	ChokedPackets++;
	IClientEntity* pTarget = nullptr;
	IClientEntity* pLocal = hackManager.pLocal();
	Vector Start = pLocal->GetViewOffset() + pLocal->GetOrigin();
	bool FindNewTarget = true;
	CSWeaponInfo* weapInfo = ((CBaseCombatWeapon*)Interfaces::EntList->GetClientEntityFromHandle(pLocal->GetActiveWeaponHandle()))->GetCSWpnData();
	CBaseCombatWeapon* pWeapon = (CBaseCombatWeapon*)Interfaces::EntList->GetClientEntityFromHandle(pLocal->GetActiveWeaponHandle());

	Vector Point;



	if (Menu::Window.RageBotTab.AutoRevolver.GetState())
		if (GameUtils::IsRevolver(pWeapon))
		{
			static int delay = 0;
			delay++;
			if (delay <= 15)pCmd->buttons |= IN_ATTACK;
			else delay = 0;
		}
	if (pWeapon)
	{
		if (pWeapon->GetAmmoInClip() == 0 || !GameUtils::IsBallisticWeapon(pWeapon)) return;
	}
	else return;
	if (IsLocked && TargetID >= 0 && HitBox >= 0)
	{
		pTarget = Interfaces::EntList->GetClientEntity(TargetID);
		if (pTarget  && TargetMeetsRequirements(pTarget))
		{
			HitBox = HitScan(pTarget);
			if (HitBox >= 0)
			{
				Vector ViewOffset = pLocal->GetOrigin() + pLocal->GetViewOffset(), View;
				Interfaces::Engine->GetViewAngles(View);
				float FoV = FovToPlayer(ViewOffset, View, pTarget, HitBox);
				if (FoV < Menu::Window.RageBotTab.AimbotFov.GetValue())	FindNewTarget = false;
			}
		}
	}


	if (FindNewTarget)
	{
		TargetID = 0;
		pTarget = nullptr;
		HitBox = -1;
		switch (Menu::Window.RageBotTab.TargetSelection.GetIndex())
		{
		case 0:TargetID = GetTargetCrosshair(); break;
		case 1:TargetID = GetTargetDistance(); break;
		case 2:TargetID = GetTargetHealth(); break;
		case 3:TargetID = GetTargetThreat(pCmd); break;
		case 4:TargetID = GetTargetNextShot(); break;
		}
		if (TargetID >= 0) pTarget = Interfaces::EntList->GetClientEntity(TargetID);
		else
		{


			pTarget = nullptr;
			HitBox = -1;
		}
	} 
	Globals::Target = pTarget;
	Globals::TargetID = TargetID;
	if (TargetID >= 0 && pTarget)
	{
		HitBox = HitScan(pTarget);

		if (!CanOpenFire()) return;

		if (Menu::Window.RageBotTab.AimbotKeyPress.GetState())
		{


			int Key = Menu::Window.RageBotTab.AimbotKeyBind.GetKey();
			if (Key >= 0 && !GUI.GetKeyState(Key))
			{
				TargetID = -1;
				pTarget = nullptr;
				HitBox = -1;
				return;
			}
		}
		float pointscale = Menu::Window.RageBotTab.TargetPointscale.GetValue() - 3.2f; 
		Vector AimPoint = GetHitboxPosition(pTarget, HitBox) + Vector(0, 0, pointscale);
		if (Menu::Window.RageBotTab.TargetMultipoint.GetState()) Point = BestPoint(pTarget, AimPoint);
		else Point = AimPoint;

		if (GameUtils::IsScopedWeapon(pWeapon) && !pWeapon->IsScoped() && Menu::Window.RageBotTab.AccuracyAutoScope.GetState()) pCmd->buttons |= IN_ATTACK2;
		else if ((Menu::Window.RageBotTab.AccuracyHitchance.GetValue() * 1.5 <= hitchance(pLocal, pWeapon)) || Menu::Window.RageBotTab.AccuracyHitchance.GetValue() == 0 || *pWeapon->m_AttributeManager()->m_Item()->ItemDefinitionIndex() == 64)
			{
			if (pTarget->GetChokedTicks() > 4)
			{
				if (Menu::Window.RageBotTab.AimbotAutoFire.GetState() && !(pCmd->buttons & IN_ATTACK))
				{
					pCmd->buttons |= IN_ATTACK;
					AimPoint -= pTarget->GetAbsOrigin();
					AimPoint += pTarget->GetNetworkOrigin();
				}
				else if (pCmd->buttons & IN_ATTACK || pCmd->buttons & IN_ATTACK2)return;
			}
			else if (AimAtPoint(pLocal, Point, pCmd, bSendPacket))
					if (Menu::Window.RageBotTab.AimbotAutoFire.GetState() && !(pCmd->buttons & IN_ATTACK))pCmd->buttons |= IN_ATTACK;
					else if (pCmd->buttons & IN_ATTACK || pCmd->buttons & IN_ATTACK2)return;
			}
		if (IsAbleToShoot(pLocal) && pCmd->buttons & IN_ATTACK) { Globals::Shots += 1;  ChokedPackets = 2; }
	}

}

bool CRageBot::TargetMeetsRequirements(IClientEntity* pEntity)
{
	if (pEntity && pEntity->IsDormant() == false && pEntity->IsAlive() && pEntity->GetIndex() != hackManager.pLocal()->GetIndex())
	{

		ClientClass *pClientClass = pEntity->GetClientClass();
		player_info_t pinfo;
		if (pClientClass->m_ClassID == (int)CSGOClassID::CCSPlayer && Interfaces::Engine->GetPlayerInfo(pEntity->GetIndex(), &pinfo))
		{
			if (pEntity->GetTeamNum() != hackManager.pLocal()->GetTeamNum() || Menu::Window.RageBotTab.TargetFriendlyFire.GetState())
			{
				if (!pEntity->HasGunGameImmunity())
				{
					return true;
				}
			}
		}

	}

	return false;
}

float CRageBot::FovToPlayer(Vector ViewOffSet, Vector View, IClientEntity* pEntity, int aHitBox)
{
	CONST FLOAT MaxDegrees = 180.0f;

	Vector Angles = View;

	Vector Origin = ViewOffSet;

	Vector Delta(0, 0, 0);

	Vector Forward(0, 0, 0);

	AngleVectors(Angles, &Forward);
	Vector AimPos = GetHitboxPosition(pEntity, aHitBox);

	VectorSubtract(AimPos, Origin, Delta);

	Normalize(Delta, Delta);

	FLOAT DotProduct = Forward.Dot(Delta);

	return (acos(DotProduct) * (MaxDegrees / PI));
}

int CRageBot::GetTargetCrosshair()
{

	int target = -1;
	float minFoV = Menu::Window.RageBotTab.AimbotFov.GetValue();

	IClientEntity* pLocal = hackManager.pLocal();
	Vector ViewOffset = pLocal->GetOrigin() + pLocal->GetViewOffset();
	Vector View; Interfaces::Engine->GetViewAngles(View);

	for (int i = 0; i < Interfaces::EntList->GetMaxEntities(); i++)
	{
		IClientEntity *pEntity = Interfaces::EntList->GetClientEntity(i);
		if (TargetMeetsRequirements(pEntity))
		{
			int NewHitBox = HitScan(pEntity);
			if (NewHitBox >= 0)
			{
				float fov = FovToPlayer(ViewOffset, View, pEntity, 0);
				if (fov < minFoV)
				{
					minFoV = fov;
					target = i;
				}
			}

		}
	}

	return target;
}

int CRageBot::GetTargetDistance()
{

	int target = -1;
	int minDist = 99999;

	IClientEntity* pLocal = hackManager.pLocal();
	Vector ViewOffset = pLocal->GetOrigin() + pLocal->GetViewOffset();
	Vector View; Interfaces::Engine->GetViewAngles(View);

	for (int i = 0; i < Interfaces::EntList->GetMaxEntities(); i++)
	{
		IClientEntity *pEntity = Interfaces::EntList->GetClientEntity(i);
		if (TargetMeetsRequirements(pEntity))
		{

			int NewHitBox = HitScan(pEntity);
			if (NewHitBox >= 0)
			{
				Vector Difference = pLocal->GetOrigin() - pEntity->GetOrigin();
				int Distance = Difference.Length();
				float fov = FovToPlayer(ViewOffset, View, pEntity, 0);
				if (Distance < minDist && fov < Menu::Window.RageBotTab.AimbotFov.GetValue())
				{
					minDist = Distance;
					target = i;
				}
			}

		}
	}

	return target;
}

int CRageBot::GetTargetNextShot()
{
	int target = -1;
	int minfov = 361;

	IClientEntity* pLocal = hackManager.pLocal();
	Vector ViewOffset = pLocal->GetOrigin() + pLocal->GetViewOffset();
	Vector View; Interfaces::Engine->GetViewAngles(View);

	for (int i = 0; i < Interfaces::EntList->GetMaxEntities(); i++)
	{

		IClientEntity *pEntity = Interfaces::EntList->GetClientEntity(i);
		if (TargetMeetsRequirements(pEntity))
		{
			int NewHitBox = HitScan(pEntity);
			if (NewHitBox >= 0)
			{
				int Health = pEntity->GetHealth();
				float fov = FovToPlayer(ViewOffset, View, pEntity, 0);
				if (fov < minfov && fov < Menu::Window.RageBotTab.AimbotFov.GetValue())
				{
					minfov = fov;
					target = i;
				}
				else
					minfov = 361;
			}

		}
	}

	return target;
}

float GetFov(const QAngle& viewAngle, const QAngle& aimAngle)
{
	Vector ang, aim;

	AngleVectors(viewAngle, &aim);
	AngleVectors(aimAngle, &ang);

	return RAD2DEG(acos(aim.Dot(ang) / aim.LengthSqr()));
}

double inline __declspec (naked) __fastcall FASTSQRT(double n)
{
	_asm fld qword ptr[esp + 4]
		_asm fsqrt
	_asm ret 8
}

float VectorDistance(Vector v1, Vector v2)
{
	return FASTSQRT(pow(v1.x - v2.x, 2) + pow(v1.y - v2.y, 2) + pow(v1.z - v2.z, 2));
}

int CRageBot::GetTargetThreat(CUserCmd* pCmd)
{
	auto iBestTarget = -1;
	float flDistance = 8192.f;

	IClientEntity* pLocal = hackManager.pLocal();

	for (int i = 0; i < Interfaces::EntList->GetHighestEntityIndex(); i++)
	{
		IClientEntity *pEntity = Interfaces::EntList->GetClientEntity(i);
		if (TargetMeetsRequirements(pEntity))
		{
			int NewHitBox = HitScan(pEntity);
			auto vecHitbox = pEntity->GetBonePos(NewHitBox);
			if (NewHitBox >= 0)
			{

				Vector Difference = pLocal->GetOrigin() - pEntity->GetOrigin();
				QAngle TempTargetAbs;
				CalcAngle(pLocal->GetEyePosition(), vecHitbox, TempTargetAbs);
				float flTempFOVs = GetFov(pCmd->viewangles, TempTargetAbs);
				float flTempDistance = VectorDistance(pLocal->GetOrigin(), pEntity->GetOrigin());
				if (flTempDistance < flDistance && flTempFOVs < Menu::Window.RageBotTab.AimbotFov.GetValue())
				{
					flDistance = flTempDistance;
					iBestTarget = i;
				}
			}
		}
	}
	return iBestTarget;
}

int CRageBot::GetTargetHealth()
{

	int target = -1;
	int minHealth = 101;

	IClientEntity* pLocal = hackManager.pLocal();
	Vector ViewOffset = pLocal->GetOrigin() + pLocal->GetViewOffset();
	Vector View; Interfaces::Engine->GetViewAngles(View);


	for (int i = 0; i < Interfaces::EntList->GetMaxEntities(); i++)
	{
		IClientEntity *pEntity = Interfaces::EntList->GetClientEntity(i);
		if (TargetMeetsRequirements(pEntity))
		{
			int NewHitBox = HitScan(pEntity);
			if (NewHitBox >= 0)
			{
				int Health = pEntity->GetHealth();
				float fov = FovToPlayer(ViewOffset, View, pEntity, 0);
				if (Health < minHealth && fov < Menu::Window.RageBotTab.AimbotFov.GetValue())
				{
					minHealth = Health;
					target = i;
				}
			}
		}

	}

	return target;
}

int CRageBot::HitScan(IClientEntity* pEntity)
{
	IClientEntity* pLocal = hackManager.pLocal();
	std::vector<int> HitBoxesToScan;
	
#pragma region GetHitboxesToScan
	int huso = (pEntity->GetHealth());
	int health = Menu::Window.RageBotTab.BaimIfUnderXHealth.GetValue();

	bool AWall = Menu::Window.RageBotTab.AccuracyAutoWall.GetState();
	bool Multipoint = Menu::Window.RageBotTab.TargetMultipoint.GetState();
	int TargetHitbox = Menu::Window.RageBotTab.TargetHitbox.GetIndex();
	static bool enemyHP = false;
	IClientEntity* WeaponEnt = Interfaces::EntList->GetClientEntityFromHandle(pLocal->GetActiveWeaponHandle());
	CBaseCombatWeapon* pWeapon = (CBaseCombatWeapon*)Interfaces::EntList->GetClientEntityFromHandle(hackManager.pLocal()->GetActiveWeaponHandle());
	int xs;
	int ys;
	Interfaces::Engine->GetScreenSize(xs, ys);
	xs /= 2; ys /= 2;

	auto accuracy = pWeapon->GetInaccuracy() * 550.f; //3000


	int AimbotBaimOnKey = Menu::Window.RageBotTab.AimbotBaimOnKey.GetKey();
	if (AimbotBaimOnKey >= 0 && GUI.GetKeyState(AimbotBaimOnKey))
	{
		HitBoxesToScan.push_back((int)CSGOHitboxID::Stomach); // 4
		HitBoxesToScan.push_back((int)CSGOHitboxID::Pelvis);
		HitBoxesToScan.push_back((int)CSGOHitboxID::LeftThigh); // 9
		HitBoxesToScan.push_back((int)CSGOHitboxID::RightThigh); // 8
		HitBoxesToScan.push_back((int)CSGOHitboxID::LeftFoot); // 13
		HitBoxesToScan.push_back((int)CSGOHitboxID::RightFoot); // 12
	}


	if (huso < health)
	{
		HitBoxesToScan.push_back((int)CSGOHitboxID::Pelvis);
		HitBoxesToScan.push_back((int)CSGOHitboxID::Stomach);
		HitBoxesToScan.push_back((int)CSGOHitboxID::Chest);
		HitBoxesToScan.push_back((int)CSGOHitboxID::UpperChest);
		HitBoxesToScan.push_back((int)CSGOHitboxID::LowerChest);
		HitBoxesToScan.push_back((int)CSGOHitboxID::LeftUpperArm);
		HitBoxesToScan.push_back((int)CSGOHitboxID::RightUpperArm);
		HitBoxesToScan.push_back((int)CSGOHitboxID::LeftThigh);
		HitBoxesToScan.push_back((int)CSGOHitboxID::RightThigh);
		HitBoxesToScan.push_back((int)CSGOHitboxID::LeftHand);
		HitBoxesToScan.push_back((int)CSGOHitboxID::RightHand);
		HitBoxesToScan.push_back((int)CSGOHitboxID::LeftFoot);
		HitBoxesToScan.push_back((int)CSGOHitboxID::RightFoot);
		HitBoxesToScan.push_back((int)CSGOHitboxID::LeftShin);
		HitBoxesToScan.push_back((int)CSGOHitboxID::RightShin);
		HitBoxesToScan.push_back((int)CSGOHitboxID::LeftLowerArm);
		HitBoxesToScan.push_back((int)CSGOHitboxID::RightLowerArm);
	}

	else if (Globals::Shots >= Menu::Window.RageBotTab.xaneafterX.GetValue())
	{
		HitBoxesToScan.push_back((int)CSGOHitboxID::Neck);
		HitBoxesToScan.push_back((int)CSGOHitboxID::Stomach);
		HitBoxesToScan.push_back((int)CSGOHitboxID::Chest);
		HitBoxesToScan.push_back((int)CSGOHitboxID::Pelvis);
		HitBoxesToScan.push_back((int)CSGOHitboxID::LeftFoot);
		HitBoxesToScan.push_back((int)CSGOHitboxID::RightFoot);
		HitBoxesToScan.push_back((int)CSGOHitboxID::LeftShin);
		HitBoxesToScan.push_back((int)CSGOHitboxID::RightShin);

	}
	else if (Menu::Window.RageBotTab.AWPAtBody.GetState() && GameUtils::AWP(pWeapon))
	{
		HitBoxesToScan.push_back((int)CSGOHitboxID::Chest);
		HitBoxesToScan.push_back((int)CSGOHitboxID::Stomach);
		HitBoxesToScan.push_back((int)CSGOHitboxID::Pelvis);
		HitBoxesToScan.push_back((int)CSGOHitboxID::LowerChest);
		HitBoxesToScan.push_back((int)CSGOHitboxID::UpperChest);
	}
	else if (TargetHitbox)
	{
		switch (Menu::Window.RageBotTab.TargetHitbox.GetIndex())
		{
		case 0:
		{ /* wont shoot anything at all. Peace and love bruuuuh */ } break;
		case 1:
			HitBoxesToScan.push_back((int)CSGOHitboxID::Head); // 1
			HitBoxesToScan.push_back((int)CSGOHitboxID::Neck); // lets compensate a bit
			if (Menu::Window.RageBotTab.baimfactor.GetIndex() != 0)
			{
				HitBoxesToScan.push_back((int)CSGOHitboxID::Head);
				HitBoxesToScan.push_back((int)CSGOHitboxID::Stomach);
				HitBoxesToScan.push_back((int)CSGOHitboxID::Pelvis);
				HitBoxesToScan.push_back((int)CSGOHitboxID::LeftShin);
				HitBoxesToScan.push_back((int)CSGOHitboxID::RightShin);
			}
			else
			{
				HitBoxesToScan.push_back((int)CSGOHitboxID::Head);
				HitBoxesToScan.push_back((int)CSGOHitboxID::Neck);
			}
			break;
		case 2:
		{
			switch (Menu::Window.RageBotTab.baimfactor.GetIndex())
			{
			case 0:
			{
				HitBoxesToScan.push_back((int)CSGOHitboxID::Head);
				HitBoxesToScan.push_back((int)CSGOHitboxID::Neck);
				HitBoxesToScan.push_back((int)CSGOHitboxID::Stomach);
				HitBoxesToScan.push_back((int)CSGOHitboxID::RightLowerArm);
				HitBoxesToScan.push_back((int)CSGOHitboxID::LeftUpperArm);
				HitBoxesToScan.push_back((int)CSGOHitboxID::LeftFoot);
				HitBoxesToScan.push_back((int)CSGOHitboxID::RightFoot);
				HitBoxesToScan.push_back((int)CSGOHitboxID::LeftShin);
				HitBoxesToScan.push_back((int)CSGOHitboxID::RightShin);
			}
			break;
			case 1:
			{
				if (accuracy > 60)
				{

					HitBoxesToScan.push_back((int)CSGOHitboxID::Neck);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Stomach);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Chest);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Pelvis);
					HitBoxesToScan.push_back((int)CSGOHitboxID::LeftFoot);
					HitBoxesToScan.push_back((int)CSGOHitboxID::RightFoot);
				}
				else
				{
					HitBoxesToScan.push_back((int)CSGOHitboxID::Head);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Neck);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Stomach);
					HitBoxesToScan.push_back((int)CSGOHitboxID::RightLowerArm);
					HitBoxesToScan.push_back((int)CSGOHitboxID::LeftUpperArm);
					HitBoxesToScan.push_back((int)CSGOHitboxID::LeftFoot);
					HitBoxesToScan.push_back((int)CSGOHitboxID::RightFoot);
					HitBoxesToScan.push_back((int)CSGOHitboxID::LeftShin);
					HitBoxesToScan.push_back((int)CSGOHitboxID::RightShin);
				}
			}
			break;
			case 2:
			{
				if (Globals::Shots >= 2)
				{
					HitBoxesToScan.push_back((int)CSGOHitboxID::Neck);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Stomach);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Chest);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Pelvis);
					HitBoxesToScan.push_back((int)CSGOHitboxID::LeftFoot);
					HitBoxesToScan.push_back((int)CSGOHitboxID::RightFoot);
				}
				else
				{
					HitBoxesToScan.push_back((int)CSGOHitboxID::Head);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Neck);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Stomach);
					HitBoxesToScan.push_back((int)CSGOHitboxID::RightLowerArm);
					HitBoxesToScan.push_back((int)CSGOHitboxID::LeftUpperArm);
					HitBoxesToScan.push_back((int)CSGOHitboxID::LeftFoot);
					HitBoxesToScan.push_back((int)CSGOHitboxID::RightFoot);
					HitBoxesToScan.push_back((int)CSGOHitboxID::LeftShin);
					HitBoxesToScan.push_back((int)CSGOHitboxID::RightShin);
				}

			}
			break;
			case 3:
			{
				if (accuracy >= 40)
				{
					HitBoxesToScan.push_back((int)CSGOHitboxID::Neck);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Stomach);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Chest);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Pelvis);
					HitBoxesToScan.push_back((int)CSGOHitboxID::LeftFoot);
					HitBoxesToScan.push_back((int)CSGOHitboxID::RightFoot);
				}
				else if (accuracy >= 40 && Globals::Shots >= 2)
				{
					HitBoxesToScan.push_back((int)CSGOHitboxID::Neck);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Stomach);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Chest);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Pelvis);
					HitBoxesToScan.push_back((int)CSGOHitboxID::LeftFoot);
					HitBoxesToScan.push_back((int)CSGOHitboxID::RightFoot);
				}
				else
				{
					HitBoxesToScan.push_back((int)CSGOHitboxID::Head);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Neck);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Stomach);
					HitBoxesToScan.push_back((int)CSGOHitboxID::RightLowerArm);
					HitBoxesToScan.push_back((int)CSGOHitboxID::LeftUpperArm);
					HitBoxesToScan.push_back((int)CSGOHitboxID::LeftFoot);
					HitBoxesToScan.push_back((int)CSGOHitboxID::RightFoot);
					HitBoxesToScan.push_back((int)CSGOHitboxID::LeftShin);
					HitBoxesToScan.push_back((int)CSGOHitboxID::RightShin);
				}

			}
			break;
			}

			break;

		}

		break;
	
		case 3:
		{
		
			switch (Menu::Window.RageBotTab.baimfactor.GetIndex())
			{
			case 0:
			{
				HitBoxesToScan.push_back((int)CSGOHitboxID::Head);
				HitBoxesToScan.push_back((int)CSGOHitboxID::Neck);
				HitBoxesToScan.push_back((int)CSGOHitboxID::UpperChest);
				HitBoxesToScan.push_back((int)CSGOHitboxID::Stomach);
				HitBoxesToScan.push_back((int)CSGOHitboxID::RightLowerArm);
				HitBoxesToScan.push_back((int)CSGOHitboxID::LeftUpperArm);
				HitBoxesToScan.push_back((int)CSGOHitboxID::LeftFoot);
				HitBoxesToScan.push_back((int)CSGOHitboxID::Head);
				HitBoxesToScan.push_back((int)CSGOHitboxID::RightFoot);
				HitBoxesToScan.push_back((int)CSGOHitboxID::LeftShin);
				HitBoxesToScan.push_back((int)CSGOHitboxID::Head);
				HitBoxesToScan.push_back((int)CSGOHitboxID::RightShin);
			}
				break;
			case 1:
			{
				if (accuracy > 60)
				{

					HitBoxesToScan.push_back((int)CSGOHitboxID::Neck);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Stomach);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Chest);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Pelvis);
					HitBoxesToScan.push_back((int)CSGOHitboxID::LeftFoot);
					HitBoxesToScan.push_back((int)CSGOHitboxID::RightFoot);
				}
				else
				{
					HitBoxesToScan.push_back((int)CSGOHitboxID::Head);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Neck);
					HitBoxesToScan.push_back((int)CSGOHitboxID::UpperChest);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Stomach);
					HitBoxesToScan.push_back((int)CSGOHitboxID::RightLowerArm);
					HitBoxesToScan.push_back((int)CSGOHitboxID::LeftUpperArm);
					HitBoxesToScan.push_back((int)CSGOHitboxID::LeftFoot);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Head);
					HitBoxesToScan.push_back((int)CSGOHitboxID::RightFoot);
					HitBoxesToScan.push_back((int)CSGOHitboxID::LeftShin);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Head);
					HitBoxesToScan.push_back((int)CSGOHitboxID::RightShin);
				}
			}
			break;
			case 2:
			{
				if (Globals::Shots >= 2)
				{
					HitBoxesToScan.push_back((int)CSGOHitboxID::Neck);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Stomach);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Chest);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Pelvis);
					HitBoxesToScan.push_back((int)CSGOHitboxID::LeftFoot);
					HitBoxesToScan.push_back((int)CSGOHitboxID::RightFoot);
				}
				else
				{
					HitBoxesToScan.push_back((int)CSGOHitboxID::Head);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Neck);
					HitBoxesToScan.push_back((int)CSGOHitboxID::UpperChest);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Stomach);
					HitBoxesToScan.push_back((int)CSGOHitboxID::RightLowerArm);
					HitBoxesToScan.push_back((int)CSGOHitboxID::LeftUpperArm);
					HitBoxesToScan.push_back((int)CSGOHitboxID::LeftFoot);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Head);
					HitBoxesToScan.push_back((int)CSGOHitboxID::RightFoot);
					HitBoxesToScan.push_back((int)CSGOHitboxID::LeftShin);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Head);
					HitBoxesToScan.push_back((int)CSGOHitboxID::RightShin);
				}

			}
			break;
			case 3:
			{
				if (accuracy >= 40)
				{
					HitBoxesToScan.push_back((int)CSGOHitboxID::Neck);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Stomach);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Chest);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Pelvis);
					HitBoxesToScan.push_back((int)CSGOHitboxID::LeftFoot);
					HitBoxesToScan.push_back((int)CSGOHitboxID::RightFoot);
				}
				else if (accuracy >= 40 && Globals::Shots >= 2)
				{
					HitBoxesToScan.push_back((int)CSGOHitboxID::Neck);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Stomach);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Chest);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Pelvis);
					HitBoxesToScan.push_back((int)CSGOHitboxID::LeftFoot);
					HitBoxesToScan.push_back((int)CSGOHitboxID::RightFoot);
				}
				else
				{
					HitBoxesToScan.push_back((int)CSGOHitboxID::Head);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Neck);
					HitBoxesToScan.push_back((int)CSGOHitboxID::UpperChest);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Stomach);
					HitBoxesToScan.push_back((int)CSGOHitboxID::RightLowerArm);
					HitBoxesToScan.push_back((int)CSGOHitboxID::LeftUpperArm);
					HitBoxesToScan.push_back((int)CSGOHitboxID::LeftFoot);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Head);
					HitBoxesToScan.push_back((int)CSGOHitboxID::RightFoot);
					HitBoxesToScan.push_back((int)CSGOHitboxID::LeftShin);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Head);
					HitBoxesToScan.push_back((int)CSGOHitboxID::RightShin);
				}

			}
			break;
			}

			break;
		
		}
		
			break;
		case 4:
		{
		

			switch (Menu::Window.RageBotTab.baimfactor.GetIndex())
			{
			case 0: 
			{
				HitBoxesToScan.push_back((int)CSGOHitboxID::Head);
				HitBoxesToScan.push_back((int)CSGOHitboxID::NeckLower);
				HitBoxesToScan.push_back((int)CSGOHitboxID::RightFoot);
				HitBoxesToScan.push_back((int)CSGOHitboxID::LeftFoot);
				HitBoxesToScan.push_back((int)CSGOHitboxID::Pelvis);
				HitBoxesToScan.push_back((int)CSGOHitboxID::Stomach);
				HitBoxesToScan.push_back((int)CSGOHitboxID::LeftLowerArm);
				HitBoxesToScan.push_back((int)CSGOHitboxID::RightUpperArm);
				HitBoxesToScan.push_back((int)CSGOHitboxID::LeftThigh);
				HitBoxesToScan.push_back((int)CSGOHitboxID::RightThigh);
				HitBoxesToScan.push_back((int)CSGOHitboxID::LeftShin);
				HitBoxesToScan.push_back((int)CSGOHitboxID::RightShin);
				HitBoxesToScan.push_back((int)CSGOHitboxID::Neck);
				HitBoxesToScan.push_back((int)CSGOHitboxID::UpperChest);
				HitBoxesToScan.push_back((int)CSGOHitboxID::RightHand);
				HitBoxesToScan.push_back((int)CSGOHitboxID::LeftHand);
			}

				break;
			case 1:
			{
				if (accuracy > 60)
				{

					HitBoxesToScan.push_back((int)CSGOHitboxID::Neck);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Stomach);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Chest);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Pelvis);
					HitBoxesToScan.push_back((int)CSGOHitboxID::LeftFoot);
					HitBoxesToScan.push_back((int)CSGOHitboxID::RightFoot);
				}
				else
				{
					HitBoxesToScan.push_back((int)CSGOHitboxID::Head);
					HitBoxesToScan.push_back((int)CSGOHitboxID::NeckLower);
					HitBoxesToScan.push_back((int)CSGOHitboxID::RightFoot);
					HitBoxesToScan.push_back((int)CSGOHitboxID::LeftFoot);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Pelvis);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Stomach);
					HitBoxesToScan.push_back((int)CSGOHitboxID::LeftLowerArm);
					HitBoxesToScan.push_back((int)CSGOHitboxID::RightUpperArm);
					HitBoxesToScan.push_back((int)CSGOHitboxID::LeftThigh);
					HitBoxesToScan.push_back((int)CSGOHitboxID::RightThigh);
					HitBoxesToScan.push_back((int)CSGOHitboxID::LeftShin);
					HitBoxesToScan.push_back((int)CSGOHitboxID::RightShin);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Neck);
					HitBoxesToScan.push_back((int)CSGOHitboxID::UpperChest);
					HitBoxesToScan.push_back((int)CSGOHitboxID::RightHand);
					HitBoxesToScan.push_back((int)CSGOHitboxID::LeftHand);
				}
			}
			break;
			case 2:
			{
				if (Globals::Shots >= 2)
				{
					HitBoxesToScan.push_back((int)CSGOHitboxID::Neck);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Stomach);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Chest);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Pelvis);
					HitBoxesToScan.push_back((int)CSGOHitboxID::LeftFoot);
					HitBoxesToScan.push_back((int)CSGOHitboxID::RightFoot);
				}
				else
				{
					HitBoxesToScan.push_back((int)CSGOHitboxID::Head);
					HitBoxesToScan.push_back((int)CSGOHitboxID::NeckLower);
					HitBoxesToScan.push_back((int)CSGOHitboxID::RightFoot);
					HitBoxesToScan.push_back((int)CSGOHitboxID::LeftFoot);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Pelvis);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Stomach);
					HitBoxesToScan.push_back((int)CSGOHitboxID::LeftLowerArm);
					HitBoxesToScan.push_back((int)CSGOHitboxID::RightUpperArm);
					HitBoxesToScan.push_back((int)CSGOHitboxID::LeftThigh);
					HitBoxesToScan.push_back((int)CSGOHitboxID::RightThigh);
					HitBoxesToScan.push_back((int)CSGOHitboxID::LeftShin);
					HitBoxesToScan.push_back((int)CSGOHitboxID::RightShin);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Neck);
					HitBoxesToScan.push_back((int)CSGOHitboxID::UpperChest);
					HitBoxesToScan.push_back((int)CSGOHitboxID::RightHand);
					HitBoxesToScan.push_back((int)CSGOHitboxID::LeftHand);
				}

			}
			case 3:
			{
				if (accuracy >= 40)
				{
					HitBoxesToScan.push_back((int)CSGOHitboxID::Neck);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Stomach);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Chest);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Pelvis);
					HitBoxesToScan.push_back((int)CSGOHitboxID::LeftFoot);
					HitBoxesToScan.push_back((int)CSGOHitboxID::RightFoot);
				}
				else if (accuracy >= 40 && Globals::Shots >= 2)
				{
					HitBoxesToScan.push_back((int)CSGOHitboxID::Neck);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Stomach);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Chest);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Pelvis);
					HitBoxesToScan.push_back((int)CSGOHitboxID::LeftFoot);
					HitBoxesToScan.push_back((int)CSGOHitboxID::RightFoot);
				}
				else
				{
					HitBoxesToScan.push_back((int)CSGOHitboxID::Head);
					HitBoxesToScan.push_back((int)CSGOHitboxID::NeckLower);
					HitBoxesToScan.push_back((int)CSGOHitboxID::RightFoot);
					HitBoxesToScan.push_back((int)CSGOHitboxID::LeftFoot);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Pelvis);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Stomach);
					HitBoxesToScan.push_back((int)CSGOHitboxID::LeftLowerArm);
					HitBoxesToScan.push_back((int)CSGOHitboxID::RightUpperArm);
					HitBoxesToScan.push_back((int)CSGOHitboxID::LeftThigh);
					HitBoxesToScan.push_back((int)CSGOHitboxID::RightThigh);
					HitBoxesToScan.push_back((int)CSGOHitboxID::LeftShin);
					HitBoxesToScan.push_back((int)CSGOHitboxID::RightShin);
					HitBoxesToScan.push_back((int)CSGOHitboxID::Neck);
					HitBoxesToScan.push_back((int)CSGOHitboxID::UpperChest);
					HitBoxesToScan.push_back((int)CSGOHitboxID::RightHand);
					HitBoxesToScan.push_back((int)CSGOHitboxID::LeftHand);
				}

			}
			}

			break;
		}
	
		}
	}
#pragma endregion Get the list of shit to scan
	for (auto HitBoxID : HitBoxesToScan)
	{
		if (AWall)
		{
			Vector Point = GetHitboxPosition(pEntity, HitBoxID);
			float Damage = 0.f;
			Color c = Color(255, 255, 255, 255);
			if (CanHit(Point, &Damage))
			{
				c = Color(0, 255, 0, 255);
				if (Damage >= Menu::Window.RageBotTab.AccuracyMinimumDamage.GetValue())
				{
					return HitBoxID;
				}
			}
		}
		else
		{
			if (GameUtils::IsVisible(hackManager.pLocal(), pEntity, HitBoxID))
				return HitBoxID;
		}
	}

	return -1;
}

void CRageBot::DoNoRecoil(CUserCmd *pCmd)
{

	IClientEntity* pLocal = hackManager.pLocal();
	if (pLocal)
	{
		Vector AimPunch = pLocal->localPlayerExclusive()->GetAimPunchAngle();
		if (AimPunch.Length2D() > 0 && AimPunch.Length2D() < 150)
		{
			pCmd->viewangles -= AimPunch * 2;
			GameUtils::NormaliseViewAngle(pCmd->viewangles);
		}
	}

}







CAntiSpread* NoSpread = new CAntiSpread;


void CAntiSpread::CalcServer(Vector vSpreadVec, Vector ViewIn, Vector &vecSpreadDir)
{
	Vector vecViewForward, vecViewRight, vecViewUp;

	angvec(ViewIn, vecViewForward, vecViewRight, vecViewUp);

	vecSpreadDir = vecViewForward + vecViewRight * vSpreadVec.x + vecViewUp * vSpreadVec.y;
}

void CAntiSpread::antispread(CUserCmd* pCmd)
{
	Vector vecForward, vecRight, vecDir, vecUp, vecAntiDir;
	float flSpread, flInaccuracy;
	Vector qAntiSpread;

//	glb::mainwep->UpdateAccuracyPenalty();

	flSpread = glb::mainwep->GetSpread();

	flInaccuracy = glb::mainwep->GetInaccuracy();
	Globals::UserCmd->random_seed = MD5_PseudoRandom(Globals::UserCmd->command_number) & 0x7FFFFFFF;
	RandomSeed((Globals::UserCmd->random_seed & 0xFF) + 1);


	float fRand1 = RandFloat(0.f, 1.f);
	float fRandPi1 = RandFloat(0.f, 2.f * (float)M_PI);
	float fRand2 = RandFloat(0.f, 1.f);
	float fRandPi2 = RandFloat(0.f, 2.f * (float)M_PI);

	float m_flRecoilIndex = glb::mainwep->GetFloatRecoilIndex();


	if (glb::mainwep->WeaponID() == 64)
	{
		if (Globals::UserCmd->buttons & IN_ATTACK2)
		{
			fRand1 = 1.f - fRand1 * fRand1;
			fRand2 = 1.f - fRand2 * fRand2;
		}
	}
	else if (glb::mainwep->WeaponID() == NEGEV && m_flRecoilIndex < 3.f)
	{
		for (int i = 3; i > m_flRecoilIndex; --i)
		{
			fRand1 *= fRand1;
			fRand2 *= fRand2;
		}

		fRand1 = 1.f - fRand1;
		fRand2 = 1.f - fRand2;
	}

	float fRandInaccuracy = fRand1 * glb::mainwep->GetInaccuracy();
	float fRandSpread = fRand2 * glb::mainwep->GetSpread();

	float fSpreadX = cos(fRandPi1) * fRandInaccuracy + cos(fRandPi2) * fRandSpread;
	float fSpreadY = sin(fRandPi1) * fRandInaccuracy + sin(fRandPi2) * fRandSpread;


	pCmd->viewangles.x += RAD2DEG(atan(sqrt(fSpreadX * fSpreadX + fSpreadY * fSpreadY)));
	pCmd->viewangles.z = RAD2DEG(atan2(fSpreadX, fSpreadY));
}

void CRageBot::ns1(CUserCmd *pCmd, IClientEntity* LocalPlayer)
{
	float recoil_value = 2;
//	glb::mainwep->UpdateAccuracyPenalty();
	QAngle punch = LocalPlayer->GetPunchAngle();

	if (Interfaces::CVar->FindVar(XorStr("weapon_recoil_scale")))
	{
		ConVar* recoil_value = Interfaces::CVar->FindVar(XorStr("weapon_recoil_scale"));
		recoil_value->SetValue("2");
	}
	Globals::UserCmd->viewangles -= punch * recoil_value;
}

void CRageBot::nospread(CUserCmd *pCmd)
{
	IClientEntity* LocalPlayer= hackManager.pLocal();
	if (Menu::Window.RageBotTab.AccuracyRecoil.GetIndex() == 2)
	{
		ns1(pCmd, LocalPlayer);
		Globals::UserCmd->viewangles = NoSpread->SpreadFactor(Globals::UserCmd->random_seed);
	}


	

}

// 	float m_fValue;
void CRageBot::aimAtPlayer(CUserCmd *pCmd)
{
	IClientEntity* pLocal = hackManager.pLocal();

	CBaseCombatWeapon* pWeapon = (CBaseCombatWeapon*)Interfaces::EntList->GetClientEntityFromHandle(hackManager.pLocal()->GetActiveWeaponHandle());

	if (!pLocal || !pWeapon)
		return;

	Vector eye_position = pLocal->GetEyePosition();

	float best_dist = pWeapon->GetCSWpnData()->m_flRange;

	IClientEntity* target = nullptr;

	for (int i = 0; i < Interfaces::Engine->GetMaxClients(); i++)
	{
		IClientEntity *pEntity = Interfaces::EntList->GetClientEntity(i);
		if (TargetMeetsRequirements(pEntity))
		{
			if (Globals::TargetID != -1)
				target = Interfaces::EntList->GetClientEntity(Globals::TargetID);
			else
				target = pEntity;

			Vector target_position = target->GetEyePosition();

			float temp_dist = eye_position.DistTo(target_position);

			if (best_dist > temp_dist)
			{
				best_dist = temp_dist;
				CalcAngle(eye_position, target_position, pCmd->viewangles);
			}
		}

	}
}


float GetBestHeadAngle(float yaw)
{
	float Back, Right, Left;
	IClientEntity* pLocal = hackManager.pLocal();
	Vector src3D, dst3D, forward, right, up, src, dst;
	trace_t tr;
	Ray_t ray, ray2, ray3, ray4, ray5;
	CTraceFilter filter;
	IEngineTrace trace;
	QAngle angles;
	QAngle engineViewAngles;
	Interfaces::Engine->GetViewAngles(angles);

	engineViewAngles.x = 0;

	Math::AngleVectors(engineViewAngles, &forward, &right, &up);

	filter.pSkip = pLocal;
	src3D = pLocal->GetEyeAngles();
	dst3D = src3D + (forward * 384);

	ray.Init(src3D, dst3D);

	Interfaces::Trace->TraceRay(ray, MASK_SHOT, &filter, &tr);

	Back = (tr.endpos - tr.startpos).Length();

	ray2.Init(src3D + right * 35, dst3D + right * 35);

	Interfaces::Trace->TraceRay(ray2, MASK_SHOT, &filter, &tr);

	Right = (tr.endpos - tr.startpos).Length();

	ray3.Init(src3D - right * 35, dst3D - right * 35);

	Interfaces::Trace->TraceRay(ray3, MASK_SHOT, &filter, &tr);

	Left = (tr.endpos - tr.startpos).Length();

	if (Left > Right)
	{
		return (yaw - 90);
	}
	else if (Right > Left)
	{
		return (yaw + 90);
	}
	else if (Back > Right || Back > Left)
	{
		return (yaw - 180);
	}
	return 0;
}


bool ShouldPredict()
{
	INetChannelInfo* nci = Interfaces::Engine->GetNetChannelInfo();

	IClientEntity* pLocal = hackManager.pLocal();
	float server_time = Interfaces::Globals->curtime + nci->GetLatency(FLOW_OUTGOING);

	float PredictedTime = 0.f;
	static bool initialized;

	bool will_update = false;

	if (!initialized && pLocal->IsMoving())
	{
		initialized = true;
		PredictedTime = server_time + 0.22f;
	}
	else if (pLocal->IsMoving())
	{
		initialized = false;
	}

	if (server_time >= (PredictedTime) && pLocal->GetFlags() & FL_ONGROUND)
	{
		PredictedTime = server_time + 1.1f;
		will_update = true;
	}
	return will_update;
}


bool CRageBot::AimAtPoint(IClientEntity* pLocal, Vector point, CUserCmd *pCmd, bool &bSendPacket)
{
	bool ReturnValue = false;

	if (point.Length() == 0) return ReturnValue;

	Vector angles;
	Vector src = pLocal->GetOrigin() + pLocal->GetViewOffset();

	CalcAngle(src, point, angles);
	GameUtils::NormaliseViewAngle(angles);

	if (angles[0] != angles[0] || angles[1] != angles[1])
	{
		return ReturnValue;
	}

	IsLocked = true;

	Vector ViewOffset = pLocal->GetOrigin() + pLocal->GetViewOffset();
	if (!IsAimStepping)
		LastAimstepAngle = LastAngle; 

	float fovLeft = FovToPlayer(ViewOffset, LastAimstepAngle, Interfaces::EntList->GetClientEntity(TargetID), 0);

	if (fovLeft > 25.0f && Menu::Window.RageBotTab.AimbotAimStep.GetState())
	{

		Vector AddAngs = angles - LastAimstepAngle;
		Normalize(AddAngs, AddAngs);
		AddAngs *= 25;
		LastAimstepAngle += AddAngs;
		GameUtils::NormaliseViewAngle(LastAimstepAngle);
		angles = LastAimstepAngle;
	}
	else
	{
		ReturnValue = true;
	}

	if (Menu::Window.RageBotTab.AimbotSilentAim.GetState())
	{
		pCmd->viewangles = angles;

	}

	if (!Menu::Window.RageBotTab.AimbotSilentAim.GetState())
	{

		Interfaces::Engine->SetViewAngles(angles);
	}

	return ReturnValue;
}

namespace AntiAims 
{
	

	void FastSpin(CUserCmd *pCmd)
	{
		static int y2 = -179;
		int spinBotSpeedFast = 75;

		y2 += spinBotSpeedFast;

		if (y2 >= 179)
			y2 = -179;

		pCmd->viewangles.y = y2;
	}



	void FastSpint(CUserCmd *pCmd)
	{
		int r1 = rand() % 100;
		int r2 = rand() % 1000;

		static bool dir;
		static float current_y = pCmd->viewangles.y;

		if (r1 == 1) dir = !dir;

		if (dir)
			current_y += 15 + rand() % 10;
		else
			current_y -= 15 + rand() % 10;

		pCmd->viewangles.y = current_y;

		if (r1 == r2)
			pCmd->viewangles.y += r1;
	}

	void Up(CUserCmd *pCmd)
	{
		pCmd->viewangles.x = -89.0f;
	}

	void Zero(CUserCmd *pCmd)
	{
		pCmd->viewangles.x = 0.f;
	}

	void fforward(CUserCmd *pCmd, bool &bSendPacket)
	{
		pCmd->viewangles.y -= 0.f;
	}

	void StaticRealYaw(CUserCmd *pCmd, bool &bSendPacket) {
		bSendPacket = false;
		pCmd->viewangles.y += 90;
	}
	void Twitch(CUserCmd *pCmd)
	{
		bool wtf = false;
		int subscribetoNuiger = rand() % 100;
		if (!wtf)
		{
			if (subscribetoNuiger = 40)
			{
				pCmd->viewangles.y += 140;
			}
			else if (subscribetoNuiger = 30)
			{
				pCmd->viewangles.y += 120;
			}
			else if (subscribetoNuiger = 79)
			{
				pCmd->viewangles.y += 150;
			}
			else if (subscribetoNuiger = 87)
			{
				pCmd->viewangles.y -= 140;
			}
			else if (subscribetoNuiger = 16)
			{
				pCmd->viewangles.y -= 140;
			}
			else if (subscribetoNuiger = 51)
			{
				pCmd->viewangles.y -= 120;
			}
			else if (subscribetoNuiger = 33)
			{
				pCmd->viewangles.y -= 150;
			}
			else if (subscribetoNuiger = 49)
			{
				pCmd->viewangles.y -= 130;
			}
			else if (subscribetoNuiger = 50)
			{
				pCmd->viewangles.y -= 0;
			}
			else if (subscribetoNuiger = 44)
			{
				pCmd->viewangles.y -= 140;
			}
			else if (subscribetoNuiger <= 12)
			{
				pCmd->viewangles.y += 160;
			}
			else if (subscribetoNuiger = 13)
			{
				pCmd->viewangles.y -= 160;
			}
			else
				pCmd->viewangles.y += 179;

		}
	}

	void Nuiger(IClientEntity* pLocal, CUserCmd *pCmd, bool &bSendPacket) 
	{
		if (pLocal->GetFlags()  & pLocal->GetVelocity().Length() <= 0.f & FL_ONGROUND)
		{
			QAngle angles;
			Interfaces::Engine->GetViewAngles(angles);

			float BestHeadPosition = GetBestHeadAngle(angles.y);

			float LowerbodyDelta = 160;

			if (bSendPacket)
			{
				pCmd->viewangles.y = BestHeadPosition + LowerbodyDelta + Math::RandomFloat2(-40.f, 40.f);
			}
			else
			{
				if (ShouldPredict())
					pCmd->viewangles.y = BestHeadPosition + LowerbodyDelta;
				else
					pCmd->viewangles.y = BestHeadPosition;
			}
		}
		


	}


	void Nuigermove(IClientEntity* pLocal, CUserCmd *pCmd, bool &bSendPacket)
	{
		if (pLocal->GetFlags() && pLocal->GetVelocity().Length() <= 245.f & FL_ONGROUND)
		{
			QAngle angles;
			Interfaces::Engine->GetViewAngles(angles);

			float BestHeadPosition = GetBestHeadAngle(angles.y);

			float LowerbodyDelta = 160;

			if (bSendPacket)
			{
				pCmd->viewangles.y = BestHeadPosition + LowerbodyDelta + Math::RandomFloat2(-40.f, 40.f);
			}
			else
			{
				if (ShouldPredict())
					pCmd->viewangles.y = BestHeadPosition + LowerbodyDelta;
				else
					pCmd->viewangles.y = BestHeadPosition;
			}
		}
		else
			pCmd->viewangles.y += 180;


	}
	void custombuildPitch(IClientEntity* pLocal, CUserCmd *pCmd, bool &bSendPacket)
	{
		static bool quickmathematics = false;
		float custombase = (Menu::Window.aabuild.pitchbase.GetValue());

		float pitch_jiiter_from = (Menu::Window.aabuild.pitch_jitter_from.GetValue());
		float pitch_jiiter_to = (Menu::Window.aabuild.pitch_jitter_to.GetValue());

		float pitch_safe_fake = (Menu::Window.aabuild.pitch_safe_fake.GetValue());
		float pitch_safe_real = (Menu::Window.aabuild.pitch_safe_real.GetValue());

		float pitch_unsafe_fake = (Menu::Window.aabuild.pitch_unsafe_fake.GetValue());
		float pitch_unsafe_real = (Menu::Window.aabuild.pitch_unsafe_real.GetValue());


		if (Menu::Window.aabuild.pitchpick.GetIndex() == 0)
		{
			pCmd->viewangles.x = custombase;
		}
		else if (Menu::Window.aabuild.pitchpick.GetIndex() == 1)
		{

			if (quickmathematics)
			{
				pCmd->viewangles.x = pitch_jiiter_from;
			}
			else
			{
				pCmd->viewangles.x = pitch_jiiter_to;
			}
			quickmathematics = !quickmathematics;
		}
		else if (Menu::Window.aabuild.pitchpick.GetIndex() == 2)
		{
			if (bSendPacket)
				pitch_safe_fake;
			else
				pitch_safe_real;
		}
		else
		{
			if (bSendPacket)
				pitch_unsafe_fake;
			else
				pitch_unsafe_real;
		}
	}


	void custombuildYaw(IClientEntity* pLocal, CUserCmd *pCmd, bool &bSendPacket)
	{
		float value;
		float rndy = rand() % 60;
		static bool switch3 = false;

		if (rndy < 15)
			rndy = 15;

		while (value > 180)
			value -= 360;
		while (value < -180)
			value += 360;

		float custombase = (Menu::Window.aabuild.yawbase.GetValue()); // Starting point

		float switch1 = (Menu::Window.aabuild.yaw_switch_from.GetValue());
		float switch2 = (Menu::Window.aabuild.yaw_switch_to.GetValue());

		float jit1 = (Menu::Window.aabuild.yaw_jitter_from.GetValue());
		float jit2 = (Menu::Window.aabuild.yaw_jitter_to.GetValue());

		float spinspeed = (Menu::Window.aabuild.spinspeed.GetValue());

		//--------------------lby-trash-idk-lmao--------------------//
		int flip = (int)floorf(Interfaces::Globals->curtime / 1.1) % 2;
		static bool bFlipYaw;
		static bool wilupdate;
		float flInterval = Interfaces::Globals->interval_per_tick;
		float flTickcount = pCmd->tick_count;
		float flTime = flInterval * flTickcount;
		if (std::fmod(flTime, 1) == 0.f)
			bFlipYaw = !bFlipYaw;
		static float LastLBYUpdateTime = 0;

		float server_time = pLocal->GetTickBase() * Interfaces::Globals->interval_per_tick;
		//--------------------lby-trash-idk-lmao--------------------//


		if (Menu::Window.aabuild.yawpick.GetIndex() == 0)
		{
			pCmd->viewangles.y -= custombase;
		}
		else if (Menu::Window.aabuild.yawpick.GetIndex() == 1) // jitter AA
		{

			if (switch3)
				pCmd->viewangles.y = jit1;
			else
				pCmd->viewangles.y = jit2;
			switch3 = !switch3;

		}
		else if (Menu::Window.aabuild.yawpick.GetIndex() == 2) // Switch AA
		{

			if (Menu::Window.aabuild.yaw_add_jitter.GetState() == false) // if we do not add jitter
			{
				if (flip)
				{
					pCmd->viewangles.y += bFlipYaw ? switch1 : switch2;

				}
				else
				{
					pCmd->viewangles.y -= hackManager.pLocal()->GetLowerBodyYaw() + bFlipYaw ? switch1 : switch2;
				}
			}
			else // if we add jitter
			{
				if (flip)
				{
					pCmd->viewangles.y += bFlipYaw ? switch1 + rndy : switch2 - rndy;

				}
				else
				{
					pCmd->viewangles.y -= hackManager.pLocal()->GetLowerBodyYaw() + bFlipYaw ? switch1 + rndy : switch2 - rndy;
				}
			}

		}
		else
		{
			static int y2 = -170;
			int spinBotSpeedFast = spinspeed;

			y2 += spinBotSpeedFast;

			if (y2 >= 170)
				y2 = -170;

			pCmd->viewangles.y = y2;

		}

	}
	void custombuildFake(IClientEntity* pLocal, CUserCmd *pCmd, bool &bSendPacket)
	{
		float value;
		float rndy = rand() % 60;
		static bool switch3 = false;

		if (rndy < 15)
			rndy = 15;

		while (value > 180)
			value -= 360;
		while (value < -180)
			value += 360;

		float custombase = (Menu::Window.aabuild.fyawbase.GetValue()); // Starting point

		float switch1 = (Menu::Window.aabuild.fyaw_switch_from.GetValue());
		float switch2 = (Menu::Window.aabuild.fyaw_switch_to.GetValue());

		float jit1 = (Menu::Window.aabuild.fyaw_jitter_from.GetValue());
		float jit2 = (Menu::Window.aabuild.fyaw_jitter_to.GetValue());

		float spinspeed = (Menu::Window.aabuild.fspinspeed.GetValue());

		//--------------------lby-trash-idk-lmao--------------------//
		int flip = (int)floorf(Interfaces::Globals->curtime / 1.1) % 2;
		static bool bFlipYaw;
		static bool wilupdate;
		float flInterval = Interfaces::Globals->interval_per_tick;
		float flTickcount = pCmd->tick_count;
		float flTime = flInterval * flTickcount;
		if (std::fmod(flTime, 1) == 0.f)
			bFlipYaw = !bFlipYaw;
		static float LastLBYUpdateTime = 0;

		float server_time = pLocal->GetTickBase() * Interfaces::Globals->interval_per_tick;
		//--------------------lby-trash-idk-lmao--------------------//


		if (Menu::Window.aabuild.fyawpick.GetIndex() == 0)
		{
			if (bSendPacket)
				pCmd->viewangles.y -= custombase;
		}
		else if (Menu::Window.aabuild.fyawpick.GetIndex() == 1) // jitter AA
		{
			if (bSendPacket)
			{
				if (switch3)
					pCmd->viewangles.y = jit1;
				else
					pCmd->viewangles.y = jit2;
				switch3 = !switch3;
			}

		}
		else if (Menu::Window.aabuild.fyawpick.GetIndex() == 2) // Switch AA
		{

			if (Menu::Window.aabuild.yaw_add_jitter.GetState() == false) // if we do not add jitter
			{
				if (bSendPacket)
				{
					if (flip)
					{
						pCmd->viewangles.y += bFlipYaw ? switch1 : switch2;

					}
					else
					{
						pCmd->viewangles.y -= hackManager.pLocal()->GetLowerBodyYaw() + bFlipYaw ? switch1 : switch2;
					}
				}
			}
			else // if we add jitter
			{
				if (bSendPacket)
				{
					if (flip)
					{
						pCmd->viewangles.y += bFlipYaw ? switch1 + rndy : switch2 + rndy;

					}
					else
					{
						pCmd->viewangles.y -= hackManager.pLocal()->GetLowerBodyYaw() + bFlipYaw ? switch1 - rndy : switch2 - rndy;
					}
				}
			}

		}
		else
		{
			if (bSendPacket)
			{
				static int y2 = -170;
				int spinBotSpeedFast = spinspeed;

				y2 += spinBotSpeedFast;

				if (y2 >= 170)
					y2 = -170;

				pCmd->viewangles.y = y2;
			}

		}
	}






	void LBYSide(CUserCmd *pCmd, bool &bSendPacket)
	{
		int i = 0; i < Interfaces::EntList->GetHighestEntityIndex(); ++i;
		IClientEntity *pEntity = Interfaces::EntList->GetClientEntity(i);
		IClientEntity *pLocal = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());

		static bool isMoving;
		float PlayerIsMoving = abs(pLocal->GetVelocity().Length());
		if (PlayerIsMoving > 0.1) isMoving = true;
		else if (PlayerIsMoving <= 0.1) isMoving = false;
		int meme = rand() % 75;
		if (meme < 17)
			meme = 17;
		int flip = (int)floorf(Interfaces::Globals->curtime / 1.1) % 2;
		static bool bFlipYaw;
		float flInterval = Interfaces::Globals->interval_per_tick;
		float flTickcount = pCmd->tick_count;
		float flTime = flInterval * flTickcount;
		if (std::fmod(flTime, 1) == 0.f)
			bFlipYaw = !bFlipYaw;

		if (PlayerIsMoving <= 0.4)
		{
			if (bSendPacket)
			{
				pCmd->viewangles.y += bFlipYaw ? (90.f + meme) : (-90 - meme);
			}
			else
			{
				if (flip)
				{
					pCmd->viewangles.y += bFlipYaw ? (90.f + meme) : (-90 - meme);

				}
				else
				{
					pCmd->viewangles.y -= hackManager.pLocal()->GetLowerBodyYaw() + bFlipYaw ? (90.f + meme) : (-90 - meme);
				}
			}
		}
		else
		{
			if (bSendPacket)
			{
				pCmd->viewangles.y += 5.f - meme;
			}
			else
			{
				pCmd->viewangles.y += 160.f - meme;
			}
		}
	}


	void SideJitter(CUserCmd *pCmd)
	{
		static bool Fast2 = false;
		if (Fast2)
		{
			pCmd->viewangles.y += 75;
		}
		else
		{
			pCmd->viewangles.y += 105;
		}
		Fast2 = !Fast2;
	}

	void SlowSpin(CUserCmd *pCmd)
	{
		int r1 = rand() % 100;
		int r2 = rand() % 1000;

		static bool dir;
		static float current_y = pCmd->viewangles.y;

		if (r1 == 1) dir = !dir;

		if (dir)
			current_y += 4 + rand() % 10;
		else
			current_y -= 4 + rand() % 10;

		pCmd->viewangles.y = current_y;

		if (r1 == r2)
			pCmd->viewangles.y += r1;
	}
}

void CorrectMovement(Vector old_angles, CUserCmd* cmd, float old_forwardmove, float old_sidemove)
{
	float delta_view, first_function, second_function;

	if (old_angles.y < 0.f) first_function = 360.0f + old_angles.y;
	else first_function = old_angles.y;
	if (cmd->viewangles.y < 0.0f) second_function = 360.0f + cmd->viewangles.y;
	else second_function = cmd->viewangles.y;

	if (second_function < first_function) delta_view = abs(second_function - first_function);
	else delta_view = 360.0f - abs(first_function - second_function);

	delta_view = 360.0f - delta_view;

	cmd->forwardmove = cos(DEG2RAD(delta_view)) * old_forwardmove + cos(DEG2RAD(delta_view + 90.f)) * old_sidemove;
	cmd->sidemove = sin(DEG2RAD(delta_view)) * old_forwardmove + sin(DEG2RAD(delta_view + 90.f)) * old_sidemove;
}

float GetLatency()
{
	INetChannelInfo *nci = Interfaces::Engine->GetNetChannelInfo();
	if (nci)
	{

		float Latency = nci->GetAvgLatency(FLOW_OUTGOING) + nci->GetAvgLatency(FLOW_INCOMING);
		return Latency;
	}
	else
	{

		return 0.0f;
	}
}
float GetOutgoingLatency()
{
	INetChannelInfo *nci = Interfaces::Engine->GetNetChannelInfo();
	if (nci)
	{

		float OutgoingLatency = nci->GetAvgLatency(FLOW_OUTGOING);
		return OutgoingLatency;
	}
	else
	{

		return 0.0f;
	}
}
float GetIncomingLatency()
{
	INetChannelInfo *nci = Interfaces::Engine->GetNetChannelInfo();
	if (nci)
	{
		float IncomingLatency = nci->GetAvgLatency(FLOW_INCOMING);
		return IncomingLatency;
	}
	else
	{

		return 0.0f;
	}
}

float OldLBY;
float LBYBreakerTimer;
float LastLBYUpdateTime;
bool bSwitch;

float CurrentVelocity(IClientEntity* LocalPlayer)
{
	int vel = LocalPlayer->GetVelocity().Length2D();
	return vel;
}
bool NextLBYUpdate()
{
	IClientEntity* LocalPlayer = hackManager.pLocal();

	float flServerTime = (float)(LocalPlayer->GetTickBase()  * Interfaces::Globals->interval_per_tick);


	if (OldLBY != LocalPlayer->GetLowerBodyYaw())
	{

		LBYBreakerTimer++;
		OldLBY = LocalPlayer->GetLowerBodyYaw();
		bSwitch = !bSwitch;
		LastLBYUpdateTime = flServerTime;
	}

	if (CurrentVelocity(LocalPlayer) > 1)
	{
		LastLBYUpdateTime = flServerTime;
		return false;
	}

	if ((LastLBYUpdateTime + 1 - (GetLatency() * 2) < flServerTime) && (LocalPlayer->GetFlags() & FL_ONGROUND))
	{
		if (LastLBYUpdateTime + 1.101 - (GetLatency() * 2) < flServerTime)
		{
			LastLBYUpdateTime += 1.1;
		}
		return true;
	}
	return false;
}

bool NextMovingLBYUpdate()
{
	IClientEntity* LocalPlayer = hackManager.pLocal();

	float flServerTime = (float)(LocalPlayer->GetTickBase()  * Interfaces::Globals->interval_per_tick);


	if (OldLBY != LocalPlayer->GetLowerBodyYaw())
	{

		LBYBreakerTimer++;
		OldLBY = LocalPlayer->GetLowerBodyYaw();
		bSwitch = !bSwitch;
		LastLBYUpdateTime = flServerTime;
	}

	if (CurrentVelocity(LocalPlayer) > 1)
	{
		LastLBYUpdateTime = flServerTime;
		return false;
	}

	if ((LastLBYUpdateTime + 1 - (GetLatency() * 2) < flServerTime) && (LocalPlayer->GetFlags() & FL_ONGROUND))
	{
		if (LastLBYUpdateTime + 0.2172 - (GetLatency() * 2) < flServerTime)
		{
			LastLBYUpdateTime += 0.21712;
		}
		return true;
	}
	return false;
}


void mmfake(CUserCmd * pCmd, IClientEntity* pLocal, bool& bSendPacket)
{
	if (bSendPacket)
	{
		if (CanOpenFire)
		{ 
			pCmd->viewangles.x = 86;

		}
		else if (pLocal->IsMoving())
		{
			pCmd->viewangles.x = 80 + rand() % 5;
		}
		else
		pCmd->viewangles.x = 40;
	}
	else
	{
		pCmd->viewangles.x = 88.9;
	}
}

#define RandomInt(min, max) (rand() % (max - min + 1) + min)
void DoLBYBreak(CUserCmd * pCmd, IClientEntity* pLocal, bool& bSendPacket)
{
	if (!bSendPacket)
	{
		pCmd->viewangles.y -= 90;
	}
	else
	{
		pCmd->viewangles.y += 90;
	}
}

void fakelby(CUserCmd* cmd, bool &bSendPacket)
{
	
	
	IClientEntity* LocalPlayer = hackManager.pLocal();
	float value = LocalPlayer->GetLowerBodyYaw();
	float flServerTime = (float)(LocalPlayer->GetTickBase()  * Interfaces::Globals->interval_per_tick);

	if ((LastLBYUpdateTime + 1 - (GetLatency() * 2) < flServerTime))
	{
		
		if (bSendPacket)
		{
			cmd->viewangles.y = value - 119 + rand() % 90;
		}
	
	
	}
	else
	{
		if (bSendPacket)
		{
			cmd->viewangles.y = -80.00 + rand() % 150;
		}
	}
	
}

void DoLBYBreakReal(CUserCmd * pCmd, IClientEntity* pLocal, bool& bSendPacket)
{
	if (!bSendPacket)
	{
		pCmd->viewangles.y += 90;
	}
	else
	{
		pCmd->viewangles.y -= 90;
	}
}


static bool peja;
static bool switchbrak;
static bool safdsfs;

void Dynamic(CUserCmd *pCmd, bool &bSendPacket)
{
	IClientEntity* pLocal = hackManager.pLocal();
	static int skeet = 179;
	int SpinSpeed = 500;
	static int ChokedPackets = -1;
	ChokedPackets++;
	skeet += SpinSpeed;

	if (pCmd->command_number % 20)
	{
		if (skeet >= pLocal->GetLowerBodyYaw() + 179 - rand() % 30);
		skeet = pLocal->GetLowerBodyYaw() - 0;
		ChokedPackets = -1;
	}
	if (pCmd->command_number % 30)
	{
		float CalculatedCurTime = (Interfaces::Globals->curtime * 1000.0);
		pCmd->viewangles.y = CalculatedCurTime;
		ChokedPackets = -1;
	}

	pCmd->viewangles.y = skeet;
}
void LowerBody(CUserCmd *pCmd, bool &bSendPacket)
{ //
	static bool wilupdate;
	static float LastLBYUpdateTime = 0;
	IClientEntity* pLocal = hackManager.pLocal();
	float server_time = pLocal->GetTickBase() * Interfaces::Globals->interval_per_tick;
	if (!bSendPacket)
	{
		if (server_time >= LastLBYUpdateTime)
		{
			LastLBYUpdateTime = server_time + 1.125f;
			wilupdate = true;
			pCmd->viewangles.y = -90.f;
		}
		else if (server_time >= LastLBYUpdateTime)
		{
			LastLBYUpdateTime = server_time + 1.000f;
			wilupdate = true;
			pCmd->viewangles.y = 90.f;
		}
		else if (server_time >= LastLBYUpdateTime)
		{
			LastLBYUpdateTime = server_time + 1.125f;
			wilupdate = true;
			pCmd->viewangles.y = 170.f;
		}
		else if (server_time >= LastLBYUpdateTime)
		{
			LastLBYUpdateTime = server_time + 0.984f;
			wilupdate = true;
			pCmd->viewangles.y -= 10.f;
		}
		else
		{
			wilupdate = false;
			pCmd->viewangles.y += 150.f;
		}
	}
	else
	{
		if (server_time >= LastLBYUpdateTime)
		{
			LastLBYUpdateTime = server_time + 1.125f;
			wilupdate = true;
			pCmd->viewangles.y = 90.f;
		}
		else if (server_time >= LastLBYUpdateTime)
		{
			LastLBYUpdateTime = server_time + 1.000f;
			wilupdate = true;
			pCmd->viewangles.y = -90.f;
		}
		else if (server_time >= LastLBYUpdateTime)
		{
			LastLBYUpdateTime = server_time + 1.125f;
			wilupdate = true;
			pCmd->viewangles.y = 10.f;
		}
		else if (server_time >= LastLBYUpdateTime)
		{
			LastLBYUpdateTime = server_time + 0.984f;
			wilupdate = true;
			pCmd->viewangles.y -= 170.f;
		}
		else
		{
			wilupdate = false;
			pCmd->viewangles.y += 150.f;
		}
	}

}

void jolt(CUserCmd *pCmd, bool &bSendPacket)
{
	// (LastLBYUpdateTime + 1 - (GetLatency() * 2) < flServerTime)
	IClientEntity* pLocal = hackManager.pLocal();
	float value = pLocal->GetLowerBodyYaw();
	float flServerTime = (float)(pLocal->GetTickBase()  * Interfaces::Globals->interval_per_tick);
	float moving = abs(pLocal->GetVelocity().Length());
	if (bSendPacket)
	{
		if (moving > 30)
		{
			pCmd->viewangles.y = value + (101 + rand() % 20);
		}
		else
		{
			pCmd->viewangles.y = (value - 90) - rand() % 110;
		}
	}

}



void Keybased(CUserCmd *pCmd, bool &bSendPacket)
{
	IClientEntity* pLocal = hackManager.pLocal();
	
	static bool dir = false;
	float side = pCmd->viewangles.y - 90;
	float server_time = pLocal->GetTickBase() * Interfaces::Globals->interval_per_tick;
	if (GetAsyncKeyState(VK_LEFT)) dir = false; 
	else if (GetAsyncKeyState(VK_RIGHT)) dir = true;

	if (dir && pLocal->GetVelocity().Length2D() >= 0)
	{
		pCmd->viewangles.y -= 90;
	}
	else if (!dir && pLocal->GetVelocity().Length2D() >= 0)
	{
		pCmd->viewangles.y += 90;
	}

	// literally tapping unityhacks omg media
}
void LowerBodyFake(CUserCmd *pCmd, bool &bSendPacket)
{

	static bool wilupdate;
	static float LastLBYUpdateTime = 0;
	IClientEntity* pLocal = hackManager.pLocal();
	float server_time = pLocal->GetTickBase() * Interfaces::Globals->interval_per_tick;

	static int ChokedPackets = -1;
	ChokedPackets++;
	if (ChokedPackets < 1)
	{
		

		if (server_time >= LastLBYUpdateTime)
		{
			LastLBYUpdateTime = server_time + 1.009f;
			wilupdate = true;
			pCmd->viewangles.y += hackManager.pLocal()->GetLowerBodyYaw() - RandomInt(80, 150, -90, -160);
		}
		else
		{
			wilupdate = false;
			pCmd->viewangles.y += hackManager.pLocal()->GetLowerBodyYaw() + RandomInt(-160, -90, 90, 170);
		}
		bSendPacket = true;
	}

}
void DoRealAA(CUserCmd* pCmd, IClientEntity* pLocal, bool& bSendPacket)
{
	int flipy = (int)floorf(Interfaces::Globals->curtime / 1.1) % 2;
	static bool bFlipYaw2;


	static int y3 = pLocal->GetLowerBodyYaw() - 150;


	float idfk;
	int retard = rand() % 100;
	if (retard <= 49)
		idfk = 45;
	else idfk = 26;


	int spinautism = idfk;

	y3 += spinautism;

	if (y3 >= 179)
		y3 = -179;

	float memeantilby = Menu::Window.aabuild.moveantilby.GetValue();

	if (pCmd->viewangles.y <= memeantilby + 20)
		pCmd->viewangles.y - 25;
	if (pCmd->viewangles.y >= memeantilby - 20)
		pCmd->viewangles.y + 25;

	int rnd = rand() % 70;

	if (rnd < 20) rnd = 20;

	int rndb = rand() % 170;

	if (rndb < 40) rnd = 40;

	static bool switch2;
	Vector oldAngle = pCmd->viewangles;
	float oldForward = pCmd->forwardmove;
	float oldSideMove = pCmd->sidemove;

	if (!Menu::Window.RageBotTab.AntiAimEnable.GetState())
		return;

	if (Menu::Window.RageBotTab.freestandtoggle.GetState())
	{
		if (Menu::Window.RageBotTab.freestandtoggle.GetState())
		{
			AntiAims::Nuigermove(pLocal, pCmd, bSendPacket);

		}
		
	}





	switch (Menu::Window.RageBotTab.AntiAimYaw.GetIndex())
	{

	case 1:
		//backwards
		pCmd->viewangles.y -= 180;
		break;
	case 2:
		if (switch2)
			pCmd->viewangles.y -= -160 - rand() % 25;
		else
			pCmd->viewangles.y -= 160 - rand() % 25;
		switch2 = !switch2;
		break;
	case 3:
	{
		int rando = rand() % 1000;
		if (rando < 499)
		{
			if (switch2)
				pCmd->viewangles.y -= 60 - rand() % 20 ;
			else
				pCmd->viewangles.y -= 120 - rand() % 20 + rand() % 19;
			switch2 = !switch2;
		}
		else if (rando > 500)
		{
			if (switch2)
				pCmd->viewangles.y -= -60 - rand() % 20 + rand() % 19;
			else
				pCmd->viewangles.y -= -120 - rand() % 20 + rand() % 19;
			switch2 = !switch2;
		}
		else
			pCmd->viewangles.y -= 180;
	
	}
		break;
	case 4:
		pCmd->viewangles.y = pLocal->GetLowerBodyYaw() - 179 + rand() % 160;
		break;
	case 5:
		AntiAims::LBYSide(pCmd, bSendPacket);
		break;
	case 6:
		Keybased(pCmd, bSendPacket);
		break;
	case 7:
		AntiAims::FastSpin(pCmd);
		break;
	case 8:
	{
		static bool dir = false;
		float side = pCmd->viewangles.y - 90;
		float server_time = pLocal->GetTickBase() * Interfaces::Globals->interval_per_tick;
		if (GetAsyncKeyState(VK_LEFT)) dir = false;
		else if (GetAsyncKeyState(VK_RIGHT)) dir = true;
		
		if (dir)
		{
			if (pLocal->GetVelocity().Length2D() <= 10 && pLocal->GetHealth() > 60)
			{
				if (switch2)
					pCmd->viewangles.y -= 159 - rndb;
				else
					pCmd->viewangles.y += 159 + rndb;
				switch2 = !switch2;
			}
			else if (pLocal->GetVelocity().Length2D() <= 10 && pLocal->GetHealth() <= 60)
			{

				if (flipy)
				{
					pCmd->viewangles.y += bFlipYaw2 ? y3 : pLocal->GetLowerBodyYaw() - 106;

				}
				else
				{
					pCmd->viewangles.y -= hackManager.pLocal()->GetLowerBodyYaw() + bFlipYaw2 ? y3 : y3 - 90;
				}
			}
			else if (pLocal->GetVelocity().Length2D() > 10 && pLocal->GetVelocity().Length2D() <= 165)
			{
				if (flipy)
				{
					pCmd->viewangles.y += bFlipYaw2 ? pCmd->viewangles.y -= 150 : pCmd->viewangles.y -= 179;

				}
				else
				{
					pCmd->viewangles.y -= bFlipYaw2 ? pCmd->viewangles.y += 179 : pCmd->viewangles.y += 150;
				}
			}
			else
			{
				if (switch2)
					pCmd->viewangles.y = 157 + rnd;
				else
					pCmd->viewangles.y -= -157 - rnd;
				switch2 = !switch2;
			}
		}
		else if (!dir)
		{
			if (pLocal->GetVelocity().Length2D() <= 10 && pLocal->GetHealth() > 60)
			{
				if (switch2)
					pCmd->viewangles.y += 159 - rndb;
				else
					pCmd->viewangles.y -= 159 + rndb;
				switch2 = !switch2;
			}
			else if (pLocal->GetVelocity().Length2D() <= 10 && pLocal->GetHealth() <= 60)
			{

				if (flipy)
				{
					pCmd->viewangles.y += bFlipYaw2 ? y3 : pLocal->GetLowerBodyYaw() + 106;

				}
				else
				{
					pCmd->viewangles.y -= hackManager.pLocal()->GetLowerBodyYaw() - bFlipYaw2 ? y3 : y3 + 90;
				}
			}
			else if (pLocal->GetVelocity().Length2D() > 10 && pLocal->GetVelocity().Length2D() <= 165)
			{
				if (flipy)
				{
					pCmd->viewangles.y += bFlipYaw2 ? pCmd->viewangles.y += 150 : pCmd->viewangles.y += 179;

				}
				else
				{
					pCmd->viewangles.y -= bFlipYaw2 ? pCmd->viewangles.y -= 179 : pCmd->viewangles.y -= 150;
				}
			}
			else
			{
				if (switch2)
					pCmd->viewangles.y = 157 + rnd;
				else
					pCmd->viewangles.y -= -157 - rnd;
				switch2 = !switch2;
			}
		}

	}
	break;
	case 9:
		AntiAims::custombuildYaw(pLocal, pCmd, bSendPacket);
		break;
	}


	if (toggleSideSwitch)
		pCmd->viewangles.y += Menu::Window.RageBotTab.AntiAimOffset.GetValue() + 180;
	else
		pCmd->viewangles.y += Menu::Window.RageBotTab.AntiAimOffset.GetValue();
}

void DoFakeAA(CUserCmd* pCmd, bool& bSendPacket, IClientEntity* pLocal)
{
	static int ChokedPackets = -1;
	ChokedPackets++;
	static bool switch2;
	Vector oldAngle = pCmd->viewangles;
	float oldForward = pCmd->forwardmove;
	float oldSideMove = pCmd->sidemove;

	int flip = (int)floorf(Interfaces::Globals->curtime / 1.1) % 2;
	static bool bFlipYaw;
	static bool wilupdate;

	static int y3 = pLocal->GetLowerBodyYaw() - 150;


	float idfk;
	int retard = rand() % 100;
	if (retard <= 49)
		idfk = 45;
	else idfk = 26;


	int spinautism = idfk;

	y3 += spinautism;

	if (y3 >= 179)
		y3 = -179;

	float memeantilby = Menu::Window.aabuild.moveantilby.GetValue();

	if (pCmd->viewangles.y <= memeantilby + 20)
		pCmd->viewangles.y - 25;
	if (pCmd->viewangles.y >= memeantilby - 20)
		pCmd->viewangles.y + 25;

	int rnd = rand() % 70;

	if (rnd < 20) rnd = 20;

	if (!Menu::Window.RageBotTab.AntiAimEnable.GetState())
		return;


	switch (Menu::Window.RageBotTab.FakeYaw.GetIndex())
	{
	case 0: { /* Muslims suck */ } break;
	case 1:
		//backwards
		pCmd->viewangles.y -= 180;
		break;
	case 2:
	{
		if (switch2)
		{
			pCmd->viewangles.y -= -150 - rand() % 20 + rand() % 19;
			ChokedPackets = 1;

		}
		else
		{
			pCmd->viewangles.y -= 150 - rand() % 20 + rand() % 19;
			ChokedPackets = 1;
		}
		switch2 = !switch2;
	}
	break;
	case 3:
	{
		int rando = rand() % 750;
		if (rando < 375)
		{
			if (switch2)
				pCmd->viewangles.y -= -60 - rand() % 20;
			else
				pCmd->viewangles.y -= -120 - rand() % 20;
			switch2 = !switch2;
		}
		else if (rando > 375)
		{
			if (switch2)
				pCmd->viewangles.y -= 60 - rand() % 20;
			else
				pCmd->viewangles.y -= 120 - rand() % 20;
			switch2 = !switch2;
		}
		else
			pCmd->viewangles.y -= 180 + rand() % 45 - rand() % 20;
	}
	break;
	case 4:
	{


		pCmd->viewangles.y = pLocal->GetLowerBodyYaw() + 140 + rand() % 123 % -rand() % 50;
	}
	break;

	case 5:
	{

		fakelby(pCmd, bSendPacket); // jolt

	}
	break;
	case 6:
	{
		AntiAims::FastSpin(pCmd);
	}
	break;
	case 7:
	{
		if (pLocal->GetVelocity().Length2D() <= 10 && pLocal->GetHealth() > 60)
		{
			if (switch2)
				pCmd->viewangles.y -= 33 - rnd;
			else
				pCmd->viewangles.y += 33 + rnd;
			switch2 = !switch2;
		}
		else if (pLocal->GetVelocity().Length2D() <= 10 && pLocal->GetHealth() <= 60)
		{

			if (flip)
			{
				pCmd->viewangles.y += bFlipYaw ? y3 : pLocal->GetLowerBodyYaw() - 105;

			}
			else
			{
				pCmd->viewangles.y -= hackManager.pLocal()->GetLowerBodyYaw() + bFlipYaw ? y3 : y3 - 90;
			}
		}
		else if (pLocal->GetVelocity().Length2D() > 10 && pLocal->GetVelocity().Length2D() <= 245)
		{
			pCmd->viewangles.y = (memeantilby - 75);
		}
		else
		{
			if (switch2)
				pCmd->viewangles.y -= 157 - rnd;
			else
				pCmd->viewangles.y += 157 + rnd;
			switch2 = !switch2;
		}

	}
	break;
	case 8:  AntiAims::custombuildFake(pLocal, pCmd, bSendPacket);
		break;


	}


	if (toggleSideSwitch)
		pCmd->viewangles.y += Menu::Window.RageBotTab.AddFakeYaw.GetValue() + 180;
	else
		pCmd->viewangles.y += Menu::Window.RageBotTab.AddFakeYaw.GetValue();
}


void CRageBot::DoAntiAim(CUserCmd *pCmd, bool &bSendPacket)
{
	IClientEntity* pLocal = hackManager.pLocal();
	static int ChokedPackets = -1;
	ChokedPackets++;
	if ((pCmd->buttons & IN_USE) || pLocal->GetMoveType() == MOVETYPE_LADDER)
		return;

	if (IsAimStepping || pCmd->buttons & IN_ATTACK)
		return;

	CBaseCombatWeapon* pWeapon = (CBaseCombatWeapon*)Interfaces::EntList->GetClientEntityFromHandle(hackManager.pLocal()->GetActiveWeaponHandle());
	if (pWeapon)
	{
		CSWeaponInfo* pWeaponInfo = pWeapon->GetCSWpnData();

		if (!GameUtils::IsBallisticWeapon(pWeapon))
		{
			if (!CanOpenFire() || pCmd->buttons & IN_ATTACK2)
				return;

		}
	}

	int flip = (int)floorf(Interfaces::Globals->curtime / 1.1) % 2;
	static bool bFlipYaw;
	static bool wilupdate;
	float flInterval = Interfaces::Globals->interval_per_tick;
	float flTickcount = pCmd->tick_count;
	float flTime = flInterval * flTickcount;
	if (std::fmod(flTime, 1) == 0.f)
		bFlipYaw = !bFlipYaw;
	static float LastLBYUpdateTime = 0;

	if (Menu::Window.RageBotTab.AntiAimTarget.GetState())
	{
		aimAtPlayer(pCmd);

	}

	FakeWalk(pCmd, bSendPacket);

	switch (Menu::Window.RageBotTab.AntiAimPitch.GetIndex())
	{
	case 0:
		break;
	case 1:
		pCmd->viewangles.x = 88.f;
		break;
	case 2:
		pCmd->viewangles.x = -88.f;
		break;
	case 3:
		pCmd->viewangles.x = 1073.000000;
		break;
	case 4:
		pCmd->viewangles.x = -1073.000000;
		break;
	case 5:
	{
		pCmd->viewangles.x = 75.000000;
		if (NextLBYUpdate())
		{

			pCmd->viewangles.x = 75.000000;

			pCmd->viewangles.x = -75.000000 - rand() % 5;
			{
				pCmd->viewangles.x = 75.000000;
			}
		}

	}
	
		break;
	case 6:
	{
		float moving = abs(pLocal->GetVelocity().Length() >= 2);
		if (bSendPacket)
		{
			if (abs(pLocal->GetVelocity().Length() < 2))
			{
				pCmd->viewangles.x = 88.f;
			}
			else if (!moving)
			{
				pCmd->viewangles.x = 160.f;
				pCmd->viewangles.z = 370.f;
			}
			else
			{
				pCmd->viewangles.x = 1060.f;
				pCmd->viewangles.z = 100.f;
			}
		}
		else
		{
			if (pLocal->IsMoving() && abs(pLocal->GetVelocity().Length() < 2))
			{
				pCmd->viewangles.x = 89.f;
			}
			else if (!moving)
			{
				pCmd->viewangles.x = 163.f;
				pCmd->viewangles.z = 320.f;
			}
			else	pCmd->viewangles.x = 1090.f;
			

		}
	}
	case 7: AntiAims::custombuildPitch(pLocal, pCmd, bSendPacket);
		break;

	}

	if (Menu::Window.RageBotTab.Active.GetState() && Menu::Window.RageBotTab.AntiAimEnable.GetState())
	{

		static bool respecc = false;
		float antilby = Menu::Window.aabuild.Antilby.GetValue();
		float moveantilby = Menu::Window.aabuild.moveantilby.GetValue();

		if (Menu::Window.aabuild.FakeLagTyp.GetIndex() != 0)
		{

			static int fake = 0;

			bool onground;
			if (pLocal->IsScoped())
				onground = true;
			else
				onground = false;

			if (Menu::Window.aabuild.FakeLagTyp.GetIndex() == 0)
			{
				int tickstochoke = 2;
				//	tickstochoke = std::min<int>(Menu::Window.aabuild.FakeLagChoke.GetValue(), 13);
				if (ChokedPackets >= 1)
				{
					fake = tickstochoke + 1;
				}


				if (fake < tickstochoke)
				{
					bSendPacket = false;

					DoRealAA(pCmd, pLocal, bSendPacket);

					if (!respecc)
					{
						if (fake == 0 && pLocal->GetVelocity().Length2D() <= 1)
						{

							if (NextLBYUpdate())
							{

								pCmd->viewangles.y += antilby;

								pCmd->viewangles.y -= antilby;
								{
									pCmd->viewangles.y += antilby;
								}
							}
						}
						else if (fake == 0 && pLocal->GetVelocity().Length2D() > 1)
						{

							if (NextMovingLBYUpdate())
							{

								pCmd->viewangles.y += moveantilby;

								pCmd->viewangles.y -= moveantilby;
								{
									pCmd->viewangles.y += moveantilby;
								}
							}

						}
					}
					fake++;

				}
				else
				{
					bSendPacket = true;
					DoFakeAA(pCmd, bSendPacket, pLocal);
					fake = 0;
				}
				if (!bSendPacket)
					++ChokedPackets;
				else
					ChokedPackets = 0;

				if (ChokedPackets > 1)
					bSendPacket = true;

			}
			else if (Menu::Window.aabuild.FakeLagTyp.GetIndex() == 1)
			{
				int tickstochoke = 4;
				//	tickstochoke = std::min<int>(Menu::Window.aabuild.FakeLagChoke.GetValue(), 13);
				if (ChokedPackets >= 3)
				{
					fake = tickstochoke + 1;
				}


				if (fake < tickstochoke)
				{
					bSendPacket = false;

					DoRealAA(pCmd, pLocal, bSendPacket);

					if (!respecc)
					{
						if (fake == 0 && pLocal->GetVelocity().Length2D() <= 1)
						{

							if (NextLBYUpdate())
							{
								
								pCmd->viewangles.y += antilby;

								pCmd->viewangles.y -= antilby;
								{
									pCmd->viewangles.y += antilby;
								}
							}
						}
						else if (fake == 0 && pLocal->GetVelocity().Length2D() > 1)
						{

							if (NextMovingLBYUpdate())
							{

								pCmd->viewangles.y += moveantilby;

								pCmd->viewangles.y -= moveantilby;
						   	{
									pCmd->viewangles.y += moveantilby;
								}
							}

						}
					}
					fake++;

				}
				else
				{
					bSendPacket = true;
					DoFakeAA(pCmd, bSendPacket, pLocal);
					fake = 0;
				}
				if (!bSendPacket)
					++ChokedPackets;
				else
					ChokedPackets = 0;

				if (ChokedPackets > 3)
					bSendPacket = true;

			}
			else if (Menu::Window.aabuild.FakeLagTyp.GetIndex() == 2)
			{
				int tickstochoke = 9;

				//	tickstochoke = std::min<int>(Menu::Window.aabuild.FakeLagChoke.GetValue(), 13);
				if (ChokedPackets >= 8)
				{
					fake = tickstochoke + 1;
				}


				if (fake < tickstochoke)
				{
					bSendPacket = false;

					DoRealAA(pCmd, pLocal, bSendPacket);

					if (!respecc)
					{
						if (fake == 0 && pLocal->GetVelocity().Length2D() <= 1)
						{

							if (NextLBYUpdate())
							{

								pCmd->viewangles.y += antilby;

								pCmd->viewangles.y -= antilby;
								{
									pCmd->viewangles.y += antilby;
								}
							}
						}
						else if (fake == 0 && pLocal->GetVelocity().Length2D() >= 1)
						{

							if (NextMovingLBYUpdate())
							{

								pCmd->viewangles.y += moveantilby;

								pCmd->viewangles.y -= moveantilby;
								{
									pCmd->viewangles.y += moveantilby;
								}
							}

						}
					}
					fake++;

				}
				else
				{
					bSendPacket = true;
					DoFakeAA(pCmd, bSendPacket, pLocal);
					fake = 0;
				}


				if (!bSendPacket)
					++ChokedPackets;
				else
					ChokedPackets = 0;

				if (ChokedPackets > 8)
					bSendPacket = true;
			}
			else if (Menu::Window.aabuild.FakeLagTyp.GetIndex() == 3)
			{
				int tickstochoke = 12;

				//	tickstochoke = std::min<int>(Menu::Window.aabuild.FakeLagChoke.GetValue(), 13);
				if (ChokedPackets >= 11)
				{
					fake = tickstochoke + 1;
				}

			
				if (fake < tickstochoke)
				{
					bSendPacket = false;

					DoRealAA(pCmd, pLocal, bSendPacket);
					if (respecc)
					{
						if (fake == 0 && pLocal->GetVelocity().Length2D() <= 0.5)
						{
							if (fake == 0 && pLocal->GetVelocity().Length2D() <= 1)
							{

								if (NextLBYUpdate())
								{

									pCmd->viewangles.y += antilby;

									pCmd->viewangles.y -= antilby;
									{
										pCmd->viewangles.y += antilby;
									}
								}
							}
							else if (fake == 0 && pLocal->GetVelocity().Length2D() > 1)
							{

								if (NextMovingLBYUpdate())
								{

									pCmd->viewangles.y += moveantilby;

									pCmd->viewangles.y -= moveantilby;
									{
										pCmd->viewangles.y += moveantilby;
									}
								}

							}
						}
						fake++;

					}
					else
					{
						bSendPacket = true;
						DoFakeAA(pCmd, bSendPacket, pLocal);
						fake = 0;
					}


					if (!bSendPacket)
						++ChokedPackets;
					else
						ChokedPackets = 0;

					if (ChokedPackets >= 11)
						bSendPacket = true;
				}


			}


			if (flipAA)
			{
				pCmd->viewangles.y - 135;
			}
		}

	}
}

