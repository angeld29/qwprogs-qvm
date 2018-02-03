#include "g_local.h"

#define	MAX_SPAWN_VARS			64
#define	MAX_SPAWN_VARS_CHARS	4096

int             numSpawnVars;
char           *spawnVars[MAX_SPAWN_VARS][2];	// key / value pairs
int             numSpawnVarChars;
char            spawnVarChars[MAX_SPAWN_VARS_CHARS];

qboolean G_SpawnString( const char *key, const char *defaultString, char **out )
{
	int             i;

/*	if ( !level.spawning ) {
		*out = (char *)defaultString;
//		G_Error( "G_SpawnString() called while not spawning" );
	}*/

	for ( i = 0; i < numSpawnVars; i++ )
	{
		if ( !Q_stricmp( key, spawnVars[i][0] ) )
		{
			*out = spawnVars[i][1];
			return true;
		}
	}

	*out = ( char * ) defaultString;
	return false;
}

qboolean G_SpawnFloat( const char *key, const char *defaultString, float *out )
{
	char           *s;
	qboolean        present;

	present = G_SpawnString( key, defaultString, &s );
	*out = atof( s );
	return present;
}

qboolean G_SpawnInt( const char *key, const char *defaultString, int *out )
{
	char           *s;
	qboolean        present;

	present = G_SpawnString( key, defaultString, &s );
	*out = atoi( s );
	return present;
}

qboolean G_SpawnVector( const char *key, const char *defaultString, float *out )
{
	char           *s;
	qboolean        present;

	present = G_SpawnString( key, defaultString, &s );
	sscanf( s, "%f %f %f", &out[0], &out[1], &out[2] );
	return present;
}

//
// fields are needed for spawning from the entity string
//

field_t         fields[] = {
	{"classname",	FOFS( s.v.classname ),	F_LSTRING},
	{"origin", 	FOFS( s.v.origin ),	F_VECTOR},
	{"model", 	FOFS( s.v.model ),	F_LSTRING},
	{"message", 	FOFS( s.v.message ),	F_LSTRING},
	{"target", 	FOFS( s.v.target ),	F_LSTRING},
	{"map", 	FOFS( map ),	 	F_LSTRING},
	{"killtarget", 	FOFS( killtarget ),	F_LSTRING},
	{"count", 	FOFS( count ),	 	F_FLOAT},
	{"targetname", 	FOFS( s.v.targetname ),	F_LSTRING},
	{"wait", 	FOFS( wait ),	 	F_FLOAT},
	{"skin", 	FOFS( s.v.skin ),	F_FLOAT},
	{"effects", 	FOFS( s.v.effects ),	F_FLOAT},
	{"speed", 	FOFS( speed ),	 	F_FLOAT},
	{"spawnflags", 	FOFS( s.v.spawnflags ),	F_FLOAT},
	{"health", 	FOFS( s.v.health ),	F_FLOAT},
	{"takedamage", 	FOFS( s.v.takedamage ),	F_FLOAT},
	{"dmg", 	FOFS( dmg ),	 	F_FLOAT},
	{"delay", 	FOFS( delay ),	 	F_FLOAT},
	{"worldtype", 	FOFS( worldtype ),	F_FLOAT},
	{"lip", 	FOFS( lip ),		F_FLOAT},
	{"height", 	FOFS( height ),		F_FLOAT},
	{"sounds", 	FOFS( s.v.sounds ),	F_FLOAT},
	{"angles", 	FOFS( s.v.angles ),	F_VECTOR},
	{"mangle", 	FOFS( mangle ),		F_VECTOR},
	{"style", 	FOFS( mangle ),		F_INT},
	{"angle", 	FOFS( s.v.angles ),	F_ANGLEHACK},
	{"light", 	0,			F_IGNORE},
	{"wad", 	0, 			F_IGNORE},
	{NULL}
};
typedef struct {
	char           *name;
	void            ( *spawn ) ();
} spawn_t;

