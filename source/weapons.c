#include "g_local.h"
// called by SP_worldspawn
void W_Precache()
{
	trap_precache_sound( "weapons/r_exp3.wav" );	// new rocket explosion
	trap_precache_sound( "weapons/rocket1i.wav" );	// spike gun
	trap_precache_sound( "weapons/sgun1.wav" );
	trap_precache_sound( "weapons/guncock.wav" );	// player shotgun
	trap_precache_sound( "weapons/ric1.wav" );	// ricochet (used in c code)
	trap_precache_sound( "weapons/ric2.wav" );	// ricochet (used in c code)
	trap_precache_sound( "weapons/ric3.wav" );	// ricochet (used in c code)
	trap_precache_sound( "weapons/spike2.wav" );	// super spikes
	trap_precache_sound( "weapons/tink1.wav" );	// spikes tink (used in c code)
	trap_precache_sound( "weapons/grenade.wav" );	// grenade launcher
	trap_precache_sound( "weapons/bounce.wav" );	// grenade bounce
	trap_precache_sound( "weapons/shotgn2.wav" );	// super shotgun
}


void            W_FireSpikes( float ox );
void            W_FireLightning();

/*
================
W_FireAxe
================
*/
void W_FireAxe()
{
	vec3_t          source, dest;
	vec3_t          org;

	makevectors( self->s.v.v_angle );

	VectorCopy( self->s.v.origin, source );
	source[2] += 16;
	VectorScale( g_globalvars.v_forward, 64, dest );
	VectorAdd( dest, source, dest )
	//source = self->s.v.origin + '0 0 16';
	
	trap_traceline( PASSVEC3( source ), PASSVEC3( dest ), false, self );
	if ( g_globalvars.trace_fraction == 1.0 )
		return;

	VectorScale( g_globalvars.v_forward, 4, org );
	VectorSubtract( g_globalvars.trace_endpos, org, org );
// org = trace_endpos - v_forward*4;

	if ( PROG_TO_EDICT( g_globalvars.trace_ent )->s.v.takedamage )
	{
		PROG_TO_EDICT( g_globalvars.trace_ent )->axhitme = 1;
		SpawnBlood( org, 20 );
		if ( deathmatch > 3 )
			T_Damage( PROG_TO_EDICT( g_globalvars.trace_ent ), self, self,
				  75 );
		else
			T_Damage( PROG_TO_EDICT( g_globalvars.trace_ent ), self, self,
				  20 );
	} else
	{			// hit wall
		trap_sound( self, CHAN_WEAPON, "player/axhit2.wav", 1, ATTN_NORM );

		trap_WriteByte( MSG_MULTICAST, SVC_TEMPENTITY );
		trap_WriteByte( MSG_MULTICAST, TE_GUNSHOT );
		trap_WriteByte( MSG_MULTICAST, 3 );
		trap_WriteCoord( MSG_MULTICAST, org[0] );
		trap_WriteCoord( MSG_MULTICAST, org[1] );
		trap_WriteCoord( MSG_MULTICAST, org[2] );
		trap_multicast( PASSVEC3( org ), MULTICAST_PVS );
	}
}


//============================================================================


//============================================================================


void wall_velocity( vec3_t v )
{
	vec3_t          vel;

	normalize( self->s.v.velocity, vel );

	vel[0] += g_globalvars.v_up[0] * ( g_random() - 0.5 ) +
	          g_globalvars.v_right[0] * ( g_random() - 0.5 );

	vel[1] += g_globalvars.v_up[1] * ( g_random() - 0.5 ) +
	          g_globalvars.v_right[1] * ( g_random() - 0.5 );
	vel[2] += g_globalvars.v_up[2] * ( g_random() - 0.5 ) +
	          g_globalvars.v_right[2] * ( g_random() - 0.5 );
	
	VectorNormalize( vel );

	vel[0] += 2 * g_globalvars.trace_plane_normal[0];
	vel[1] += 2 * g_globalvars.trace_plane_normal[1];
	vel[2] += 2 * g_globalvars.trace_plane_normal[2];

	VectorScale( vel, 200, v );

	//return vel;
}


/*
================
SpawnMeatSpray
================
*/
void SpawnMeatSpray( vec3_t org, vec3_t vel )
{
	gedict_t       *missile;

	//vec3_t  org;

	missile = spawn();
	missile->s.v.owner = EDICT_TO_PROG( self );
	missile->s.v.movetype = MOVETYPE_BOUNCE;
	missile->s.v.solid = SOLID_NOT;

	makevectors( self->s.v.angles );

	VectorCopy( vel, missile->s.v.velocity );
// missile->s.v.velocity = vel;
	missile->s.v.velocity[2] = missile->s.v.velocity[2] + 250 + 50 * g_random();

	SetVector( missile->s.v.avelocity, 3000, 1000, 2000 );
// missile.avelocity = '3000 1000 2000';

// set missile duration
	missile->s.v.nextthink = g_globalvars.time + 1;
	missile->s.v.think = ( func_t ) SUB_Remove;

	trap_setmodel( missile, "progs/zom_gib.mdl" );
	trap_setsize( missile, 0, 0, 0, 0, 0, 0 );
	trap_setorigin( missile, PASSVEC3( org ) );
}

/*
================
SpawnBlood
================
*/
void SpawnBlood( vec3_t org, float damage )
{
	trap_WriteByte( MSG_MULTICAST, SVC_TEMPENTITY );
	trap_WriteByte( MSG_MULTICAST, TE_BLOOD );
	trap_WriteByte( MSG_MULTICAST, 1 );
	trap_WriteCoord( MSG_MULTICAST, org[0] );
	trap_WriteCoord( MSG_MULTICAST, org[1] );
	trap_WriteCoord( MSG_MULTICAST, org[2] );
	trap_multicast( PASSVEC3( org ), MULTICAST_PVS );
}

