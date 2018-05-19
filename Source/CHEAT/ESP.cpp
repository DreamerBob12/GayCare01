#include "ESP.h"
#include "Interfaces.h"
#include "RenderManager.h"
#include "GlowManager.h"
#include "Autowall.h"
#include <stdio.h>
#include <stdlib.h>
#include "EdgyLagComp.h"
#include "Hooks.h"
#ifdef NDEBUG
#define strenc( s ) std::string( cx_make_encrypted_string( s ) )
#define charenc( s ) strenc( s ).c_str()
#define wstrenc( s ) std::wstring( strenc( s ).begin(), strenc( s ).end() )
#define wcharenc( s ) wstrenc( s ).c_str()
#else
#define strenc( s ) ( s )
#define charenc( s ) ( s )
#define wstrenc( s ) ( s )
#define wcharenc( s ) ( s )
#endif

#ifdef NDEBUG
#define XorStr( s ) ( XorCompileTime::XorString< sizeof( s ) - 1, __COUNTER__ >( s, std::make_index_sequence< sizeof( s ) - 1>() ).decrypt() )
#else
#define XorStr( s ) ( s )
#endif

void CEsp::Init()
{
	BombCarrier = nullptr;
}

void CEsp::Move(CUserCmd *pCmd, bool &bSendPacket)
{

}

void CEsp::Draw()
{
	if (!Interfaces::Engine->IsConnected() || !Interfaces::Engine->IsInGame())
		return;

	Color Color;

	IClientEntity *pLocal = hackManager.pLocal();

	for (int i = 0; i < Interfaces::EntList->GetHighestEntityIndex(); i++)
	{
		IClientEntity *pEntity = Interfaces::EntList->GetClientEntity(i);
		player_info_t pinfo;
		CEsp::ESPBox size;
		BoxE stuff;
		if (pEntity && !pEntity->IsDormant())
		{

			if (Menu::Window.VisualsTab.OtherRadar.GetState())
			{

				DWORD m_bSpotted = NetVar.GetNetVar(0x839EB159);
				*(char*)((DWORD)(pEntity)+m_bSpotted) = 1;
			}
			if (Menu::Window.VisualsTab.OptionsName.GetState())
			{
				draw_name(pEntity, stuff);
			}
			if (Menu::Window.VisualsTab.FiltersPlayers.GetState() && Interfaces::Engine->GetPlayerInfo(i, &pinfo) && pEntity->IsAlive())
			{

				DrawPlayer(pEntity, pinfo);
			}

			ClientClass* cClass = (ClientClass*)pEntity->GetClientClass();

			if (Menu::Window.VisualsTab.FiltersNades.GetState() && strstr(cClass->m_pNetworkName, "Projectile"))
			{

				DrawThrowable(pEntity);
			}

			if (Menu::Window.VisualsTab.FiltersWeapons.GetState() && cClass->m_ClassID != (int)CSGOClassID::CBaseWeaponWorldModel && ((strstr(cClass->m_pNetworkName, "Weapon") || cClass->m_ClassID == (int)CSGOClassID::CDEagle || cClass->m_ClassID == (int)CSGOClassID::CAK47)))
			{

				DrawDrop(pEntity, cClass);
			}

			if (Menu::Window.VisualsTab.FiltersC4.GetState())
			{
				if (cClass->m_ClassID == (int)CSGOClassID::CPlantedC4)
					DrawBombPlanted(pEntity, cClass);

				if (cClass->m_ClassID == (int)CSGOClassID::CPlantedC4)
					BombTimer(pEntity, cClass);

				if (cClass->m_ClassID == (int)CSGOClassID::CC4)
					DrawBomb(pEntity, cClass);
			}
			if (Menu::Window.VisualsTab.FiltersNades.GetState() && strstr(cClass->m_pNetworkName, "Projectile"))
			{

				DrawThrowable(pEntity);
			}

			if (Menu::Window.VisualsTab.FiltersWeapons.GetState() && cClass->m_ClassID != (int)CSGOClassID::CBaseWeaponWorldModel && ((strstr(cClass->m_pNetworkName, "Weapon") || cClass->m_ClassID == (int)CSGOClassID::CDEagle || cClass->m_ClassID == (int)CSGOClassID::CAK47)))
			{

				DrawDrop(pEntity, cClass);
			}
		}
	}




	if (Menu::Window.VisualsTab.OtherNoFlash.GetState())
	{
		DWORD m_flFlashMaxAlpha = NetVar.GetNetVar(0xFE79FB98);
		*(float*)((DWORD)pLocal + m_flFlashMaxAlpha) = 0;
	}



}


float espA = Menu::Window.colourtab.alpEsp.GetValue();

float flPlayerAlpha[65];



void CEsp::DrawLinesAA(Color color)  // Strictly paste yo

{
	Vector src3D, dst3D, forward, src, dst;
	trace_t tr;
	Ray_t ray;
	CTraceFilter filter;

	filter.pSkip = hackManager.pLocal();

	// LBY
	AngleVectors(QAngle(0, lineLBY, 0), &forward);
	src3D = hackManager.pLocal()->GetOrigin();
	dst3D = src3D + (forward * 30.f);

	ray.Init(src3D, dst3D);

	Interfaces::Trace->TraceRay(ray, 0, &filter, &tr);

	if (!Render::WorldToScreen(src3D, src) || !Render::WorldToScreen(tr.endpos, dst))
		return;

	Render::Line(src.x, src.y, dst.x, dst.y, Color(180, 10, 230, 255));
	Render::Text(dst.x, dst.y, Color(180, 10, 230, 255), Render::Fonts::ESP, "LBY");


	AngleVectors(QAngle(0, lineRealAngle, 0), &forward);
	dst3D = src3D + (forward * 30.f);

	ray.Init(src3D, dst3D);

	Interfaces::Trace->TraceRay(ray, 0, &filter, &tr);

	if (!Render::WorldToScreen(src3D, src) || !Render::WorldToScreen(tr.endpos, dst))
		return;

	Render::Line(src.x, src.y, dst.x, dst.y, Color(1, 180, 250, 255));
	Render::Text(dst.x, dst.y, Color(1, 180, 250, 255), Render::Fonts::ESP, "Real");



	AngleVectors(QAngle(0, lineFakeAngle, 0), &forward);
	dst3D = src3D + (forward * 30.f);

	ray.Init(src3D, dst3D);

	Interfaces::Trace->TraceRay(ray, 0, &filter, &tr);

	if (!Render::WorldToScreen(src3D, src) || !Render::WorldToScreen(tr.endpos, dst))
		return;

	Render::Line(src.x, src.y, dst.x, dst.y, Color(255, 0, 90, 255));
	Render::Text(dst.x, dst.y, Color(255, 0, 90, 255), Render::Fonts::ESP, "Fake");


}

