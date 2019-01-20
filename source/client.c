//===========================================================================
// Client
// 
//===========================================================================
#include "g_local.h"
vec3_t          VEC_ORIGIN = { 0, 0, 0 };
vec3_t          VEC_HULL_MIN = { -16, -16, -24 };
vec3_t          VEC_HULL_MAX = { 16, 16, 32 };

vec3_t          VEC_HULL2_MIN = { -32, -32, -24 };
vec3_t          VEC_HULL2_MAX = { 32, 32, 64 };
int             modelindex_eyes, modelindex_player;

/*
=============================================================================

    LEVEL CHANGING / INTERMISSION

=============================================================================
*/
float           intermission_running;
float           intermission_exittime;
char            nextmap[64] = "";

/*QUAKED info_intermission (1 0.5 0.5) (-16 -16 -16) (16 16 16)
This is the camera point for the intermission.
Use mangle instead of angle, so you can set pitch or roll as well as yaw.  'pitch roll yaw'
*/
void SP_info_intermission()
{
	// so C can get at it
	VectorCopy( self->mangle, self->s.v.angles );	//self.angles = self.mangle;      
}

void SetNewParms( void )
{
	g_globalvars.parm1 = IT_SHOTGUN | IT_AXE;
	g_globalvars.parm2 = 100;
	g_globalvars.parm3 = 0;
	g_globalvars.parm4 = 25;
	g_globalvars.parm5 = 0;
	g_globalvars.parm6 = 0;
	g_globalvars.parm7 = 0;
	g_globalvars.parm8 = 1;
	g_globalvars.parm9 = 0;
}

void SetChangeParms()
{

	if ( self->s.v.health <= 0 )
	{
		SetNewParms();
		return;
	}
// remove items
	self->s.v.items -= ( ( int ) self->s.v.items &
			     ( IT_KEY1 | IT_KEY2 | IT_INVISIBILITY | IT_INVULNERABILITY |
			       IT_SUIT | IT_QUAD ) );

// cap super health
	if ( self->s.v.health > 100 )
		self->s.v.health = 100;

	if ( self->s.v.health < 50 )
		self->s.v.health = 50;

	g_globalvars.parm1 = self->s.v.items;
	g_globalvars.parm2 = self->s.v.health;
	g_globalvars.parm3 = self->s.v.armorvalue;

	if ( self->s.v.ammo_shells < 25 )
		g_globalvars.parm4 = 25;
	else
		g_globalvars.parm4 = self->s.v.ammo_shells;

	g_globalvars.parm5 = self->s.v.ammo_nails;
	g_globalvars.parm6 = self->s.v.ammo_rockets;
	g_globalvars.parm7 = self->s.v.ammo_cells;
	g_globalvars.parm8 = self->s.v.weapon;
	g_globalvars.parm9 = self->s.v.armortype * 100;
}



void DecodeLevelParms()
{
	if ( g_globalvars.serverflags )
	{
		if ( streq( world->s.v.model, "maps/start.bsp" ) )
			SetNewParms();	// take away all stuff on starting new episode
	}

	self->s.v.items		= g_globalvars.parm1;
	self->s.v.health	= g_globalvars.parm2;
	self->s.v.armorvalue 	= g_globalvars.parm3;
	self->s.v.ammo_shells 	= g_globalvars.parm4;
	self->s.v.ammo_nails 	= g_globalvars.parm5;
	self->s.v.ammo_rockets 	= g_globalvars.parm6;
	self->s.v.ammo_cells 	= g_globalvars.parm7;
	self->s.v.weapon 	= g_globalvars.parm8;
	self->s.v.armortype 	= g_globalvars.parm9 * 0.01;
}

/*
============
FindIntermission

Returns the entity to view from
============
*/
gedict_t       *FindIntermission()
{
	gedict_t       *spot;
	int             cyc;

// look for info_intermission first
	spot = find( world, FOFS( s.v.classname ), "info_intermission" );
	if ( spot )
	{			// pick a random one
		cyc = g_random() * 4;
		while ( cyc > 1 )
		{
			spot = find( spot, FOFS( s.v.classname ), "info_intermission" );
			if ( !spot )
				spot =
				    find( world, FOFS( s.v.classname ),
					  "info_intermission" );
			cyc = cyc - 1;
		}
		return spot;
	}

// then look for the start position
	spot = find( world, FOFS( s.v.classname ), "info_player_start" );
	if ( spot )
		return spot;

	G_Error( "FindIntermission: no spot" );
	return NULL;
}

void GotoNextMap()
{
	char            newmap[64];

//ZOID: 12-13-96, samelevel is overloaded, only 1 works for same level

	if ( trap_cvar( "samelevel" ) == 1 )	// if samelevel is set, stay on same level
		trap_changelevel( g_globalvars.mapname, "" );
	else
	{
		// configurable map lists, see if the current map exists as a
		// serverinfo/localinfo var
		infokey( world, g_globalvars.mapname, newmap, sizeof( newmap ) );

		if ( newmap[0] )
			trap_changelevel( newmap, "" );
		else
			trap_changelevel( nextmap, "" );
	}
}

/*
============
IntermissionThink

When the player presses attack or jump, change to the next level
============
*/
void IntermissionThink()
{
	if ( g_globalvars.time < intermission_exittime )
		return;

	if ( !self->s.v.button0 && !self->s.v.button1 && !self->s.v.button2 )
		return;

	GotoNextMap();
}

/*
============
execute_changelevel

The global "nextmap" has been set previously.
Take the players to the intermission spot
============
*/

