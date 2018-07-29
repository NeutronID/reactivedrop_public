#include "cbase.h"
#include "baseentity.h"
#include "asw_spawner.h"
//#include "asw_simpleai_senses.h"
#include "asw_marine.h"
#include "asw_gamerules.h"
#include "asw_marine_resource.h"
#include "asw_game_resource.h"
#include "entityapi.h"
#include "entityoutput.h"
#include "props.h"
#include "asw_alien.h"
#include "asw_buzzer.h"
#include "asw_director.h"
#include "asw_fail_advice.h"
#include "asw_spawn_manager.h"
#include "entitylist.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS( asw_spawner, CASW_Spawner );

//ConVar asw_uber_drone_chance("asw_uber_drone_chance", "0.25f", FCVAR_CHEAT, "Chance of an uber drone spawning when playing in uber mode");
extern ConVar asw_debug_spawners;
extern ConVar asw_drone_health;
ConVar asw_carnage_debug("asw_carnage_debug", "0", FCVAR_NONE | FCVAR_DEVELOPMENTONLY, "Print debug messages on each spawner that asw_carnage tries to change.", true, 0.0f, true, 1.0f);
ConVar rd_carnage_antlionguard_normal("rd_carnage_antlionguard_normal", "1.0", FCVAR_CHEAT, "Multiplies antlionguard normal in the level by this amount.");
ConVar rd_carnage_antlionguard_cavern("rd_carnage_antlionguard_cavern", "1.0", FCVAR_CHEAT, "Multiplies antlionguard cavern in the level by this amount.");
ConVar rd_carnage_parasite_defanged("rd_carnage_parasite_defanged", "1.0", FCVAR_CHEAT, "Multiplies defanged parasites in the level by this amount.");
ConVar rd_carnage_min_interval("rd_carnage_min_interval", "0", FCVAR_CHEAT, "Sets the minimum spawn interval when using asw_carnage commands.");
ConVar rd_carnage_drone_jumper("rd_carnage_drone_jumper", "1.0", FCVAR_CHEAT, "Multiplies jumper drones in the level by this amount.");
ConVar rd_carnage_drone_uber("rd_carnage_drone_uber", "1.0", FCVAR_CHEAT, "Multiplies uber drones in the level by this amount.");
ConVar rd_carnage_shieldbug("rd_carnage_shieldbug", "1.0", FCVAR_CHEAT, "Multiplies shieldbugs in the level by this amount.");
ConVar rd_carnage_harvester("rd_carnage_harvester", "1.0", FCVAR_CHEAT, "Multiplies harvesters in the level by this amount.");
ConVar rd_carnage_parasite("rd_carnage_parasite", "1.0", FCVAR_CHEAT, "Multiplies parasites in the level by this amount.");
ConVar asw_spawning_enabled( "asw_spawning_enabled", "1", FCVAR_CHEAT, "If set to 0, asw_spawners won't spawn aliens" );
ConVar rd_carnage_boomer("rd_carnage_boomer", "1.0", FCVAR_CHEAT, "Multiplies boomers in the level by this amount.");
ConVar rd_carnage_ranger("rd_carnage_ranger", "1.0", FCVAR_CHEAT, "Multiplies rangers in the level by this amount.");
ConVar rd_carnage_mortar("rd_carnage_mortar", "1.0", FCVAR_CHEAT, "Multiplies mortars in the level by this amount.");
ConVar rd_carnage_shamen("rd_carnage_shamen", "1.0", FCVAR_CHEAT, "Multiplies shamen in the level by this amount.");
ConVar rd_carnage_buzzer("rd_carnage_buzzer", "1.0", FCVAR_CHEAT, "Multiplies buzzer in the level by this amount.");
ConVar rd_carnage_drone("rd_carnage_drone", "1.0", FCVAR_CHEAT, "Multiplies drones in the level by this amount.");
ConVar rd_carnage_queen("rd_carnage_queen", "1.0", FCVAR_CHEAT, "Multiplies queen in the level by this amount.");
ConVar rd_carnage_grub("rd_carnage_grub", "1.0", FCVAR_CHEAT, "Multiplies grub in the level by this amount.");
ConVar rd_cc_debug_spawners("rd_cc_debug_spawners", "0", FCVAR_NONE, "Shows debug messages on spawners.");
ConVar rd_spawner_impossimode("rd_spawner_impossimode", "0", FCVAR_CHEAT, "Makes ALL spawners infinite");