/*
================
spawn_touchblood
================
*/
void spawn_touchblood( float damage )
{
	vec3_t          vel;

	wall_velocity( vel );
	VectorScale( vel, 0.2 * 0.01, vel );
	VectorAdd( self->s.v.origin, vel, vel );
	SpawnBlood( vel, damage );
}

/*
==============================================================================

MULTI-DAMAGE

Collects multiple small damages into a single damage

==============================================================================
*/
gedict_t       *multi_ent;
float           multi_damage;

vec3_t          blood_org;
float           blood_count;

vec3_t          puff_org;
float           puff_count;
void ClearMultiDamage()
{
	multi_ent = world;
	multi_damage = 0;
	blood_count = 0;
	puff_count = 0;
}

void ApplyMultiDamage()
{
	if ( multi_ent == world )
		return;
	T_Damage( multi_ent, self, self, multi_damage );
}

void AddMultiDamage( gedict_t * hit, float damage )
{
	if ( !hit )
		return;

	if ( hit != multi_ent )
	{
		ApplyMultiDamage();
		multi_damage = damage;
		multi_ent = hit;
	} else
		multi_damage = multi_damage + damage;
}

void Multi_Finish()
{
	if ( puff_count )
	{
		trap_WriteByte( MSG_MULTICAST, SVC_TEMPENTITY );
		trap_WriteByte( MSG_MULTICAST, TE_GUNSHOT );
		trap_WriteByte( MSG_MULTICAST, puff_count );
		trap_WriteCoord( MSG_MULTICAST, puff_org[0] );
		trap_WriteCoord( MSG_MULTICAST, puff_org[1] );
		trap_WriteCoord( MSG_MULTICAST, puff_org[2] );
		trap_multicast( PASSVEC3( puff_org ), MULTICAST_PVS );
	}

	if ( blood_count )
	{
		trap_WriteByte( MSG_MULTICAST, SVC_TEMPENTITY );
		trap_WriteByte( MSG_MULTICAST, TE_BLOOD );
		trap_WriteByte( MSG_MULTICAST, blood_count );
		trap_WriteCoord( MSG_MULTICAST, blood_org[0] );
		trap_WriteCoord( MSG_MULTICAST, blood_org[1] );
		trap_WriteCoord( MSG_MULTICAST, blood_org[2] );
		trap_multicast( PASSVEC3( puff_org ), MULTICAST_PVS );
	}
}

/*
==============================================================================
BULLETS
==============================================================================
*/
/*
================
TraceAttack
================
*/
void TraceAttack( float damage, vec3_t dir )
{

	vec3_t          vel, org, tmp;

	VectorScale( g_globalvars.v_up, crandom(), tmp );
	VectorAdd( dir, tmp, vel );
	VectorScale( g_globalvars.v_right, crandom(), tmp );
	VectorAdd( vel, tmp, vel );
	VectorNormalize( vel );
	VectorScale( g_globalvars.trace_plane_normal, 2, tmp );
	VectorAdd( vel, tmp, vel );
	VectorScale( vel, 200, vel );
	/*vel = normalize(dir + v_up*crandom() + v_right*crandom());
	   vel = vel + 2*trace_plane_normal;
	   vel = vel * 200; */
	VectorScale( dir, 4, tmp );
	VectorSubtract( g_globalvars.trace_endpos, tmp, org );
// org = trace_endpos - dir*4;

	if ( PROG_TO_EDICT( g_globalvars.trace_ent )->s.v.takedamage )
	{
		blood_count = blood_count + 1;
		VectorCopy( org, blood_org );	//  blood_org = org;
		AddMultiDamage( PROG_TO_EDICT( g_globalvars.trace_ent ), damage );
	} else
	{
		puff_count = puff_count + 1;
	}
}

/*
================
FireBullets

Used by shotgun, super shotgun, and enemy soldier firing
Go to the trouble of combining multiple pellets into a single damage call.
================
*/
void FireBullets( float shotcount, vec3_t dir, float spread_x, float spread_y,
		  float spread_z )
{
	vec3_t          direction;
	vec3_t          src, tmp;

	makevectors( self->s.v.v_angle );
	VectorScale( g_globalvars.v_forward, 10, tmp );
	VectorAdd( self->s.v.origin, tmp, src );
	//src = self->s.v.origin + v_forward*10;
	src[2] = self->s.v.absmin[2] + self->s.v.size[2] * 0.7;

	ClearMultiDamage();

	trap_traceline( PASSVEC3( src ), src[0] + dir[0] * 2048, src[1] + dir[1] * 2048,
			src[2] + dir[2] * 2048, false, self );
	VectorScale( dir, 4, tmp );
	VectorSubtract( g_globalvars.trace_endpos, tmp, puff_org );	// puff_org = trace_endpos - dir*4;

	while ( shotcount > 0 )
	{
		VectorScale( g_globalvars.v_right, crandom() * spread_x, tmp );
		VectorAdd( dir, tmp, direction );
		VectorScale( g_globalvars.v_up, crandom() * spread_y, tmp );
		VectorAdd( direction, tmp, direction );

//  direction = dir + crandom()*spread[0]*v_right + crandom()*spread[1]*v_up;
		VectorScale( direction, 2048, tmp );
		VectorAdd( src, tmp, tmp );
		trap_traceline( PASSVEC3( src ), PASSVEC3( tmp ), false, self );
		if ( g_globalvars.trace_fraction != 1.0 )
			TraceAttack( 4, direction );

		shotcount = shotcount - 1;
	}
	ApplyMultiDamage();
	Multi_Finish();
}

/*
================
W_FireShotgun
================
*/
void W_FireShotgun()
{
	vec3_t          dir;

	trap_sound( self, CHAN_WEAPON, "weapons/guncock.wav", 1, ATTN_NORM );

	g_globalvars.msg_entity = EDICT_TO_PROG( self );

	trap_WriteByte( MSG_ONE, SVC_SMALLKICK );

	if ( deathmatch != 4 )
		self->s.v.currentammo = --( self->s.v.ammo_shells );

	//dir = aim (self, 100000);
	aim( dir );
	FireBullets( 6, dir, 0.04, 0.04, 0 );
}