void execute_changelevel()
{
	gedict_t       *pos;

	intermission_running = 1;

// enforce a wait time before allowing changelevel
	intermission_exittime = g_globalvars.time + 5;

	pos = FindIntermission();

// play intermission music
	trap_WriteByte( MSG_ALL, SVC_CDTRACK );
	trap_WriteByte( MSG_ALL, 3 );

	trap_WriteByte ( MSG_ALL, SVC_INTERMISSION );
	trap_WriteCoord( MSG_ALL, pos->s.v.origin[0] );
	trap_WriteCoord( MSG_ALL, pos->s.v.origin[1] );
	trap_WriteCoord( MSG_ALL, pos->s.v.origin[2] );
	trap_WriteAngle( MSG_ALL, pos->mangle[0] );
	trap_WriteAngle( MSG_ALL, pos->mangle[1] );
	trap_WriteAngle( MSG_ALL, pos->mangle[2] );

	other = find( world, FOFS( s.v.classname ), "player" );
	while ( other )
	{
		other->s.v.takedamage = DAMAGE_NO;
		other->s.v.solid = SOLID_NOT;
		other->s.v.movetype = MOVETYPE_NONE;
		other->s.v.modelindex = 0;
		other = find( other, FOFS( s.v.classname ), "player" );
	}
}

void changelevel_touch()
{
	//gedict_t*    pos;

	if ( strneq( other->s.v.classname, "player" ) )
		return;

// if "noexit" is set, blow up the player trying to leave
//ZOID, 12-13-96, noexit isn't supported in QW.  Overload samelevel
//      if ((cvar("noexit") == 1) || ((cvar("noexit") == 2) && (mapname != "start")))
	if ( ( trap_cvar( "samelevel" ) == 2 )
	     || ( ( trap_cvar( "samelevel" ) == 3 )
		  && ( strneq( g_globalvars.mapname, "start" ) ) ) )
	{
		T_Damage( other, self, self, 50000 );
		return;
	}

	G_bprint( PRINT_HIGH, "%s exited the level\n", other->s.v.netname );

	strcpy( nextmap, self->map );
// nextmap = self.map;
	activator = other;
	SUB_UseTargets();

	self->s.v.touch = ( func_t ) SUB_Null;

// we can't move people right now, because touch functions are called
// in the middle of C movement code, so set a think g_globalvars.time to do it
	self->s.v.think = ( func_t ) execute_changelevel;
	self->s.v.nextthink = g_globalvars.time + 0.1;
}

/*QUAKED trigger_changelevel (0.5 0.5 0.5) ? NO_INTERMISSION
When the player touches this, he gets sent to the map listed in the "map" variable.  Unless the NO_INTERMISSION flag is set, the view will go to the info_intermission spot and display stats.
*/
void SP_trigger_changelevel()
{
	if ( !self->map )
		G_Error( "chagnelevel trigger doesn't have map" );

	InitTrigger();
	self->s.v.touch = ( func_t ) changelevel_touch;
}

/*
=============================================================================

    PLAYER GAME EDGE FUNCTIONS

=============================================================================
*/

void            set_suicide_frame();

// called by ClientKill and DeadThink
void respawn()
{
	// make a copy of the dead body for appearances sake
	CopyToBodyQue( self );
	// set default spawn parms
	SetNewParms();
	// respawn              
	PutClientInServer();
}


/*
============
ClientKill

Player entered the suicide command
============
*/
void ClientKill()
{
	G_bprint( PRINT_MEDIUM, "%s suicides\n", self->s.v.netname );

	set_suicide_frame();
	self->s.v.modelindex = modelindex_player;
	logfrag( self, self );
	self->s.v.frags -= 2;	// extra penalty

	respawn();
}

float CheckSpawnPoint( vec3_t v )
{
	return false;
}

/*
============
SelectSpawnPoint

Returns the entity to spawn at
============
*/
gedict_t       *SelectSpawnPoint()
{
	gedict_t       *spot;
	gedict_t       *thing;
	int             numspots, totalspots;
	float           pcount;

// float   rs;
	gedict_t       *spots;

	numspots = 0;
	totalspots = 0;

// testinfo_player_start is only found in regioned levels
	spot = find( world, FOFS( s.v.classname ), "testplayerstart" );
	if ( spot )
		return spot;

// choose a info_player_deathmatch point

// ok, find all spots that don't have players nearby

	spots = world;
	spot = find( world, FOFS( s.v.classname ), "info_player_deathmatch" );
	while ( spot )
	{
		totalspots++;

		thing = findradius( world, spot->s.v.origin, 84 );
		pcount = 0;
		while ( thing )
		{
			if ( streq( thing->s.v.classname, "player" ) )
				pcount = pcount + 1;
			thing = findradius( thing, spot->s.v.origin, 84 );
		}
		if ( pcount == 0 )
		{
			spot->s.v.goalentity = EDICT_TO_PROG( spots );
			spots = spot;
			numspots++;
		}
		// Get the next spot in the chain
		spot = find( spot, FOFS( s.v.classname ), "info_player_deathmatch" );
	}

	totalspots--;
	if ( !numspots )
	{
		// ack, they are all full, just pick one at random
//  bprint (PRINT_HIGH, "Ackk! All spots are full. Selecting random spawn spot\n");
		totalspots = ( ( g_random() * totalspots ) );
		spot = find( world, FOFS( s.v.classname ), "info_player_deathmatch" );
		while ( totalspots > 0 )
		{
			totalspots--;
			spot =
			    find( spot, FOFS( s.v.classname ), "info_player_deathmatch" );
		}
		return spot;
	}
// We now have the number of spots available on the map in numspots

	// Generate a random number between 1 and numspots

	numspots = numspots - 1;

	numspots = g_random() * numspots;

	spot = spots;
	while ( numspots > 0 )
	{
		spot = PROG_TO_EDICT( spot->s.v.goalentity );
		numspots = numspots - 1;
	}
	return spot;

}