void CEsp::DrawPlayer(IClientEntity* pEntity, player_info_t pinfo)
{

	ESPBox Box;
	Color Color;
	BoxE stuff;
	BoxE Nuigerratskids;
	IGameEvent* event;
	Vector max = pEntity->GetCollideable()->OBBMaxs(); //
	Vector pos, pos3D;
	Vector top, top3D;
	pos3D = pEntity->GetOrigin();
	top3D = pos3D + Vector(0, 0, max.z);

	if (!Render::WorldToScreen(pos3D, pos) || !Render::WorldToScreen(top3D, top))
		return;

	if (Menu::Window.VisualsTab.FiltersEnemiesOnly.GetState() && (pEntity->GetTeamNum() == hackManager.pLocal()->GetTeamNum()))
		return;

	if (!Menu::Window.VisualsTab.FiltersSelf.GetState() && pEntity == hackManager.pLocal())
		return;

	if (GetBox(pEntity, Box))
	{
		Color = GetPlayerColor(pEntity);

		if (Menu::Window.VisualsTab.OptionsBox.GetIndex())
			DrawBox(Box, Color, pEntity);

		if (Menu::Window.VisualsTab.OptionsName.GetState())
		{
			draw_name(pEntity, stuff);
		}


		if (Menu::Window.VisualsTab.OptionHealthEnable.GetState())
		{

			hp1(pEntity, Box);
		}
		if (Menu::Window.VisualsTab.OptionsInfo.GetState() || Menu::Window.VisualsTab.OptionsWeapon.GetState())
			DrawInfo(pEntity, Box);

		if (Menu::Window.VisualsTab.OptionsArmor.GetState())
			armor1(pEntity, Box);

		if (Menu::Window.VisualsTab.Barrels.GetState())
			traceAim(pEntity, Color);


		if (Menu::Window.VisualsTab.OptionsAimSpot.GetIndex() == 0)
		{

		}
		if (Menu::Window.VisualsTab.OptionsAimSpot.GetIndex() == 1)
			DrawCross(pEntity);
		if (Menu::Window.VisualsTab.OptionsAimSpot.GetIndex() == 2)
			DrawMainPoints(pEntity);


		if (Menu::Window.VisualsTab.OptionsSkeleton.GetState())
			DrawSkeleton(pEntity);



		if (Menu::Window.VisualsTab.BacktrackingLol.GetState())
			BacktrackingCross(pEntity);


		if (hackManager.pLocal()->IsAlive())
		{
			if (Menu::Window.VisualsTab.AALines.GetState() == 1)
				CEsp::DrawLinesAA(Color);

			else
			{
				//Don't draw the lines
			}
		}

	}
}


void CEsp::traceAim(IClientEntity* pEntity, Color color)
{
	Vector src3D, dst3D, forward, src, dst;
	trace_t tr;
	Ray_t ray;
	CTraceFilter filter;

	AngleVectors(pEntity->GetEyeAngles(), &forward);
	filter.pSkip = pEntity;
	src3D = pEntity->GetEyeAngles() - Vector(0, 0, 0);
	dst3D = src3D + (forward * 120);

	ray.Init(src3D, dst3D);

	Interfaces::Trace->TraceRay(ray, MASK_SHOT, &filter, &tr);

	if (!Render::WorldToScreen(src3D, src) || !Render::WorldToScreen(tr.endpos, dst))
		return;

	Render::DrawLine(src.x, src.y, dst.x, dst.y, color);
	Render::DrawOutlinedRect(dst.x - 3, dst.y - 3, 6, 6, Color(200, 200, 200, 255));
};


bool CEsp::GetBox(IClientEntity* pEntity, CEsp::ESPBox &result)
{
	Vector  vOrigin, min, max, sMin, sMax, sOrigin,
		flb, brt, blb, frt, frb, brb, blt, flt;
	float left, top, right, bottom;

	vOrigin = pEntity->GetOrigin();
	min = pEntity->collisionProperty()->GetMins() + vOrigin;
	max = pEntity->collisionProperty()->GetMaxs() + vOrigin;

	Vector points[] = { Vector(min.x, min.y, min.z),
		Vector(min.x, max.y, min.z),
		Vector(max.x, max.y, min.z),
		Vector(max.x, min.y, min.z),
		Vector(max.x, max.y, max.z),
		Vector(min.x, max.y, max.z),
		Vector(min.x, min.y, max.z),
		Vector(max.x, min.y, max.z) };

	if (!Render::WorldToScreen(points[3], flb) || !Render::WorldToScreen(points[5], brt)
		|| !Render::WorldToScreen(points[0], blb) || !Render::WorldToScreen(points[4], frt)
		|| !Render::WorldToScreen(points[2], frb) || !Render::WorldToScreen(points[1], brb)
		|| !Render::WorldToScreen(points[6], blt) || !Render::WorldToScreen(points[7], flt))
		return false;

	Vector arr[] = { flb, brt, blb, frt, frb, brb, blt, flt };

	left = flb.x;
	top = flb.y;
	right = flb.x;
	bottom = flb.y;

	for (int i = 1; i < 8; i++)
	{
		if (left > arr[i].x)
			left = arr[i].x;
		if (bottom < arr[i].y)
			bottom = arr[i].y;
		if (right < arr[i].x)
			right = arr[i].x;
		if (top > arr[i].y)
			top = arr[i].y;
	}

	result.x = left;
	result.y = top;
	result.w = right - left;
	result.h = bottom - top;

	return true;
}

Color CEsp::GetPlayerColor(IClientEntity* pEntity)
{
	int TeamNum = pEntity->GetTeamNum();
	bool IsVis = GameUtils::IsVisible(hackManager.pLocal(), pEntity, (int)CSGOHitboxID::Head);

	Color color;

	if (TeamNum == TEAM_CS_T)
	{
		if (IsVis)
			color = Color(250, 250, 250, 255);
		else
			color = Color(240, 240, 240, 255);
	}
	else
	{

		if (IsVis)
			color = Color(250, 250, 250, 255);
		else
			color = Color(240, 240, 240, 255);
	}


	return color;
}



float tr = Menu::Window.colourtab.TespR.GetValue();
float tg = Menu::Window.colourtab.TespG.GetValue();
float tb = Menu::Window.colourtab.TespB.GetValue();

float ctr = Menu::Window.colourtab.CTespR.GetValue();
float ctg = Menu::Window.colourtab.CTespG.GetValue();
float ctb = Menu::Window.colourtab.CTespB.GetValue();

float alpEsp = Menu::Window.colourtab.alpEsp.GetValue();



