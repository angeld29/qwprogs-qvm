/*
 triggers.qc

 trigger functions

 Copyright (C) 1996-1997  Id Software, Inc.

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

 See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to:

  Free Software Foundation, Inc.
  59 Temple Place - Suite 330
  Boston, MA  02111-1307, USA

*/
#include "g_local.h"
gedict_t       *stemp, *otemp, *s, *old;


void trigger_reactivate()
{
	self->s.v.solid = SOLID_TRIGGER;
}

//=============================================================================

#define SPAWNFLAG_NOMESSAGE  1
#define SPAWNFLAG_NOTOUCH  1

// the wait g_globalvars.time has passed, so set back up for another activation
void multi_wait()
{
	if ( self->s.v.max_health )
	{
		self->s.v.health = self->s.v.max_health;
		self->s.v.takedamage = DAMAGE_YES;
		self->s.v.solid = SOLID_BBOX;
	}
}


// the trigger was just touched/killed/used
// self->s.v.enemy should be set to the activator so it can be held through a delay
// so wait for the delay g_globalvars.time before firing
void multi_trigger()
{
	if ( self->s.v.nextthink > g_globalvars.time )
	{
		return;		// allready been triggered
	}

	if ( streq( self->s.v.classname, "trigger_secret" ) )
	{
		if ( strneq( PROG_TO_EDICT( self->s.v.enemy )->s.v.classname, "player" ) )
			return;
		g_globalvars.found_secrets = g_globalvars.found_secrets + 1;
		trap_WriteByte( MSG_ALL, SVC_FOUNDSECRET );
	}

	if ( self->s.v.noise )
		trap_sound( self, CHAN_VOICE, self->s.v.noise, 1, ATTN_NORM );

// don't trigger again until reset
	self->s.v.takedamage = DAMAGE_NO;

	activator = PROG_TO_EDICT( self->s.v.enemy );

	SUB_UseTargets();

	if ( self->wait > 0 )
	{
		self->s.v.think = ( func_t ) multi_wait;
		self->s.v.nextthink = g_globalvars.time + self->wait;
	} else
	{			// we can't just ent_remove(self) here, because this is a touch function
		// called wheil C code is looping through area links...
		self->s.v.touch = ( func_t ) SUB_Null;
		self->s.v.nextthink = g_globalvars.time + 0.1;
		self->s.v.think = ( func_t ) SUB_Remove;
	}
}

void multi_killed()
{
	self->s.v.enemy = EDICT_TO_PROG( damage_attacker );
	multi_trigger();
}

void multi_use()
{
	self->s.v.enemy = EDICT_TO_PROG( activator );
	multi_trigger();
}

void multi_touch()
{
	if ( !other->s.v.classname )
		return;
	if ( strneq( other->s.v.classname, "player" ) )
		return;

// if the trigger has an angles field, check player's facing direction
	if ( self->s.v.movedir[0] != 0 && self->s.v.movedir[1] != 0
	     && self->s.v.movedir[2] != 0 )
	{
		makevectors( other->s.v.angles );
		if ( DotProduct( g_globalvars.v_forward, self->s.v.movedir ) < 0 )
			return;	// not facing the right way
	}

	self->s.v.enemy = EDICT_TO_PROG( other );
	multi_trigger();
}

/*QUAKED trigger_multiple (.5 .5 .5) ? notouch
Variable sized repeatable trigger.  Must be targeted at one or more entities.  If "health" is set, the trigger must be killed to activate each g_globalvars.time.
If "delay" is set, the trigger waits some g_globalvars.time after activating before firing.
"wait" : Seconds between triggerings. (.2 default)
If notouch is set, the trigger is only fired by other entities, not by touching.
NOTOUCH has been obsoleted by trigger_relay!
sounds
1) secret
2) beep beep
3) large switch
4)
set "message" to text string
*/
void SP_trigger_multiple()
{
	if ( self->s.v.sounds == 1 )
	{
		trap_precache_sound( "misc/secret.wav" );
		self->s.v.noise = "misc/secret.wav";
	} else if ( self->s.v.sounds == 2 )
	{
		trap_precache_sound( "misc/talk.wav" );
		self->s.v.noise = "misc/talk.wav";
	} else if ( self->s.v.sounds == 3 )
	{
		trap_precache_sound( "misc/trigger1.wav" );
		self->s.v.noise = "misc/trigger1.wav";
	}

	if ( !self->wait )
		self->wait = 0.2;
	self->s.v.use = ( func_t ) multi_use;

	InitTrigger();

	if ( self->s.v.health )
	{
		if ( ( int ) ( self->s.v.spawnflags ) & SPAWNFLAG_NOTOUCH )
			G_Error( "health and notouch don't make sense\n" );
		self->s.v.max_health = self->s.v.health;
		self->th_die = multi_killed;
		self->s.v.takedamage = DAMAGE_YES;
		self->s.v.solid = SOLID_BBOX;
		trap_setorigin( self, PASSVEC3( self->s.v.origin ) );	// make sure it links into the world
	} else
	{
		if ( !( ( int ) ( self->s.v.spawnflags ) & SPAWNFLAG_NOTOUCH ) )
		{
			self->s.v.touch = ( func_t ) multi_touch;
		}
	}
}