////////////////
// GlobalParams:
// time
// self
// params
///////////////
void ClientConnect()
{
	G_bprint( PRINT_HIGH, "%s entered the game\n", self->s.v.netname );

// a client connecting during an intermission can cause problems
 	if (intermission_running)
		GotoNextMap ();

}

////////////////
// GlobalParams:
// time
// self
// called after ClientConnect
///////////////
void PutClientInServer()
{

	gedict_t       *spot;
	vec3_t          v;
	int             items;
	char            s[20];

	self->s.v.classname = "player";
	self->s.v.health = 100;
	self->s.v.takedamage = DAMAGE_AIM;
	self->s.v.solid = SOLID_SLIDEBOX;
	self->s.v.movetype = MOVETYPE_WALK;
	self->show_hostile = 0;
	self->s.v.max_health = 100;
	self->s.v.flags = FL_CLIENT;
	self->air_finished = g_globalvars.time + 12;
	self->dmg = 2;		// initial water damage
	self->super_damage_finished = 0;
	self->radsuit_finished = 0;
	self->invisible_finished = 0;
	self->invincible_finished = 0;
	self->s.v.effects = 0;
	self->invincible_time = 0;
//	self->maxspeed =200;
//	self->gravity=0;
	DecodeLevelParms();

	W_SetCurrentAmmo();

	self->attack_finished = g_globalvars.time;
	self->th_pain = player_pain;
	self->th_die = PlayerDie;

	self->s.v.deadflag = DEAD_NO;
// paustime is set by teleporters to keep the player from moving a while
	self->pausetime = 0;

	spot = SelectSpawnPoint();
	VectorCopy( spot->s.v.origin, self->s.v.origin );
	self->s.v.origin[2] += 1;
	VectorCopy( spot->s.v.angles, self->s.v.angles );

	self->s.v.fixangle = true;


// oh, this is a hack!
	setmodel( self, "progs/eyes.mdl" );
	modelindex_eyes = self->s.v.modelindex;

	setmodel( self, "progs/player.mdl" );
	modelindex_player = self->s.v.modelindex;

	setsize( self, PASSVEC3( VEC_HULL_MIN ), PASSVEC3( VEC_HULL_MAX ) );
	SetVector( self->s.v.view_ofs, 0, 0, 22 );
	SetVector( self->s.v.velocity, 0, 0, 0 );

	player_stand1();

	makevectors( self->s.v.angles );
	VectorScale( g_globalvars.v_forward, 20, v );
	VectorAdd( v, self->s.v.origin, v );
	spawn_tfog( v );

	spawn_tdeath( self->s.v.origin, self );

	// Set Rocket Jump Modifiers
	infokey( world, "rj", s, sizeof( s ) );
	if ( atof( s ) != 0 )
	{
		rj = atof( s );
	}

	if ( deathmatch == 4 )
	{
		self->s.v.ammo_shells = 0;
		infokey( world, "axe", s, sizeof( s ) );
		if ( atof( s ) == 0 )
		{
			self->s.v.ammo_nails   = 255;
			self->s.v.ammo_shells  = 255;
			self->s.v.ammo_rockets = 255;
			self->s.v.ammo_cells   = 255;
			self->s.v.items = ( int ) self->s.v.items | IT_NAILGUN;
			self->s.v.items = ( int ) self->s.v.items | IT_SUPER_NAILGUN;
			self->s.v.items = ( int ) self->s.v.items | IT_SUPER_SHOTGUN;
			self->s.v.items = ( int ) self->s.v.items | IT_ROCKET_LAUNCHER;
//  self.items = self.items | IT_GRENADE_LAUNCHER;
			self->s.v.items = ( int ) self->s.v.items | IT_LIGHTNING;
		}
		self->s.v.items -=
		    ( ( int ) self->s.v.items & ( IT_ARMOR1 | IT_ARMOR2 | IT_ARMOR3 ) ) -
		    IT_ARMOR3;
		self->s.v.armorvalue = 200;
		self->s.v.armortype = 0.8;
		self->s.v.health = 250;
		self->s.v.items = ( int ) self->s.v.items | IT_INVULNERABILITY;
		self->invincible_time = 1;
		self->invincible_finished = g_globalvars.time + 3;
	}

	if ( deathmatch == 5 )
	{
		self->s.v.ammo_nails   = 80;
		self->s.v.ammo_shells  = 30;
		self->s.v.ammo_rockets = 10;
		self->s.v.ammo_cells   = 30;
		items = self->s.v.items;
		items |= IT_NAILGUN;
		items |= IT_SUPER_NAILGUN;
		items |= IT_SUPER_SHOTGUN;
		items |= IT_ROCKET_LAUNCHER;
		items |= IT_GRENADE_LAUNCHER;
		items |= IT_LIGHTNING;
		items -= ( items & ( IT_ARMOR1 | IT_ARMOR2 | IT_ARMOR3 ) ) - IT_ARMOR3;
		self->s.v.armorvalue = 200;
		self->s.v.armortype = 0.8;
		self->s.v.health = 200;
		self->s.v.items = items | IT_INVULNERABILITY;
		self->invincible_time = 1;
		self->invincible_finished = g_globalvars.time + 3;
	}
}

/*
===============================================================================

RULES

===============================================================================
*/