void SUB_Remove()
{
	ent_remove( self );
}

void SUB_Null()
{
}

void            SP_light();
void            SP_light_fluoro();
void            SP_light_fluorospark();
void            SP_light_globe();
void            SP_light_torch_small_walltorch();
void            SP_light_flame_large_yellow();
void            SP_light_flame_small_yellow();
void            SP_light_flame_small_white();

void            SP_ambient_suck_wind();
void            SP_ambient_drone();
void            SP_ambient_flouro_buzz();
void            SP_ambient_drip();
void            SP_ambient_comp_hum();
void            SP_ambient_thunder();
void            SP_ambient_light_buzz();
void            SP_ambient_swamp1();
void            SP_ambient_swamp2();
void            SP_misc_noisemaker();
void            SP_misc_explobox();
void            SP_misc_explobox2();
void            SP_air_bubbles();

void            SP_trap_spikeshooter();
void            SP_trap_shooter();
void            SP_func_wall();
void            SP_func_illusionary();
void            SP_func_episodegate();
void            SP_func_bossgate();

void            SP_func_door();
void            SP_func_door_secret();
void            SP_func_plat();
void            SP_func_train();
void            SP_misc_teleporttrain();
void            SP_func_button();

void            SP_trigger_multiple();
void            SP_trigger_once();
void            SP_trigger_relay();
void            SP_trigger_secret();
void            SP_trigger_counter();
void            SP_info_teleport_destination();
void            SP_trigger_teleport();
void            SP_trigger_setskill();
void            SP_trigger_onlyregistered();
void            SP_trigger_hurt();
void            SP_trigger_push();
void            SP_trigger_monsterjump();
void            SP_trigger_changelevel();
void            SP_path_corner();

void            SP_item_health();
void            SP_item_armor1();
void            SP_item_armor2();
void            SP_item_armorInv();
void            SP_weapon_supershotgun();
void            SP_weapon_nailgun();
void            SP_weapon_supernailgun();
void            SP_weapon_grenadelauncher();
void            SP_weapon_rocketlauncher();
void            SP_weapon_lightning();
void            SP_item_shells();
void            SP_item_spikes();
void            SP_item_rockets();
void            SP_item_cells();
void            SP_item_weapon();

void            SP_item_artifact_invulnerability();
void            SP_item_artifact_envirosuit();
void            SP_item_artifact_invisibility();
void            SP_item_artifact_super_damage();
void            SP_item_sigil();
void            SP_item_key1();
void            SP_item_key2();