BEGIN_DATADESC( CASW_Spawner )
	DEFINE_KEYFIELD( m_nSetMaxLiveAliens,		FIELD_INTEGER,	"MaxLiveAliens" ),
	DEFINE_KEYFIELD( m_nSetNumAliens,			FIELD_INTEGER,	"NumAliens" ),	
	DEFINE_KEYFIELD( m_bInfiniteAliens,			FIELD_BOOLEAN,	"InfiniteAliens" ),
    DEFINE_KEYFIELD( m_flSetSpawnInterval,      FIELD_FLOAT,	"SpawnInterval" ),
	DEFINE_KEYFIELD( m_flSpawnIntervalJitter,	FIELD_FLOAT,	"SpawnIntervalJitter" ),
	DEFINE_KEYFIELD( m_AlienClassNum,			FIELD_INTEGER,	"AlienClass" ),
	DEFINE_KEYFIELD( m_SpawnerState,			FIELD_INTEGER,	"SpawnerState" ),
    DEFINE_INPUT(    m_bAllowDirectorSpawns,    FIELD_BOOLEAN,  "AllowDirectorSpawns" ),

	DEFINE_INPUTFUNC( FIELD_VOID,	"SpawnOneAlien",	InputSpawnAlien ),
	DEFINE_INPUTFUNC( FIELD_VOID,	"StartSpawning",	InputStartSpawning ),
	DEFINE_INPUTFUNC( FIELD_VOID,	"StopSpawning",		InputStopSpawning ),
	DEFINE_INPUTFUNC( FIELD_VOID,	"ToggleSpawning",	InputToggleSpawning ),

	DEFINE_OUTPUT( m_OnDirectorSpawned, "OnDirectorSpawned" ),
	DEFINE_OUTPUT( m_OnAllSpawned,		"OnAllSpawned" ),
	DEFINE_OUTPUT( m_OnAllSpawnedDead,	"OnAllSpawnedDead" ),

	DEFINE_FIELD( m_nCurrentLiveAliens, FIELD_INTEGER ),
	DEFINE_FIELD( m_AlienClassName,		FIELD_STRING ),

	DEFINE_THINKFUNC( SpawnerThink ),
END_DATADESC()

CASW_Spawner::CASW_Spawner()
{
	m_hAlienOrderTarget = NULL;
	m_bAllowDirectorSpawns = false;
	m_flLastDirectorSpawn = -FLT_MAX;
}

CASW_Spawner::~CASW_Spawner()
{

}

void CASW_Spawner::InitAlienClassName()
{
	if ( m_AlienClassNum < 0 || m_AlienClassNum >= ASWSpawnManager()->GetNumAlienClasses() )
	{
		m_AlienClassNum = 0;
	}

	m_AlienClassName = ASWSpawnManager()->GetAlienClass( m_AlienClassNum )->m_iszAlienClass;
}

void CASW_Spawner::Spawn()
{
	InitAlienClassName();

	BaseClass::Spawn();
	
	m_flSpawnIntervalJitter /= 100.0f;
	m_flSpawnIntervalJitter = clamp<float>(m_flSpawnIntervalJitter, 0, 100);

	SetSolid( SOLID_NONE );
	m_nCurrentLiveAliens = 0;

	// trigger any begin state stuff
	if (rd_spawner_impossimode.GetBool())
		m_bInfiniteAliens = true;
	SetSpawnerState(m_SpawnerState);
}

void CASW_Spawner::Precache()
{
	BaseClass::Precache();

	InitAlienClassName();
	const char *pszNPCName = STRING( m_AlienClassName );
	if ( !pszNPCName || !pszNPCName[0] )
	{
		Warning("asw_spawner %s has no specified alien-to-spawn classname.\n", STRING(GetEntityName()) );
	}
	else
	{
		UTIL_PrecacheOther( pszNPCName );
	}
}

