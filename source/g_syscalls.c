// Copyright (C) 1999-2000 Id Software, Inc.
//
//#include "g_local.h"

// this file is only included when building a dll
// g_syscalls.asm is included instead when building a qvm
#ifdef Q3_VM
#error "Do not use in VM build"
#endif


#include "g_local.h"

static int      ( QDECL * syscall ) ( int arg, ... ) =
    ( int ( QDECL * ) ( int, ... ) ) -1;




void dllEntry( int ( QDECL * syscallptr ) ( int arg, ... ) )
{
	syscall = syscallptr;
}

int trap_GetApiVersion()
{
	return syscall( G_GETAPIVERSION );
}

qboolean trap_GetEntityToken( char *token, int size )
{
	return ( qboolean ) syscall( G_GetEntityToken, token, size );
}
void trap_DPrintf( const char *fmt )
{
	syscall( G_DPRINT, fmt );
}
void trap_BPrint( int level, const char *fmt )
{
	syscall( G_BPRINT, level, fmt );
}

void trap_SPrint( gedict_t * ed, int level, const char *fmt )
{
	syscall( G_SPRINT, NUM_FOR_EDICT( ed ), level, fmt );
}
void trap_CenterPrint( gedict_t * ed, const char *fmt )
{
	syscall( G_CENTERPRINT, NUM_FOR_EDICT( ed ), fmt );
}

void trap_Error( const char *fmt )
{
	syscall( G_ERROR, fmt );
}

gedict_t       *trap_spawn()
{
	return PROG_TO_EDICT( syscall( G_SPAWN_ENT ) );
}

void trap_remove( gedict_t * ed )
{
	syscall( G_REMOVE_ENT, EDICT_TO_PROG( ed ) );
}

void trap_precache_sound( char *name )
{
	syscall( G_PRECACHE_SOUND, name );
}
void trap_precache_model( char *name )
{
	syscall( G_PRECACHE_MODEL, name );
}

void trap_setorigin( gedict_t * ed, float origin_x, float origin_y, float origin_z )
{
	syscall( G_SETORIGIN, EDICT_TO_PROG( ed ), ( double ) origin_x,
		 ( double ) origin_y, ( double ) origin_z );
}

void trap_setsize( gedict_t * ed, float min_x, float min_y, float min_z, float max_x,
		   float max_y, float max_z )
{
	syscall( G_SETSIZE, EDICT_TO_PROG( ed ), ( double ) min_x, ( double ) min_y,
		 ( double ) min_z, ( double ) max_x, ( double ) max_y, ( double ) max_z );
}

void trap_setmodel( gedict_t * ed, char *model )
{
	syscall( G_SETMODEL, EDICT_TO_PROG( ed ), model );
}

void trap_ambientsound( float pos_x, float pos_y, float pos_z, char *samp, float vol,
			float atten )
{
	syscall( G_AMBIENTSOUND, ( double ) pos_x, ( double ) pos_y, ( double ) pos_z,
		 samp, ( double ) vol, ( double ) atten );
}

void trap_sound( gedict_t * ed, int channel, char *samp, int vol, float att )
{
	syscall( G_SOUND, EDICT_TO_PROG( ed ), channel, samp, vol, ( double ) att );
}

gedict_t       *trap_checkclient()
{
	return PROG_TO_EDICT( syscall( G_CHECKCLIENT ) );
}
void trap_traceline( float v1_x, float v1_y, float v1_z, float v2_x, float v2_y,
		     float v2_z, int nomonst, gedict_t * ed )
{
	syscall( G_TRACELINE, ( double ) v1_x, ( double ) v1_y, ( double ) v1_z,
		 ( double ) v2_x, ( double ) v2_y, ( double ) v2_z, nomonst,
		 EDICT_TO_PROG( ed ) );
}

void trap_stuffcmd( gedict_t * ed, const char *fmt )
{
	syscall( G_STUFFCMD, NUM_FOR_EDICT( ed ), fmt );
}