void CEsp::DrawBox(CEsp::ESPBox size, Color color, IClientEntity *pEntity)
{
	{
		{
			int VertLine;
			int HorzLine;
			// Corner Box


			//	CSlider alpMenu;
			//	CSlider alpEsp;

		
			if (Menu::Window.VisualsTab.OptionsBox.GetIndex() == 0)
			{

			}
			if (Menu::Window.VisualsTab.OptionsBox.GetIndex() == 1)
			{
				// Corner Box
				int VertLine = (((float)size.w) * (0.20f));
				int HorzLine = (((float)size.h) * (1.00f));
				int TeamNum = pEntity->GetTeamNum();
				if (TeamNum == TEAM_CS_T)
				{
					Render::Clear(size.x, size.y - 1, VertLine, 1, Color(tr, tg, tb, alpEsp));
					Render::Clear(size.x + size.w - VertLine, size.y - 1, VertLine, 1, Color(tr, tg, tb, alpEsp));
					Render::Clear(size.x, size.y + size.h - 1, VertLine, 1, Color(tr, tg, tb, alpEsp));
					Render::Clear(size.x + size.w - VertLine, size.y + size.h - 1, VertLine, 1, Color(tr, tg, tb, alpEsp));

					Render::Clear(size.x - 1, size.y, 1, HorzLine, Color(tr, tg, tb, alpEsp));
					Render::Clear(size.x - 1, size.y + size.h - HorzLine, 1, HorzLine, Color(tr, tg, tb, alpEsp));
					Render::Clear(size.x + size.w - 1, size.y, 1, HorzLine, Color(tr, tg, tb, alpEsp));
					Render::Clear(size.x + size.w - 1, size.y + size.h - HorzLine, 1, HorzLine, Color(tr, tg, tb, alpEsp));

					Render::Clear(size.x, size.y, VertLine, 1.2, color);
					Render::Clear(size.x + size.w - VertLine, size.y, VertLine, 1.2, color);
					Render::Clear(size.x, size.y + size.h, VertLine, 1.2, color);
					Render::Clear(size.x + size.w - VertLine, size.y + size.h, VertLine, 1.2, color);

					Render::Clear(size.x, size.y, 1, HorzLine, color);
					Render::Clear(size.x, size.y + size.h - HorzLine, 1, HorzLine, color);
					Render::Clear(size.x + size.w, size.y, 1, HorzLine, color);
					Render::Clear(size.x + size.w, size.y + size.h - HorzLine, 1, HorzLine, color);
				}
				else if (TeamNum == TEAM_CS_CT)
				{
					Render::Clear(size.x, size.y - 1, VertLine, 1, Color(ctr, ctg, ctb, alpEsp));
					Render::Clear(size.x + size.w - VertLine, size.y - 1, VertLine, 1, Color(ctr, ctg, ctb, alpEsp));
					Render::Clear(size.x, size.y + size.h - 1, VertLine, 1, Color(ctr, ctg, ctb, alpEsp));
					Render::Clear(size.x + size.w - VertLine, size.y + size.h - 1, VertLine, 1, Color(ctr, ctg, ctb, alpEsp));

					Render::Clear(size.x - 1, size.y, 1, HorzLine, Color(ctr, ctg, ctb, alpEsp));
					Render::Clear(size.x - 1, size.y + size.h - HorzLine, 1, HorzLine, Color(ctr, ctg, ctb, alpEsp));
					Render::Clear(size.x + size.w - 1, size.y, 1, HorzLine, Color(ctr, ctg, ctb, alpEsp));
					Render::Clear(size.x + size.w - 1, size.y + size.h - HorzLine, 1, HorzLine, Color(ctr, ctg, ctb, alpEsp));

					Render::Clear(size.x, size.y, VertLine, 1.2, color);
					Render::Clear(size.x + size.w - VertLine, size.y, VertLine, 1.2, color);
					Render::Clear(size.x, size.y + size.h, VertLine, 1.2, color);
					Render::Clear(size.x + size.w - VertLine, size.y + size.h, VertLine, 1.2, color);

					Render::Clear(size.x, size.y, 1, HorzLine, color);
					Render::Clear(size.x, size.y + size.h - HorzLine, 1, HorzLine, color);
					Render::Clear(size.x + size.w, size.y, 1, HorzLine, color);
					Render::Clear(size.x + size.w, size.y + size.h - HorzLine, 1, HorzLine, color);
				}

			}
			if (Menu::Window.VisualsTab.OptionsBox.GetIndex() == 2)
			{
				int TeamNum = pEntity->GetTeamNum();
				if (TeamNum == TEAM_CS_T)
				{
					int VertLine = (((float)size.w) * (1.00f));
					int HorzLine = (((float)size.h) * (1.00f));

					int BVertLine = (((float)size.w) * (1.00f));
					int BHorzLine = (((float)size.h) * (1.00f));


					Render::Clear(size.x, size.y - 1, VertLine, 1, Color(tr, tg, tb, alpEsp));
					Render::Clear(size.x + size.w - VertLine, size.y - 1, VertLine, 1, Color(tr, tg, tb, alpEsp));
					Render::Clear(size.x, size.y + size.h - 1, VertLine, 1, Color(tr, tg, tb, alpEsp));
					Render::Clear(size.x + size.w - VertLine, size.y + size.h - 1, VertLine, 1, Color(tr, tg, tb, alpEsp));
					Render::Clear(size.x - 1, size.y, 1, BHorzLine, Color(tr, tg, tb, alpEsp));
					Render::Clear(size.x + size.w - 1, size.y, 1, BHorzLine, Color(tr, tg, tb, alpEsp));

					Render::Clear(size.x, size.y, VertLine, 1, color);	Render::Clear(size.x + size.w - VertLine, size.y, VertLine, 1, color);
					Render::Clear(size.x, size.y + size.h, VertLine, 1, color);
					Render::Clear(size.x + size.w - VertLine, size.y + size.h, VertLine, 1, color);
					Render::Clear(size.x, size.y, 1, BHorzLine, color);

					Render::Clear(size.x, size.y + size.h - BHorzLine, 1, BHorzLine, color);
					Render::Clear(size.x + size.w, size.y, 1, BHorzLine, color);
					Render::Clear(size.x + size.w, size.y + size.h - BHorzLine, 1, BHorzLine, color);
				}
				else
				{
					int VertLine = (((float)size.w) * (1.00f));
					int HorzLine = (((float)size.h) * (1.00f));

					int BVertLine = (((float)size.w) * (1.00f));
					int BHorzLine = (((float)size.h) * (1.00f));



					Render::Clear(size.x, size.y - 1, VertLine, 1, Color(ctr, ctg, ctb, alpEsp));
					Render::Clear(size.x + size.w - VertLine, size.y - 1, VertLine, 1, Color(ctr, ctg, ctb, alpEsp));
					Render::Clear(size.x, size.y + size.h - 1, VertLine, 1, Color(ctr, ctg, ctb, alpEsp));
					Render::Clear(size.x + size.w - VertLine, size.y + size.h - 1, VertLine, 1, Color(ctr, ctg, ctb, alpEsp));
					Render::Clear(size.x - 1, size.y, 1, BHorzLine, Color(ctr, ctg, ctb, alpEsp));
					Render::Clear(size.x + size.w - 1, size.y, 1, BHorzLine, Color(ctr, ctg, ctb, alpEsp));

					Render::Clear(size.x, size.y, VertLine, 1, Color(ctr, ctg, ctb, alpEsp));
					Render::Clear(size.x + size.w - VertLine, size.y, VertLine, 1, Color(ctr, ctg, ctb, alpEsp));
					Render::Clear(size.x, size.y + size.h, VertLine, 1, Color(ctr, ctg, ctb, alpEsp));
					Render::Clear(size.x + size.w - VertLine, size.y + size.h, VertLine, 1, Color(ctr, ctg, ctb, alpEsp));
					Render::Clear(size.x, size.y, 1, BHorzLine, Color(ctr, ctg, ctb, alpEsp));

					Render::Clear(size.x, size.y + size.h - BHorzLine, 1, BHorzLine, Color(ctr, ctg, ctb, alpEsp));
					Render::Clear(size.x + size.w, size.y, 1, BHorzLine, Color(ctr, ctg, ctb, alpEsp));
					Render::Clear(size.x + size.w, size.y + size.h - BHorzLine, 1, BHorzLine, Color(ctr, ctg, ctb, alpEsp));

					/* CARTOONY SHIT
					Render::Clear(size.x, size.y - 1, VertLine, 1, Color(10, 140, 255, 255));
					Render::Clear(size.x + size.w - VertLine, size.y - 1, VertLine, 1, Color(10, 140, 255, 255));
					Render::Clear(size.x, size.y + size.h - 1, VertLine, 1, Color(10, 140, 255, 255));
					Render::Clear(size.x + size.w - VertLine, size.y + size.h - 1, VertLine, 1, Color(10, 140, 255, 255));
					Render::Clear(size.x - 1, size.y, 1, BHorzLine, Color(10, 140, 255, 255));
					Render::Clear(size.x + size.w - 1, size.y, 1, BHorzLine, Color(10, 140, 255, 255));

					Render::Clear(size.x, size.y, VertLine, 1, color);	Render::Clear(size.x + size.w - VertLine, size.y, VertLine, 1, color);
					Render::Clear(size.x, size.y + size.h, VertLine, 1, color);
					Render::Clear(size.x + size.w - VertLine, size.y + size.h, VertLine, 1, color);
					Render::Clear(size.x, size.y, 1, BHorzLine, color);

					Render::Clear(size.x, size.y + size.h - BHorzLine, 1, BHorzLine, color);
					Render::Clear(size.x + size.w, size.y, 1, BHorzLine, color);
					Render::Clear(size.x + size.w, size.y + size.h - BHorzLine, 1, BHorzLine, color);

					*/
				}

			}


		}
	}
}