IASW_Spawnable_NPC* CASW_Spawner::SpawnAlien( const char *szAlienClassName, const Vector &vecHullMins, const Vector &vecHullMaxs, CASW_Spawn_NPC *pDirectorNPC )
{
	IASW_Spawnable_NPC *pSpawnable = BaseClass::SpawnAlien( szAlienClassName, vecHullMins, vecHullMaxs, pDirectorNPC );
	if ( pSpawnable && pDirectorNPC )
	{
		m_OnDirectorSpawned.FireOutput( dynamic_cast<CBaseEntity *>( pSpawnable ), this );
		m_flLastDirectorSpawn = gpGlobals->curtime;
	}
	else if ( pSpawnable )
	{
		m_nCurrentLiveAliens++;

		if (!m_bInfiniteAliens)
		{
			m_nNumAliens--;
			if (m_nNumAliens <= 0)
			{
				SpawnedAllAliens();
			}
		}
		else
		{
			ASWFailAdvice()->OnAlienSpawnedInfinite();
		}
	}
	return pSpawnable;
}

bool CASW_Spawner::CanSpawn( const Vector &vecHullMins, const Vector &vecHullMaxs, CASW_Spawn_NPC *pDirectorNPC )
{
	if ( pDirectorNPC )
		return BaseClass::CanSpawn( vecHullMins, vecHullMaxs, pDirectorNPC );

	if ( !asw_spawning_enabled.GetBool() )
		return false;

	// too many alive already?
	if (gEntList.NumberOfEdicts() > 1900)
	{
		if (rd_cc_debug_spawners.GetBool())
		{
			Warning("Unable to spawn alien: edict# = %i\n", m_AlienClassNum, gEntList.NumberOfEdicts());
		}
		return false;
    }

	if (m_nMaxLiveAliens>0 && m_nCurrentLiveAliens>=m_nMaxLiveAliens)
	{
		if (rd_cc_debug_spawners.GetInt() == 3)
		{
			Warning("Unable to spawn alien: too many alive\n");
		}
		return false;
    }

	// have we run out?
	if (!m_bInfiniteAliens && m_nNumAliens<=0)
	{
		if (rd_cc_debug_spawners.GetBool())
		{
			Warning("Unable to spawn alien: out of aliens\n");
		}
		return false;
	}

	return BaseClass::CanSpawn( vecHullMins, vecHullMaxs );
}

// called when we've spawned all the aliens we can,
//  spawner should go to sleep
void CASW_Spawner::SpawnedAllAliens()
{
	m_OnAllSpawned.FireOutput( this, this );

	SetSpawnerState(SST_Finished); // disables think functions and so on
}

void CASW_Spawner::AlienKilled( CBaseEntity *pVictim )
{
	BaseClass::AlienKilled( pVictim );

	m_nCurrentLiveAliens--;

	if (asw_debug_spawners.GetBool())
		Msg("%d AlienKilled NumLive = %d\n", entindex(), m_nCurrentLiveAliens );

	// If we're here, we're getting erroneous death messages from children we haven't created
	AssertMsg( m_nCurrentLiveAliens >= 0, "asw_spawner receiving child death notice but thinks has no children\n" );

	if ( m_nCurrentLiveAliens <= 0 )
	{
		// See if we've exhausted our supply of NPCs
		if (!m_bInfiniteAliens && m_nNumAliens <= 0 )
		{
			if (asw_debug_spawners.GetBool())
				Msg("%d OnAllSpawnedDead (%s)\n", entindex(), STRING(GetEntityName()));
			// Signal that all our children have been spawned and are now dead
			m_OnAllSpawnedDead.FireOutput( this, this );
		}
	}
}