/*
go to the next level for deathmatch
*/
void NextLevel()
{
	gedict_t       *o;

	if (  nextmap[0] )
		return;		// already done

	if ( streq( g_globalvars.mapname, "start" ) )
	{
		if ( !trap_cvar( "registered" ) )
		{
			strcpy( g_globalvars.mapname, "e1m1" );

		} else if ( !( ( int ) ( g_globalvars.serverflags ) & 1 ) )
		{
			strcpy( g_globalvars.mapname, "e1m1" );
			g_globalvars.serverflags =
			    ( int ) ( g_globalvars.serverflags ) | 1;

		} else if ( !( ( int ) ( g_globalvars.serverflags ) & 2 ) )
		{
			strcpy( g_globalvars.mapname, "e2m1" );
			g_globalvars.serverflags =
			    ( int ) ( g_globalvars.serverflags ) | 2;

		} else if ( !( ( int ) ( g_globalvars.serverflags ) & 4 ) )
		{
			strcpy( g_globalvars.mapname, "e3m1" );
			g_globalvars.serverflags =
			    ( int ) ( g_globalvars.serverflags ) | 4;

		} else if ( !( ( int ) ( g_globalvars.serverflags ) & 8 ) )
		{
			strcpy( g_globalvars.mapname, "e4m1" );
			g_globalvars.serverflags =
			    ( int ) ( g_globalvars.serverflags ) - 7;
		}

		o = spawn();
		o->map = g_globalvars.mapname;
	} else
	{
		// find a trigger changelevel
		o = find( world, FOFS( s.v.classname ), "trigger_changelevel" );
		if ( !o || streq( g_globalvars.mapname, "start" ) )
		{		// go back to same map if no trigger_changelevel
			o = spawn();
			o->map = g_globalvars.mapname;
		}
	}

	strcpy( nextmap, o->map );

	if ( o->s.v.nextthink < g_globalvars.time )
	{
		o->s.v.think = ( func_t ) execute_changelevel;
		o->s.v.nextthink = g_globalvars.time + 0.1;
	}
}

/*
============
CheckRules

Exit deathmatch games upon conditions
============
*/
void CheckRules()
{
	if ( timelimit && g_globalvars.time >= timelimit )
		NextLevel();

	if ( fraglimit && self->s.v.frags >= fraglimit )
		NextLevel();
}

//============================================================================
void PlayerDeathThink()
{
// gedict_t*    old_self;
	float           forward;

	if ( ( ( int ) ( self->s.v.flags ) ) & FL_ONGROUND )
	{
		forward = vlen( self->s.v.velocity );
		forward = forward - 20;
		if ( forward <= 0 )
			SetVector( self->s.v.velocity, 0, 0, 0 );
		else
		{
			VectorNormalize( self->s.v.velocity );
			VectorScale( self->s.v.velocity, forward, self->s.v.velocity );
		}
	}
// wait for all buttons released
	if ( self->s.v.deadflag == DEAD_DEAD )
	{
		if ( self->s.v.button2 || self->s.v.button1 || self->s.v.button0 )
			return;
		self->s.v.deadflag = DEAD_RESPAWNABLE;
		return;
	}
// wait for any button down
	if ( !self->s.v.button2 && !self->s.v.button1 && !self->s.v.button0 )
		return;

	self->s.v.button0 = 0;
	self->s.v.button1 = 0;
	self->s.v.button2 = 0;
	respawn();
}


void PlayerJump()
{
	//vec3_t start, end;

	if ( ( ( int ) ( self->s.v.flags ) ) & FL_WATERJUMP )
		return;

	if ( self->s.v.waterlevel >= 2 )
	{
// play swiming sound
		if ( self->swim_flag < g_globalvars.time )
		{
			self->swim_flag = g_globalvars.time + 1;
			if ( g_random() < 0.5 )
				sound( self, CHAN_BODY, "misc/water1.wav", 1,
					    ATTN_NORM );
			else
				sound( self, CHAN_BODY, "misc/water2.wav", 1,
					    ATTN_NORM );
		}

		return;
	}

	if ( !( ( ( int ) ( self->s.v.flags ) ) & FL_ONGROUND ) )
		return;

	if ( !( ( ( int ) ( self->s.v.flags ) ) & FL_JUMPRELEASED ) )
		return;		// don't pogo stick

	self->s.v.flags -= ( ( ( int ) ( self->s.v.flags ) ) & FL_JUMPRELEASED );
	self->s.v.button2 = 0;

// player jumping sound
	sound( self, CHAN_BODY, "player/plyrjmp8.wav", 1, ATTN_NORM );
}


/*
===========
WaterMove

============
*/

