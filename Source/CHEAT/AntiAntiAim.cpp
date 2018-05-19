#include "Interfaces.h"
#include "Menu.h"
#include "Proxies.h"
#include "Skins.h"

void FixY(const CRecvProxyData *pData, void *pStruct, void *pOut)
{
	static Vector vLast[65];
	static bool bShotLastTime[65];
	static bool bJitterFix[65];

	float *flPitch = (float*)((DWORD)pOut - 4);
	float flYaw = pData->m_Value.m_Float;



	static float OldLowerBodyYaws[65];
	static float OldYawDeltas[65];
	static float oldTimer[65];
	static bool isLBYPredictited[65];
	IClientEntity* player = (IClientEntity*)pStruct;
	IClientEntity* pLocal = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
	INetChannelInfo *nci = Interfaces::Engine->GetNetChannelInfo();
	if (!player || !pLocal || pLocal == player || player->GetTeamNum() == pLocal->GetTeamNum()) {
		return;
	}

		 if (Menu::Window.RageBotTab.Resolverhelper.GetState()) {

			float CurYaw = player->GetLowerBodyYaw();
			if (OldLowerBodyYaws[player->GetIndex()] != CurYaw) {
				OldYawDeltas[player->GetIndex()] = player->GetEyeAngles().y - CurYaw;
				OldLowerBodyYaws[player->GetIndex()] = CurYaw;
				//*(float*)((uintptr_t)player + offsets.m_angEyeAngles + 4) = CurYaw;
				enemyLBYDelta[player->GetIndex()] = CurYaw;
				*(float*)(pOut) = CurYaw;

				return;
			}
			else {
				//*(float*)((uintptr_t)player + offsets.m_angEyeAngles + 4) = player->GetEyeAngles().y - OldYawDeltas[i];
				enemyLBYDelta[player->GetIndex()] = player->GetEyeAngles().y - OldYawDeltas[player->GetIndex()];
				*(float*)(pOut) = player->GetEyeAngles().y - OldYawDeltas[player->GetIndex()];

			}

			float yClamped = player->GetEyeAngles().y;
			while (yClamped < -180.0f)
				yClamped += 360.0f;

			while (yClamped > 180.0f)
				yClamped -= 360.0f;
			//*(float*)((uintptr_t)player + offsets.m_angEyeAngles + 4) = yClamped;
			enemyLBYDelta[player->GetIndex()] = yClamped;
			*(float*)(pOut) = yClamped;
		}
		else if (missedLogHits[player->GetIndex()] == 5) {
			flYaw = flYaw + 180;
			*(float*)(pOut) = flYaw;
		}
		else if (missedLogHits[player->GetIndex()] == 6) {
			flYaw = flYaw + 90;
			*(float*)(pOut) = flYaw;
		}
		else if (missedLogHits[player->GetIndex()] == 7 && missedLogHits[player->GetIndex()] < 8) {
			flYaw = flYaw - 90;
			*(float*)(pOut) = flYaw;
		}
		else {
			*(float*)(pOut) = flYaw;
		}
}

RecvVarProxyFn oRecvnModelIndex;