// mission started
void CASW_Spawner::MissionStart()
{
	if (asw_debug_spawners.GetBool())
		Msg("Spawner mission start, always inf=%d infinitealiens=%d\n", HasSpawnFlags( ASW_SF_ALWAYS_INFINITE ), m_bInfiniteAliens );
	// remove infinite spawns on easy mode
	if ( !HasSpawnFlags( ASW_SF_ALWAYS_INFINITE ) && ASWGameRules() && ASWGameRules()->GetSkillLevel() == 1
			&& m_bInfiniteAliens)
	{
		m_bInfiniteAliens = false;
		if (m_nNumAliens < 8)
			m_nNumAliens = 8;

		if (asw_debug_spawners.GetBool())
			Msg("  removed infinite and set num aliens to %d\n", m_nNumAliens);
	}

	m_nNumAliens = m_nSetNumAliens;
	m_nMaxLiveAliens = m_nSetMaxLiveAliens;

	if (rd_carnage_drone.GetFloat() > 0 && m_AlienClassNum == g_nDroneClassEntry)  //Regular drones
	{
		m_nNumAliens = m_nSetNumAliens * rd_carnage_drone.GetFloat();
		m_nMaxLiveAliens = m_nSetMaxLiveAliens * rd_carnage_drone.GetFloat();
		m_flSpawnInterval = m_flSetSpawnInterval / rd_carnage_drone.GetFloat();
		if (m_flSpawnInterval < rd_carnage_min_interval.GetFloat())
			m_flSpawnInterval = rd_carnage_min_interval.GetFloat();
	}

	if (rd_carnage_drone_jumper.GetFloat() > 0 && m_AlienClassNum == g_nDroneJumperClassEntry)  //Drone jumpers
	{
		m_nNumAliens = m_nSetNumAliens * rd_carnage_drone_jumper.GetFloat();
		m_nMaxLiveAliens = m_nSetMaxLiveAliens * rd_carnage_drone_jumper.GetFloat();
		m_flSpawnInterval = m_flSetSpawnInterval / rd_carnage_drone_jumper.GetFloat();
		if (m_flSpawnInterval < rd_carnage_min_interval.GetFloat())
			m_flSpawnInterval = rd_carnage_min_interval.GetFloat();
	}

	if (rd_carnage_drone_uber.GetFloat() > 0 && m_AlienClassNum == g_nUberDroneClassEntry)  //Uber drones
	{
		m_nNumAliens = m_nSetNumAliens * rd_carnage_drone_uber.GetFloat();
		m_nMaxLiveAliens = m_nSetMaxLiveAliens * rd_carnage_drone_uber.GetFloat();
		m_flSpawnInterval = m_flSetSpawnInterval / rd_carnage_drone_uber.GetFloat();
		if (m_flSpawnInterval < rd_carnage_min_interval.GetFloat())
			m_flSpawnInterval = rd_carnage_min_interval.GetFloat();
    }

	if (rd_carnage_buzzer.GetFloat() > 0 && m_AlienClassNum == g_nBuzzerClassEntry)  //Buzzers
	{
		m_nNumAliens = m_nSetNumAliens * rd_carnage_buzzer.GetFloat();
		m_nMaxLiveAliens = m_nSetMaxLiveAliens * rd_carnage_buzzer.GetFloat();
		m_flSpawnInterval = m_flSetSpawnInterval / rd_carnage_buzzer.GetFloat();
		if (m_flSpawnInterval < rd_carnage_min_interval.GetFloat())
			m_flSpawnInterval = rd_carnage_min_interval.GetFloat();
	}

	if (rd_carnage_parasite.GetFloat() > 0 && m_AlienClassNum == g_nParasiteClassEntry)  //Parasites
	{
		m_nNumAliens = m_nSetNumAliens * rd_carnage_parasite.GetFloat();
		m_nMaxLiveAliens = m_nSetMaxLiveAliens * rd_carnage_parasite.GetFloat();
		m_flSpawnInterval = m_flSetSpawnInterval / rd_carnage_parasite.GetFloat();
		if (m_flSpawnInterval < rd_carnage_min_interval.GetFloat())
			m_flSpawnInterval = rd_carnage_min_interval.GetFloat();
	}

	if (rd_carnage_shieldbug.GetFloat() > 0 && m_AlienClassNum == g_nShieldbugClassEntry)  //Shieldbugs
	{
		m_nNumAliens = m_nSetNumAliens * rd_carnage_shieldbug.GetFloat();
		m_nMaxLiveAliens = m_nSetMaxLiveAliens * rd_carnage_shieldbug.GetFloat();
		m_flSpawnInterval = m_flSetSpawnInterval / rd_carnage_shieldbug.GetFloat();
		if (m_flSpawnInterval < rd_carnage_min_interval.GetFloat())
			m_flSpawnInterval = rd_carnage_min_interval.GetFloat();
	}

	if (rd_carnage_harvester.GetFloat() > 0 && m_AlienClassNum == g_nHarvesterClassEntry)  //Harvesters
	{
		m_nNumAliens = m_nSetNumAliens * rd_carnage_harvester.GetFloat();
		m_nMaxLiveAliens = m_nSetMaxLiveAliens * rd_carnage_harvester.GetFloat();
		m_flSpawnInterval = m_flSetSpawnInterval / rd_carnage_harvester.GetFloat();
		if (m_flSpawnInterval < rd_carnage_min_interval.GetFloat())
			m_flSpawnInterval = rd_carnage_min_interval.GetFloat();
	}

	if (rd_carnage_parasite_defanged.GetFloat() > 0 && m_AlienClassNum == g_nSafeParasiteClassEntry)  //Defanged parasites
	{
		m_nNumAliens = m_nSetNumAliens * rd_carnage_parasite_defanged.GetFloat();
		m_nMaxLiveAliens = m_nSetMaxLiveAliens * rd_carnage_parasite_defanged.GetFloat();
		m_flSpawnInterval = m_flSetSpawnInterval / rd_carnage_parasite_defanged.GetFloat();
		if (m_flSpawnInterval < rd_carnage_min_interval.GetFloat())
			m_flSpawnInterval = rd_carnage_min_interval.GetFloat();
	}

	if (rd_carnage_boomer.GetFloat() > 0 && m_AlienClassNum == g_nBoomerClassEntry) //Boomers
	{
		m_nNumAliens = m_nSetNumAliens * rd_carnage_boomer.GetFloat();
		m_nMaxLiveAliens = m_nSetMaxLiveAliens * rd_carnage_boomer.GetFloat();
		m_flSpawnInterval = m_flSetSpawnInterval / rd_carnage_boomer.GetFloat();
		if (m_flSpawnInterval < rd_carnage_min_interval.GetFloat())
			m_flSpawnInterval = rd_carnage_min_interval.GetFloat();
	}

	if (rd_carnage_ranger.GetFloat() > 0 && m_AlienClassNum == g_nRangerClassEntry) //Rangers
	{
		m_nNumAliens = m_nSetNumAliens * rd_carnage_ranger.GetFloat();
		m_nMaxLiveAliens = m_nSetMaxLiveAliens * rd_carnage_ranger.GetFloat();
		m_flSpawnInterval = m_flSetSpawnInterval / rd_carnage_ranger.GetFloat();
		if (m_flSpawnInterval < rd_carnage_min_interval.GetFloat())
			m_flSpawnInterval = rd_carnage_min_interval.GetFloat();
	}

	if (rd_carnage_mortar.GetFloat() > 0 && m_AlienClassNum == g_nMortarClassEntry) //Mortars
	{
		m_nNumAliens = m_nSetNumAliens * rd_carnage_mortar.GetFloat();
		m_nMaxLiveAliens = m_nSetMaxLiveAliens * rd_carnage_mortar.GetFloat();
		m_flSpawnInterval = m_flSetSpawnInterval / rd_carnage_mortar.GetFloat();
		if (m_flSpawnInterval < rd_carnage_min_interval.GetFloat())
			m_flSpawnInterval = rd_carnage_min_interval.GetFloat();
	}

	if (rd_carnage_shamen.GetFloat() > 0 && m_AlienClassNum == g_nShamenClassEntry) //Shamen
	{
		m_nNumAliens = m_nSetNumAliens * rd_carnage_shamen.GetFloat();
		m_nMaxLiveAliens = m_nSetMaxLiveAliens * rd_carnage_shamen.GetFloat();
		m_flSpawnInterval = m_flSetSpawnInterval / rd_carnage_shamen.GetFloat();
		if (m_flSpawnInterval < rd_carnage_min_interval.GetFloat())
			m_flSpawnInterval = rd_carnage_min_interval.GetFloat();
	}

	if (rd_carnage_queen.GetFloat() > 0 && m_AlienClassNum == g_nQueenClassEntry)	 //Queen
	{
		m_nNumAliens = m_nSetNumAliens * rd_carnage_queen.GetFloat();
		m_nMaxLiveAliens = m_nSetMaxLiveAliens * rd_carnage_queen.GetFloat();
		m_flSpawnInterval = m_flSetSpawnInterval / rd_carnage_queen.GetFloat();
		if (m_flSpawnInterval < rd_carnage_min_interval.GetFloat())
			m_flSpawnInterval = rd_carnage_min_interval.GetFloat();
	}

	if (rd_carnage_grub.GetFloat() > 0 && m_AlienClassNum == g_nGrubClassEntry)	 //Grub
	{
		m_nNumAliens = m_nSetNumAliens * rd_carnage_grub.GetFloat();
		m_nMaxLiveAliens = m_nSetMaxLiveAliens * rd_carnage_grub.GetFloat();
		m_flSpawnInterval = m_flSetSpawnInterval / rd_carnage_grub.GetFloat();
		if (m_flSpawnInterval < rd_carnage_min_interval.GetFloat())
			m_flSpawnInterval = rd_carnage_min_interval.GetFloat();
	}

	if (rd_carnage_antlionguard_normal.GetFloat() > 0 && m_AlienClassNum == npc_antlionguard_normal)	 //antlionguard normal
	{
		m_nNumAliens = m_nSetNumAliens * rd_carnage_antlionguard_normal.GetFloat();
		m_nMaxLiveAliens = m_nSetMaxLiveAliens * rd_carnage_antlionguard_normal.GetFloat();
		m_flSpawnInterval = m_flSetSpawnInterval / rd_carnage_antlionguard_normal.GetFloat();
		if (m_flSpawnInterval < rd_carnage_min_interval.GetFloat())
			m_flSpawnInterval = rd_carnage_min_interval.GetFloat();
	}

	if (rd_carnage_antlionguard_cavern.GetFloat() > 0 && m_AlienClassNum == npc_antlionguard_cavern)	 //antlionguard cavern
	{
		m_nNumAliens = m_nSetNumAliens * rd_carnage_antlionguard_cavern.GetFloat();
		m_nMaxLiveAliens = m_nSetMaxLiveAliens * rd_carnage_antlionguard_cavern.GetFloat();
		m_flSpawnInterval = m_flSetSpawnInterval / rd_carnage_antlionguard_cavern.GetFloat();
		if (m_flSpawnInterval < rd_carnage_min_interval.GetFloat())
			m_flSpawnInterval = rd_carnage_min_interval.GetFloat();
	}

	if (rd_cc_debug_spawners.GetBool())
	{
		Msg("Set #aliens to %i, max live to %i, interval to %f\n", m_nNumAliens, m_nMaxLiveAliens, m_flSpawnInterval);
	}
		if (rd_spawner_impossimode.GetBool())
			m_bInfiniteAliens = true;

	if (m_SpawnerState == SST_StartSpawningWhenMissionStart)
		SetSpawnerState(SST_Spawning);
}