/*
================
W_FireSuperShotgun
================
*/
void W_FireSuperShotgun()
{
	vec3_t          dir;

	if ( self->s.v.currentammo == 1 )
	{
		W_FireShotgun();
		return;
	}
	trap_sound( self, CHAN_WEAPON, "weapons/shotgn2.wav", 1, ATTN_NORM );
	g_globalvars.msg_entity = EDICT_TO_PROG( self );

	trap_WriteByte( MSG_ONE, SVC_BIGKICK );

	if ( deathmatch != 4 )
		self->s.v.currentammo = self->s.v.ammo_shells = self->s.v.ammo_shells - 2;
	//dir = aim (self, 100000);
	aim( dir );
	FireBullets( 14, dir, 0.14, 0.08, 0 );
}

/*
==============================================================================

ROCKETS

==============================================================================
*/
void T_MissileTouch()
{
	float           damg;
	vec3_t          tmp;

// if (deathmatch == 4)
// {
// if ( ((other.weapon == 32) || (other.weapon == 16)))
//  { 
//   if (g_random() < 0.1)
//   {
//    if (other != world)
//    {
// //    bprint (PRINT_HIGH, "Got here\n");
//     other->deathtype = "blaze";
//     T_Damage (other, self, self->s.v.owner, 1000 );
//     T_RadiusDamage (self, self->s.v.owner, 1000, other);
//    }
//   }
//  } 
// }

	if ( other == PROG_TO_EDICT( self->s.v.owner ) )
		return;		// don't explode on owner

	if ( self->voided )
	{
		return;
	}
	self->voided = 1;

	if ( trap_pointcontents( PASSVEC3( self->s.v.origin ) ) == CONTENT_SKY )
	{
		ent_remove( self );
		return;
	}

	damg = 100 + g_random() * 20;

	if ( other->s.v.health )
	{
		other->deathtype = "rocket";
		T_Damage( other, self, PROG_TO_EDICT( self->s.v.owner ), damg );
	}
	// don't do radius damage to the other, because all the damage
	// was done in the impact


	T_RadiusDamage( self, PROG_TO_EDICT( self->s.v.owner ), 120, other, "rocket" );

//  trap_sound (self, CHAN_WEAPON, "weapons/r_exp3.wav", 1, ATTN_NORM);
	normalize( self->s.v.velocity, tmp );
	VectorScale( tmp, 8, tmp );
	VectorAdd( self->s.v.origin, tmp, self->s.v.origin )
// self->s.v.origin = self->s.v.origin - 8 * normalize(self->s.v.velocity);
	    trap_WriteByte( MSG_MULTICAST, SVC_TEMPENTITY );
	trap_WriteByte( MSG_MULTICAST, TE_EXPLOSION );
	trap_WriteCoord( MSG_MULTICAST, self->s.v.origin[0] );
	trap_WriteCoord( MSG_MULTICAST, self->s.v.origin[1] );
	trap_WriteCoord( MSG_MULTICAST, self->s.v.origin[2] );
	trap_multicast( PASSVEC3( self->s.v.origin ), MULTICAST_PHS );

	ent_remove( self );
}

/*
================
W_FireRocket
================
*/

void W_FireRocket()
{
	if ( deathmatch != 4 )
		self->s.v.currentammo = self->s.v.ammo_rockets =
		    self->s.v.ammo_rockets - 1;

	trap_sound( self, CHAN_WEAPON, "weapons/sgun1.wav", 1, ATTN_NORM );

	g_globalvars.msg_entity = EDICT_TO_PROG( self );
	trap_WriteByte( MSG_ONE, SVC_SMALLKICK );

	newmis = spawn();
	newmis->s.v.owner = EDICT_TO_PROG( self );
	newmis->s.v.movetype = MOVETYPE_FLYMISSILE;
	newmis->s.v.solid = SOLID_BBOX;

// set newmis speed     

	makevectors( self->s.v.v_angle );
	aim( newmis->s.v.velocity );	// = aim(self, 1000);
	VectorScale( newmis->s.v.velocity, 1000, newmis->s.v.velocity );
// newmis->s.v.velocity = newmis->s.v.velocity * 1000;

	vectoangles( newmis->s.v.velocity, newmis->s.v.angles );

	newmis->s.v.touch = ( func_t ) T_MissileTouch;
	newmis->voided = 0;

// set newmis duration
	newmis->s.v.nextthink = g_globalvars.time + 5;
	newmis->s.v.think = ( func_t ) SUB_Remove;
	newmis->s.v.classname = "rocket";

	trap_setmodel( newmis, "progs/missile.mdl" );
	trap_setsize( newmis, 0, 0, 0, 0, 0, 0 );

// trap_setorigin (newmis, self->s.v.origin + v_forward*8 + '0 0 16');
	trap_setorigin( newmis, self->s.v.origin[0] + g_globalvars.v_forward[0] * 8,
			self->s.v.origin[1] + g_globalvars.v_forward[1] * 8,
			self->s.v.origin[2] + g_globalvars.v_forward[2] * 8 + 16 );
}

/*
===============================================================================
LIGHTNING
===============================================================================
*/

void LightningHit( gedict_t * from, float damage )
{
	trap_WriteByte( MSG_MULTICAST, SVC_TEMPENTITY );
	trap_WriteByte( MSG_MULTICAST, TE_LIGHTNINGBLOOD );
	trap_WriteCoord( MSG_MULTICAST, g_globalvars.trace_endpos[0] );
	trap_WriteCoord( MSG_MULTICAST, g_globalvars.trace_endpos[1] );
	trap_WriteCoord( MSG_MULTICAST, g_globalvars.trace_endpos[2] );
	trap_multicast( PASSVEC3( g_globalvars.trace_endpos ), MULTICAST_PVS );

	T_Damage( PROG_TO_EDICT( g_globalvars.trace_ent ), from, from, damage );
}