void CEsp::Barrel(CEsp::ESPBox size, Color color, IClientEntity* pEntity)
{

	Vector src3D, src;
	src3D = pEntity->GetOrigin() - Vector(0, 0, 0);

	if (!Render::WorldToScreen(src3D, src))
		return;

	int ScreenWidth, ScreenHeight;
	Interfaces::Engine->GetScreenSize(ScreenWidth, ScreenHeight);

	int x = (int)(ScreenWidth * 0.5f);
	int y = 0;


	y = ScreenHeight;

	Render::Line((int)(src.x), (int)(src.y), x, y, Color(255, 255, 255, 255));
}
void CEsp::DrawNameX(player_info_t player_info, CEsp::ESPBox size)
{



	RECT name_size = Render::get_text_size(player_info.name, Render::Fonts::ESP);
	Render::text(size.x + (size.w / 2) - (name_size.right / 2), size.y - 14, player_info.name, Render::Fonts::otheresp, Color(225, 225, 225, 255));

	//		Color(255, 255, 255, 255), Render::Fonts::ESP, pinfo.name);
}
std::string CleanItemName(std::string name)
{

	std::string Name = name;
	if (Name[0] == 'C')
		Name.erase(Name.begin());

	auto startOfWeap = Name.find("Weapon");
	if (startOfWeap != std::string::npos)
		Name.erase(Name.begin() + startOfWeap, Name.begin() + startOfWeap + 6);

	return Name;
}



void CEsp::draw_name(IClientEntity* pEntity, BoxE box)
{


	player_info_t player_info;
	if (Interfaces::Engine->GetPlayerInfo(pEntity->GetIndex(), &player_info))
	{
		RECT name_size = Render::get_text_size(player_info.name, Render::Fonts::ESP);
		Render::text(box.x + (box.w / 2) - (name_size.right / 2), box.y - 14, player_info.name, Render::Fonts::ESP, Color(225, 225, 225, 255));
	}

}


static wchar_t* CharToWideChar(const char* text)
{
	size_t size = strlen(text) + 1;
	wchar_t* wa = new wchar_t[size];
	mbstowcs_s(NULL, wa, size / 4, text, size);
	return wa;
}

void CEsp::BacktrackingCross(IClientEntity* pEntity)
{
	for (int i = 0; i < Interfaces::EntList->GetHighestEntityIndex(); i++)
	{

		player_info_t pinfo;
		if (pEntity && pEntity != hackManager.pLocal() && !pEntity->IsDormant())
		{
			if (Interfaces::Engine->GetPlayerInfo(i, &pinfo) && pEntity->IsAlive())
			{

				if (Menu::Window.LegitBotTab.AimbotBacktrack.GetState())
				{

					if (hackManager.pLocal()->IsAlive())
					{
						for (int t = 0; t < 12; ++t)
						{

							Vector screenbacktrack[64][12];

							if (headPositions[i][t].simtime && headPositions[i][t].simtime + 1 > hackManager.pLocal()->GetSimulationTime())
							{

								if (Render::WorldToScreen(headPositions[i][t].hitboxPos, screenbacktrack[i][t]))
								{

									Interfaces::Surface->DrawSetColor(Color(180, 10, 250, 255));
									Interfaces::Surface->DrawOutlinedRect(screenbacktrack[i][t].x, screenbacktrack[i][t].y, screenbacktrack[i][t].x + 2, screenbacktrack[i][t].y + 2);

								}

							}
						}

					}
					else
					{

						memset(&headPositions[0][0], 0, sizeof(headPositions));
					}
				}
				if (Menu::Window.RageBotTab.ragetrack.GetKey())
				{

					if (hackManager.pLocal()->IsAlive())
					{
						for (int t = 0; t < 12; ++t)
						{

							Vector screenbacktrack[64];

							if (backtracking->records[i].tick_count + 12 > Interfaces::Globals->tickcount)
							{
								if (Render::WorldToScreen(backtracking->records[i].headPosition, screenbacktrack[i]))
								{
									Interfaces::Surface->DrawSetColor(Color(195, 0, 250, 255));
									Interfaces::Surface->DrawOutlinedRect(screenbacktrack[i].x, screenbacktrack[i].y, screenbacktrack[i].x + 2, screenbacktrack[i].y + 2);

								}

							}
						}

					}
					else
					{
						memset(&backtracking->records[0], 0, sizeof(backtracking->records));
					}




				}
			}

		}
	}

}



void CEsp::BombTimer(IClientEntity* pEntity, ClientClass* cClass)
{
	BombCarrier = nullptr;

	Vector vOrig; Vector vScreen;
	vOrig = pEntity->GetOrigin();
	CCSBomb* Bomb = (CCSBomb*)pEntity;

	if (Render::WorldToScreen(vOrig, vScreen))
	{

		ESPBox Box;
		GetBox(pEntity, Box);

		float flBlow = Bomb->GetC4BlowTime();
		float TimeRemaining = flBlow - (Interfaces::Globals->interval_per_tick * hackManager.pLocal()->GetTickBase());
		float TimeRemaining2;
		bool exploded = true;
		if (TimeRemaining < 0)
		{
			!exploded;

			TimeRemaining2 = 0;
		}
		else
		{
			exploded = true;
			TimeRemaining2 = TimeRemaining;
		}
		char buffer[64];
		if (exploded)
		{

			sprintf_s(buffer, "Bomb: [ %.1f ]", TimeRemaining2);
			Render::Text(vScreen.x, vScreen.y, Color(255, 150, 30, alpEsp), Render::Fonts::ESP, buffer);
		}
		else
		{
			float red = 180 + rand() % 75;
			float green = 55 + rand() % 75;
			float blue = 10 + rand() % 30;
			sprintf_s(buffer, "Bomb: [ Run! ]", TimeRemaining2);
			Render::Text(vScreen.x, vScreen.y, Color(red, green, blue, alpEsp), Render::Fonts::ESP, buffer);
		}
	}

}


CEsp::ESPBox CEsp::GetBOXX(IClientEntity* pEntity)
{

	ESPBox result;
	// Variables
	Vector  vOrigin, min, max, sMin, sMax, sOrigin,
		flb, brt, blb, frt, frb, brb, blt, flt;
	float left, top, right, bottom;

	// Get the locations
	vOrigin = pEntity->GetOrigin();
	min = pEntity->collisionProperty()->GetMins() + vOrigin;
	max = pEntity->collisionProperty()->GetMaxs() + vOrigin;

	// Points of a 3d bounding box
	Vector points[] = { Vector(min.x, min.y, min.z),
		Vector(min.x, max.y, min.z),
		Vector(max.x, max.y, min.z),
		Vector(max.x, min.y, min.z),
		Vector(max.x, max.y, max.z),
		Vector(min.x, max.y, max.z),
		Vector(min.x, min.y, max.z),
		Vector(max.x, min.y, max.z) };

	// Get screen positions
	if (!Render::WorldToScreen(points[3], flb) || !Render::WorldToScreen(points[5], brt)
		|| !Render::WorldToScreen(points[0], blb) || !Render::WorldToScreen(points[4], frt)
		|| !Render::WorldToScreen(points[2], frb) || !Render::WorldToScreen(points[1], brb)
		|| !Render::WorldToScreen(points[6], blt) || !Render::WorldToScreen(points[7], flt))
		return result;

	// Put them in an array (maybe start them off in one later for speed?)
	Vector arr[] = { flb, brt, blb, frt, frb, brb, blt, flt };

	// Init this shit
	left = flb.x;
	top = flb.y;
	right = flb.x;
	bottom = flb.y;

	// Find the bounding corners for our box
	for (int i = 1; i < 8; i++)
	{
		if (left > arr[i].x)
			left = arr[i].x;
		if (bottom < arr[i].y)
			bottom = arr[i].y;
		if (right < arr[i].x)
			right = arr[i].x;
		if (top > arr[i].y)
			top = arr[i].y;
	}

	// Width / height
	result.x = left;
	result.y = top;
	result.w = right - left;
	result.h = bottom - top;

	return result;
}