void            SP_misc_fireball();
void            SP_info_intermission();
spawn_t         spawns[] = {
	// info entities don't do anything at all, but provide positional
	// information for things controlled by other processes
	{"info_player_start",		SUB_Null},
	{"info_player_start2",		SUB_Null},
	{"info_player_deathmatch", 	SUB_Null},
	{"info_player_coop", 		SUB_Null},
	{"info_intermission", 		SP_info_intermission},
	{"trigger_changelevel", 	SP_trigger_changelevel},
/*QUAKED info_notnull (0 0.5 0) (-4 -4 -4) (4 4 4)
Used as a positional target for lightning.
*/
	{"info_notnull", 		SUB_Null},

	{"light", 			SP_light},
	{"light_fluoro", 		SP_light_fluoro},
	{"light_fluorospark", 		SP_light_fluorospark},
	{"light_globe", 		SP_light_globe},
	{"light_torch_small_walltorch", SP_light_torch_small_walltorch},
	{"light_flame_large_yellow", 	SP_light_flame_large_yellow},
	{"light_flame_small_yellow", 	SP_light_flame_small_yellow},
	{"light_flame_small_white", 	SP_light_flame_small_white},
	{"air_bubbles", 		SP_air_bubbles},

	{"misc_fireball",		SP_misc_fireball},

	{"ambient_suck_wind",		SP_ambient_suck_wind},
	{"ambient_drone",		SP_ambient_drone},
	{"ambient_flouro_buzz",		SP_ambient_flouro_buzz},
	{"ambient_drip",		SP_ambient_drip},
	{"ambient_comp_hum",		SP_ambient_comp_hum},
	{"ambient_thunder",		SP_ambient_thunder},
	{"ambient_light_buzz",		SP_ambient_light_buzz},
	{"ambient_swamp1",		SP_ambient_swamp1},
	{"ambient_swamp2",		SP_ambient_swamp2},
	{"misc_noisemaker",		SP_misc_noisemaker},
	{"misc_explobox",		SP_misc_explobox},
	{"misc_explobox2",		SP_misc_explobox2},
	{"trap_spikeshooter",		SP_trap_spikeshooter},
	{"trap_shooter",		SP_trap_shooter},
	{"func_wall",			SP_func_wall},
	{"func_illusionary",		SP_func_illusionary},
	{"func_episodegate",		SP_func_episodegate},
	{"func_bossgate",		SP_func_bossgate},


	{"func_door",			SP_func_door},
	{"func_door_secret",		SP_func_door_secret},
	{"func_plat",			SP_func_plat},
	{"func_train",			SP_func_train},
	{"misc_teleporttrain",		SP_misc_teleporttrain},
	{"func_button",			SP_func_button},

	{"trigger_multiple",		SP_trigger_multiple},
	{"trigger_once",		SP_trigger_once},
	{"trigger_relay",		SP_trigger_relay},
	{"trigger_secret",		SP_trigger_secret},
	{"trigger_counter",		SP_trigger_counter},
	{"info_teleport_destination",	SP_info_teleport_destination},
	{"trigger_teleport",		SP_trigger_teleport},
	{"trigger_setskill",		SP_trigger_setskill},
	{"trigger_onlyregistered",	SP_trigger_onlyregistered},
	{"trigger_hurt",		SP_trigger_hurt},
	{"trigger_push",		SP_trigger_push},
	{"trigger_monsterjump",		SP_trigger_monsterjump},
	{"path_corner",			SP_path_corner},

	{"item_health",			SP_item_health},
	{"item_armor1",			SP_item_health},
	{"item_armor2",			SP_item_health},
	{"item_armorInv",		SP_item_health},
	{"weapon_supershotgun",		SP_weapon_supershotgun},
	{"weapon_nailgun",		SP_weapon_nailgun},
	{"weapon_supernailgun",		SP_weapon_supernailgun},
	{"weapon_grenadelauncher",	SP_weapon_grenadelauncher},
	{"weapon_rocketlauncher",	SP_weapon_rocketlauncher},
	{"weapon_lightning",		SP_weapon_lightning},
	{"item_shells",			SP_item_health},
	{"item_spikes",			SP_item_health},
	{"item_rockets",		SP_item_health},
	{"item_cells",			SP_item_health},
	{"item_weapon",			SP_item_weapon},
	{"item_artifact_invulnerability",	SP_item_artifact_invulnerability},
	{"item_artifact_envirosuit",		SP_item_artifact_envirosuit},
	{"item_artifact_invisibility",		SP_item_artifact_invisibility},
	{"item_artifact_super_damage",		SP_item_artifact_super_damage},
	{"item_sigil",			SP_item_sigil},
	{"item_key1",			SP_item_key1},
	{"item_key2",			SP_item_key2},

//not used ents
/*QUAKED info_null (0 0.5 0) (-4 -4 -4) (4 4 4)
Used as a positional target for spotlights, etc.
*/
	{"info_null",			SUB_Remove},
	{"monster_ogre",		SUB_Remove},
	{"monster_demon1",		SUB_Remove},
	{"monster_shambler",		SUB_Remove},
	{"monster_knight",		SUB_Remove},
	{"monster_army",		SUB_Remove},
	{"monster_wizard",		SUB_Remove},
	{"monster_dog",			SUB_Remove},
	{"monster_zombie",		SUB_Remove},
	{"monster_boss",		SUB_Remove},
	{"monster_tarbaby",		SUB_Remove},
	{"monster_hell_knight",		SUB_Remove},
	{"monster_fish",		SUB_Remove},
	{"monster_shalrath",		SUB_Remove},
	{"monster_enforcer",		SUB_Remove},
	{"monster_oldone",		SUB_Remove},
	{"event_lightning",		SUB_Remove},
	{0, 0}
};