/*QUAKED trigger_once (.5 .5 .5) ? notouch
Variable sized trigger. Triggers once, then removes itself.  You must set the key "target" to the name of another object in the level that has a matching
"targetname".  If "health" is set, the trigger must be killed to activate.
If notouch is set, the trigger is only fired by other entities, not by touching.
if "killtarget" is set, any objects that have a matching "target" will be removed when the trigger is fired.
if "angle" is set, the trigger will only fire when someone is facing the direction of the angle.  Use "360" for an angle of 0.
sounds
1) secret
2) beep beep
3) large switch
4)
set "message" to text string
*/
void SP_trigger_once()
{
	self->wait = -1;
	SP_trigger_multiple();
}

//=============================================================================

/*QUAKED trigger_relay (.5 .5 .5) (-8 -8 -8) (8 8 8)
This fixed size trigger cannot be touched, it can only be fired by other events.  It can contain killtargets, targets, delays, and messages.
*/
void SP_trigger_relay()
{
	self->s.v.use = ( func_t ) SUB_UseTargets;
}


//=============================================================================

/*QUAKED trigger_secret (.5 .5 .5) ?
secret counter trigger
sounds
1) secret
2) beep beep
3)
4)
set "message" to text string
*/
void SP_trigger_secret()
{
	g_globalvars.total_secrets = g_globalvars.total_secrets + 1;
	self->wait = -1;
	if ( !self->s.v.message )
		self->s.v.message = "You found a secret area!";
	if ( !self->s.v.sounds )
		self->s.v.sounds = 1;

	if ( self->s.v.sounds == 1 )
	{
		trap_precache_sound( "misc/secret.wav" );
		self->s.v.noise = "misc/secret.wav";
	} else if ( self->s.v.sounds == 2 )
	{
		trap_precache_sound( "misc/talk.wav" );
		self->s.v.noise = "misc/talk.wav";
	}

	SP_trigger_multiple();
}

//=============================================================================


void counter_use()
{
// char* junk;

	self->count = self->count - 1;
	if ( self->count < 0 )
		return;

	if ( self->count != 0 )
	{
		if ( streq( activator->s.v.classname, "player" )
		     && ( ( int ) ( self->s.v.spawnflags ) & SPAWNFLAG_NOMESSAGE ) == 0 )
		{
			if ( self->count >= 4 )
				G_centerprint( activator, "There are more to go..." );
			else if ( self->count == 3 )
				G_centerprint( activator, "Only 3 more to go..." );
			else if ( self->count == 2 )
				G_centerprint( activator, "Only 2 more to go..." );
			else
				G_centerprint( activator, "Only 1 more to go..." );
		}
		return;
	}

	if ( streq( activator->s.v.classname, "player" )
	     && ( ( int ) ( self->s.v.spawnflags ) & SPAWNFLAG_NOMESSAGE ) == 0 )
		G_centerprint( activator, "Sequence completed!" );
	self->s.v.enemy = EDICT_TO_PROG( activator );
	multi_trigger();
}

/*QUAKED trigger_counter (.5 .5 .5) ? nomessage
Acts as an intermediary for an action that takes multiple inputs.

If nomessage is not set, t will print "1 more.. " etc when triggered and "sequence complete" when finished.

After the counter has been triggered "count" g_globalvars.times (default 2), it will fire all of it's targets and remove itself.
*/
void SP_trigger_counter()
{
	self->wait = -1;
	if ( !self->count )
		self->count = 2;

	self->s.v.use = ( func_t ) counter_use;
}


/*
==============================================================================

TELEPORT TRIGGERS

==============================================================================
*/

#define PLAYER_ONLY  1
#define SILENT  2