void Hooked_RecvProxy_Viewmodel(CRecvProxyData *pData, void *pStruct, void *pOut)
{

	int default_t = Interfaces::ModelInfo->GetModelIndex("models/weapons/v_knife_default_t.mdl");
	int default_ct = Interfaces::ModelInfo->GetModelIndex("models/weapons/v_knife_default_ct.mdl");
	int iBayonet = Interfaces::ModelInfo->GetModelIndex("models/weapons/v_knife_bayonet.mdl");
	int iButterfly = Interfaces::ModelInfo->GetModelIndex("models/weapons/v_knife_butterfly.mdl");
	int iFlip = Interfaces::ModelInfo->GetModelIndex("models/weapons/v_knife_flip.mdl");
	int iGut = Interfaces::ModelInfo->GetModelIndex("models/weapons/v_knife_gut.mdl");
	int iKarambit = Interfaces::ModelInfo->GetModelIndex("models/weapons/v_knife_karam.mdl");
	int iM9Bayonet = Interfaces::ModelInfo->GetModelIndex("models/weapons/v_knife_m9_bay.mdl");
	int iHuntsman = Interfaces::ModelInfo->GetModelIndex("models/weapons/v_knife_tactical.mdl");
	int iFalchion = Interfaces::ModelInfo->GetModelIndex("models/weapons/v_knife_falchion_advanced.mdl");
	int iDagger = Interfaces::ModelInfo->GetModelIndex("models/weapons/v_knife_push.mdl");
	int iBowie = Interfaces::ModelInfo->GetModelIndex("models/weapons/v_knife_survival_bowie.mdl");

	int iGunGame = Interfaces::ModelInfo->GetModelIndex("models/weapons/v_knife_gg.mdl");

	IClientEntity* pLocal = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
	if (Menu::Window.MiscTab.SkinEnable.GetState() && pLocal)
	{

		if (pLocal->IsAlive() && (
			pData->m_Value.m_Int == default_t ||
			pData->m_Value.m_Int == default_ct ||
			pData->m_Value.m_Int == iBayonet ||
			pData->m_Value.m_Int == iButterfly ||
			pData->m_Value.m_Int == iFlip ||
			pData->m_Value.m_Int == iGunGame ||
			pData->m_Value.m_Int == iGut ||
			pData->m_Value.m_Int == iKarambit ||
			pData->m_Value.m_Int == iM9Bayonet ||
			pData->m_Value.m_Int == iHuntsman ||
			pData->m_Value.m_Int == iFalchion ||
			pData->m_Value.m_Int == iDagger ||
			pData->m_Value.m_Int == iBowie))
		{

			if (Menu::Window.MiscTab.KnifeModel.GetIndex() == 0)
				pData->m_Value.m_Int = iBayonet;
			else if (Menu::Window.MiscTab.KnifeModel.GetIndex() == 1)
				pData->m_Value.m_Int = iBowie;
			else if (Menu::Window.MiscTab.KnifeModel.GetIndex() == 2)
				pData->m_Value.m_Int = iButterfly;
			else if (Menu::Window.MiscTab.KnifeModel.GetIndex() == 3)
				pData->m_Value.m_Int = iFalchion;
			else if (Menu::Window.MiscTab.KnifeModel.GetIndex() == 4)
				pData->m_Value.m_Int = iFlip;
			else if (Menu::Window.MiscTab.KnifeModel.GetIndex() == 5)
				pData->m_Value.m_Int = iGut;
			else if (Menu::Window.MiscTab.KnifeModel.GetIndex() == 6)
				pData->m_Value.m_Int = iHuntsman;
			else if (Menu::Window.MiscTab.KnifeModel.GetIndex() == 7)
				pData->m_Value.m_Int = iKarambit;
			else if (Menu::Window.MiscTab.KnifeModel.GetIndex() == 8)
				pData->m_Value.m_Int = iM9Bayonet;
			else if (Menu::Window.MiscTab.KnifeModel.GetIndex() == 9)
				pData->m_Value.m_Int = iDagger;
		}
	}
	oRecvnModelIndex(pData, pStruct, pOut);
}

void SkinChanger()
{
	ClientClass *pClass = Interfaces::Client->GetAllClasses();
	while (pClass)
	{
		const char *pszName = pClass->m_pRecvTable->m_pNetTableName;
		if (!strcmp(pszName, "DT_CSPlayer"))
		{
			for (int i = 0; i < pClass->m_pRecvTable->m_nProps; i++)
			{
				RecvProp *pProp = &(pClass->m_pRecvTable->m_pProps[i]);
				const char *name = pProp->m_pVarName;

				if (!strcmp(name, "m_angEyeAngles[0]"))
				{
				}

				if (!strcmp(name, "m_angEyeAngles[1]"))
				{
				}
			}
		}
		else if (!strcmp(pszName, "DT_BaseViewModel"))
		{
			for (int i = 0; i < pClass->m_pRecvTable->m_nProps; i++)
			{
				RecvProp *pProp = &(pClass->m_pRecvTable->m_pProps[i]);
				const char *name = pProp->m_pVarName;


				if (!strcmp(name, "m_nModelIndex"))
				{
					oRecvnModelIndex = (RecvVarProxyFn)pProp->m_ProxyFn;
					pProp->m_ProxyFn = Hooked_RecvProxy_Viewmodel;
				}
			}
			for (ClientClass* pClass = Interfaces::Client->GetAllClasses(); pClass; pClass = pClass->m_pNext)
			{
				if (!strcmp(pClass->m_pNetworkName, "CBaseViewModel"))
				{
					RecvTable* pClassTable = pClass->m_pRecvTable;

					for (int nIndex = 0; nIndex < pClassTable->m_nProps; nIndex++)
					{
						RecvProp* pProp = &pClassTable->m_pProps[nIndex];

						if (!pProp || strcmp(pProp->m_pVarName, "m_nSequence"))
							continue;

						fnSequenceProxyFn = (RecvVarProxyFn)pProp->m_ProxyFn;

						pProp->m_ProxyFn = SetViewModelSequence;

						break;
					}

					break;
				}
			}
		}
		pClass = pClass->m_pNext;
	}

}