/*
=================
LightningDamage
=================
*/
void LightningDamage( vec3_t p1, vec3_t p2, gedict_t * from, float damage )
{
	gedict_t       *e1, *e2;
	vec3_t          f;


	VectorSubtract( p2, p1, f );	// f = p2 - p1;
	VectorNormalize( f );	// normalize (f);

	f[0] = 0 - f[1];
	f[1] = f[0];
	f[2] = 0;
	VectorScale( f, 16, f );	//f = f*16;

	e1 = e2 = world;

	trap_traceline( PASSVEC3( p1 ), PASSVEC3( p2 ), false, self );

	if ( PROG_TO_EDICT( g_globalvars.trace_ent )->s.v.takedamage )
	{
		LightningHit( from, damage );
		if ( streq( self->s.v.classname, "player" ) )
		{
			if ( streq( other->s.v.classname, "player" ) )
				PROG_TO_EDICT( g_globalvars.trace_ent )->s.v.
				    velocity[2] += 400;
		}
	}
	e1 = PROG_TO_EDICT( g_globalvars.trace_ent );

	//trap_traceline (p1 + f, p2 + f, FALSE, self);
	trap_traceline( p1[0] + f[0], p1[1] + f[1], p1[2] + f[2], p2[0] + f[0],
			p2[1] + f[1], p2[2] + f[2], false, self );
	if ( PROG_TO_EDICT( g_globalvars.trace_ent ) != e1
	     && PROG_TO_EDICT( g_globalvars.trace_ent )->s.v.takedamage )
	{
		LightningHit( from, damage );
	}
	e2 = PROG_TO_EDICT( g_globalvars.trace_ent );

	trap_traceline( p1[0] - f[0], p1[1] - f[1], p1[2] - f[2], p2[0] - f[0],
			p2[1] - f[1], p2[2] - f[2], false, self );
	if ( PROG_TO_EDICT( g_globalvars.trace_ent ) != e1
	     && PROG_TO_EDICT( g_globalvars.trace_ent ) != e2
	     && PROG_TO_EDICT( g_globalvars.trace_ent )->s.v.takedamage )
	{
		LightningHit( from, damage );
	}
}

void W_FireLightning()
{
	vec3_t          org;
	float           cells;
	vec3_t          tmp;

	if ( self->s.v.ammo_cells < 1 )
	{
		self->s.v.weapon = W_BestWeapon();
		W_SetCurrentAmmo();
		return;
	}
// explode if under water
	if ( self->s.v.waterlevel > 1 )
	{
		if ( deathmatch > 3 )
		{
			if ( g_random() <= 0.5 )
			{
				self->deathtype = "selfwater";
				T_Damage( self, self, PROG_TO_EDICT( self->s.v.owner ),
					  4000 );
			} else
			{
				cells = self->s.v.ammo_cells;
				self->s.v.ammo_cells = 0;
				W_SetCurrentAmmo();
				T_RadiusDamage( self, self, 35 * cells, world, "" );
				return;
			}
		} else
		{
			cells = self->s.v.ammo_cells;
			self->s.v.ammo_cells = 0;
			W_SetCurrentAmmo();
			T_RadiusDamage( self, self, 35 * cells, world, "" );
			return;
		}
	}

	if ( self->t_width < g_globalvars.time )
	{
		trap_sound( self, CHAN_WEAPON, "weapons/lhit.wav", 1, ATTN_NORM );
		self->t_width = g_globalvars.time + 0.6;
	}
	g_globalvars.msg_entity = EDICT_TO_PROG( self );
	trap_WriteByte( MSG_ONE, SVC_SMALLKICK );

	if ( deathmatch != 4 )
		self->s.v.currentammo = self->s.v.ammo_cells = self->s.v.ammo_cells - 1;

	VectorCopy( self->s.v.origin, org );	//org = self->s.v.origin + '0 0 16';
	org[2] += 16;


	trap_traceline( PASSVEC3( org ), org[0] + g_globalvars.v_forward[0] * 600,
			org[1] + g_globalvars.v_forward[1] * 600,
			org[2] + g_globalvars.v_forward[2] * 600, true, self );

	trap_WriteByte( MSG_MULTICAST, SVC_TEMPENTITY );
	trap_WriteByte( MSG_MULTICAST, TE_LIGHTNING2 );
	trap_WriteEntity( MSG_MULTICAST, self );
	trap_WriteCoord( MSG_MULTICAST, org[0] );
	trap_WriteCoord( MSG_MULTICAST, org[1] );
	trap_WriteCoord( MSG_MULTICAST, org[2] );
	trap_WriteCoord( MSG_MULTICAST, g_globalvars.trace_endpos[0] );
	trap_WriteCoord( MSG_MULTICAST, g_globalvars.trace_endpos[1] );
	trap_WriteCoord( MSG_MULTICAST, g_globalvars.trace_endpos[2] );

	trap_multicast( PASSVEC3( org ), MULTICAST_PHS );

	VectorScale( g_globalvars.v_forward, 4, tmp );
	VectorAdd( g_globalvars.trace_endpos, tmp, tmp );
	LightningDamage( self->s.v.origin, tmp, self, 30 );
}

//=============================================================================

void GrenadeExplode()
{
	if ( self->voided )
	{
		return;
	}
	self->voided = 1;

	T_RadiusDamage( self, PROG_TO_EDICT( self->s.v.owner ), 120, world, "grenade" );

	trap_WriteByte( MSG_MULTICAST, SVC_TEMPENTITY );
	trap_WriteByte( MSG_MULTICAST, TE_EXPLOSION );
	trap_WriteCoord( MSG_MULTICAST, self->s.v.origin[0] );
	trap_WriteCoord( MSG_MULTICAST, self->s.v.origin[1] );
	trap_WriteCoord( MSG_MULTICAST, self->s.v.origin[2] );
	trap_multicast( PASSVEC3( self->s.v.origin ), MULTICAST_PHS );

	ent_remove( self );
}