void CASW_Spawner::SetSpawnerState(SpawnerState_t newState)
{
	m_SpawnerState = newState;

	// begin state stuff
	if (m_SpawnerState == SST_Spawning)
	{
		SetThink ( &CASW_Spawner::SpawnerThink );
		SetNextThink( gpGlobals->curtime );
	}
	else if (m_SpawnerState == SST_Finished)
	{
		// Disable this forever.  Don't kill it because it still gets death notices
		SetThink( NULL );
		SetUse( NULL );
	}
	else if (m_SpawnerState == SST_WaitForInputs)
	{
		SetThink( NULL );	// stop thinking
	}
}

void CASW_Spawner::SpawnerThink()
{
	// calculate jitter
	float fInterval = random->RandomFloat(1.0f - m_flSpawnIntervalJitter, 1.0f + m_flSpawnIntervalJitter) * m_flSpawnInterval;
	SetNextThink( gpGlobals->curtime + fInterval );

	if ( ASWDirector() && ASWDirector()->CanSpawnAlien( this ) )
	{
		SpawnAlien( STRING( m_AlienClassName ), GetAlienMins(), GetAlienMaxs() );
	}
}

// =====================
// Inputs
// =====================

void CASW_Spawner::SpawnOneAlien()
{
	SpawnAlien( STRING( m_AlienClassName ), GetAlienMins(), GetAlienMaxs() );
}