void play_teleport()
{
	float           v;
	char           *tmpstr;

	v = g_random() * 5;
	if ( v < 1 )
		tmpstr = "misc/r_tele1.wav";
	else if ( v < 2 )
		tmpstr = "misc/r_tele2.wav";
	else if ( v < 3 )
		tmpstr = "misc/r_tele3.wav";
	else if ( v < 4 )
		tmpstr = "misc/r_tele4.wav";
	else
		tmpstr = "misc/r_tele5.wav";

	trap_sound( self, CHAN_VOICE, tmpstr, 1, ATTN_NORM );
	ent_remove( self );
}

void spawn_tfog( vec3_t org )
{
	s = spawn();
	VectorCopy( org, s->s.v.origin );
// s->s.v.origin = org;
	s->s.v.nextthink = g_globalvars.time + 0.2;
	s->s.v.think = ( func_t ) play_teleport;

	trap_WriteByte( MSG_MULTICAST, SVC_TEMPENTITY );
	trap_WriteByte( MSG_MULTICAST, TE_TELEPORT );
	trap_WriteCoord( MSG_MULTICAST, org[0] );
	trap_WriteCoord( MSG_MULTICAST, org[1] );
	trap_WriteCoord( MSG_MULTICAST, org[2] );
	trap_multicast( PASSVEC3( org ), MULTICAST_PHS );
}


void tdeath_touch()
{
	gedict_t       *other2;

	if ( other == PROG_TO_EDICT( self->s.v.owner ) )
		return;

// frag anyone who teleports in on top of an invincible player
	if ( streq( other->s.v.classname, "player" ) )
	{
		if ( other->invincible_finished > g_globalvars.time &&
		     PROG_TO_EDICT( self->s.v.owner )->invincible_finished >
		     g_globalvars.time )
		{
			self->s.v.classname = "teledeath3";
			other->invincible_finished = 0;
			PROG_TO_EDICT( self->s.v.owner )->invincible_finished = 0;
			T_Damage( other, self, self, 50000 );
			other2 = PROG_TO_EDICT( self->s.v.owner );
			self->s.v.owner = EDICT_TO_PROG( other );
			T_Damage( other2, self, self, 50000 );
		}

		if ( other->invincible_finished > g_globalvars.time )
		{
			self->s.v.classname = "teledeath2";
			T_Damage( PROG_TO_EDICT( self->s.v.owner ), self, self, 50000 );
			return;
		}

	}

	if ( other->s.v.health )
	{
		T_Damage( other, self, self, 50000 );
	}
}


void spawn_tdeath( vec3_t org, gedict_t * death_owner )
{

	gedict_t       *death;

	death = spawn();
	death->s.v.classname = "teledeath";
	death->s.v.movetype = MOVETYPE_NONE;
	death->s.v.solid = SOLID_TRIGGER;
	SetVector( death->s.v.angles, 0, 0, 0 );

	trap_setsize( death, death_owner->s.v.mins[0] - 1, death_owner->s.v.mins[1] - 1,
		      death_owner->s.v.mins[2] - 1, death_owner->s.v.maxs[0] + 1,
		      death_owner->s.v.maxs[1] + 1, death_owner->s.v.maxs[2] + 1 );
	trap_setorigin( death, PASSVEC3( org ) );

	death->s.v.touch = ( func_t ) tdeath_touch;
	death->s.v.nextthink = g_globalvars.time + 0.2;
	death->s.v.think = ( func_t ) SUB_Remove;
	death->s.v.owner = EDICT_TO_PROG( death_owner );

	g_globalvars.force_retouch = 2;	// make sure even still objects get hit
}