void WaterMove()
{
//dprint (ftos(self->s.v.waterlevel));
	if ( self->s.v.movetype == MOVETYPE_NOCLIP )
		return;

	if ( self->s.v.health < 0 )
		return;

	if ( self->s.v.waterlevel != 3 )
	{
		if ( self->air_finished < g_globalvars.time )
			sound( self, CHAN_VOICE, "player/gasp2.wav", 1, ATTN_NORM );
		else if ( self->air_finished < g_globalvars.time + 9 )
			sound( self, CHAN_VOICE, "player/gasp1.wav", 1, ATTN_NORM );

		self->air_finished = g_globalvars.time + 12;
		self->dmg = 2;

	} else if ( self->air_finished < g_globalvars.time )
	{			// drown!
		if ( self->pain_finished < g_globalvars.time )
		{
			self->dmg = self->dmg + 2;
			if ( self->dmg > 15 )
				self->dmg = 10;

			T_Damage( self, world, world, self->dmg );
			self->pain_finished = g_globalvars.time + 1;
		}
	}

	if ( !self->s.v.waterlevel )
	{
		if ( ( ( int ) ( self->s.v.flags ) ) & FL_INWATER )
		{
			// play leave water sound
			sound( self, CHAN_BODY, "misc/outwater.wav", 1, ATTN_NORM );
			self->s.v.flags -= FL_INWATER;
		}
		return;
	}

	if ( self->s.v.watertype == CONTENT_LAVA )
	{			// do damage
		if ( self->dmgtime < g_globalvars.time )
		{
			if ( self->radsuit_finished > g_globalvars.time )
				self->dmgtime = g_globalvars.time + 1;
			else
				self->dmgtime = g_globalvars.time + 0.2;

			T_Damage( self, world, world, 10 * self->s.v.waterlevel );
		}

	} else if ( self->s.v.watertype == CONTENT_SLIME )
	{			// do damage
		if ( self->dmgtime < g_globalvars.time
		     && self->radsuit_finished < g_globalvars.time )
		{
			self->dmgtime = g_globalvars.time + 1;
			T_Damage( self, world, world, 4 * self->s.v.waterlevel );
		}
	}

	if ( !( ( ( int ) ( self->s.v.flags ) ) & FL_INWATER ) )
	{

// player enter water sound

		if ( self->s.v.watertype == CONTENT_LAVA )
			sound( self, CHAN_BODY, "player/inlava.wav", 1, ATTN_NORM );
		if ( self->s.v.watertype == CONTENT_WATER )
			sound( self, CHAN_BODY, "player/inh2o.wav", 1, ATTN_NORM );
		if ( self->s.v.watertype == CONTENT_SLIME )
			sound( self, CHAN_BODY, "player/slimbrn2.wav", 1,
				    ATTN_NORM );

		self->s.v.flags += FL_INWATER;
		self->dmgtime = 0;
	}
}

void CheckWaterJump()
{
	vec3_t          start, end;

// check for a jump-out-of-water
	makevectors( self->s.v.angles );
	
	VectorCopy( self->s.v.origin, start );	//start = self->s.v.origin;
	start[2] = start[2] + 8;
	g_globalvars.v_forward[2] = 0;

	VectorNormalize( g_globalvars.v_forward );
	VectorScale( g_globalvars.v_forward, 24, end );
	VectorAdd( start, end, end );	//end = start + v_forward*24;

	traceline( PASSVEC3( start ), PASSVEC3( end ), true, self );

	if ( g_globalvars.trace_fraction < 1 )
	{			// solid at waist
		start[2] = start[2] + self->s.v.maxs[2] - 8;
		VectorScale( g_globalvars.v_forward, 24, end );
		VectorAdd( start, end, end );	//end = start + v_forward*24;
		VectorScale( g_globalvars.trace_plane_normal, -50, self->s.v.movedir );	//self->s.v.movedir = trace_plane_normal * -50;

		traceline( PASSVEC3( start ), PASSVEC3( end ), true, self );

		if ( g_globalvars.trace_fraction == 1 )
		{		// open at eye level
			self->s.v.flags = ( ( int ) ( self->s.v.flags ) ) | FL_WATERJUMP;
			self->s.v.velocity[2] = 225;
			self->s.v.flags -=
			    ( ( ( int ) ( self->s.v.flags ) ) & FL_JUMPRELEASED );
			self->s.v.teleport_time = g_globalvars.time + 2;	// safety net
			return;
		}
	}
}

////////////////
// GlobalParams:
// self
///////////////
void ClientDisconnect()
{
	G_bprint( PRINT_HIGH, "%s left the game with %.0f frags\n", self->s.v.netname,
		  self->s.v.frags );

	sound( self, CHAN_BODY, "player/tornoff2.wav", 1, ATTN_NONE );
	set_suicide_frame();
}

////////////////
// GlobalParams:
// time
// frametime
// self
///////////////
/*
================
PlayerPreThink

Called every frame before physics are run
================
*/

void PlayerPreThink()
{
//  float   mspeed, aspeed;
//  float   r;

	if ( intermission_running )
	{
		IntermissionThink();	// otherwise a button could be missed between
		return;		// the think tics
	}

	if ( self->s.v.view_ofs[0] == 0 && self->s.v.view_ofs[1] == 0
	     && self->s.v.view_ofs[2] == 0 )
		return;		// intermission or finale

	makevectors( self->s.v.v_angle );	// is this still used

	self->deathtype = "";

	CheckRules();
	WaterMove();
/*
 if (self->s.v.waterlevel == 2)
  CheckWaterJump ();
*/

	if ( self->s.v.deadflag >= DEAD_DEAD )
	{
		PlayerDeathThink();
		return;
	}

	if ( self->s.v.deadflag == DEAD_DYING )
		return;		// dying, so do nothing

	if ( self->s.v.button2 )
	{
		PlayerJump();
	} else
		self->s.v.flags = ( ( int ) ( self->s.v.flags ) ) | FL_JUMPRELEASED;

// teleporters can force a non-moving pause g_globalvars.time        
	if ( g_globalvars.time < self->pausetime )
		SetVector( self->s.v.velocity, 0, 0, 0 );

	if ( g_globalvars.time > self->attack_finished && self->s.v.currentammo == 0
	     && self->s.v.weapon != IT_AXE )
	{
		self->s.v.weapon = W_BestWeapon();
		W_SetCurrentAmmo();
	}

}