void trap_localcmd( const char *fmt )
{
	syscall( G_LOCALCMD, fmt );
}
float trap_cvar( const char *var )
{
	int             tmp = syscall( G_CVAR, var );

	return *( float * ) &tmp;
}

void trap_cvar_set( const char *var, const char *val )
{
	syscall( G_CVAR_SET, var, val );
}

int trap_droptofloor( gedict_t * ed )
{
	return syscall( G_DROPTOFLOOR, EDICT_TO_PROG( ed ) );
}

int trap_walkmove( gedict_t * ed, float yaw, float dist )
{
	return syscall( G_WALKMOVE, EDICT_TO_PROG( ed ), ( double ) yaw,
			( double ) dist );
}

void trap_lightstyle( int style, char *val )
{
	syscall( G_LIGHTSTYLE, style, val );
}

int trap_checkbottom( gedict_t * ed )
{
	return syscall( G_CHECKBOTTOM, EDICT_TO_PROG( ed ) );
}

int trap_pointcontents( float origin_x, float origin_y, float origin_z )
{
	return syscall( G_POINTCONTENTS, ( double ) origin_x, ( double ) origin_y,
			( double ) origin_z );
}

int trap_nextent( int n )
{
	return syscall( G_NEXTENT, n );
//      return &g_edicts[syscall( G_NEXTENT,NUM_FOR_EDICT(ed))];
}

void trap_makestatic( gedict_t * ed )
{
	syscall( G_MAKESTATIC, EDICT_TO_PROG( ed ) );
}

void trap_setspawnparam( gedict_t * ed )
{
	syscall( G_SETSPAWNPARAMS, EDICT_TO_PROG( ed ) );
}

void trap_changelevel( char *name )
{
	syscall( G_CHANGELEVEL, name );
}

int trap_multicast( float origin_x, float origin_y, float origin_z, int to )
{
	return syscall( G_MULTICAST, ( double ) origin_x, ( double ) origin_y,
			( double ) origin_z, to );
}


void trap_logfrag( gedict_t * killer, gedict_t * killee )
{
	syscall( G_LOGFRAG, NUM_FOR_EDICT( killer ), NUM_FOR_EDICT( killee ) );
}

void trap_infokey( gedict_t * ed, char *key, char *valbuff, int sizebuff )
{
	syscall( G_GETINFOKEY, NUM_FOR_EDICT( ed ), key, valbuff, sizebuff );
}

void trap_WriteByte( int to, int data )
{
	syscall( G_WRITEBYTE, to, data );
}

void trap_WriteChar( int to, int data )
{
	syscall( G_WRITECHAR, to, data );
}

void trap_WriteShort( int to, int data )
{
	syscall( G_WRITESHORT, to, data );
}

void trap_WriteLong( int to, int data )
{
	syscall( G_WRITELONG, to, data );
}

void trap_WriteAngle( int to, float data )
{
	syscall( G_WRITEANGLE, to, ( double ) data );
}

void trap_WriteCoord( int to, float data )
{
	syscall( G_WRITECOORD, to, ( double ) data );
}

void trap_WriteString( int to, char *data )
{
	syscall( G_WRITESTRING, to, data );
}

void trap_WriteEntity( int to, gedict_t * ed )
{
	syscall( G_WRITEENTITY, to, NUM_FOR_EDICT( ed ) );
}

void trap_FlushSignon()
{
	syscall( G_FLUSHSIGNON );
}


void trap_disableupdates( gedict_t * ed, float time )
{
	syscall( G_DISABLEUPDATES, NUM_FOR_EDICT( ed ), ( double ) time );
}

int trap_CmdArgc()
{
	return syscall( G_CMD_ARGC );
}
void trap_CmdArgv( int arg, char *valbuff, int sizebuff )
{
	syscall( G_CMD_ARGV, arg, valbuff, sizebuff );
}