void GrenadeTouch()
{
	if ( other == PROG_TO_EDICT( self->s.v.owner ) )
		return;		// don't explode on owner
	if ( other->s.v.takedamage == DAMAGE_AIM )
	{
		GrenadeExplode();
		return;
	}
	trap_sound( self, CHAN_WEAPON, "weapons/bounce.wav", 1, ATTN_NORM );	// bounce sound
	if ( self->s.v.velocity[0] == 0 && self->s.v.velocity[1] == 0
	     && self->s.v.velocity[2] == 0 )
		VectorClear( self->s.v.avelocity );
}

/*
================
W_FireGrenade
================
*/
void W_FireGrenade()
{
	if ( deathmatch != 4 )
		self->s.v.currentammo = self->s.v.ammo_rockets =
		    self->s.v.ammo_rockets - 1;

	trap_sound( self, CHAN_WEAPON, "weapons/grenade.wav", 1, ATTN_NORM );

	g_globalvars.msg_entity = EDICT_TO_PROG( self );

	trap_WriteByte( MSG_ONE, SVC_SMALLKICK );

	newmis = spawn();
	newmis->voided = 0;
	newmis->s.v.owner = EDICT_TO_PROG( self );
	newmis->s.v.movetype = MOVETYPE_BOUNCE;
	newmis->s.v.solid = SOLID_BBOX;
	newmis->s.v.classname = "grenade";

// set newmis speed     

	makevectors( self->s.v.v_angle );

	if ( self->s.v.v_angle[0] )
	{
		newmis->s.v.velocity[0] =
		    g_globalvars.v_forward[0] * 600 + g_globalvars.v_up[0] * 200 +
		    crandom() * g_globalvars.v_right[0] * 10 +
		    crandom() * g_globalvars.v_up[0] * 10;
		newmis->s.v.velocity[1] =
		    g_globalvars.v_forward[1] * 600 + g_globalvars.v_up[1] * 200 +
		    crandom() * g_globalvars.v_right[1] * 10 +
		    crandom() * g_globalvars.v_up[1] * 10;
		newmis->s.v.velocity[2] =
		    g_globalvars.v_forward[2] * 600 + g_globalvars.v_up[2] * 200 +
		    crandom() * g_globalvars.v_right[2] * 10 +
		    crandom() * g_globalvars.v_up[0] * 10;
	} else
	{
		aim( newmis->s.v.velocity );	// = aim(self, 10000);
		VectorScale( newmis->s.v.velocity, 600, newmis->s.v.velocity );	// * 600;
		newmis->s.v.velocity[2] = 200;
	}
	SetVector( newmis->s.v.avelocity, 300, 300, 300 );
// newmis.avelocity = '300 300 300';

	vectoangles( newmis->s.v.velocity, newmis->s.v.angles );

	newmis->s.v.touch = ( func_t ) GrenadeTouch;

// set newmis duration
	if ( deathmatch == 4 )
	{
		newmis->s.v.nextthink = g_globalvars.time + 2.5;
		self->attack_finished = g_globalvars.time + 1.1;
//  self->s.v.health = self->s.v.health - 1;
		T_Damage( self, self, PROG_TO_EDICT( self->s.v.owner ), 10 );
	} else
		newmis->s.v.nextthink = g_globalvars.time + 2.5;

	newmis->s.v.think = ( func_t ) GrenadeExplode;

	trap_setmodel( newmis, "progs/grenade.mdl" );
	trap_setsize( newmis, 0, 0, 0, 0, 0, 0 );
	trap_setorigin( newmis, PASSVEC3( self->s.v.origin ) );
}

//=============================================================================

void            spike_touch();

/*
===============
launch_spike

Used for both the player and the traps
===============
*/

void launch_spike( vec3_t org, vec3_t dir )
{

	newmis = spawn();
	g_globalvars.newmis = EDICT_TO_PROG( newmis );
	newmis->voided = 0;
	newmis->s.v.owner = EDICT_TO_PROG( self );
	newmis->s.v.movetype = MOVETYPE_FLYMISSILE;
	newmis->s.v.solid = SOLID_BBOX;

	vectoangles( dir, newmis->s.v.angles );

	newmis->s.v.touch = ( func_t ) spike_touch;
	newmis->s.v.classname = "spike";
	newmis->s.v.think = ( func_t ) SUB_Remove;
	newmis->s.v.nextthink = g_globalvars.time + 6;
	trap_setmodel( newmis, "progs/spike.mdl" );
	trap_setsize( newmis, 0, 0, 0, 0, 0, 0 );
	trap_setorigin( newmis, PASSVEC3( org ) );
	VectorScale( dir, 1000, newmis->s.v.velocity );
	//newmis->s.v.velocity = dir * 1000;
}

void spike_touch()
{
//float rand;
	if ( other == PROG_TO_EDICT( self->s.v.owner ) )
		return;

	if ( self->voided )
	{
		return;
	}
	self->voided = 1;

	if ( other->s.v.solid == SOLID_TRIGGER )
		return;		// trigger field, do nothing

	if ( trap_pointcontents( PASSVEC3( self->s.v.origin ) ) == CONTENT_SKY )
	{
		ent_remove( self );
		return;
	}
// hit something that bleeds
	if ( other->s.v.takedamage )
	{
		spawn_touchblood( 9 );
		other->deathtype = "nail";
		T_Damage( other, self, PROG_TO_EDICT( self->s.v.owner ), 9 );
	} else
	{
		trap_WriteByte( MSG_MULTICAST, SVC_TEMPENTITY );
		if ( !strcmp( self->s.v.classname, "wizspike" ) )
			trap_WriteByte( MSG_MULTICAST, TE_WIZSPIKE );
		else if ( !strcmp( self->s.v.classname, "knightspike" ) )
			trap_WriteByte( MSG_MULTICAST, TE_KNIGHTSPIKE );
		else
			trap_WriteByte( MSG_MULTICAST, TE_SPIKE );
		trap_WriteCoord( MSG_MULTICAST, self->s.v.origin[0] );
		trap_WriteCoord( MSG_MULTICAST, self->s.v.origin[1] );
		trap_WriteCoord( MSG_MULTICAST, self->s.v.origin[2] );
		trap_multicast( PASSVEC3( self->s.v.origin ), MULTICAST_PHS );
	}

	ent_remove( self );

}