/////////////////////////////////////////////////////////////////
/*
================
CheckPowerups

Check for turning off powerups
================
*/
void CheckPowerups()
{
	if ( self->s.v.health <= 0 )
		return;

// invisibility
	if ( self->invisible_finished )
	{
// sound and screen flash when items starts to run out
		if ( self->invisible_sound < g_globalvars.time )
		{
			sound( self, CHAN_AUTO, "items/inv3.wav", 0.5, ATTN_IDLE );
			self->invisible_sound =
			    g_globalvars.time + ( ( g_random() * 3 ) + 1 );
		}


		if ( self->invisible_finished < g_globalvars.time + 3 )
		{
			if ( self->invisible_time == 1 )
			{
				G_sprint( self, PRINT_HIGH,
					  "Ring of Shadows magic is fading\n" );
				stuffcmd( self, "bf\n" );
				sound( self, CHAN_AUTO, "items/inv2.wav", 1,
					    ATTN_NORM );
				self->invisible_time = g_globalvars.time + 1;
			}

			if ( self->invisible_time < g_globalvars.time )
			{
				self->invisible_time = g_globalvars.time + 1;
				stuffcmd( self, "bf\n" );
			}
		}

		if ( self->invisible_finished < g_globalvars.time )
		{		// just stopped
			self->s.v.items -= IT_INVISIBILITY;
			self->invisible_finished = 0;
			self->invisible_time = 0;
		}
		// use the eyes
		self->s.v.frame = 0;
		self->s.v.modelindex = modelindex_eyes;
	} else
		self->s.v.modelindex = modelindex_player;	// don't use eyes

// invincibility
	if ( self->invincible_finished )
	{
// sound and screen flash when items starts to run out
		if ( self->invincible_finished < g_globalvars.time + 3 )
		{
			if ( self->invincible_time == 1 )
			{
				G_sprint( self, PRINT_HIGH,
					  "Protection is almost burned out\n" );
				stuffcmd( self, "bf\n" );
				sound( self, CHAN_AUTO, "items/protect2.wav", 1,
					    ATTN_NORM );
				self->invincible_time = g_globalvars.time + 1;
			}

			if ( self->invincible_time < g_globalvars.time )
			{
				self->invincible_time = g_globalvars.time + 1;
				stuffcmd( self, "bf\n" );
			}
		}

		if ( self->invincible_finished < g_globalvars.time )
		{		// just stopped
			self->s.v.items -= IT_INVULNERABILITY;
			self->invincible_time = 0;
			self->invincible_finished = 0;
		}
		if ( self->invincible_finished > g_globalvars.time )
		{
			self->s.v.effects = ( int ) self->s.v.effects | EF_DIMLIGHT;
			self->s.v.effects = ( int ) self->s.v.effects | EF_RED;
		} else
		{
			self->s.v.effects -= ( ( int ) self->s.v.effects & EF_DIMLIGHT );
			self->s.v.effects -= ( ( int ) self->s.v.effects & EF_RED );
		}
	}
// super damage
	if ( self->super_damage_finished )
	{
// sound and screen flash when items starts to run out

		if ( self->super_damage_finished < g_globalvars.time + 3 )
		{
			if ( self->super_time == 1 )
			{
				if ( deathmatch == 4 )
					G_sprint( self, PRINT_HIGH,
						  "OctaPower is wearing off\n" );
				else
					G_sprint( self, PRINT_HIGH,
						  "Quad Damage is wearing off\n" );
				stuffcmd( self, "bf\n" );
				sound( self, CHAN_AUTO, "items/damage2.wav", 1,
					    ATTN_NORM );
				self->super_time = g_globalvars.time + 1;
			}

			if ( self->super_time < g_globalvars.time )
			{
				self->super_time = g_globalvars.time + 1;
				stuffcmd( self, "bf\n" );
			}
		}

		if ( self->super_damage_finished < g_globalvars.time )
		{		// just stopped
			self->s.v.items -= IT_QUAD;
			if ( deathmatch == 4 )
			{
				self->s.v.ammo_cells = 255;
				self->s.v.armorvalue = 1;
				self->s.v.armortype = 0.8;
				self->s.v.health = 100;
			}
			self->super_damage_finished = 0;
			self->super_time = 0;
		}
		if ( self->super_damage_finished > g_globalvars.time )
		{
			self->s.v.effects = ( int ) self->s.v.effects | EF_DIMLIGHT;
			self->s.v.effects = ( int ) self->s.v.effects | EF_BLUE;
		} else
		{
			self->s.v.effects -= ( ( int ) self->s.v.effects & EF_DIMLIGHT );
			self->s.v.effects -= ( ( int ) self->s.v.effects & EF_BLUE );
		}
	}
// suit 
	if ( self->radsuit_finished )
	{
		self->air_finished = g_globalvars.time + 12;	// don't drown

// sound and screen flash when items starts to run out
		if ( self->radsuit_finished < g_globalvars.time + 3 )
		{
			if ( self->rad_time == 1 )
			{
				G_sprint( self, PRINT_HIGH,
					  "Air supply in Biosuit expiring\n" );
				stuffcmd( self, "bf\n" );
				sound( self, CHAN_AUTO, "items/suit2.wav", 1,
					    ATTN_NORM );
				self->rad_time = g_globalvars.time + 1;
			}

			if ( self->rad_time < g_globalvars.time )
			{
				self->rad_time = g_globalvars.time + 1;
				stuffcmd( self, "bf\n" );
			}
		}

		if ( self->radsuit_finished < g_globalvars.time )
		{		// just stopped
			self->s.v.items -= IT_SUIT;
			self->rad_time = 0;
			self->radsuit_finished = 0;
		}
	}

}

void            W_WeaponFrame();

