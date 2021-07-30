/*
    Plugin-SDK (Grand Theft Auto San Andreas) source file
    Authors: GTA Community. See more here
    https://github.com/DK22Pac/plugin-sdk
    Do not delete this comment block. Respect others' work!
*/
#include "StdInc.h"

bool& CHeli::bPoliceHelisAllowed = *(bool*)0x8D338C;
unsigned int& CHeli::TestForNewRandomHelisTimer = *(unsigned int*)0xC1C960;
CHeli** CHeli::pHelis = (CHeli**)0xC1C964;
unsigned int& CHeli::NumberOfSearchLights = *(unsigned int*)0xC1C96C;
bool& CHeli::bHeliControlsCheat = *(bool*)0xC1C970;
tHeliLight* CHeli::HeliSearchLights = (tHeliLight*)0xC1C990;

void CHeli::InjectHooks() {
    ReversibleHooks::Install("CHeli", "AddHeliSearchLight", 0x6C45B0, &CHeli::AddHeliSearchLight);
    ReversibleHooks::Install("CHeli", "BlowUpCar", 0x6C6D30, &CHeli::BlowUpCar_Reversed);
    ReversibleHooks::Install("CHeli", "Fix", 0x6C4530, &CHeli::Fix_Reversed);
    //ReversibleHooks::Install("CHeli", "ProcessControl", 0x6B1880, &CHeli::ProcessControl_Reversed);
    ReversibleHooks::Install("CHeli", "SwitchPoliceHelis", 0x6C4800, &CHeli::SwitchPoliceHelis);
}

// Converted from thiscall void CHeli::CHeli(int modelIndex,uchar createdBy) 0x6C4190
CHeli::CHeli(int modelIndex, unsigned char createdBy) : CAutomobile(plugin::dummy) {
    plugin::CallMethod<0x6C4190, CHeli*, int, unsigned char>(this, modelIndex, createdBy);
}

// Converted from cdecl void CHeli::InitHelis(void) 0x6C4560
void CHeli::InitHelis() {
    ((void(__cdecl*)())0x6C4560)();
}

// Converted from cdecl void CHeli::AddHeliSearchLight(CVector const& origin,CVector const& target,float targetRadius,float power,uint coronaIndex,uchar unknownFlag,uchar drawShadow) 0x6C45B0
void CHeli::AddHeliSearchLight(CVector const& origin, CVector const& target, float targetRadius, float power, unsigned int coronaIndex, bool unknownFlag, bool drawShadow) {
    tHeliLight& light = HeliSearchLights[NumberOfSearchLights];

    light.m_vecOrigin = origin;
    light.m_vecTarget = target;
    light.m_fTargetRadius = targetRadius;
    light.m_fPower = power;
    light.m_nCoronaIndex = coronaIndex;
    light.field_24 = unknownFlag;
    light.m_bDrawShadow = drawShadow;

    NumberOfSearchLights++;
}

