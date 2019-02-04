/*
 *  QWProgs-DM
 *  Copyright (C) 2004  [sd] angel
 *
 *  This code is based on Q3 VM code by Id Software, Inc.
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *
 *  $Id$
 */

#include "g_local.h"
/* global 4 fix
   entity          self;
   entity          other;
   entity          world;
   entity          newmis;                         // if this is set, the entity that just
// run created a new missile that should
// be simulated immediately
entity          trace_ent;
entity          msg_entity;                             // destination of single entity writes

*/
gedict_t        g_edicts[MAX_EDICTS];	//768
gedict_t       *world = g_edicts;
gedict_t       *self, *other;

//short shortvar=0xfedc;
globalvars_t    g_globalvars;
field_t         expfields[] = {
    {"maxspeed", FOFS( maxspeed ), F_FLOAT}	,
    {"gravity", FOFS( gravity ), F_FLOAT},
    {"isBot", FOFS( isBot ), F_INT},
    {"items2",      FOFS( items2 ),      F_FLOAT},

    {"vw_index",    FOFS( vw_index ),    F_FLOAT},
    {"movement",    FOFS( movement ),    F_VECTOR},
    {"brokenankle", FOFS( brokenankle ), F_FLOAT},
    {"mod_admin",   FOFS( k_admin ),     F_INT},
    {"hideentity",  FOFS( hideentity ),  F_INT},
    {"trackent",	FOFS( trackent ),	 F_INT},
    {"hideplayers", FOFS( hideplayers ), F_INT},
    {"visclients",  FOFS( visclients ),  F_INT},
    {"teleported",  FOFS( teleported ),  F_INT},
    {NULL}
};
char     mapname[64];
static char     worldmodel[64] = "worldmodel";
static char     netnames[MAX_CLIENTS][32];

#define MIN_API_VERSION GAME_API_VERSION
gameData_t      gamedata =
{ ( edict_t * ) g_edicts, sizeof( gedict_t ), &g_globalvars, expfields , GAME_API_VERSION, MAX_EDICTS};
float           starttime;
void            G_InitGame( int levelTime, int randomSeed );
void            StartFrame( int time );
qboolean        ClientCommand();
qboolean        ClientUserInfoChanged();
qboolean        ClientUserInfoChanged_after();
void            G_EdictTouch();
void            G_EdictThink();
void            G_EdictBlocked();
void            ClearGlobals();
void ModCommand();
void initialise_spawned_ent(gedict_t* ent);

/*
   ================
   vmMain

   This is the only way control passes into the module.
   This must be the very first function compiled into the .q3vm file
   ================
   */
#define RestoreGlobals()  \
    damage_attacker = damage_attacker;\
    damage_inflictor = damage_inflictor_; \
    activator = activator_; \
    self = self_; \
    other = other_; \
    newmis = newmis_; 