void superspike_touch()
{
//float rand;
	if ( other == PROG_TO_EDICT( self->s.v.owner ) )
		return;

	if ( self->voided )
	{
		return;
	}
	self->voided = 1;


	if ( other->s.v.solid == SOLID_TRIGGER )
		return;		// trigger field, do nothing

	if ( trap_pointcontents( PASSVEC3( self->s.v.origin ) ) == CONTENT_SKY )
	{
		ent_remove( self );
		return;
	}
// hit something that bleeds
	if ( other->s.v.takedamage )
	{
		spawn_touchblood( 18 );
		other->deathtype = "supernail";
		T_Damage( other, self, PROG_TO_EDICT( self->s.v.owner ), 18 );
	} else
	{
		trap_WriteByte( MSG_MULTICAST, SVC_TEMPENTITY );
		trap_WriteByte( MSG_MULTICAST, TE_SUPERSPIKE );
		trap_WriteCoord( MSG_MULTICAST, self->s.v.origin[0] );
		trap_WriteCoord( MSG_MULTICAST, self->s.v.origin[1] );
		trap_WriteCoord( MSG_MULTICAST, self->s.v.origin[2] );
		trap_multicast( PASSVEC3( self->s.v.origin ), MULTICAST_PHS );
	}

	ent_remove( self );

}

void W_FireSuperSpikes()
{
	vec3_t          dir, tmp;

// gedict_t*    old;

	trap_sound( self, CHAN_WEAPON, "weapons/spike2.wav", 1, ATTN_NORM );
	self->attack_finished = g_globalvars.time + 0.2;
	if ( deathmatch != 4 )
		self->s.v.currentammo = self->s.v.ammo_nails = self->s.v.ammo_nails - 2;
	aim( dir );		//dir = aim (self, 1000);

	VectorCopy( self->s.v.origin, tmp );
	tmp[2] += 16;
	launch_spike( tmp, dir );
	newmis->s.v.touch = ( func_t ) superspike_touch;
	trap_setmodel( newmis, "progs/s_spike.mdl" );
	trap_setsize( newmis, 0, 0, 0, 0, 0, 0 );
	g_globalvars.msg_entity = EDICT_TO_PROG( self );
	trap_WriteByte( MSG_ONE, SVC_SMALLKICK );
}

void W_FireSpikes( float ox )
{
	vec3_t          dir, tmp;

// gedict_t*    old;

	makevectors( self->s.v.v_angle );

	if ( self->s.v.ammo_nails >= 2 && self->s.v.weapon == IT_SUPER_NAILGUN )
	{
		W_FireSuperSpikes();
		return;
	}

	if ( self->s.v.ammo_nails < 1 )
	{
		self->s.v.weapon = W_BestWeapon();
		W_SetCurrentAmmo();
		return;
	}

	trap_sound( self, CHAN_WEAPON, "weapons/rocket1i.wav", 1, ATTN_NORM );
	self->attack_finished = g_globalvars.time + 0.2;
	if ( deathmatch != 4 )
		self->s.v.currentammo = self->s.v.ammo_nails = self->s.v.ammo_nails - 1;
	aim( dir );		// dir = aim (self, 1000);
	VectorScale( g_globalvars.v_right, ox, tmp );
	VectorAdd( tmp, self->s.v.origin, tmp );
	tmp[2] += 16;
	launch_spike( tmp, dir );

	g_globalvars.msg_entity = EDICT_TO_PROG( self );
	trap_WriteByte( MSG_ONE, SVC_SMALLKICK );
}

/*
===============================================================================

PLAYER WEAPON USE

===============================================================================
*/

void W_SetCurrentAmmo()
{
	int             items;

 player_run ();          // get out of any weapon firing states
	items = self->s.v.items;
	items -= items & ( IT_SHELLS | IT_NAILS | IT_ROCKETS | IT_CELLS );
	switch ( ( int ) self->s.v.weapon )
	{
	case IT_AXE:
		self->s.v.currentammo = 0;
		self->s.v.weaponmodel = "progs/v_axe.mdl";
		self->s.v.weaponframe = 0;
		break;

	case IT_SHOTGUN:
		self->s.v.currentammo = self->s.v.ammo_shells;
		self->s.v.weaponmodel = "progs/v_shot.mdl";
		self->s.v.weaponframe = 0;
		items |= IT_SHELLS;
		break;

	case IT_SUPER_SHOTGUN:
		self->s.v.currentammo = self->s.v.ammo_shells;
		self->s.v.weaponmodel = "progs/v_shot2.mdl";
		self->s.v.weaponframe = 0;
		items |= IT_SHELLS;
		break;

	case IT_NAILGUN:
		self->s.v.currentammo = self->s.v.ammo_nails;
		self->s.v.weaponmodel = "progs/v_nail.mdl";
		self->s.v.weaponframe = 0;
		items |= IT_NAILS;
		break;

	case IT_SUPER_NAILGUN:
		self->s.v.currentammo = self->s.v.ammo_nails;
		self->s.v.weaponmodel = "progs/v_nail2.mdl";
		self->s.v.weaponframe = 0;
		items |= IT_NAILS;
		break;

	case IT_GRENADE_LAUNCHER:
		self->s.v.currentammo = self->s.v.ammo_rockets;
		self->s.v.weaponmodel = "progs/v_rock.mdl";
		self->s.v.weaponframe = 0;
		items |= IT_ROCKETS;
		break;

	case IT_ROCKET_LAUNCHER:
		self->s.v.currentammo = self->s.v.ammo_rockets;
		self->s.v.weaponmodel = "progs/v_rock2.mdl";
		self->s.v.weaponframe = 0;
		items |= IT_ROCKETS;
		break;

	case IT_LIGHTNING:
		self->s.v.currentammo = self->s.v.ammo_cells;
		self->s.v.weaponmodel = "progs/v_light.mdl";
		self->s.v.weaponframe = 0;
		items |= IT_CELLS;
		break;

	default:
		self->s.v.currentammo = 0;
		self->s.v.weaponmodel = "";
		self->s.v.weaponframe = 0;
		break;
	}
	self->s.v.items = items;
}