void teleport_touch()
{
	gedict_t       *t;
	vec3_t          org;

	if ( self->s.v.targetname )
	{
		if ( self->s.v.nextthink < g_globalvars.time )
		{
			return;	// not fired yet
		}
	}

	if ( ( int ) ( self->s.v.spawnflags ) & PLAYER_ONLY )
	{
		if ( strneq( other->s.v.classname, "player" ) )
			return;
	}
// only teleport living creatures
	if ( other->s.v.health <= 0 || other->s.v.solid != SOLID_SLIDEBOX )
		return;

// activator = other;
	SUB_UseTargets();

	//put a tfog where the player was
	spawn_tfog( other->s.v.origin );

	t = find( world, FOFS( s.v.targetname ), self->s.v.target );
	if ( !t )
		G_Error( "couldn't find target" );

// spawn a tfog flash in front of the destination
	makevectors( t->mangle );
	org[0] = t->s.v.origin[0] + 32 * g_globalvars.v_forward[0];
	org[1] = t->s.v.origin[1] + 32 * g_globalvars.v_forward[1];
	org[2] = t->s.v.origin[2] + 32 * g_globalvars.v_forward[2];

	spawn_tfog( org );
	spawn_tdeath( t->s.v.origin, other );

// move the player and lock him down for a little while
	if ( !other->s.v.health )
	{
		VectorCopy( t->s.v.origin, other->s.v.origin );
		other->s.v.velocity[0] =
		    ( g_globalvars.v_forward[0] * other->s.v.velocity[0] ) +
		    ( g_globalvars.v_forward[0] * other->s.v.velocity[1] );
		other->s.v.velocity[1] =
		    ( g_globalvars.v_forward[1] * other->s.v.velocity[0] ) +
		    ( g_globalvars.v_forward[1] * other->s.v.velocity[1] );
		other->s.v.velocity[2] =
		    ( g_globalvars.v_forward[2] * other->s.v.velocity[0] ) +
		    ( g_globalvars.v_forward[2] * other->s.v.velocity[1] );

		//other->s.v.velocity = (v_forward * other->s.v.velocity[0]) + (v_forward * other->s.v.velocity[1]);
		return;
	}

	trap_setorigin( other, PASSVEC3( t->s.v.origin ) );
	VectorCopy( t->mangle, other->s.v.angles );
// other.angles = t.mangle;
	if ( streq( other->s.v.classname, "player" ) )
	{
		other->s.v.fixangle = 1;	// turn this way immediately
		other->s.v.teleport_time = g_globalvars.time + 0.7;
		if ( ( int ) other->s.v.flags & FL_ONGROUND )
			other->s.v.flags = other->s.v.flags - FL_ONGROUND;
		VectorScale( g_globalvars.v_forward, 300, other->s.v.velocity );
//  other->s.v.velocity = v_forward * 300;
	}
	other->s.v.flags -= ( int ) other->s.v.flags & FL_ONGROUND;
}

/*QUAKED info_teleport_destination (.5 .5 .5) (-8 -8 -8) (8 8 32)
This is the destination marker for a teleporter.  It should have a "targetname" field with the same value as a teleporter's "target" field.
*/
void SP_info_teleport_destination()
{
// this does nothing, just serves as a target spot
	VectorCopy( self->s.v.angles, self->mangle );
// self.mangle = self.angles;
	SetVector( self->s.v.angles, 0, 0, 0 );
	self->s.v.model = "";
	self->s.v.origin[2] += 27;
	if ( !self->s.v.targetname )
		G_Error( "no targetname" );
}

void teleport_use()
{
	self->s.v.nextthink = g_globalvars.time + 0.2;
	g_globalvars.force_retouch = 2;	// make sure even still objects get hit
	self->s.v.think = ( func_t ) SUB_Null;
}

/*QUAKED trigger_teleport (.5 .5 .5) ? PLAYER_ONLY SILENT
Any object touching this will be transported to the corresponding info_teleport_destination gedict_t*. You must set the "target" field, and create an object with a "targetname" field that matches.

If the trigger_teleport has a targetname, it will only teleport entities when it has been fired.
*/
void SP_trigger_teleport()
{
	vec3_t          o;

	InitTrigger();
	self->s.v.touch = ( func_t ) teleport_touch;
	// find the destination 
	if ( !self->s.v.target )
		G_Error( "no target" );
	self->s.v.use = ( func_t ) teleport_use;

	if ( !( ( int ) ( self->s.v.spawnflags ) & SILENT ) )
	{
		trap_precache_sound( "ambience/hum1.wav" );
		VectorAdd( self->s.v.mins, self->s.v.maxs, o );
		VectorScale( o, 0.5, o );
		//o = (self.mins + self.maxs)*0.5;
		trap_ambientsound( PASSVEC3( o ), "ambience/hum1.wav", 0.5, ATTN_STATIC );
	}
}

/*
==============================================================================

trigger_setskill

==============================================================================
*/

/*QUAKED trigger_setskill (.5 .5 .5) ?
sets skill level to the value of "message".
Only used on start map.
*/
void SP_trigger_setskill()
{
	ent_remove( self );
}


/*
==============================================================================

ONLY REGISTERED TRIGGERS

==============================================================================
*/