void CHeli::BlowUpCar_Reversed(CEntity* damager, unsigned char bHideExplosion) {
    if (!vehicleFlags.bCanBeDamaged)
        return;

    if ((m_nType != ENTITY_TYPE_NOTHING) &&
        (m_autoPilot.m_nCarMission != MISSION_CRASH_HELI_AND_BURN) &&
        (m_nModelIndex != MODEL_RCRAIDER) &&
        (m_nModelIndex != MODEL_RCGOBLIN)
    ) {
        m_autoPilot.m_nCarMission = MISSION_CRASH_HELI_AND_BURN;
        m_fHealth = 0;
        return;
    }

    CPlayerPed* player = FindPlayerPed();
    if (player == damager || FindPlayerVehicle(-1, 0) == damager) {
        CPlayerInfo* playerInfo = player->GetPlayerInfoForThisPlayerPed();
        playerInfo->m_nHavocCaused += 20;
        playerInfo->m_fCurrentChaseValue += 10.0;

        CStats::IncrementStat(STAT_COST_OF_PROPERTY_DAMAGED, CGeneral::GetRandomNumberInRange(4000, 10000));
    }

    if (m_nModelIndex == MODEL_VCNMAV) {
        CWanted::bUseNewsHeliInAdditionToPolice = false;
    }

    if (m_nType == ENTITY_TYPE_NOTHING) {
        // m_nFlags &= 0xffffff7e;
        m_bUsesCollision = false;
        m_bIsVisible = false;
        m_vecMoveSpeed = CVector();
        m_vecTurnSpeed = CVector();
    }
    // m_nType = m_nType & 7 | 0x28
    // I just guessed that flag as drank too much that day and today
    m_nStatus = STATUS_WRECKED;
    // m_nPhysicalFlags |= 0x20000000
    physicalFlags.bDestroyed = true;
    m_nTimeWhenBlowedUp = CTimer::m_snTimeInMilliseconds;

    CVisibilityPlugins::SetClumpForAllAtomicsFlag(m_pRwClump, ATOMIC_IS_BLOWN_UP);

    m_damageManager.FuckCarCompletely(false);

    if ((m_nModelIndex != MODEL_RCRAIDER) && (m_nModelIndex != MODEL_RCGOBLIN)) {
        SetBumperDamage(BUMP_FRONT, false);
        SetBumperDamage(BUMP_REAR, false);
        SetDoorDamage(DOOR_BONNET, false);
        SetDoorDamage(DOOR_BOOT, false);
        SetDoorDamage(DOOR_LEFT_FRONT, false);
        SetDoorDamage(DOOR_RIGHT_FRONT, false);
        SetDoorDamage(DOOR_LEFT_REAR, false);
        SetDoorDamage(DOOR_RIGHT_REAR, false);

        SpawnFlyingComponent(5, 1);

        RpAtomic* currentObj;
        RwFrameForAllObjects((m_aCarNodes[HELI_WHEEL_LF]), &GetCurrentAtomicObjectCB, &currentObj);

        if (currentObj) {
            rwObjectSetFlags(currentObj, 0);
        }
    }

    // m_nBombOnBoard &= 0xe7
    m_nOverrideLights = NO_CAR_LIGHT_OVERRIDE;

    m_fHealth = 0;
    m_wBombTimer = 0;

    const CVector& vecPos = GetPosition();
    TheCamera.CamShake(0.4F, vecPos.x, vecPos.y, vecPos.z);

    KillPedsInVehicle();

    // m_nVehicleUpperFlags &= 0xaf;
    // m_nVehicleLowerFlags &= 0x7f;
    vehicleFlags.bEngineOn = false;
    vehicleFlags.bLightsOn = false;
    vehicleFlags.bSirenOrAlarm = false;

    // ucNPCVehicleFlags &= 0xfe;
    npcFlags.bTaxiLightOn = false;

    if (vehicleFlags.bIsAmbulanceOnDuty) {
        vehicleFlags.bIsAmbulanceOnDuty = false;
        CCarCtrl::NumAmbulancesOnDuty--;
    }
    if (vehicleFlags.bIsFireTruckOnDuty) {
        vehicleFlags.bIsFireTruckOnDuty = false;
        CCarCtrl::NumFireTrucksOnDuty--;
    }

    ChangeLawEnforcerState(false);

    gFireManager.StartFire(this, damager, 0.8F, 1, 7000, 0);

    CDarkel::RegisterCarBlownUpByPlayer(this, 0);

    eExplosionType explType = (m_nModelIndex == MODEL_RCRAIDER) || (m_nModelIndex == MODEL_RCGOBLIN) ? EXPLOSION_RC_VEHICLE : EXPLOSION_AIRCRAFT;
    // CHeli::BlowUpCar doesnt use the var bHideExplosion and always shows the explosion.
    // Changed that here to respect bHideExplosion
    CExplosion::AddExplosion(this, damager, explType, vecPos, 0, true, -1.0F, bHideExplosion);
}