float W_BestWeapon()
{
	int             it;

	it = self->s.v.items;

	if ( self->s.v.waterlevel <= 1 && self->s.v.ammo_cells >= 1
	     && ( it & IT_LIGHTNING ) )
		return IT_LIGHTNING;

	else if ( self->s.v.ammo_nails >= 2 && ( it & IT_SUPER_NAILGUN ) )
		return IT_SUPER_NAILGUN;

	else if ( self->s.v.ammo_shells >= 2 && ( it & IT_SUPER_SHOTGUN ) )
		return IT_SUPER_SHOTGUN;

	else if ( self->s.v.ammo_nails >= 1 && ( it & IT_NAILGUN ) )
		return IT_NAILGUN;

	else if ( self->s.v.ammo_shells >= 1 && ( it & IT_SHOTGUN ) )
		return IT_SHOTGUN;

/*
 if(self.ammo_rockets >= 1 && (it & IT_ROCKET_LAUNCHER) )
  return IT_ROCKET_LAUNCHER;
 else if(self.ammo_rockets >= 1 && (it & IT_GRENADE_LAUNCHER) )
  return IT_GRENADE_LAUNCHER;

*/

	return IT_AXE;
}

int W_CheckNoAmmo()
{
	if ( self->s.v.currentammo > 0 )
		return true;

	if ( self->s.v.weapon == IT_AXE )
		return true;

	self->s.v.weapon = W_BestWeapon();

	W_SetCurrentAmmo();

// drop the weapon down
	return false;
}

/*
============
W_Attack

An attack impulse can be triggered now
============
*/
void W_Attack()
{
	float           r;

	if ( !W_CheckNoAmmo() )
		return;

	makevectors( self->s.v.v_angle );	// calculate forward angle for velocity
	self->show_hostile = g_globalvars.time + 1;	// wake monsters up

	switch ( ( int ) self->s.v.weapon )
	{
	case IT_AXE:
		self->attack_finished = g_globalvars.time + 0.5;
		trap_sound( self, CHAN_WEAPON, "weapons/ax1.wav", 1, ATTN_NORM );
		r = g_random();
		if ( r < 0.25 )
			player_axe1();
		else if ( r < 0.5 )
			player_axeb1();
		else if ( r < 0.75 )
			player_axec1();
		else
			player_axed1();
		break;

	case IT_SHOTGUN:
		player_shot1();
		self->attack_finished = g_globalvars.time + 0.5;
		W_FireShotgun();
		break;

	case IT_SUPER_SHOTGUN:
		player_shot1();
		self->attack_finished = g_globalvars.time + 0.7;
		W_FireSuperShotgun();
		break;

	case IT_NAILGUN:
		player_nail1();
		break;

	case IT_SUPER_NAILGUN:
		player_nail1();
		break;

	case IT_GRENADE_LAUNCHER:
		player_rocket1();
		self->attack_finished = g_globalvars.time + 0.6;
		W_FireGrenade();
		break;

	case IT_ROCKET_LAUNCHER:
		player_rocket1();
		self->attack_finished = g_globalvars.time + 0.8;
		W_FireRocket();
		break;

	case IT_LIGHTNING:
		self->attack_finished = g_globalvars.time + 0.1;
		trap_sound( self, CHAN_AUTO, "weapons/lstart.wav", 1, ATTN_NORM );
		player_light1();
		break;
	}
}

/*
============
W_ChangeWeapon

============
*/
void W_ChangeWeapon()
{
	int             it, am, fl = 0;

	it = self->s.v.items;
	am = 0;

	switch ( impulse )
	{
	case 1:
		fl = IT_AXE;
		break;
	case 2:
		fl = IT_SHOTGUN;
		if ( self->s.v.ammo_shells < 1 )
			am = 1;
		break;
	case 3:
		fl = IT_SUPER_SHOTGUN;
		if ( self->s.v.ammo_shells < 2 )
			am = 1;
		break;
	case 4:
		fl = IT_NAILGUN;
		if ( self->s.v.ammo_nails < 1 )
			am = 1;
		break;
	case 5:
		fl = IT_SUPER_NAILGUN;
		if ( self->s.v.ammo_nails < 2 )
			am = 1;
		break;
	case 6:
		fl = IT_GRENADE_LAUNCHER;
		if ( self->s.v.ammo_rockets < 1 )
			am = 1;
		break;
	case 7:
		fl = IT_ROCKET_LAUNCHER;
		if ( self->s.v.ammo_rockets < 1 )
			am = 1;
		break;
	case 8:
		fl = IT_LIGHTNING;
		if ( self->s.v.ammo_cells < 1 )
			am = 1;
		break;
	default:
		break;
	}

	impulse = 0;

	if ( !( ( int ) self->s.v.items & fl ) )
	{			// don't have the weapon or the ammo
		G_sprint( self, PRINT_HIGH, "no weapon.\n" );
		return;
	}

	if ( am )
	{			// don't have the ammo
		G_sprint( self, PRINT_HIGH, "not enough ammo.\n" );
		return;
	}
//
// set weapon, set ammo
//
	self->s.v.weapon = fl;
	W_SetCurrentAmmo();
}

