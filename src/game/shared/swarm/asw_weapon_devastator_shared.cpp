#include "cbase.h"

#include "asw_weapon_devastator_shared.h"
#include "in_buttons.h"
#ifdef CLIENT_DLL
#include "c_asw_player.h"
#include "c_asw_marine.h"
#else
#include "asw_player.h"
#include "asw_marine.h"
#endif

#include "asw_marine_skills.h"
#include "asw_weapon_parse.h"
#include "asw_deathmatch_mode_light.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_NETWORKCLASS_ALIASED(ASW_Weapon_Devastator, DT_ASW_Weapon_Devastator)

BEGIN_NETWORK_TABLE(CASW_Weapon_Devastator, DT_ASW_Weapon_Devastator)
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA(CASW_Weapon_Devastator)
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS(asw_weapon_devastator, CASW_Weapon_Devastator);
PRECACHE_WEAPON_REGISTER(asw_weapon_devastator);

ConVar rd_devastator_fire_rate("rd_devastator_fire_rate", "0.2", FCVAR_CHEAT, "Fire rate for the devastator.");
ConVar rd_devastator_piercing("rd_devastator_piercing", "0", FCVAR_CHEAT, "Number of aliens for devastator to pierce. Set 0 to disable.");
ConVar rd_devastator_piercing_chance("rd_devastator_piercing_chance", "0.2", FCVAR_CHEAT, "Base pierce chance for each pellet.");
ConVar rd_devastator_pellet_num("rd_devastator_pellet_num", "7", FCVAR_CHEAT, "Number of pellets for the devastator.");

#ifndef CLIENT_DLL
BEGIN_DATADESC(CASW_Weapon_Devastator)
//DEFINE_FIELD( m_bCanShoot, FIELD_TIME ),
END_DATADESC()
#endif

CASW_Weapon_Devastator::CASW_Weapon_Devastator()
{
}

CASW_Weapon_Devastator::~CASW_Weapon_Devastator()
{

}

void CASW_Weapon_Devastator::Precache()
{
	PrecacheModel("swarm/sprites/whiteglow1.vmt");
	PrecacheModel("swarm/sprites/greylaser1.vmt");
	PrecacheScriptSound("ASW_Weapon.Empty");
	PrecacheScriptSound("ASW_Weapon.Reload3");
	PrecacheScriptSound("ASW_Weapon_Devastator.SingleFP");
	PrecacheScriptSound("ASW_Weapon_Devastator.Single");


	BaseClass::Precache();
}

int CASW_Weapon_Devastator::GetPierceNum( void )
{
	return rd_devastator_piercing.GetInt();
}

float CASW_Weapon_Devastator::GetFireRate()
{
	return rd_devastator_fire_rate.GetFloat();	
}

float CASW_Weapon_Devastator::GetPierceChance( void )
{
	return rd_devastator_piercing_chance.GetFloat();
}

int CASW_Weapon_Devastator::GetNumPellets()
{
	return rd_devastator_pellet_num.GetInt();
}

float CASW_Weapon_Devastator::GetWeaponDamage()
{
	//float flDamage = 18.0f;
	float flDamage = GetWeaponInfo()->m_flBaseDamage;

	if ( ASWDeathmatchMode() )
	{
		extern ConVar rd_pvp_devastator_dmg;
		flDamage = rd_pvp_devastator_dmg.GetFloat();
	}

	if (GetMarine())
	{
		flDamage += MarineSkills()->GetSkillBasedValueByMarine(GetMarine(), ASW_MARINE_SKILL_ACCURACY, ASW_MARINE_SUBSKILL_AUTOGUN_DMG);
	}

	return flDamage;
}


float CASW_Weapon_Devastator::GetMovementScale()
{
	return ShouldMarineMoveSlow() ? 0.3f : 0.7f;
}

bool CASW_Weapon_Devastator::ShouldMarineMoveSlow()
{
	bool bAttack1, bAttack2, bReload, bOldReload, bOldAttack1;
	GetButtons(bAttack1, bAttack2, bReload, bOldReload, bOldAttack1);

	return (BaseClass::ShouldMarineMoveSlow() || bAttack1 );
}
