#define _CRT_SECURE_NO_WARNINGS

#include "Hacks.h"
#include "Interfaces.h"
#include "RenderManager.h"


#include "ESP.h"
#include "Visuals.h"
#include "RageBot.h"
#include "MiscHacks.h"
#include "LegitBot.h"

CEsp Esp;
CVisuals Visuals;
CMiscHacks MiscHacks;
CRageBot RageBot;
CLegitBot LegitBot;




void Hacks::SetupHacks()
{
	Esp.Init();
	Visuals.Init();
	MiscHacks.Init();
	RageBot.Init();
	LegitBot.Init();

	hackManager.RegisterHack(&Esp);
	hackManager.RegisterHack(&Visuals);
	hackManager.RegisterHack(&MiscHacks);
	hackManager.RegisterHack(&RageBot);
	hackManager.RegisterHack(&LegitBot);

	hackManager.Ready();
}


IClientEntity* HeckManager::pLocal()
{

	return pLocalInstance;

}

void HeckManager::RegisterHeck(CHecke * hake)
{
	Hecks.push_back(hake);
	hake->Init();
}

void HeckManager::Ready()
{

	IsReady = true;
}
 
void Hecks::SetupHecks()
{
	Esp.Init();
	Visuals.Init();


	hackManager.RegisterHack(&Esp);
	hackManager.RegisterHack(&Visuals);


	hackManager.Ready();
}

void SpecList()
{
	IClientEntity *pLocal = hackManager.pLocal();

	RECT scrn = Render::GetViewport();
	int ayy = 0;

	// Loop through all active entitys
	for (int i = 0; i < Interfaces::EntList->GetHighestEntityIndex(); i++)
	{

		// Get the entity
		IClientEntity *pEntity = Interfaces::EntList->GetClientEntity(i);
		player_info_t pinfo;

		// The entity isn't some laggy peice of shit or something
		if (pEntity &&  pEntity != pLocal)
		{
			if (Interfaces::Engine->GetPlayerInfo(i, &pinfo) && !pEntity->IsAlive() && !pEntity->IsDormant())
			{
				HANDLE obs = pEntity->GetObserverTargetHandle();

				if (obs)
				{
					IClientEntity *pTarget = Interfaces::EntList->GetClientEntityFromHandle(obs);
					player_info_t pinfo2;
					if (pTarget)
					{

						if (Interfaces::Engine->GetPlayerInfo(pTarget->GetIndex(), &pinfo2))
						{
							if (strlen(pinfo.name) > 16)
							{

								pinfo.name[12] = 0;
								strcat(pinfo.name, "...");
								RECT TextSize = Render::GetTextSize(Render::Fonts::ESP, pinfo.name);
								RECT nameSize = Render::GetTextSize(Render::Fonts::ESP, pinfo.name);
								Render::Text(scrn.right - TextSize.right - 4, (scrn.top / 4) + (16 * ayy), pTarget->GetIndex() == pLocal->GetIndex() ? Color(255, 0, 0, 255) : Color(255, 255, 255, 255), Render::Fonts::ESP, pinfo.name);
								ayy++;
							}
						}	

					}
				}

			}
		}
	}

}

void Hacks::DrawHacks()
{
	if (!Menu::Window.VisualsTab.Active.GetState())
		return;


	if (Menu::Window.VisualsTab.SpecList.GetState())
		SpecList();

	if (Menu::Window.VisualsTab.OptionsCompRank.GetState() && GUI.GetKeyState(VK_TAB))
		GameUtils::ServerRankRevealAll();

	hackManager.Draw();


}
void Hecks::DrawHecks()
{
	if (!Menu::Window.VisualsTab.Active.GetState())
		return;



	HeckManager extra();


}

void Hacks::MoveHacks(CUserCmd *pCmd,bool &bSendPacket)
{

	Vector origView = pCmd->viewangles;
	IClientEntity *pLocal = hackManager.pLocal();
	hackManager.Move(pCmd, bSendPacket);

	int AirStuckKey = Menu::Window.MiscTab.OtherAirStuck.GetKey();
	if (AirStuckKey > 0 && GUI.GetKeyState(AirStuckKey))
	{
		if (!(pCmd->buttons & IN_ATTACK))
		{
			pCmd->tick_count = INT_MAX;
		}

	}

}

HackManager hackManager;

void HackManager::RegisterHack(CHack* hake)
{

	Hacks.push_back(hake);
	hake->Init();

}

void HeckManager::Register(CHecke* hake)
{

	Hecks.push_back(hake);
	hake->Init();

}

void HackManager::Draw()
{
	if (!IsReady)
		return;

	static float time;
	int AimbotManualAA = Menu::Window.RageBotTab.ManualAA.GetKey();
	if (AimbotManualAA >= 0 && GUI.GetKeyState(AimbotManualAA) && abs(time - Interfaces::Globals->curtime) > 0.5)
	{
		time = Interfaces::Globals->curtime;
		toggleSideSwitch = !toggleSideSwitch;
	}


	pLocalInstance = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
	if (!pLocalInstance) return;

	for (auto &hack : Hacks)
	{
		hack->Draw();
	}
}



void HeckManager::extra()
{
	if (!IsReady)
		return;



	pLocalInstance = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
	if (!pLocalInstance) return;

	for (auto &heck : Hecks)
	{
		heck->extra();
	}
}

void HackManager::Move(CUserCmd *pCmd,bool &bSendPacket)
{

	if (!IsReady)
		return;

	pLocalInstance = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
	if (!pLocalInstance) return;

	for (auto &hack : Hacks)
	{
		hack->Move(pCmd,bSendPacket); 
	}
}

IClientEntity* HackManager::pLocal()
{

	return pLocalInstance;

}

void HackManager::Ready()
{

	IsReady = true;
}