////////////////
// GlobalParams:
// time
// self
///////////////
void PlayerPostThink()
{
//   float   mspeed, aspeed;
	//  float   r;

//dprint ("post think\n");
	if ( self->s.v.view_ofs[0] == 0 && self->s.v.view_ofs[1] == 0
	     && self->s.v.view_ofs[2] == 0 )
		return;		// intermission or finale
	if ( self->s.v.deadflag )
		return;

// check to see if player landed and play landing sound 
	if ( ( self->jump_flag < -300 )
	     && ( ( ( int ) ( self->s.v.flags ) ) & FL_ONGROUND ) )
	{
		if ( self->s.v.watertype == CONTENT_WATER )
			sound( self, CHAN_BODY, "player/h2ojump.wav", 1, ATTN_NORM );
		else if ( self->jump_flag < -650 )
		{
			self->deathtype = "falling";
			T_Damage( self, world, world, 5 );
			sound( self, CHAN_VOICE, "player/land2.wav", 1, ATTN_NORM );
		} else
			sound( self, CHAN_VOICE, "player/land.wav", 1, ATTN_NORM );
	}

	self->jump_flag = self->s.v.velocity[2];

	CheckPowerups();

	W_WeaponFrame();
}

/*
===========
ClientObituary

called when a player dies
============
*/