#define SurfaceX Interfaces::Surface
void DrawLin(int x0, int y0, int x1, int y1, Color col)
{
	SurfaceX->DrawSetColor(col);
	SurfaceX->DrawLine(x0, y0, x1, y1);
}

void CEsp::hp1(IClientEntity* pEntity, CEsp::ESPBox size)
{
	/*

	ESPBox HealthBar = size;
	HealthBar.y += (HealthBar.h + 6);
	HealthBar.h = 4;

	float HealthValue = pEntity->GetHealth();
	float HealthPerc = HealthValue / 100.f;
	float flBoxes = std::ceil(HealthValue / 10.f);
	float Width = (size.w * HealthPerc);
	HealthBar.w = Width;
	float h = (size.h);
	float offset = (h / 4.f) + 5;
	float w = h / 64.f;
	float health = pEntity->GetHealth();
	float flMultiplier = 12 / 360.f; flMultiplier *= flBoxes - 1;
	Color ColHealth = Color::FromHSB(flMultiplier, 1, 1);

	UINT hp = h - (UINT)((h * health) / 100); // Percentage

	int Red = 255 - (health*2.55);
	int Green = health * 2.55;

	Render::Outline((size.x - 6) + 1, size.y - 2, 3, h + 3, Color(0, 0, 0, 230));
	Render::Outline((size.x - 6) - 2, size.y - 2, 3, h + 3, Color(0, 0, 0, 230));
	Render::Outline((size.x - 6) - 1, size.y - 1, 3, h + 2, Color(0, 0, 0, 255));
	Render::Outline((size.x - 6) - 1, size.y - 1, 3, h + 2, Color(0, 0, 0, 255));
	Render::Line((size.x - 6), size.y + hp, (size.x - 6), size.y + h, Color(Red, Green, 0, 255));
	Render::Outline((size.x - 6), size.y - 1, 3, h + 2, Color(0, 0, 0, 255));
	Render::Outline(size.x - 5, size.y - 1, 1, (h / 10) * flBoxes + 1, ColHealth);
	Render::Line((size.x - 6), size.y + hp, (size.x - 6), size.y + h, Color(Red, Green, 0, 255)); 

	for (int i = 0; i < 10; i++)
	{
	Render::Line((size.x - 6), size.y + i * (h / 10) - 1, size.x - 3, size.y + i * (h / 10), Color::Black());
	}


 */ESPBox HealthBar = size;
if (Menu::Window.VisualsTab.OptionHealthEnable.GetState())
HealthBar.y += (HealthBar.h + 6);
else
HealthBar.y += (HealthBar.h + 4);

HealthBar.h = 4;

float HealthValue = pEntity->GetHealth();
float HealthPerc = HealthValue / 100.f;
float Width = (size.w * HealthPerc);
HealthBar.w = Width;

//  Main Bar  //
Vertex_t Verts[4];
Verts[0].Init(Vector2D(HealthBar.x, HealthBar.y));
Verts[1].Init(Vector2D(HealthBar.x + size.w, HealthBar.y));
Verts[2].Init(Vector2D(HealthBar.x + size.w, HealthBar.y + 5));
Verts[3].Init(Vector2D(HealthBar.x, HealthBar.y + 5));

Render::PolygonOutline(4, Verts, Color(0, 0, 0, 255), Color(0, 0, 0, 255));

Vertex_t Verts2[4];
Verts2[0].Init(Vector2D(HealthBar.x + 1, HealthBar.y + 1));
Verts2[1].Init(Vector2D(HealthBar.x + HealthBar.w, HealthBar.y + 1));
Verts2[2].Init(Vector2D(HealthBar.x + HealthBar.w, HealthBar.y + 5));
Verts2[3].Init(Vector2D(HealthBar.x, HealthBar.y + 5));

Color c = Color(0, 230, 20, 255);
Render::Polygon(4, Verts2, c);

Verts2[0].Init(Vector2D(HealthBar.x + 1, HealthBar.y + 1));
Verts2[1].Init(Vector2D(HealthBar.x + HealthBar.w, HealthBar.y + 1));
Verts2[2].Init(Vector2D(HealthBar.x + HealthBar.w, HealthBar.y + 2));
Verts2[3].Init(Vector2D(HealthBar.x, HealthBar.y + 2));

Render::Polygon(4, Verts2, Color(200, 200, 200, 255));

}
void CEsp::armor1(IClientEntity* pEntity, CEsp::ESPBox size)
{

	ESPBox ArmorBar = size;
	if (Menu::Window.VisualsTab.OptionsArmor.GetState())
		ArmorBar.y += (ArmorBar.h + 15);
	else
		ArmorBar.y += (ArmorBar.h + 9);

	ArmorBar.h = 4;

	float ArmorValue = pEntity->ArmorValue();
	float ArmorPerc = ArmorValue / 100.f;
	float Width = (size.w * ArmorPerc);
	ArmorBar.w = Width;

	//  Main Bar  //
	Vertex_t Verts[4];
	Verts[0].Init(Vector2D(ArmorBar.x, ArmorBar.y));
	Verts[1].Init(Vector2D(ArmorBar.x + size.w, ArmorBar.y));
	Verts[2].Init(Vector2D(ArmorBar.x + size.w, ArmorBar.y + 5));
	Verts[3].Init(Vector2D(ArmorBar.x, ArmorBar.y + 5));

	Render::PolygonOutline(4, Verts, Color(0, 0, 0, 255), Color(0, 0, 0, 255));

	Vertex_t Verts2[4];
	Verts2[0].Init(Vector2D(ArmorBar.x + 1, ArmorBar.y + 1));
	Verts2[1].Init(Vector2D(ArmorBar.x + ArmorBar.w, ArmorBar.y + 1));
	Verts2[2].Init(Vector2D(ArmorBar.x + ArmorBar.w, ArmorBar.y + 5));
	Verts2[3].Init(Vector2D(ArmorBar.x, ArmorBar.y + 5));

	Color c = Color(0, 15, 230, 255);  // Darker?
	Render::Polygon(4, Verts2, c);

	Verts2[0].Init(Vector2D(ArmorBar.x + 1, ArmorBar.y + 1));
	Verts2[1].Init(Vector2D(ArmorBar.x + ArmorBar.w, ArmorBar.y + 1));
	Verts2[2].Init(Vector2D(ArmorBar.x + ArmorBar.w, ArmorBar.y + 2));
	Verts2[3].Init(Vector2D(ArmorBar.x, ArmorBar.y + 2));

	Render::Polygon(4, Verts2, Color(0, 0, 250, 255));


	/// If anyone wants the old Armor bar, here it is.

	/*

	ESPBox ArmorBar = size;
	ArmorBar.y += (ArmorBar.h + 6);
	ArmorBar.h = 4;

	float ArmorValue = pEntity->ArmorValue();
	float ArmorPerc = ArmorValue / 100.f;
	float flBoxes = std::ceil(ArmorValue / 10.f);
	float Width = (size.w * ArmorPerc);
	ArmorBar.w = Width;
	float h = (size.h);
	float offset = (h / -4.f) - 5;
	float w = h / 64.f;
	float Armor = pEntity->ArmorValue();
	float flMultiplier = 12 / 360.f; flMultiplier *= flBoxes - 1;
	Color ColArmor = Color::FromHSB(flMultiplier, 1, 1);

	UINT hp = h - (UINT)((h * Armor) / 100); // Percentage

	int Red = 255 - (Armor*2.55);
	int Green = Armor * 2.55;

	Render::Outline((size.x - 6) + 1, size.y - 2, 3, h + 3, Color(0, 0, 0, 230));
	Render::Outline((size.x - 6) - 2, size.y - 2, 3, h + 3, Color(0, 0, 0, 230));
	Render::Outline((size.x - 6) - 1, size.y - 1, 3, h + 2, Color(0, 0, 0, 255));
	Render::Outline((size.x - 6) - 1, size.y - 1, 3, h + 2, Color(0, 0, 0, 255));
	Render::Line((size.x - 6), size.y + hp, (size.x - 6), size.y + h, Color(Red, Green, 0, 255));
	Render::Outline((size.x - 6), size.y - 1, 3, h + 2, Color(0, 0, 0, 255));
	Render::Outline(size.x - 5, size.y - 1, 1, (h / 10) * flBoxes + 1, ColArmor);
	Render::Line((size.x - 6), size.y + hp, (size.x - 6), size.y + h, Color(Red, Green, 0, 255));

	for (int i = 0; i < 10; i++)
	{
	Render::Line((size.x - 6), size.y + i * (h / 10) - 1, size.x - 3, size.y + i * (h / 10), Color::Black());
	}


	*/
}




