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

typedef union fi_s
{
	float			_float;
	int			_int;
} fi_t;	

int PASSFLOAT(float x)
{
	fi_t rc;
	rc._float = x;
	return rc._int;
}



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
	return ( qboolean ) syscall( G_GetEntityToken, (int)token, size );
}
void trap_DPrintf( const char *fmt )
{
	syscall( G_DPRINT, (int)fmt );
}
void trap_BPrint( int level, const char *fmt )
{
	syscall( G_BPRINT, level, (int) fmt );
}

void trap_SPrint( int edn, int level, const char *fmt )
{
	syscall( G_SPRINT, edn, level, (int)fmt );
}
void trap_CenterPrint( int edn, const char *fmt )
{
	syscall( G_CENTERPRINT, edn , (int)fmt );
}

void trap_Error( const char *fmt )
{
	syscall( G_ERROR, (int) fmt );
}

int	trap_spawn()
{
	return syscall( G_SPAWN_ENT ) ;
}

void trap_remove( int edn )
{
	syscall( G_REMOVE_ENT, edn  );
}

void trap_precache_sound( char *name )
{
	syscall( G_PRECACHE_SOUND, (int) name );
}
void trap_precache_model( char *name )
{
	syscall( G_PRECACHE_MODEL, (int) name );
}

void trap_setorigin( int edn, float origin_x, float origin_y, float origin_z )
{
	syscall( G_SETORIGIN,  edn, PASSFLOAT(origin_x),
		 PASSFLOAT(origin_y), PASSFLOAT(origin_z) );
}

void trap_setsize( int edn, float min_x, float min_y, float min_z, float max_x,
		   float max_y, float max_z )
{
	syscall( G_SETSIZE, edn, PASSFLOAT( min_x), PASSFLOAT( min_y),
		 PASSFLOAT( min_z), PASSFLOAT( max_x), PASSFLOAT( max_y), PASSFLOAT( max_z ));
}

void trap_setmodel( int edn, char *model )
{
	syscall( G_SETMODEL, edn, (int)model );
}

void trap_ambientsound( float pos_x, float pos_y, float pos_z, char *samp, float vol,
			float atten )
{
	syscall( G_AMBIENTSOUND, PASSFLOAT( pos_x), PASSFLOAT( pos_y), PASSFLOAT( pos_z),
		 (int)samp, PASSFLOAT( vol), PASSFLOAT( atten ));
}

void trap_sound( int edn, int channel, char *samp, int vol, float att )
{
	syscall( G_SOUND, edn, channel, (int)samp, vol, PASSFLOAT( att ));
}

int trap_checkclient()
{
	return  syscall( G_CHECKCLIENT ) ;
}
void trap_traceline( float v1_x, float v1_y, float v1_z, float v2_x, float v2_y,
		     float v2_z, int nomonst, int edn )
{
	syscall( G_TRACELINE, PASSFLOAT( v1_x), PASSFLOAT( v1_y), PASSFLOAT( v1_z),
		 PASSFLOAT( v2_x), PASSFLOAT( v2_y), PASSFLOAT( v2_z), nomonst,
		 edn );
}

void trap_stuffcmd( int edn, const char *fmt )
{
	syscall( G_STUFFCMD, edn, (int)fmt );
}

void trap_localcmd( const char *fmt )
{
	syscall( G_LOCALCMD,(int) fmt );
}
float trap_cvar( const char *var )
{
	fi_t tmp;
	
	tmp._int = syscall( G_CVAR, (int)var );

	return tmp._float;
}

void trap_cvar_set( const char *var, const char *val )
{
	syscall( G_CVAR_SET, (int)var, (int)val );
}

int trap_droptofloor( int edn )
{
	return syscall( G_DROPTOFLOOR, edn );
}

int trap_walkmove( int edn, float yaw, float dist )
{
	return syscall( G_WALKMOVE, edn, PASSFLOAT( yaw),
			PASSFLOAT( dist ));
}

void trap_lightstyle( int style, char *val )
{
	syscall( G_LIGHTSTYLE, style, (int)val );
}

int trap_checkbottom( int edn )
{
	return syscall( G_CHECKBOTTOM, edn );
}

int trap_pointcontents( float origin_x, float origin_y, float origin_z )
{
	return syscall( G_POINTCONTENTS, PASSFLOAT( origin_x), PASSFLOAT( origin_y),
			PASSFLOAT( origin_z ));
}

int trap_nextent( int n )
{
	return syscall( G_NEXTENT, n );
//      return &g_edicts[syscall( G_NEXTENT,NUM_FOR_EDICT(ed))];
}

void trap_makestatic( int edn )
{
	syscall( G_MAKESTATIC, edn );
}

void trap_setspawnparam( int edn )
{
	syscall( G_SETSPAWNPARAMS, edn );
}

void trap_changelevel( char *name )
{
	syscall( G_CHANGELEVEL, name );
}

int trap_multicast( float origin_x, float origin_y, float origin_z, int to )
{
	return syscall( G_MULTICAST, PASSFLOAT( origin_x), PASSFLOAT( origin_y),
			PASSFLOAT( origin_z), to );
}


void trap_logfrag( int killer, int killee )
{
	syscall( G_LOGFRAG,  killer , killee  );
}

void trap_infokey( int edn, char *key, char *valbuff, int sizebuff )
{
	syscall( G_GETINFOKEY, edn, (int)key, (int)valbuff, sizebuff );
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
	syscall( G_WRITEANGLE, to, PASSFLOAT( data ));
}

void trap_WriteCoord( int to, float data )
{
	syscall( G_WRITECOORD, to, PASSFLOAT( data ));
}

void trap_WriteString( int to, char *data )
{
	syscall( G_WRITESTRING, to, (int)data );
}

void trap_WriteEntity( int to, int edn )
{
	syscall( G_WRITEENTITY, to, edn );
}

void trap_FlushSignon()
{
	syscall( G_FLUSHSIGNON );
}


void trap_disableupdates( int edn, float time )
{
	syscall( G_DISABLEUPDATES, edn , PASSFLOAT( time ));
}

int trap_CmdArgc()
{
	return syscall( G_CMD_ARGC );
}
void trap_CmdArgv( int arg, char *valbuff, int sizebuff )
{
	syscall( G_CMD_ARGV, arg, (int)valbuff, sizebuff );
}