int vmMain( int command, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5,
        int arg6, int arg7, int arg8, int arg9, int arg10, int arg11 )
{
    int api_ver;
    gedict_t* damage_attacker_= damage_attacker;
    gedict_t* damage_inflictor_= damage_inflictor;
    gedict_t* activator_ = activator ;
    gedict_t* self_ = self;
    gedict_t* other_ = other;
    gedict_t* newmis_ = newmis;

    ClearGlobals();
    switch ( command )
    {
        case GAME_INIT:
            api_ver = trap_GetApiVersion();
#ifdef idx64
            // We set references
            cvar_fset("sv_pr2references", 1);
#endif
            G_dprint( "Init Game %d %d %d %d %d\n", api_ver, GAME_API_VERSION, sizeof( edict_t), sizeof( entvars_t), sizeof( edict_t)- sizeof( entvars_t) );
            if ( api_ver < MIN_API_VERSION ) 
            {
                G_conprintf("Mod requried API_VERSION %d or higher, server have %d\n", MIN_API_VERSION, api_ver);
                RestoreGlobals();
                return 0;
            }

            G_InitGame( arg0, arg1 );
            RestoreGlobals();
            return ( int ) ( &gamedata );

        case GAME_LOADENTS:
            infokey( world, "mapname", mapname, sizeof(mapname) );
            G_SpawnEntitiesFromString();
            RestoreGlobals();
            return 1;

        case GAME_START_FRAME:
            StartFrame( arg0 );
            RestoreGlobals();
            return 1;

        case GAME_CLIENT_CONNECT:
            self = PROG_TO_EDICT( g_globalvars.self );
            self->s.v.netname = netnames[NUM_FOR_EDICT(self)-1]; //Init client names
            infokey( self, "netname", self->s.v.netname,  32);
            if ( arg0 )
                SpectatorConnect();
            else
                ClientConnect();
            RestoreGlobals();
            return 1;

        case GAME_PUT_CLIENT_IN_SERVER:
            self = PROG_TO_EDICT( g_globalvars.self );
            if ( !arg0 )
                PutClientInServer();
            RestoreGlobals();
            return 1;

        case GAME_CLIENT_DISCONNECT:
            self = PROG_TO_EDICT( g_globalvars.self );
            if ( arg0 )
                SpectatorDisconnect();
            else
                ClientDisconnect();
            RestoreGlobals();
            return 1;

        case GAME_SETNEWPARMS:
            SetNewParms();
            RestoreGlobals();
            return 1;

        case GAME_CLIENT_PRETHINK:
            self = PROG_TO_EDICT( g_globalvars.self );
            if ( !arg0 )
                PlayerPreThink();
            RestoreGlobals();
            return 1;

        case GAME_CLIENT_POSTTHINK:
            self = PROG_TO_EDICT( g_globalvars.self );
            if ( !arg0 )
                PlayerPostThink();
            else
                SpectatorThink();
            RestoreGlobals();
            return 1;

        case GAME_EDICT_TOUCH:
            G_EdictTouch();
            RestoreGlobals();
            return 1;

        case GAME_EDICT_THINK:
            G_EdictThink();
            RestoreGlobals();
            return 1;

        case GAME_EDICT_BLOCKED:
            G_EdictBlocked();
            RestoreGlobals();
            return 1;

        case GAME_SETCHANGEPARMS:
            self = PROG_TO_EDICT( g_globalvars.self );
            SetChangeParms();
            RestoreGlobals();
            return 1;

        case GAME_CLIENT_COMMAND:
            RestoreGlobals();
            return ClientCommand();

        case GAME_SHUTDOWN:
            RestoreGlobals();
            return 0;

        case GAME_CLIENT_USERINFO_CHANGED:
            {
                int ret = arg0?ClientUserInfoChanged():ClientUserInfoChanged_after();
                RestoreGlobals();
                return ret;
            }
        case GAME_CONSOLE_COMMAND:

            // called on server console command "mod"
            // params like GAME_CLIENT_COMMAND, but argv(0) always "mod"
            // self - rconner if can detect else world
            // other 
            //SV_CMD_CONSOLE		0          
            //SV_CMD_RCON			1  
            //SV_CMD_MASTER		2          
            //SV_CMD_BOT			3  
            self = PROG_TO_EDICT( g_globalvars.self );
            ModCommand();
            RestoreGlobals();
            return 0;
        case GAME_CLIENT_SAY:
            // called on user /say or /say_team
            // arg0 non zero if say_team
            // return non zero if say/say_team handled by mod
            // params like GAME_CLIENT_COMMAND

            RestoreGlobals();
            return 0;

        case GAME_PAUSED_TIC:
            // called every frame when the game is paused
            RestoreGlobals();
            return 0;

        case GAME_CLEAR_EDICT:
            // Don't ClearGlobals() as this will be called during spawn()
            initialise_spawned_ent(PROG_TO_EDICT( g_globalvars.self ));
            RestoreGlobals();
            return 0;
    }

    return 0;
}

//===================================================================
void G_Printf( const char *fmt, ... )
{
    va_list         argptr;
    char            text[1024];

    va_start( argptr, fmt );
    vsprintf( text, fmt, argptr );
    va_end( argptr );

    trap_DPrintf( text );
}

void Com_Error( int level, const char *error, ... )
{
    va_list         argptr;
    char            text[1024];

    va_start( argptr, error );
    vsprintf( text, error, argptr );
    va_end( argptr );

    G_Error( "%s", text );
}

