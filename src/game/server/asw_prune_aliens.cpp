#include "cbase.h"
#include "convar.h"
#include "entitylist.h"
#include "vprof.h"
#include "asw_prune_aliens.h"
#include "util_shared.h"
#include "asw_alien.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

CASW_Prune_Aliens::CASW_Prune_Aliens(void)
{

}

CASW_Prune_Aliens::~CASW_Prune_Aliens(void)
{

}

LINK_ENTITY_TO_CLASS( asw_alien_pruner, CASW_Prune_Aliens );

void CASW_Prune_Aliens::Spawn()
{
	BaseClass::Spawn();
}

void CASW_Prune_Aliens::Think()
{

}