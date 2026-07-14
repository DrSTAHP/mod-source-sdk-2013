//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "gameinterface.h"
#include "mapentities.h"
#include "hl2mp_gameinterface.h"
#include "tier0/icommandline.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// -------------------------------------------------------------------------------------------- //
// Mod-specific CServerGameClients implementation.
// -------------------------------------------------------------------------------------------- //

#if defined ( INTERLOPER_DLL ) && defined ( DEBUG ) 
static void SvIntrMaxPlayersOverrideListen(IConVar* var, const char* pOldValue, float flOldValue)
{
	if (!var)
		return;

	ConVar* pVarDef = cvar->FindVar(var->GetName());
	if ( !pVarDef )
		return;

	bool bIsActive = pVarDef->GetBool();
	bool bPrevious = static_cast<bool>(V_atoi(pOldValue));

	// Revert the default configuration for Interloper
	if (!bIsActive && bPrevious && engine)
	{
		const int iNewMaxPlayers = (/* TODO: Check (coming soon) Interloper API if we are playing with "somebody"... */ 1) ? 1 : MAX_PLAYERS;
		
		char szCommand[32];
		sprintf_s(szCommand, sizeof(szCommand), "maxplayers %i\n", iNewMaxPlayers);
		
		engine->ServerCommand(szCommand);
		engine->ServerExecute();
	}
}

ConVar sv_intr_maxplayers_override("sv_intr_maxplayers_override", "0", FCVAR_DEVELOPMENTONLY | FCVAR_GAMEDLL | FCVAR_NOT_CONNECTED | FCVAR_PROTECTED, "Bypass the Interloper API checks for foreign presence.", &SvIntrMaxPlayersOverrideListen);
#endif

void CServerGameClients::GetPlayerLimits( int& minplayers, int& maxplayers, int &defaultMaxPlayers ) const
{
#ifdef INTERLOPER_DLL
	minplayers = 1;
#else
	minplayers = 2;
#endif
#ifdef PLATFORM_64BITS
	maxplayers = MAX_PLAYERS;
#else
	if ( CommandLine()->HasParm("-unrestricted_maxplayers") )
	{
		static bool s_bWarned = false;
		if ( !s_bWarned )
		{
			Warning( "The use of -unrestricted_maxplayers is NOT supported and definitely NOT recommended and may be unstable.\n" );
			s_bWarned = true;
		}
		maxplayers = MAX_PLAYERS;
	}
	else
		maxplayers = 33;
#endif
#ifdef INTERLOPER_DLL
#ifdef DEBUG
	bool bActiveMaxPlayersOverride = sv_intr_maxplayers_override.GetBool();
	if ( !bActiveMaxPlayersOverride ) // Default otherwise.
#endif
	{
		maxplayers = (/* TODO: Check (coming soon) Interloper API if we are playing with "somebody"... */ 1) ? 1 : MAX_PLAYERS; // TODO: Will be a method in Interloper API. Hopefully...
		defaultMaxPlayers = 1;
	}
#else
	defaultMaxPlayers = 16; // misyl: Was 2... but why would the default be 2?! Is there some very intimate HL2DM going on?
#endif
}

// -------------------------------------------------------------------------------------------- //
// Mod-specific CServerGameDLL implementation.
// -------------------------------------------------------------------------------------------- //

void CServerGameDLL::LevelInit_ParseAllEntities( const char *pMapEntities )
{
#ifdef INTERLOPER_DLL
	ConVar* cl_localnetworkbackdoor = cvar->FindVar( "cl_localnetworkbackdoor" );
	if ( cl_localnetworkbackdoor )
	{
		cl_localnetworkbackdoor->SetValue( (gpGlobals->maxClients) == 1 ? "0" : "1" );
	}
#endif
}