void CASW_Spawner::InputSpawnAlien( inputdata_t &inputdata )
{
	if (m_SpawnerState == SST_WaitForInputs)
	{
		if ( ASWDirector() && ASWDirector()->CanSpawnAlien( this ) )
		{
			SpawnOneAlien();
		}
	}
}

void CASW_Spawner::InputStartSpawning( inputdata_t &inputdata )
{
	if (m_SpawnerState == SST_WaitForInputs)
		SetSpawnerState(SST_Spawning);
}

void CASW_Spawner::InputStopSpawning( inputdata_t &inputdata )
{
	if (m_SpawnerState == SST_Spawning)
		SetSpawnerState(SST_WaitForInputs);
}

void CASW_Spawner::InputToggleSpawning( inputdata_t &inputdata )
{
	if (m_SpawnerState == SST_Spawning)
		SetSpawnerState(SST_WaitForInputs);
	else if (m_SpawnerState == SST_WaitForInputs)
		SetSpawnerState(SST_Spawning);
}

const Vector& CASW_Spawner::GetAlienMins()
{
	return NAI_Hull::Mins( ASWSpawnManager()->GetAlienClass( m_AlienClassNum )->m_nHullType );
}

const Vector& CASW_Spawner::GetAlienMaxs()
{
	return NAI_Hull::Maxs( ASWSpawnManager()->GetAlienClass( m_AlienClassNum )->m_nHullType );
}