// 0x6C4530
void CHeli::Fix_Reversed() {
    m_damageManager.ResetDamageStatus();
    vehicleFlags.bIsDamaged = false;
}


// 0x6C4640
void CHeli::PreRenderAlways() {
    // NOP
}

// Converted from cdecl void CHeli::Pre_SearchLightCone(void) 0x6C4650
void CHeli::Pre_SearchLightCone() {
    ((void(__cdecl*)())0x6C4650)();
}

// Converted from cdecl void CHeli::Post_SearchLightCone(void) 0x6C46E0
void CHeli::Post_SearchLightCone() {
    ((void(__cdecl*)())0x6C46E0)();
}

// 0x6C4750
void CHeli::SpecialHeliPreRender() {
    // NOP
}

// Converted from thiscall CVector CHeli::FindSwatPositionRelativeToHeli(int swatNumber) 0x6C4760
CVector CHeli::FindSwatPositionRelativeToHeli(int swatNumber) {
    CVector result;
    ((void(__thiscall*)(CHeli*, CVector*, int))0x6C4760)(this, &result, swatNumber);
    return result;
}

// 0x6C4800
void CHeli::SwitchPoliceHelis(bool enable) {
    bPoliceHelisAllowed = enable;
}

// Converted from cdecl void CHeli::SearchLightCone(int coronaIndex,CVector origin,CVector target,float targetRadius,float power,uchar unknownFlag,uchar drawShadow,CVector*
// ,CVector* ,CVector* ,bool,float baseRadius) 0x6C58E0
void CHeli::SearchLightCone(int coronaIndex, CVector origin, CVector target, float targetRadius, float power, unsigned char unknownFlag, unsigned char drawShadow, CVector* arg7, CVector* arg8, CVector* arg9, bool arg10, float baseRadius) {
    ((void(__cdecl*)(int, CVector, CVector, float, float, unsigned char, unsigned char, CVector*, CVector*, CVector*, bool, float))0x6C58E0)(coronaIndex, origin, target, targetRadius, power, unknownFlag, drawShadow, arg7, arg8, arg9, arg10, baseRadius);
}

// Converted from cdecl CHeli* CHeli::GenerateHeli(CPed *target,bool newsHeli) 0x6C6520
CHeli* CHeli::GenerateHeli(CPed* target, bool newsHeli) {
    return ((CHeli * (__cdecl*)(CPed*, bool))0x6C6520)(target, newsHeli);
}

// Converted from cdecl bool CHeli::TestSniperCollision(CVector *origin,CVector *target) 0x6C6890
bool CHeli::TestSniperCollision(CVector* origin, CVector* target) {
    return ((bool(__cdecl*)(CVector*, CVector*))0x6C6890)(origin, target);
}

// Converted from thiscall bool CHeli::SendDownSwat(void) 0x6C69C0
bool CHeli::SendDownSwat() {
    return ((bool(__thiscall*)(CHeli*))0x6C69C0)(this);
}

// Converted from cdecl void CHeli::UpdateHelis(void) 0x6C79A0
void CHeli::UpdateHelis() {
    ((void(__cdecl*)())0x6C79A0)();
}

// Converted from cdecl void CHeli::RenderAllHeliSearchLights(void) 0x6C7C50
void CHeli::RenderAllHeliSearchLights() {
    ((void(__cdecl*)())0x6C7C50)();
}



void CHeli::ProcessControl_Reversed() {
    /*CAutomobile::ProcessControl();
    if (!(m_nVehicleLowerFlags & 0x10U) && m_pDustParticle) {
        m_pDustParticle->Kill();
        m_pDustParticle = nullptr;
        field_984 = 0.0;
    }*/
}

// 0x6C6D30
void CHeli::BlowUpCar(CEntity* damager, unsigned char bHideExplosion) {
    return BlowUpCar_Reversed(damager, bHideExplosion);
}

// 0x6C4530
void CHeli::Fix() {
    Fix_Reversed();
}

// 0x6B1880
void CHeli::ProcessControl() {
    ProcessControl_Reversed();
}