/*
===============
G_CallSpawn

Finds the spawn function for the entity and calls it,
returning qfalse if not found
===============
*/
qboolean G_CallSpawn( gedict_t * ent )
{
	spawn_t        *s;

//      gitem_t *item;

	if ( !ent->s.v.classname )
	{
		G_Printf( "G_CallSpawn: NULL classname\n" );
		return false;
	}

/*	// check item spawn functions
	for ( item=bg_itemlist+1 ; item->classname ; item++ ) {
		if ( !strcmp(item->classname, ent->classname) ) {
			G_SpawnItem( ent, item );
			return qtrue;
		}
	}*/

	// check normal spawn functions
	for ( s = spawns; s->name; s++ )
	{
		if ( !strcmp( s->name, ent->s.v.classname ) )
		{
			// found it
			self = ent;
			//G_Printf("%8i %s\n",ent->s.v.classname,ent->s.v.classname);
			s->spawn();
			return true;
		}
	}
	G_Printf( "%s doesn't have a spawn function\n", ent->s.v.classname );
	return false;
}

/*
=============
G_NewString

Builds a copy of the string, translating \n to real linefeeds
so message texts can be multi-line
=============
*/
char           *G_NewString( const char *string )
{
	char           *newb, *new_p;
	int             i, l;

	l = strlen( string ) + 1;

	newb = G_Alloc( l );

	new_p = newb;

	// turn \n into a real linefeed
	for ( i = 0; i < l; i++ )
	{
		if ( string[i] == '\\' && i < l - 1 )
		{
			i++;
			if ( string[i] == 'n' )
			{
				*new_p++ = '\n';
			} else
			{
				*new_p++ = '\\';
			}
		} else
		{
			*new_p++ = string[i];
		}
	}

	return newb;
}

/*
===============
G_ParseField

Takes a key/value pair and sets the binary values
in a gentity
===============
*/
void G_ParseField( const char *key, const char *value, gedict_t * ent )
{
	field_t        *f;
	byte           *b;
	float           v;
	vec3_t          vec;

	for ( f = fields; f->name; f++ )
	{
		if ( !Q_stricmp( f->name, key ) )
		{
			// found it
			b = ( byte * ) ent;

			switch ( f->type )
			{
			case F_LSTRING:
				*( char ** ) ( b + f->ofs ) = G_NewString( value );
				break;
			case F_VECTOR:
				sscanf( value, "%f %f %f", &vec[0], &vec[1], &vec[2] );
				( ( float * ) ( b + f->ofs ) )[0] = vec[0];
				( ( float * ) ( b + f->ofs ) )[1] = vec[1];
				( ( float * ) ( b + f->ofs ) )[2] = vec[2];
				break;
			case F_INT:
				*( int * ) ( b + f->ofs ) = atoi( value );
				break;
			case F_FLOAT:
				*( float * ) ( b + f->ofs ) = atof( value );
				break;
			case F_ANGLEHACK:
				v = atof( value );
				( ( float * ) ( b + f->ofs ) )[0] = 0;
				( ( float * ) ( b + f->ofs ) )[1] = v;
				( ( float * ) ( b + f->ofs ) )[2] = 0;
				break;
			default:
			case F_IGNORE:
				break;
			}
			return;
		}
	}
	G_Printf( "unknown field: %s\n", key );
}