void ClientObituary( gedict_t * targ, gedict_t * attacker )
{
	float           rnum;
	char           *deathstring = NULL, *deathstring2 = NULL;

//   char * s;
	char            attackerteam[10], targteam[10];

	rnum = g_random();
	//ZOID 12-13-96: self.team doesn't work in QW.  Use keys

	infokey( attacker, "team", attackerteam, sizeof( attackerteam ) );
	infokey( targ, "team", targteam, sizeof( targteam ) );

	if ( streq( targ->s.v.classname, "player" ) )
	{

		if ( deathmatch > 3 )
		{
			if ( streq( targ->deathtype, "selfwater" ) )
			{
				G_bprint( PRINT_MEDIUM, "%s electrocutes himself.\n ",
					  targ->s.v.netname );
				targ->s.v.frags = targ->s.v.frags - 1;
				return;
			}
		}

		if ( streq( attacker->s.v.classname, "teledeath" ) )
		{
			G_bprint( PRINT_MEDIUM, "%s was telefragged by %s\n",
				  targ->s.v.netname,
				  PROG_TO_EDICT( attacker->s.v.owner )->s.v.netname );
			logfrag( PROG_TO_EDICT( attacker->s.v.owner ), targ );

			PROG_TO_EDICT( attacker->s.v.owner )->s.v.frags += 1;
			return;
		}

		if ( streq( attacker->s.v.classname, "teledeath2" ) )
		{
			G_bprint( PRINT_MEDIUM, "Satan's power deflects %s's telefrag\n",
				  targ->s.v.netname );

			targ->s.v.frags = targ->s.v.frags - 1;
			logfrag( targ, targ );
			return;
		}
		// double 666 telefrag (can happen often in deathmatch 4)
		if ( streq( attacker->s.v.classname, "teledeath3" ) )
		{
			G_bprint( PRINT_MEDIUM,
				  "%s was telefragged by %s's Satan's power\n",
				  targ->s.v.netname,
				  PROG_TO_EDICT( attacker->s.v.owner )->s.v.netname );
			targ->s.v.frags = targ->s.v.frags - 1;
			logfrag( targ, targ );
			return;
		}


		if ( streq( targ->deathtype, "squish" ) )
		{
			if ( teamplay && streq( targteam, attackerteam )
			     && strneq( attackerteam, "" ) && targ != attacker )
			{
				logfrag( attacker, attacker );
				attacker->s.v.frags = attacker->s.v.frags - 1;
				G_bprint( PRINT_MEDIUM, "%s squished a teammate\n",
					  attacker->s.v.netname );
				return;
			} else if ( streq( attacker->s.v.classname, "player" )
				    && attacker != targ )
			{
				G_bprint( PRINT_MEDIUM, "%s squishes %s\n",
					  attacker->s.v.netname, targ->s.v.netname );
				logfrag( attacker, targ );
				attacker->s.v.frags = attacker->s.v.frags + 1;
				return;
			} else
			{
				logfrag( targ, targ );
				targ->s.v.frags = targ->s.v.frags - 1;	// killed self
				G_bprint( PRINT_MEDIUM, "%s was squished\n",
					  targ->s.v.netname );
				return;
			}
		}

		if ( streq( attacker->s.v.classname, "player" ) )
		{
			if ( targ == attacker )
			{
				// killed self
				logfrag( attacker, attacker );
				attacker->s.v.frags = attacker->s.v.frags - 1;
				G_bprint( PRINT_MEDIUM, targ->s.v.netname );
				if ( streq( targ->deathtype, "grenade" ) )
					G_bprint( PRINT_MEDIUM,
						  " tries to put the pin back in\n" );
				else if ( streq( targ->deathtype, "rocket" ) )
					G_bprint( PRINT_MEDIUM,
						  " becomes bored with life\n" );
				else if ( targ->s.v.weapon == 64
					  && targ->s.v.waterlevel > 1 )
				{
					if ( targ->s.v.watertype == CONTENT_SLIME )
						G_bprint( PRINT_MEDIUM,
							  " discharges into the slime\n" );
					else if ( targ->s.v.watertype == CONTENT_LAVA )
						G_bprint( PRINT_MEDIUM,
							  " discharges into the lava\n" );
					else
						G_bprint( PRINT_MEDIUM,
							  " discharges into the water.\n" );
				} else
					G_bprint( PRINT_MEDIUM,
						  " becomes bored with life\n" );
				return;
			} else if ( ( teamplay == 2 ) && streq( targteam, attackerteam )
				    && strneq( attackerteam, "" ) )
			{
				if ( rnum < 0.25 )
					deathstring = " mows down a teammate\n";
				else if ( rnum < 0.50 )
					deathstring = " checks his glasses\n";
				else if ( rnum < 0.75 )
					deathstring = " gets a frag for the other team\n";
				else
					deathstring = " loses another friend\n";
				G_bprint( PRINT_MEDIUM, "%s%s", attacker->s.v.netname,
					  deathstring );
				attacker->s.v.frags = attacker->s.v.frags - 1;
				//ZOID 12-13-96:  killing a teammate logs as suicide
				logfrag( attacker, attacker );
				return;
			} else
			{
				logfrag( attacker, targ );
				attacker->s.v.frags = attacker->s.v.frags + 1;

				rnum = attacker->s.v.weapon;
				if ( streq( targ->deathtype, "nail" ) )
				{
					deathstring = " was nailed by ";
					deathstring2 = "\n";
				} else if ( streq( targ->deathtype, "supernail" ) )
				{
					deathstring = " was punctured by ";
					deathstring2 = "\n";
				} else if ( streq( targ->deathtype, "grenade" ) )
				{
					deathstring = " eats ";
					deathstring2 = "'s pineapple\n";
					if ( targ->s.v.health < -40 )
					{
						deathstring = " was gibbed by ";
						deathstring2 = "'s grenade\n";
					}
				} else if ( streq( targ->deathtype, "rocket" ) )
				{
					if ( attacker->super_damage_finished > 0
					     && targ->s.v.health < -40 )
					{
						rnum = g_random();
						if ( rnum < 0.3 )
							deathstring =
							    " was brutalized by ";
						else if ( rnum < 0.6 )
							deathstring = " was smeared by ";
						else
						{
							G_bprint( PRINT_MEDIUM,
								  "%s rips %s a new one\n",
								  attacker->s.v.netname,
								  targ->s.v.netname );
							return;
						}
						deathstring2 = "'s quad rocket\n";
					} else
					{
						deathstring = " rides ";
						deathstring2 = "'s rocket\n";
						if ( targ->s.v.health < -40 )
						{
							deathstring = " was gibbed by ";
							deathstring2 = "'s rocket\n";
						}
					}
				} else if ( rnum == IT_AXE )
				{
					deathstring = " was ax-murdered by ";
					deathstring2 = "\n";
				} else if ( rnum == IT_SHOTGUN )
				{
					deathstring = " chewed on ";
					deathstring2 = "'s boomstick\n";
				} else if ( rnum == IT_SUPER_SHOTGUN )
				{
					deathstring = " ate 2 loads of ";
					deathstring2 = "'s buckshot\n";
				} else if ( rnum == IT_LIGHTNING )
				{
					deathstring = " accepts ";
					if ( attacker->s.v.waterlevel > 1 )
						deathstring2 = "'s discharge\n";
					else
						deathstring2 = "'s shaft\n";
				}
				G_bprint( PRINT_MEDIUM, "%s%s%s%s", targ->s.v.netname,
					  deathstring,
					  attacker->s.v.netname, deathstring2 );
			}
			return;
		} else
		{
			logfrag( targ, targ );
			targ->s.v.frags = targ->s.v.frags - 1;	// killed self
			rnum = targ->s.v.watertype;

			G_bprint( PRINT_MEDIUM, targ->s.v.netname );
			if ( rnum == -3 )
			{
				if ( g_random() < 0.5 )
					G_bprint( PRINT_MEDIUM,
						  " sleeps with the fishes\n" );
				else
					G_bprint( PRINT_MEDIUM, " sucks it down\n" );
				return;
			} else if ( rnum == -4 )
			{
				if ( g_random() < 0.5 )
					G_bprint( PRINT_MEDIUM,
						  " gulped a load of slime\n" );
				else
					G_bprint( PRINT_MEDIUM,
						  " can't exist on slime alone\n" );
				return;
			} else if ( rnum == -5 )
			{
				if ( targ->s.v.health < -15 )
				{
					G_bprint( PRINT_MEDIUM, " burst into flames\n" );
					return;
				}
				if ( g_random() < 0.5 )
					G_bprint( PRINT_MEDIUM,
						  " turned into hot slag\n" );
				else
					G_bprint( PRINT_MEDIUM,
						  " visits the Volcano God\n" );
				return;
			}

			if ( streq( attacker->s.v.classname, "explo_box" ) )
			{
				G_bprint( PRINT_MEDIUM, " blew up\n" );
				return;
			}
			if ( streq( targ->deathtype, "falling" ) )
			{
				G_bprint( PRINT_MEDIUM, " fell to his death\n" );
				return;
			}
			if ( streq( targ->deathtype, "nail" )
			     || streq( targ->deathtype, "supernail" ) )
			{
				G_bprint( PRINT_MEDIUM, " was spiked\n" );
				return;
			}
			if ( streq( targ->deathtype, "laser" ) )
			{
				G_bprint( PRINT_MEDIUM, " was zapped\n" );
				return;
			}
			if ( streq( attacker->s.v.classname, "fireball" ) )
			{
				G_bprint( PRINT_MEDIUM, " ate a lavaball\n" );
				return;
			}
			if ( streq( attacker->s.v.classname, "trigger_changelevel" ) )
			{
				G_bprint( PRINT_MEDIUM, " tried to leave\n" );
				return;
			}

			G_bprint( PRINT_MEDIUM, " died\n" );
		}
	}
}