void trigger_onlyregistered_touch()
{
	if ( strneq( other->s.v.classname, "player" ) )
		return;
	if ( self->attack_finished > g_globalvars.time )
		return;

	self->attack_finished = g_globalvars.time + 2;
	if ( trap_cvar( "registered" ) )
	{
		self->s.v.message = "";
		activator = other;
		SUB_UseTargets();
		ent_remove( self );
	} else
	{
		if ( self->s.v.message && strneq( self->s.v.message, "" ) )
		{
			G_centerprint( other, self->s.v.message );
			trap_sound( other, CHAN_BODY, "misc/talk.wav", 1, ATTN_NORM );
		}
	}
}

/*QUAKED trigger_onlyregistered (.5 .5 .5) ?
Only fires if playing the registered version, otherwise prints the message
*/
void SP_trigger_onlyregistered()
{
	trap_precache_sound( "misc/talk.wav" );
	InitTrigger();
	self->s.v.touch = ( func_t ) trigger_onlyregistered_touch;
}

//============================================================================

void hurt_on()
{
	self->s.v.solid = SOLID_TRIGGER;
	self->s.v.nextthink = -1;
}

void hurt_touch()
{
	if ( other->s.v.takedamage )
	{
		self->s.v.solid = SOLID_NOT;
		T_Damage( other, self, self, self->dmg );
		self->s.v.think = ( func_t ) hurt_on;
		self->s.v.nextthink = g_globalvars.time + 1;
	}

	return;
}

/*QUAKED trigger_hurt (.5 .5 .5) ?
Any object touching this will be hurt
set dmg to damage amount
defalt dmg = 5
*/
void SP_trigger_hurt()
{
	InitTrigger();
	self->s.v.touch = ( func_t ) hurt_touch;
	if ( !self->dmg )
		self->dmg = 5;
}

//============================================================================

#define PUSH_ONCE  1

void trigger_push_touch()
{
	if ( streq( other->s.v.classname, "grenade" ) )
	{
		other->s.v.velocity[0] = self->speed * self->s.v.movedir[0] * 10;
		other->s.v.velocity[1] = self->speed * self->s.v.movedir[1] * 10;
		other->s.v.velocity[2] = self->speed * self->s.v.movedir[2] * 10;
	} else if ( other->s.v.health > 0 )
	{
//  other->s.v.velocity = self->speed  * self->s.v.movedir * 10;
		other->s.v.velocity[0] = self->speed * self->s.v.movedir[0] * 10;
		other->s.v.velocity[1] = self->speed * self->s.v.movedir[1] * 10;
		other->s.v.velocity[2] = self->speed * self->s.v.movedir[2] * 10;

		if ( streq( other->s.v.classname, "player" ) )
		{
			if ( other->fly_sound < g_globalvars.time )
			{
				other->fly_sound = g_globalvars.time + 1.5;
				trap_sound( other, CHAN_AUTO, "ambience/windfly.wav", 1,
					    ATTN_NORM );
			}
		}
	}
	if ( ( int ) ( self->s.v.spawnflags ) & PUSH_ONCE )
		ent_remove( self );
}


/*QUAKED trigger_push (.5 .5 .5) ? PUSH_ONCE
Pushes the player
*/
void SP_trigger_push()
{
	InitTrigger();
	trap_precache_sound( "ambience/windfly.wav" );
	self->s.v.touch = ( func_t ) trigger_push_touch;
	if ( !self->speed )
		self->speed = 1000;
}

//============================================================================

void trigger_monsterjump_touch()
{

	if ( ( (int)other->s.v.flags & ( FL_MONSTER | FL_FLY | FL_SWIM ) ) !=
	     FL_MONSTER )
		return;

// set XY even if not on ground, so the jump will clear lips
	other->s.v.velocity[0] = self->s.v.movedir[0] * self->speed;
	other->s.v.velocity[1] = self->s.v.movedir[1] * self->speed;

	if ( !( ( int ) other->s.v.flags & FL_ONGROUND ) )
		return;

	other->s.v.flags = other->s.v.flags - FL_ONGROUND;

	other->s.v.velocity[2] = self->height;
}

/*QUAKED trigger_monsterjump (.5 .5 .5) ?
Walking monsters that touch this will jump in the direction of the trigger's angle
"speed" default to 200, the speed thrown forward
"height" default to 200, the speed thrown upwards
*/
void SP_trigger_monsterjump()
{
	if ( !self->speed )
		self->speed = 200;

	if ( !self->height )
		self->height = 200;

	if ( self->s.v.angles[0] == 0 && self->s.v.angles[1] == 0
	     && self->s.v.angles[2] == 0 )
		SetVector( self->s.v.angles, 0, 360, 0 );

	InitTrigger();
	self->s.v.touch = ( func_t ) trigger_monsterjump_touch;
}