/*
===================
G_SpawnGEntityFromSpawnVars

Spawn an entity and fill in all of the level fields from
level.spawnVars[], then call the class specfic spawn function
===================
*/
void G_SpawnGEntityFromSpawnVars( void )
{
	int             i;
	gedict_t       *ent;

	// get the next free entity
	ent = spawn();

	for ( i = 0; i < numSpawnVars; i++ )
	{
		G_ParseField( spawnVars[i][0], spawnVars[i][1], ent );
	}

	if ( ( ( int ) ent->s.v.spawnflags & SPAWNFLAG_NOT_DEATHMATCH ) )
	{
		ent_remove( ent );
		return;
	}

	// if we didn't get a classname, don't bother spawning anything
	if ( !G_CallSpawn( ent ) )
	{
		ent_remove( ent );
		return;
	}
//      if(!ent->s.v.classname)ent->s.v.classname="";
}


/*
====================
G_AddSpawnVarToken
====================
*/
char           *G_AddSpawnVarToken( const char *string )
{
	int             l;
	char           *dest;

	l = strlen( string );
	if ( numSpawnVarChars + l + 1 > MAX_SPAWN_VARS_CHARS )
	{
		G_Error( "G_AddSpawnVarToken: MAX_SPAWN_CHARS" );
	}

	dest = spawnVarChars + numSpawnVarChars;
	memcpy( dest, string, l + 1 );

	numSpawnVarChars += l + 1;

	return dest;
}

/*
====================
G_ParseSpawnVars

Parses a brace bounded set of key / value pairs out of the
level's entity strings into level.spawnVars[]

This does not actually spawn an entity.
====================
*/
qboolean G_ParseSpawnVars( void )
{
	char            keyname[MAX_TOKEN_CHARS];
	char            com_token[MAX_TOKEN_CHARS];

	numSpawnVars = 0;
	numSpawnVarChars = 0;

	// parse the opening brace
	if ( !trap_GetEntityToken( com_token, sizeof( com_token ) ) )
	{
		// end of spawn string
		return false;
	}
	if ( com_token[0] != '{' )
	{
		G_Error( "G_ParseSpawnVars: found %s when expecting {", com_token );
	}
	// go through all the key / value pairs
	while ( 1 )
	{
		// parse key
		if ( !trap_GetEntityToken( keyname, sizeof( keyname ) ) )
		{
			G_Error( "G_ParseSpawnVars: EOF without closing brace" );
		}

		if ( keyname[0] == '}' )
		{
			break;
		}
		// parse value  
		if ( !trap_GetEntityToken( com_token, sizeof( com_token ) ) )
		{
			G_Error( "G_ParseSpawnVars: EOF without closing brace" );
		}
//              G_Printf("%s\t%s\n",keyname,com_token);
		if ( com_token[0] == '}' )
		{
			G_Error( "G_ParseSpawnVars: closing brace without data" );
		}
		if ( numSpawnVars == MAX_SPAWN_VARS )
		{
			G_Error( "G_ParseSpawnVars: MAX_SPAWN_VARS" );
		}
		spawnVars[numSpawnVars][0] = G_AddSpawnVarToken( keyname );
		spawnVars[numSpawnVars][1] = G_AddSpawnVarToken( com_token );
		numSpawnVars++;
	}

	return true;
}

void            SP_worldspawn();

/*
==============
G_SpawnEntitiesFromString

Parses textual entity definitions out of an entstring and spawns gentities.
==============
*/

void G_SpawnEntitiesFromString( void )
{
	int             i;

	// the worldspawn is not an actual entity, but it still
	// has a "spawn" function to perform any global setup
	// needed by a level (setting configstrings or cvars, etc)
	if ( !G_ParseSpawnVars() )
	{
		G_Error( "SpawnEntities: no entities" );
	}
	self = world;
	for ( i = 0; i < numSpawnVars; i++ )
	{
		G_ParseField( spawnVars[i][0], spawnVars[i][1], world );
	}

	SP_worldspawn( world );

	// parse ents

	while ( G_ParseSpawnVars() )
	{
		G_SpawnGEntityFromSpawnVars();
		trap_FlushSignon();
	}
}