bool CASW_Spawner::ApplyCarnageMode( float fScaler, float fInvScaler )
{
	if ( m_AlienClassNum == g_nDroneClassEntry ||  m_AlienClassNum == g_nDroneJumperClassEntry )
	{
		float flNumAliens = m_nNumAliens * fScaler;
		float flMaxLiveAliens = m_nMaxLiveAliens * fScaler;

		// fractional part is randomly rounded - 1.25 rounds down to 1 75% of the time and up to 2 25% of the time.
		float flNumAliensPartial = fmodf( flNumAliens, 1 );
		if ( flNumAliensPartial != 0 )
		{
			flNumAliens += RandomFloat() < flNumAliensPartial ? 1 : 0;
		}
		float flMaxLiveAliensPartial = fmodf( flMaxLiveAliens, 1 );
		if ( flMaxLiveAliensPartial != 0 )
		{
			flMaxLiveAliens += RandomFloat() < flMaxLiveAliensPartial ? 1 : 0;
		}

		m_nNumAliens = flNumAliens;
		m_nMaxLiveAliens = flMaxLiveAliens;
		m_flSpawnInterval *= fInvScaler;

		return true;
	}
	return false;
}

bool CASW_Spawner::RandomizeUber(float percent)
{
	if (m_AlienClassNum == g_nDroneClassEntry || m_AlienClassNum == g_nUberDroneClassEntry)
		m_AlienClassName = ASWSpawnManager()->GetAlienClass( g_nDroneClassEntry )->m_iszAlienClass;	//Reset spawner

	if ( (m_AlienClassNum == g_nDroneClassEntry || m_AlienClassNum == g_nUberDroneClassEntry) && random->RandomFloat() <= percent)
	{
		m_AlienClassName = ASWSpawnManager()->GetAlienClass( g_nUberDroneClassEntry )->m_iszAlienClass;
		DevMsg("Set m_AlienClassNum to %i\n", m_AlienClassNum);
		return true;
	}
	else if (m_AlienClassNum == g_nDroneClassEntry || m_AlienClassNum == g_nUberDroneClassEntry)	//Prevents asw_carnage_randomize_uber from compounding on itself if used multiple times, and asw_carnage_randomize_uber 0 should change all uber spawners back to drone ones.
	{
		m_AlienClassName = ASWSpawnManager()->GetAlienClass( g_nDroneClassEntry )->m_iszAlienClass;
		DevMsg("Set m_AlienClassNum to %i\n", m_AlienClassNum);
		return false;
	}

	return false;
}

