#include "cbase.h"
#include "asw_weapon_heavy_rifle_shared.h"
#include "in_buttons.h"

#ifdef CLIENT_DLL
#include "c_asw_player.h"
#include "c_asw_weapon.h"
#include "c_asw_marine.h"
#define CASW_Marine C_ASW_Marine
#else
#include "asw_marine.h"
#include "asw_player.h"
#include "asw_weapon.h"
#include "npcevent.h"
#include "shot_manipulator.h"
#include "asw_fail_advice.h"
#include "effect_dispatch_data.h"
#endif
#include "asw_marine_skills.h"
#include "asw_weapon_parse.h"
#include "particle_parse.h"
#include "asw_deathmatch_mode_light.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_NETWORKCLASS_ALIASED( ASW_Weapon_Heavy_Rifle, DT_ASW_Weapon_Heavy_Rifle )

BEGIN_NETWORK_TABLE( CASW_Weapon_Heavy_Rifle, DT_ASW_Weapon_Heavy_Rifle )
#ifdef CLIENT_DLL
	// recvprops
#else
	// sendprops
#endif
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CASW_Weapon_Heavy_Rifle )
	
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( asw_weapon_heavy_rifle, CASW_Weapon_Heavy_Rifle );
PRECACHE_WEAPON_REGISTER(asw_weapon_heavy_rifle);

#ifndef CLIENT_DLL
extern ConVar asw_debug_marine_damage;
//---------------------------------------------------------
// Save/Restore
//---------------------------------------------------------
BEGIN_DATADESC( CASW_Weapon_Heavy_Rifle )
	
END_DATADESC()

#endif /* not client */

CASW_Weapon_Heavy_Rifle::CASW_Weapon_Heavy_Rifle()
{

}


CASW_Weapon_Heavy_Rifle::~CASW_Weapon_Heavy_Rifle()
{

}

void CASW_Weapon_Heavy_Rifle::Precache()
{
	BaseClass::Precache();
}


float CASW_Weapon_Heavy_Rifle::GetWeaponDamage()
{
	float flDamage = GetWeaponInfo()->m_flBaseDamage;

	if ( ASWDeathmatchMode() )
	{
		extern ConVar rd_pvp_heavy_rifle_dmg;
		flDamage = rd_pvp_heavy_rifle_dmg.GetFloat();
	}

	if ( GetMarine() )
	{
		flDamage += MarineSkills()->GetSkillBasedValueByMarine(GetMarine(), ASW_MARINE_SKILL_ACCURACY, ASW_MARINE_SUBSKILL_ACCURACY_HEAVY_RIFLE_DMG);
	}

	return flDamage;
}

// just dry fire by default
void CASW_Weapon_Heavy_Rifle::SecondaryAttack()
{
	// Only the player fires this way so we can cast
	CASW_Player *pPlayer = GetCommander();
	if ( !pPlayer )
		return;

	CASW_Marine *pMarine = GetMarine();
	if ( !pMarine )
		return;

	SendWeaponAnim( ACT_VM_DRYFIRE );
	BaseClass::WeaponSound( EMPTY );
	m_flNextSecondaryAttack = gpGlobals->curtime + 0.5f;
}