/*
============
CheatCommand
============
*/
void CheatCommand()
{
//      if (deathmatch || coop)
	return;

	self->s.v.ammo_rockets = 100;
	self->s.v.ammo_nails = 200;
	self->s.v.ammo_shells = 100;
	self->s.v.items = ( int ) self->s.v.items |
	    IT_AXE |
	    IT_SHOTGUN |
	    IT_SUPER_SHOTGUN |
	    IT_NAILGUN |
	    IT_SUPER_NAILGUN |
	    IT_GRENADE_LAUNCHER | IT_ROCKET_LAUNCHER | IT_KEY1 | IT_KEY2;

	self->s.v.ammo_cells = 200;
	self->s.v.items = ( int ) self->s.v.items | IT_LIGHTNING;

	self->s.v.weapon = IT_ROCKET_LAUNCHER;
	impulse = 0;
	W_SetCurrentAmmo();
}

/*
============
CycleWeaponCommand

Go to the next weapon with ammo
============
*/
void CycleWeaponCommand()
{
	int             it, am;

	it = self->s.v.items;
	impulse = 0;

	while ( 1 )
	{
		am = 0;
		switch ( ( int ) self->s.v.weapon )
		{
		case IT_LIGHTNING:
			self->s.v.weapon = IT_AXE;
			break;

		case IT_AXE:
			self->s.v.weapon = IT_SHOTGUN;
			if ( self->s.v.ammo_shells < 1 )
				am = 1;
			break;

		case IT_SHOTGUN:
			self->s.v.weapon = IT_SUPER_SHOTGUN;
			if ( self->s.v.ammo_shells < 2 )
				am = 1;
			break;

		case IT_SUPER_SHOTGUN:
			self->s.v.weapon = IT_NAILGUN;
			if ( self->s.v.ammo_nails < 1 )
				am = 1;
			break;

		case IT_NAILGUN:
			self->s.v.weapon = IT_SUPER_NAILGUN;
			if ( self->s.v.ammo_nails < 2 )
				am = 1;
			break;

		case IT_SUPER_NAILGUN:
			self->s.v.weapon = IT_GRENADE_LAUNCHER;
			if ( self->s.v.ammo_rockets < 1 )
				am = 1;
			break;

		case IT_GRENADE_LAUNCHER:
			self->s.v.weapon = IT_ROCKET_LAUNCHER;
			if ( self->s.v.ammo_rockets < 1 )
				am = 1;
			break;

		case IT_ROCKET_LAUNCHER:
			self->s.v.weapon = IT_LIGHTNING;
			if ( self->s.v.ammo_cells < 1 )
				am = 1;
			break;
		}

		if ( ( it & ( int ) self->s.v.weapon ) && am == 0 )
		{
			W_SetCurrentAmmo();
			return;
		}
	}

}


/*
============
CycleWeaponReverseCommand

Go to the prev weapon with ammo
============
*/
void CycleWeaponReverseCommand()
{
	int             it, am;

	it = self->s.v.items;
	impulse = 0;

	while ( 1 )
	{
		am = 0;
		switch ( ( int ) self->s.v.weapon )
		{
		case IT_LIGHTNING:
			self->s.v.weapon = IT_ROCKET_LAUNCHER;
			if ( self->s.v.ammo_rockets < 1 )
				am = 1;
			break;

		case IT_ROCKET_LAUNCHER:
			self->s.v.weapon = IT_GRENADE_LAUNCHER;
			if ( self->s.v.ammo_rockets < 1 )
				am = 1;
			break;

		case IT_GRENADE_LAUNCHER:
			self->s.v.weapon = IT_SUPER_NAILGUN;
			if ( self->s.v.ammo_nails < 2 )
				am = 1;
			break;

		case IT_SUPER_NAILGUN:
			self->s.v.weapon = IT_NAILGUN;
			if ( self->s.v.ammo_nails < 1 )
				am = 1;
			break;

		case IT_NAILGUN:
			self->s.v.weapon = IT_SUPER_SHOTGUN;
			if ( self->s.v.ammo_shells < 2 )
				am = 1;
			break;

		case IT_SUPER_SHOTGUN:
			self->s.v.weapon = IT_SHOTGUN;
			if ( self->s.v.ammo_shells < 1 )
				am = 1;
			break;

		case IT_SHOTGUN:
			self->s.v.weapon = IT_AXE;
			break;

		case IT_AXE:
			self->s.v.weapon = IT_LIGHTNING;
			if ( self->s.v.ammo_cells < 1 )
				am = 1;
			break;
		}
		if ( ( it & ( int ) self->s.v.weapon ) && am == 0 )
		{
			W_SetCurrentAmmo();
			return;
		}
	}

}


/*
============
ServerflagsCommand

Just for development
============
*/
void ServerflagsCommand()
{
	g_globalvars.serverflags = ( int ) ( g_globalvars.serverflags ) * 2 + 1;
}


/*
============
ImpulseCommands

============
*/
int             impulse;
void ImpulseCommands()
{
	impulse = self->s.v.impulse;
	if ( impulse >= 1 && impulse <= 8 )
		W_ChangeWeapon();

	if ( impulse == 9 )
		CheatCommand();

	if ( impulse == 10 )
		CycleWeaponCommand();

	if ( impulse == 11 )
		ServerflagsCommand();

	if ( impulse == 12 )
		CycleWeaponReverseCommand();

	impulse = 0;

	self->s.v.impulse = impulse;
}

/*
============
W_WeaponFrame

Called every frame so impulse events can be handled as well as possible
============
*/
void W_WeaponFrame()
{
	if ( g_globalvars.time < self->attack_finished )
		return;

	ImpulseCommands();

// check for attack
	if ( self->s.v.button0 )
	{
		SuperDamageSound();
		W_Attack();
	}
}


/*
========
SuperDamageSound

Plays sound if needed
========
*/
void SuperDamageSound()
{
	if ( self->super_damage_finished > g_globalvars.time )
	{
		if ( self->super_sound < g_globalvars.time )
		{
			self->super_sound = g_globalvars.time + 1;
			trap_sound( self, CHAN_BODY, "items/damage3.wav", 1, ATTN_NORM );
		}
	}
	return;
}