void Com_Printf( const char *msg, ... )
{
    va_list         argptr;
    char            text[1024];

    va_start( argptr, msg );
    vsprintf( text, msg, argptr );
    va_end( argptr );

    G_Printf( "%s", text );
}

//===================================================================
void G_InitGame( int levelTime, int randomSeed )
{
    int             i;

    srand( randomSeed );
    framecount = 0;
    starttime = levelTime * 0.001;
    G_InitMemory();
    memset( g_edicts, 0, sizeof( gedict_t ) * MAX_EDICTS );


    world->s.v.model = worldmodel;
    //g_globalvars.mapname = mapname;
    for ( i = 0; i < MAX_CLIENTS; i++ )
    {
        g_edicts[i + 1].s.v.netname = netnames[i];
    }

}




//===========================================================================
// Physics
// 
//===========================================================================
////////////////
// GlobalParams:
// time
// self
// other
///////////////
void G_EdictTouch()
{
    self = PROG_TO_EDICT( g_globalvars.self );
    other = PROG_TO_EDICT( g_globalvars.other );
    if ( self->s.v.touch )
    {
        /*#ifdef DEBUG
          if(self->s.v.classname && other->s.v.classname)
          if(!strcmp(self->s.v.classname,"player")||!strcmp(other->s.v.classname,"player"))
          G_dprint( "touch %s <-> %s\n", self->s.v.classname,other->s.v.classname);
#endif*/
        ( ( void ( * )() ) ( self->s.v.touch ) ) ();
    } else
    {
        G_Printf( "Null touch func" );
    }
}

////////////////
// GlobalParams:
// time
// self
// other=world
///////////////
void G_EdictThink()
{
    self = PROG_TO_EDICT( g_globalvars.self );
    other = PROG_TO_EDICT( g_globalvars.other );
    if ( self->s.v.think )
    {
        ( ( void ( * )() ) ( self->s.v.think ) ) ();
    } else
    {
        G_Printf( "Null think func" );
    }

}

////////////////
// GlobalParams:
// time
// self=pusher
// other=check
// if the pusher has a "blocked" function, call it
// otherwise, just stay in place until the obstacle is gone
///////////////
void G_EdictBlocked()
{
    self = PROG_TO_EDICT( g_globalvars.self );
    other = PROG_TO_EDICT( g_globalvars.other );

    if ( self->s.v.blocked )
    {
        ( ( void ( * )() ) ( self->s.v.blocked ) ) ();
    } else
    {
        //G_Printf("Null blocked func");
    }

}


void ClearGlobals()
{
    damage_attacker = damage_inflictor = activator = self = other = newmis = world;
}

#ifdef idx64
// Should be "" but too many references in code simply checking for 0 to mean null string...
#define	FOFS_s(x) ((intptr_t)&(((gedict_t *)0)->s.v.x))
#define PR2SetStringFieldOffset(ent, field) \
	ent->s.v.field ## _ = NUM_FOR_EDICT(ent) * sizeof(gedict_t) + FOFS_s(field); \
	ent->s.v.field = 0;

#define PR2SetFuncFieldOffset(ent, field) \
	ent->s.v.field ## _ = NUM_FOR_EDICT(ent) * sizeof(gedict_t) + FOFS_s(field); \
	ent->s.v.field = (func_t) SUB_Null;
#endif

void initialise_spawned_ent(gedict_t* ent)
{
#ifdef idx64
	PR2SetStringFieldOffset(ent, classname);
	PR2SetStringFieldOffset(ent, model);
	PR2SetFuncFieldOffset(ent, touch);
	PR2SetFuncFieldOffset(ent, use);
	PR2SetFuncFieldOffset(ent, think);
	PR2SetFuncFieldOffset(ent, blocked);
	PR2SetStringFieldOffset(ent, weaponmodel);
	PR2SetStringFieldOffset(ent, netname);
	PR2SetStringFieldOffset(ent, target);
	PR2SetStringFieldOffset(ent, targetname);
	PR2SetStringFieldOffset(ent, message);
	PR2SetStringFieldOffset(ent, noise);
	PR2SetStringFieldOffset(ent, noise1);
	PR2SetStringFieldOffset(ent, noise2);
	PR2SetStringFieldOffset(ent, noise3);
#endif
}