void DrawOutlinedRect(int x, int y, int w, int h, Color col)
{
	Interfaces::Surface->DrawSetColor(col);
	Interfaces::Surface->DrawOutlinedRect(x, y, x + w, y + h);
}
#define SurfaceX Interfaces::Surface

void DrawOutlinedRect2(int x, int y, int w, int h, Color col)
{
	SurfaceX->DrawSetColor(col);
	SurfaceX->DrawOutlinedRect(x, y, x + w, y + h);
}

void DrawLine(int x0, int y0, int x1, int y1, Color col)
{
	Interfaces::Surface->DrawSetColor(col);
	Interfaces::Surface->DrawLine(x0, y0, x1, y1);
}

void CEsp::DrawInfo(IClientEntity* pEntity, CEsp::ESPBox size)
{

	std::vector<std::string> Info;
	std::vector<std::string> Info2;

	IClientEntity* pWeapon = Interfaces::EntList->GetClientEntityFromHandle((HANDLE)pEntity->GetActiveWeaponHandle());
	if (Menu::Window.VisualsTab.OptionsWeapon.GetState() && pWeapon)
	{
		std::vector<std::string> Info;

		// Player Weapon ESP
		IClientEntity* pWeapon = Interfaces::EntList->GetClientEntityFromHandle((HANDLE)pEntity->GetActiveWeaponHandle());
		if (Menu::Window.VisualsTab.OptionsWeapon.GetState() && pWeapon)
		{
			ClientClass* cClass = (ClientClass*)pWeapon->GetClientClass();
			if (cClass)
			{
				// Draw it
				Info.push_back(CleanItemName(cClass->m_pNetworkName));
			}
		}



		static RECT Size = Render::GetTextSize(Render::Fonts::Menu, "Text");
		int i = 0;
		for (auto Text : Info)
		{

			Render::Text(size.x + size.w + 3, size.y + (i*(Size.bottom + 2)), Color(255, 255, 255, 255), Render::Fonts::Menu, Text.c_str());
			i++;
		}

	}
	RECT defSize = Render::GetTextSize(Render::Fonts::otheresp, "");
	if (Menu::Window.VisualsTab.OptionsInfo.GetState() && pEntity->IsDefusing())
	{
		Render::Text(size.x + size.w + 3, size.y + (0.3*(defSize.bottom + 15)),
			Color(255, 0, 0, 255), Render::Fonts::otheresp, charenc("Defusing"));
	}

	if (Menu::Window.VisualsTab.OptionsInfo.GetState() && pEntity == BombCarrier)
	{
		Info.push_back("Bomb Carrier");
	}

	if (Menu::Window.VisualsTab.ResolverInfo.GetState())
	{
		if (resolvokek::resom == 1)
			Info.push_back(" Active ");
		else if (pEntity->GetVelocity().Length2D() < 36 && pEntity->GetVelocity().Length2D() > 0.4f)
			Info.push_back("FakeWalk (?)");
		else if (resolvokek::resom == 2)
			Info.push_back("Estimating");

		else if (resolvokek::resom == 3)
			Info.push_back("Tracking");
		else if (resolvokek::resom == 4)
			Info2.push_back("Found Fake");

	}

	if (Menu::Window.VisualsTab.ResolverInfo.GetState())
	{
		int BaimKey = Menu::Window.RageBotTab.AimbotBaimOnKey.GetKey();


	}

	static RECT Size = Render::GetTextSize(Render::Fonts::Default, "Sup");
	int i = 0;
	for (auto Text : Info)
	{
		RECT nameSize = Render::GetTextSize(Render::Fonts::ESP, Text.c_str());
		Render::Text(size.x + (size.w / 2) - (nameSize.right / 2), size.y - 27, Color(180, 29, 250, 255), Render::Fonts::ESP, Text.c_str());
		//Render::Text(size.x + size.w + 3, size.y + (i*(Size.bottom + 2)), Color(255, 255, 255, 255), Render::Fonts::ESP, Text.c_str());
		i++;
	}
	for (auto Text : Info2)
	{
		RECT nameSize = Render::GetTextSize(Render::Fonts::ESP, Text.c_str());
		Render::Text(size.x + (size.w / 2) - (nameSize.right / 2), size.y - 27, Color(10, 250, 140, 255), Render::Fonts::ESP, Text.c_str());
		//Render::Text(size.x + size.w + 3, size.y + (i*(Size.bottom + 2)), Color(255, 255, 255, 255), Render::Fonts::ESP, Text.c_str());
		i++;
	}
}

void CEsp::DrawCross(IClientEntity* pEntity)
{

	Vector cross = pEntity->GetHeadPos(), screen;
	static int Scale = 2;
	if (Render::WorldToScreen(cross, screen))
	{
		Render::Clear(screen.x - Scale, screen.y - (Scale * 2), (Scale * 2), (Scale * 4), Color(20, 20, 20, alpEsp));
		Render::Clear(screen.x - (Scale * 2), screen.y - Scale, (Scale * 4), (Scale * 2), Color(20, 20, 20, alpEsp));
		Render::Clear(screen.x - Scale - 1, screen.y - (Scale * 2) - 1, (Scale * 2) - 2, (Scale * 4) - 2, Color(250, 250, 250, alpEsp));
		Render::Clear(screen.x - (Scale * 2) - 1, screen.y - Scale - 1, (Scale * 4) - 2, (Scale * 2) - 2, Color(250, 250, 250, alpEsp));
	}

}

/*
Vector GetRightHand()
{
return this->GetBonePos(43);
}
Vector GetLeftHand()
{
return this->GetBonePos(13);
}
Vector GetUpRightLeg()
{
return this->GetBonePos(78);
}
Vector GetUpLeftLeg()
{
return this->GetBonePos(71);
}
Vector GetLowRightLeg()
{
return this->GetBonePos(79);
}
Vector GetLowLeftLag()
{
return this->GetBonePos(72);
}
*/

