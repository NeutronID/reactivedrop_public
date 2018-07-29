// A tougher version of the standard Swarm drone.  It's green, bigger and has more health.
#include "cbase.h"
#include "asw_drone_uber.h"
#include "asw_gamerules.h"
#include "asw_marine.h"
#include "asw_weapon_assault_shotgun_shared.h"
#include "asw_weapon_deagle_shared.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern ConVar asw_drone_melee_force;
extern ConVar asw_drone_melee_range;
ConVar asw_drone_uber_health("asw_drone_uber_health", "500", FCVAR_CHEAT, "How much health the uber Swarm drones have");
ConVar asw_uber_speed_scale("asw_uber_speed_scale", "0.5f", FCVAR_CHEAT, "Speed scale of uber drone compared to normal");
ConVar asw_uber_auto_speed_scale("asw_uber_auto_speed_scale", "0.3f", FCVAR_CHEAT, "Speed scale of uber drones when attacking");
ConVar rd_drone_uber_bones("rd_drone_uber_bones", "1", FCVAR_NONE, "Set bodygroups on ubers to the scariest appendage.");
ConVar rd_drone_uber_damage("rd_drone_uber_damage", "20", FCVAR_CHEAT, "Damage inflicted by uber drone attacks.");
extern ConVar asw_alien_hurt_speed;
extern ConVar asw_alien_stunned_speed;
extern ConVar rd_deagle_bigalien_dmg_scale;

#define	SWARM_DRONE_UBER_MODEL	"models/swarm/drone/UberDrone.mdl"

CASW_Drone_Uber::CASW_Drone_Uber()	
{

}

CASW_Drone_Uber::~CASW_Drone_Uber()
{
	
}

LINK_ENTITY_TO_CLASS( asw_drone_uber, CASW_Drone_Uber );

BEGIN_DATADESC( CASW_Drone_Uber )

END_DATADESC()

ConVar rd_drone_uber_model_scale("rd_drone_uber_model_scale", "1.3", FCVAR_CHEAT, "Scales uber drone model size" );
void CASW_Drone_Uber::Spawn( void )
{	
	BaseClass::Spawn();

	SetModel( SWARM_NEW_DRONE_MODEL );
	SetModelScale(rd_drone_uber_model_scale.GetFloat());
	Precache();	

	SetHullType( HULL_MEDIUMBIG );	// Setting HULL_MEDIUMBIG(like a regular drone) instead of HULL_LARGE to prevent uber drones getting stuck in doors and windows
	SetHullSizeNormal();

	UTIL_SetSize(this, Vector(-17,-17,20), Vector(17,17,69));

	// make sure uber drones are green
	m_nSkin = 0;
	SetHitboxSet(0);
	if (rd_drone_uber_bones.GetBool())
	{
		SetBodygroup ( 0, 0 );	//beefier body
		SetBodygroup ( 1, 2 );	//longest claws
		SetBodygroup ( 2, 2 );
		SetBodygroup ( 3, 2 );
		SetBodygroup ( 4, 2 );
		SetBodygroup ( 5, 1 );	//bones from back
     }
}

void CASW_Drone_Uber::Precache( void )
{
	PrecacheModel( SWARM_NEW_DRONE_MODEL );

	BaseClass::Precache();
}


void CASW_Drone_Uber::SetHealthByDifficultyLevel()
{
	SetHealth(ASWGameRules()->ModifyAlienHealthBySkillLevel(asw_drone_uber_health.GetInt()) + m_iHealthBonus);
	SetMaxHealth(GetHealth());
    SetHitboxSet(0);
}

float CASW_Drone_Uber::GetDamage()	//Easy customizing of alien damages.
{
		CBaseEntity *pHurt = CheckTraceHullAttack(asw_drone_melee_range.GetFloat(), -Vector(16,16,32), Vector(16,16,32), 0, DMG_SLASH, asw_drone_melee_force.GetFloat());
		if ( pHurt )
		{
			CASW_Marine *pMarine = CASW_Marine::AsMarine( pHurt );
			if ( pMarine )
			{
				CTakeDamageInfo info( this, this, rd_drone_uber_damage.GetFloat(), DMG_SLASH );
			}
		}

	return rd_drone_uber_damage.GetFloat();
}

float CASW_Drone_Uber::GetIdealSpeed() const
{
	return BaseClass::GetIdealSpeed() * asw_uber_speed_scale.GetFloat();
}

int CASW_Drone_Uber::OnTakeDamage_Alive( const CTakeDamageInfo &info )
{
	int result = 0;

	CTakeDamageInfo newInfo(info);
	float damage = info.GetDamage();

	// reduce damage from shotguns and mining laser
	if (info.GetDamageType() & DMG_ENERGYBEAM)
	{
		damage *= 0.5f;
	}
	if (info.GetDamageType() & DMG_BUCKSHOT)
	{
		// hack to reduce vindicator damage (not reducing normal shotty as much as it's not too strong)
		if (info.GetAttacker() && info.GetAttacker()->Classify() == CLASS_ASW_MARINE)
		{
			CASW_Marine *pMarine = dynamic_cast<CASW_Marine*>(info.GetAttacker());
			if (pMarine)
			{
				CASW_Weapon_Assault_Shotgun *pVindicator = dynamic_cast<CASW_Weapon_Assault_Shotgun*>(pMarine->GetActiveASWWeapon());
				if (pVindicator)
					damage *= 0.45f;
				else
					damage *= 0.6f;
			}
		}		
	}
	if (info.GetDamageType() & DMG_BULLET)
	{
		if (info.GetAttacker() && info.GetAttacker()->Classify() == CLASS_ASW_MARINE)
		{
			CASW_Marine *pMarine = dynamic_cast<CASW_Marine*>(info.GetAttacker());
			if ( pMarine && pMarine->GetActiveASWWeapon() )
			{
				extern ConVar rd_heavy_rifle_bigalien_dmg_scale;
				switch ( (int)pMarine->GetActiveASWWeapon()->Classify() )
				{
				case CLASS_ASW_DEAGLE:
					damage *= rd_deagle_bigalien_dmg_scale.GetFloat(); break;
				case CLASS_ASW_HEAVY_RIFLE:
					damage *= rd_heavy_rifle_bigalien_dmg_scale.GetFloat(); break;
				}
			}
		}
	}

	newInfo.SetDamage(damage);
	result = BaseClass::OnTakeDamage_Alive(newInfo);

	return result;
}

bool CASW_Drone_Uber::ModifyAutoMovement( Vector &vecNewPos )
{
	// melee auto movement on the drones seems way too fast
	float fFactor = asw_uber_auto_speed_scale.GetFloat();
	if ( ShouldMoveSlow() )
	{
		if ( m_bElectroStunned.Get() )
		{
			fFactor *= asw_alien_stunned_speed.GetFloat() * 0.1f;
		}
		else
		{
			fFactor *= asw_alien_hurt_speed.GetFloat() * 0.1f;
		}
	}
	Vector vecRelPos = vecNewPos - GetAbsOrigin();
	vecRelPos *= fFactor;
	vecNewPos = GetAbsOrigin() + vecRelPos;
	return true;
}
