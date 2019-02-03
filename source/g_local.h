//
// g_local.h -- local definitions for game module

#include "q_shared.h"
#include "mathlib.h"
#include "progs.h"
#include "g_public.h"
#include "g_consts.h"
#include "g_syscalls.h"
#include "player.h"

#ifdef DEBUG
#define DebugTrap(x) *(char*)0=x
#else
#define DebugTrap(x) G_Error(x)
#endif


#define	MAX_STRING_CHARS	1024	// max length of a string passed to Cmd_TokenizeString
#define	MAX_STRING_TOKENS	1024	// max tokens resulting from Cmd_TokenizeString
#define	MAX_TOKEN_CHARS		1024	// max length of an individual token
#define MAX_STRINGS 128

#define	FOFS(x) ((intptr_t)&(((gedict_t *)0)->x))
int             NUM_FOR_EDICT( gedict_t * e );



extern gedict_t g_edicts[MAX_EDICTS];
extern char mapname[64];
extern globalvars_t g_globalvars;
extern gedict_t *world;
extern gedict_t *self, *other;
extern gedict_t *newmis;
extern int      timelimit, fraglimit, teamplay, deathmatch, framecount, rj;

#define	EDICT_TO_PROG(e) ((byte *)(e) - (byte *)g_edicts)
#define PROG_TO_EDICT(e) ((gedict_t *)((byte *)g_edicts + (e)))

void            G_Printf( const char *fmt, ... );
void            G_Error( const char *fmt, ... );

#define PASSVEC3(x) (x[0]),(x[1]),(x[2])
#define SetVector(v,x,y,z) (v[0]=x,v[1]=y,v[2]=z)
//g_utils.c
float           g_random( void );
float           crandom( void );
gedict_t       *spawn( void );
void            ent_remove( gedict_t * t );

gedict_t       *nextent( gedict_t * ent );
gedict_t       *find( gedict_t * start, int fieldoff, char *str );
gedict_t       *findradius( gedict_t * start, vec3_t org, float rad );
void            normalize( vec3_t value, vec3_t newvalue );
float           vlen( vec3_t value1 );
float           vectoyaw( vec3_t value1 );
void            vectoangles( vec3_t value1, vec3_t ret );
void            changeyaw( gedict_t * ent );
void            makevectors( vec3_t vector );
void            G_sprint( gedict_t * ed, int level, const char *fmt, ... );
void            G_bprint( int level, const char *fmt, ... );
void            G_centerprint( gedict_t * ed, const char *fmt, ... );
void            G_dprint( const char *fmt, ... );
int             streq( const char *s1, const char *s2 );
int             strneq( const char *s1, const char *s2 );
void            aim( vec3_t ret );
void    	setorigin( gedict_t * ed, float origin_x, float origin_y, float origin_z );
void    	setsize( gedict_t * ed, float min_x, float min_y, float min_z, float max_x,
		 float max_y, float max_z );
void    	setmodel( gedict_t * ed, char *model );
void    	sound( gedict_t * ed, int channel, char *samp, float vol, float att );
gedict_t 	*checkclient(  );
void    	traceline( float v1_x, float v1_y, float v1_z, float v2_x, float v2_y, float v2_z,
		   int nomonst, gedict_t * ed );
void    	stuffcmd( gedict_t * ed, const char *fmt , ...);
int     	droptofloor( gedict_t * ed );
int     	walkmove( gedict_t * ed, float yaw, float dist );
int     	checkbottom( gedict_t * ed );
void    	makestatic( gedict_t * ed );
void    	setspawnparam( gedict_t * ed );
void    	logfrag( gedict_t * killer, gedict_t * killee );
void    	infokey( gedict_t * ed, const char *key, char *valbuff, int sizebuff );
void    	WriteEntity( int to, gedict_t * ed );
void    	disableupdates( gedict_t * ed, float time );

//
//  subs.c
//
void            SUB_CalcMove( vec3_t tdest, float tspeed, void ( *func ) () );
void            SUB_CalcMoveEnt( gedict_t * ent, vec3_t tdest, float tspeed,
				 void ( *func ) () );
void            SUB_UseTargets();
void            SetMovedir();
void            InitTrigger();
extern gedict_t *activator;

//
// g_mem.c
//
void           *G_Alloc( int size );
void            G_InitMemory( void );

//
// g_spawn.c
//

void            G_SpawnEntitiesFromString( void );
qboolean        G_SpawnString( const char *key, const char *defaultString, char **out );
qboolean        G_SpawnFloat( const char *key, const char *defaultString, float *out );
qboolean        G_SpawnInt( const char *key, const char *defaultString, int *out );
qboolean        G_SpawnVector( const char *key, const char *defaultString, float *out );
void            SUB_Remove();
void            SUB_Null();

//world.c
void            CopyToBodyQue( gedict_t * ent );

// client.c
extern float    intermission_running;
extern float    intermission_exittime;
extern int      modelindex_eyes, modelindex_player;

void            SetChangeParms();
void            SetNewParms();
void            ClientConnect();
void            PutClientInServer();
void            ClientDisconnect();
void            PlayerPreThink();
void            PlayerPostThink();
void            SuperDamageSound();

//spectate.c
void            SpectatorConnect();
void            SpectatorDisconnect();
void            SpectatorThink();

// weapons.c
extern int      impulse;
float           W_BestWeapon();
void            W_Precache();
void            W_SetCurrentAmmo();
void            SpawnBlood( vec3_t, float );
void            W_FireAxe();
void            W_FireSpikes( float ox );
void            W_FireLightning();

//combat 
extern gedict_t *damage_attacker, *damage_inflictor;
void            T_Damage( gedict_t * targ, gedict_t * inflictor, gedict_t * attacker,
			  float damage );
void            T_RadiusDamage( gedict_t * inflictor, gedict_t * attacker, float damage,
				gedict_t * ignore, char *dtype );
void            T_BeamDamage( gedict_t * attacker, float damage );

//items
void            DropQuad( float timeleft );
void            DropRing( float timeleft );
void            DropBackpack();

//triggers.c
void            spawn_tfog( vec3_t org );
void            spawn_tdeath( vec3_t org, gedict_t * death_owner );