void CEsp::DrawMainPoints(IClientEntity* pEntity)
{

	Vector cross = pEntity->GetHeadPos(), screen1;

	Vector cross2 = pEntity->GetTopChest(), screen2;

	Vector cross3 = pEntity->GetLowChest(), screen3;

	Vector cross4 = pEntity->GetRightHand(), screen4;

	Vector crossfourandahalf = pEntity->GetLeftHand(), screenfoundandahalf;

	Vector cross5 = pEntity->GetUpRightLeg(), screen5;

	Vector cross6 = pEntity->GetUpLeftLeg(), screen6;

	Vector cross7 = pEntity->GetLowRightLeg(), screen7;

	Vector cross8 = pEntity->GetLowLeftLeg(), screen8;


	static int Scale = 2;
	if (Render::WorldToScreen(cross, screen1))
	{
		Render::Clear(screen1.x - Scale, screen1.y - (Scale * 2), (Scale * 2), (Scale * 4), Color(20, 20, 20, 160));
		Render::Clear(screen1.x - (Scale * 2), screen1.y - Scale, (Scale * 4), (Scale * 2), Color(20, 20, 20, 160));
		Render::Clear(screen1.x - Scale - 1, screen1.y - (Scale * 2) - 1, (Scale * 2) - 2, (Scale * 4) - 2, Color(10, 90, 250, 230));
		Render::Clear(screen1.x - (Scale * 2) - 1, screen1.y - Scale - 1, (Scale * 4) - 2, (Scale * 2) - 2, Color(10, 90, 250, 230));
	}

	if (Render::WorldToScreen(cross2, screen2))
	{
		Render::Clear(screen2.x - Scale, screen2.y - (Scale * 2), (Scale * 2), (Scale * 4), Color(20, 20, 20, 160));
		Render::Clear(screen2.x - (Scale * 2), screen2.y - Scale, (Scale * 4), (Scale * 2), Color(20, 20, 20, 160));
		Render::Clear(screen2.x - Scale - 1, screen2.y - (Scale * 2) - 1, (Scale * 2) - 2, (Scale * 4) - 2, Color(10, 90, 250, 230));
		Render::Clear(screen2.x - (Scale * 2) - 1, screen2.y - Scale - 1, (Scale * 4) - 2, (Scale * 2) - 2, Color(10, 90, 250, 230));
	}

	if (Render::WorldToScreen(cross3, screen3))
	{
		Render::Clear(screen3.x - Scale, screen3.y - (Scale * 2), (Scale * 2), (Scale * 4), Color(20, 20, 20, 160));
		Render::Clear(screen3.x - (Scale * 2), screen3.y - Scale, (Scale * 4), (Scale * 2), Color(20, 20, 20, 160));
		Render::Clear(screen3.x - Scale - 1, screen3.y - (Scale * 2) - 1, (Scale * 2) - 2, (Scale * 4) - 2, Color(10, 240, 110, 230));
		Render::Clear(screen3.x - (Scale * 2) - 1, screen3.y - Scale - 1, (Scale * 4) - 2, (Scale * 2) - 2, Color(10, 240, 110, 230));
	}

	if (Render::WorldToScreen(cross4, screen4))
	{
		Render::Clear(screen4.x - Scale, screen4.y - (Scale * 2), (Scale * 2), (Scale * 4), Color(20, 20, 20, 160));
		Render::Clear(screen4.x - (Scale * 2), screen4.y - Scale, (Scale * 4), (Scale * 2), Color(20, 20, 20, 160));
		Render::Clear(screen4.x - Scale - 1, screen4.y - (Scale * 2) - 1, (Scale * 2) - 2, (Scale * 4) - 2, Color(10, 90, 250, 230));
		Render::Clear(screen4.x - (Scale * 2) - 1, screen4.y - Scale - 1, (Scale * 4) - 2, (Scale * 2) - 2, Color(10, 90, 250, 230));
	}
	if (Render::WorldToScreen(crossfourandahalf, screenfoundandahalf))
	{
		Render::Clear(screenfoundandahalf.x - Scale, screenfoundandahalf.y - (Scale * 2), (Scale * 2), (Scale * 4), Color(20, 20, 20, 160));
		Render::Clear(screenfoundandahalf.x - (Scale * 2), screenfoundandahalf.y - Scale, (Scale * 4), (Scale * 2), Color(20, 20, 20, 160));
		Render::Clear(screenfoundandahalf.x - Scale - 1, screenfoundandahalf.y - (Scale * 2) - 1, (Scale * 2) - 2, (Scale * 4) - 2, Color(10, 90, 250, 230));
		Render::Clear(screenfoundandahalf.x - (Scale * 2) - 1, screenfoundandahalf.y - Scale - 1, (Scale * 4) - 2, (Scale * 2) - 2, Color(10, 90, 250, 230));
	}
	if (Render::WorldToScreen(cross5, screen5))
	{
		Render::Clear(screen5.x - Scale, screen5.y - (Scale * 2), (Scale * 2), (Scale * 4), Color(20, 20, 20, 160));
		Render::Clear(screen5.x - (Scale * 2), screen5.y - Scale, (Scale * 4), (Scale * 2), Color(20, 20, 20, 160));
		Render::Clear(screen5.x - Scale - 1, screen5.y - (Scale * 2) - 1, (Scale * 2) - 2, (Scale * 4) - 2, Color(10, 240, 110, 230));
		Render::Clear(screen5.x - (Scale * 2) - 1, screen5.y - Scale - 1, (Scale * 4) - 2, (Scale * 2) - 2, Color(10, 240, 110, 230));
	}

	if (Render::WorldToScreen(cross6, screen6))
	{
		Render::Clear(screen6.x - Scale, screen6.y - (Scale * 2), (Scale * 2), (Scale * 4), Color(20, 20, 20, 160));
		Render::Clear(screen6.x - (Scale * 2), screen6.y - Scale, (Scale * 4), (Scale * 2), Color(20, 20, 20, 160));
		Render::Clear(screen6.x - Scale - 1, screen6.y - (Scale * 2) - 1, (Scale * 2) - 2, (Scale * 4) - 2, Color(10, 240, 110, 230));
		Render::Clear(screen6.x - (Scale * 2) - 1, screen6.y - Scale - 1, (Scale * 4) - 2, (Scale * 2) - 2, Color(10, 240, 110, 230));
	}

	if (Render::WorldToScreen(cross7, screen7))
	{
		Render::Clear(screen7.x - Scale, screen7.y - (Scale * 2), (Scale * 2), (Scale * 4), Color(20, 20, 20, 160));
		Render::Clear(screen7.x - (Scale * 2), screen7.y - Scale, (Scale * 4), (Scale * 2), Color(20, 20, 20, 160));
		Render::Clear(screen7.x - Scale - 1, screen7.y - (Scale * 2) - 1, (Scale * 2) - 2, (Scale * 4) - 2, Color(10, 90, 250, 230));
		Render::Clear(screen7.x - (Scale * 2) - 1, screen7.y - Scale - 1, (Scale * 4) - 2, (Scale * 2) - 2, Color(10, 90, 250, 230));
	}

	if (Render::WorldToScreen(cross8, screen8))
	{
		Render::Clear(screen8.x - Scale, screen8.y - (Scale * 2), (Scale * 2), (Scale * 4), Color(20, 20, 20, 160));
		Render::Clear(screen8.x - (Scale * 2), screen8.y - Scale, (Scale * 4), (Scale * 2), Color(20, 20, 20, 160));
		Render::Clear(screen8.x - Scale - 1, screen8.y - (Scale * 2) - 1, (Scale * 2) - 2, (Scale * 4) - 2, Color(10, 90, 250, 230));
		Render::Clear(screen8.x - (Scale * 2) - 1, screen8.y - Scale - 1, (Scale * 4) - 2, (Scale * 2) - 2, Color(10, 90, 250, 230));
	}

}

void CEsp::DrawDrop(IClientEntity* pEntity, ClientClass* cClass)
{

	Vector Box;
	IClientEntity* Weapon = (IClientEntity*)pEntity;
	IClientEntity* plr = Interfaces::EntList->GetClientEntityFromHandle((HANDLE)Weapon->GetOwnerHandle());
	if (!plr && Render::WorldToScreen(Weapon->GetOrigin(), Box))
	{
		if (Menu::Window.VisualsTab.FiltersWeapons.GetState())
		{

			std::string ItemName = CleanItemName(cClass->m_pNetworkName);
			RECT TextSize = Render::GetTextSize(Render::Fonts::otheresp, ItemName.c_str());
			Render::Text(Box.x - (TextSize.right / 2), Box.y - 16, Color(255, 255, 255, 255), Render::Fonts::otheresp, ItemName.c_str());
		}
	}

}