void CASW_Spawner::SetInfinitelySpawnAliens(bool spawn_infinitely /*= true */)
{
	m_bInfiniteAliens = spawn_infinitely;
}


int	CASW_Spawner::DrawDebugTextOverlays()
{
	int text_offset = BaseClass::DrawDebugTextOverlays();

	if (m_debugOverlays & OVERLAY_TEXT_BIT)
	{
		NDebugOverlay::EntityText( entindex(),text_offset,CFmtStr( "Num Live Aliens: %d", m_nCurrentLiveAliens ),0 );
		text_offset++;
		NDebugOverlay::EntityText( entindex(),text_offset,CFmtStr( "Max Live Aliens: %d", m_nMaxLiveAliens ),0 );
		text_offset++;
		NDebugOverlay::EntityText( entindex(),text_offset,CFmtStr( "Alien supply: %d", m_bInfiniteAliens ? -1 : m_nNumAliens ),0 );
		text_offset++;
	}
	return text_offset;
}

void ASW_ApplyCarnage_f(float fScaler)
{
	if ( fScaler <= 0 )
		fScaler = 1.0f;

	float fInvScaler = 1.0f / fScaler;

	//int iNewHealth = fInvScaler * 80.0f;	// note: boosted health a bit here so this mode is harder than normal
	//asw_drone_health.SetValue(iNewHealth);

	CBaseEntity* pEntity = NULL;
	int iSpawnersChanged = 0;
	while ((pEntity = gEntList.FindEntityByClassname( pEntity, "asw_spawner" )) != NULL)
	{
		CASW_Spawner* pSpawner = dynamic_cast<CASW_Spawner*>(pEntity);			
		if (pSpawner)
		{
			if ( pSpawner->ApplyCarnageMode( fScaler, fInvScaler ) )
			{
				iSpawnersChanged++;
			}
		}
	}
	DevMsg("%i asw_spawners had their output changed by asw_carnage.\n", iSpawnersChanged);
}

void ASW_ApplyInfiniteSpawners_f(void)
{
	CBaseEntity* pEntity = NULL;
	int iSpawnersChanged = 0;
	while ((pEntity = gEntList.FindEntityByClassname(pEntity, "asw_spawner")) != NULL)
	{
		CASW_Spawner* pSpawner = dynamic_cast<CASW_Spawner*>(pEntity);
		if (pSpawner)
		{
			pSpawner->SetInfinitelySpawnAliens();
			iSpawnersChanged++;
		}
	}
}

void ASW_RandomUber_f(float percent)
{
	CBaseEntity* pEntity = NULL;
	int iSpawnersChanged = 0;
	while ((pEntity = gEntList.FindEntityByClassname( pEntity, "asw_spawner" )) != NULL)
	{
		CASW_Spawner* pSpawner = dynamic_cast<CASW_Spawner*>(pEntity);
		if (pSpawner)
		{
			if ( pSpawner->RandomizeUber(percent) )
			{
				iSpawnersChanged++;
			}
		}
	}
	DevMsg("%i asw_spawners had their output changed by asw_carnage_randomize_uber.\n", iSpawnersChanged);
}

void asw_carnage_f(const CCommand &args)
{
	if ( args.ArgC() < 2 )
	{
		Msg( "Please supply a scale\n" );
	}
	ASW_ApplyCarnage_f( atof( args[1] ) );
}

void asw_carnage_random_uber_f(const CCommand &args)
{
	if (args.ArgC() < 2)
	{
		Msg("Please supply a percentage of spawners to randomize by\n");
	}
	else if (atof(args[1]) < 0.0f || atof(args[1]) > 1.0f)
	{
		Msg("Value must be between 0 and 1.0\n");
	}
	else
	{
		ASW_RandomUber_f(atof(args[1]));
	}
}

ConCommand asw_carnage_randomize_uber( "asw_carnage_randomize_uber", asw_carnage_random_uber_f, "Randomizes asw_drone spawners with asw_drone_uber spawners.", FCVAR_CHEAT);