void CEsp::DrawBombPlanted(IClientEntity* pEntity, ClientClass* cClass)
{
	BombCarrier = nullptr;

	Vector vOrig; Vector vScreen;
	vOrig = pEntity->GetOrigin();
	CCSBomb* Bomb = (CCSBomb*)pEntity;

	float flBlow = Bomb->GetC4BlowTime();
	float TimeRemaining = flBlow - (Interfaces::Globals->interval_per_tick * hackManager.pLocal()->GetTickBase());
	char buffer[64];
	sprintf_s(buffer, "%.1fs", TimeRemaining);
	float TimeRemaining2;
	bool exploded = true;
	if (TimeRemaining < 0)
	{
		!exploded;

		TimeRemaining2 = 0;
	}
	else
	{
		exploded = true;
		TimeRemaining2 = TimeRemaining;
	}
	if (exploded)
	{
		sprintf_s(buffer, "Bomb: [ %.1f ]", TimeRemaining2);
		Render::Text(vScreen.x, vScreen.y, Color(255, 150, 30, 255), Render::Fonts::ESP, buffer);
	}
	else
	{
		float red = 180 + rand() % 75;
		float green = 55 + rand() % 75;
		float blue = 10 + rand() % 30;
		sprintf_s(buffer, "Bomb: [ Run! ]", TimeRemaining2);
		Render::Text(vScreen.x, vScreen.y, Color(red, green, blue, 255), Render::Fonts::ESP, buffer);
	}

}

void CEsp::DrawBomb(IClientEntity* pEntity, ClientClass* cClass)
{

	BombCarrier = nullptr;
	CBaseCombatWeapon *BombWeapon = (CBaseCombatWeapon *)pEntity;
	Vector vOrig; Vector vScreen;
	vOrig = pEntity->GetOrigin();
	bool adopted = true;
	HANDLE parent = BombWeapon->GetOwnerHandle();
	if (parent || (vOrig.x == 0 && vOrig.y == 0 && vOrig.z == 0))
	{
		IClientEntity* pParentEnt = (Interfaces::EntList->GetClientEntityFromHandle(parent));
		if (pParentEnt && pParentEnt->IsAlive())
		{

			BombCarrier = pParentEnt;
			adopted = false;
		}
	}

	if (adopted)
	{
		if (Render::WorldToScreen(vOrig, vScreen))
		{
			Render::Text(vScreen.x, vScreen.y, Color(112, 230, 20, 255), Render::Fonts::ESP, "[ C4 ]");
		}
	}
}

void DrawBoneArray(int* boneNumbers, int amount, IClientEntity* pEntity, Color color)
{

	Vector LastBoneScreen;
	for (int i = 0; i < amount; i++)
	{
		Vector Bone = pEntity->GetBonePos(boneNumbers[i]);
		Vector BoneScreen;

		if (Render::WorldToScreen(Bone, BoneScreen))
		{
			if (i>0)
			{
				Render::Line(LastBoneScreen.x, LastBoneScreen.y, BoneScreen.x, BoneScreen.y, color);
			}
		}
		LastBoneScreen = BoneScreen;
	}
}

void CEsp::tracer(IClientEntity* pEntity, Color color) // mark
{
	Vector src3D, dst3D, forward, src, dst;
	trace_t tr;
	Ray_t ray;
	CTraceFilter filter;

	AngleVectors(pEntity->GetEyeAngles(), &forward);
	filter.pSkip = pEntity;
	src3D = pEntity->GetBonePos(6) - Vector(0, 0, 0);
	dst3D = src3D + (forward * 150);

	ray.Init(src3D, dst3D);

	Interfaces::Trace->TraceRay(ray, MASK_SHOT, &filter, &tr);

	if (!Render::WorldToScreen(src3D, src) || !Render::WorldToScreen(tr.endpos, dst))
		return;

	Render::DrawLine(src.x, src.y, dst.x, dst.y, color);
	Render::DrawRect(dst.x - 3, dst.y - 3, 6, 6, color);
};



void DrawBoneTest(IClientEntity *pEntity)
{
	for (int i = 0; i < 127; i++)
	{
		Vector BoneLoc = pEntity->GetBonePos(i);
		Vector BoneScreen;
		if (Render::WorldToScreen(BoneLoc, BoneScreen))
		{
			char buf[10];
			_itoa_s(i, buf, 10);
			Render::Text(BoneScreen.x, BoneScreen.y, Color(255, 255, 255, 180), Render::Fonts::ESP, buf);
		}
	}
}

void CEsp::DrawSkeleton(IClientEntity* pEntity)
{

	studiohdr_t* pStudioHdr = Interfaces::ModelInfo->GetStudiomodel(pEntity->GetModel());
	float health = pEntity->GetHealth();

	if (health > 100)
		health = 100;



	int junkcode = rand() % 100;

	if (!pStudioHdr)
		return;

	Vector vParent, vChild, sParent, sChild;

	for (int j = 0; j < pStudioHdr->numbones; j++)
	{
		mstudiobone_t* pBone = pStudioHdr->GetBone(j);

		if (pBone && (pBone->flags & BONE_USED_BY_HITBOX) && (pBone->parent != -1))
		{
			vChild = pEntity->GetBonePos(j);
			vParent = pEntity->GetBonePos(pBone->parent);
			
			if (Render::WorldToScreen(vParent, sParent) && Render::WorldToScreen(vChild, sChild))
			{
					Render::Line(sParent[0], sParent[1], sChild[0], sChild[1], Color(250, 250, 250, 255));
			}
			
		}
	}
}

void CEsp::BoxAndText(IClientEntity* entity, std::string text)
{

	ESPBox Box;
	std::vector<std::string> Info;
	if (GetBox(entity, Box))
	{
		Info.push_back(text);
		if (Menu::Window.VisualsTab.FiltersNades.GetState())
		{
			int i = 0;
			for (auto kek : Info)
			{
				Render::Text(Box.x + 1, Box.y + 1, Color(255, 255, 255, 255), Render::Fonts::ESP, kek.c_str());
				i++;
			}
		}

	}
}

void CEsp::DrawThrowable(IClientEntity* throwable)
{
	model_t* nadeModel = (model_t*)throwable->GetModel();

	if (!nadeModel)
		return;

	studiohdr_t* hdr = Interfaces::ModelInfo->GetStudiomodel(nadeModel);

	if (!hdr)
		return;

	if (!strstr(hdr->name, "thrown") && !strstr(hdr->name, "dropped"))
		return;

	std::string nadeName = "Unknown Grenade";

	IMaterial* mats[32];
	Interfaces::ModelInfo->GetModelMaterials(nadeModel, hdr->numtextures, mats);

	for (int i = 0; i < hdr->numtextures; i++)
	{
		IMaterial* mat = mats[i];
		if (!mat)
			continue;

		if (strstr(mat->GetName(), "flashbang"))
		{
			nadeName = "Flashbang";
			break;
		}
		else if (strstr(mat->GetName(), "m67_grenade") || strstr(mat->GetName(), "hegrenade"))
		{
			nadeName = "HE Grenade";
			break;
		}
		else if (strstr(mat->GetName(), "smoke"))
		{
			nadeName = "Smoke";
			break;
		}
		else if (strstr(mat->GetName(), "decoy"))
		{
			nadeName = "Decoy";
			break;
		}
		else if (strstr(mat->GetName(), "incendiary") || strstr(mat->GetName(), "molotov"))
		{
			nadeName = "Molotov";
			break;
		}
	}
	BoxAndText(throwable, nadeName);
}