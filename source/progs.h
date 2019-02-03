#include "progdefs.h"


typedef struct shared_edict_s {
	entvars_t       v;	// C exported fields from progs
// other fields from progs come immediately after
} edict_t;

struct gedict_s;
typedef void (*th_die_funcref_t)();
typedef void (*th_pain_funcref_t)(struct gedict_s *, float);

//typedef (void(*)(gedict_t *)) one_edict_func;
typedef struct gedict_s {
	edict_t         s;

	//custom fields
	float	items2;				   // using  ZQ_ITEMS2 extension in mvdsv we can use per client sigils for runes
	int 	k_admin;         // if player is an admin, flags
	float 			brokenankle; // player can't jump for a while after falling and get damaged
	float           vw_index;
	int         hideentity;             // links to entity to hide in eye chasecam
	int			trackent;			// pseudo spectating for players.
	qboolean       hideplayers;            // if set, all players hidden (read by mvdsv)
// { highlights which clients this entity was visible to
	unsigned int visclients;
// }
	// let mvdsv know when player has teleported, and adjust for high-ping
	int          teleported;
	float           maxspeed, gravity;
	char           *mdl;
	char           *killtarget;
	float           worldtype;	// 0=medieval 1=metal 2=base
	char           *map;
//player
	int             walkframe;

//
// quakeed fields
//
	int             style;
	float           speed;
	int             state;
	int             voided;
//items.c
	float           healamount, healtype;
//ammo
	int             aflag;
//
// doors, etc
//
	vec3_t		    movement;
	vec3_t          dest, dest1, dest2;
	vec3_t          pos1, pos2, oldorigin;
	vec3_t          mangle;
	float           t_length, t_width, height;
	float           wait;	// time from firing to restarting
	float           delay;	// time from activation to firing
	struct gedict_s *trigger_field;	// door's trigger entity
	struct gedict_s *movetarget;
	float           pausetime;
	char           *noise4;
	float           count;
//
// misc
//
	float           cnt;	// misc flag

	void            ( *think1 ) ();	//calcmove
	vec3_t          finaldest;
//combat
	float           dmg;
	float           lip;
	float           attack_finished;
	float           pain_finished;

	float           invincible_finished;
	float           invisible_finished;
	float           super_damage_finished;
	float           radsuit_finished;


	float           invincible_time, invincible_sound;
	float           invisible_time, invisible_sound;
	float           super_time, super_sound;
	float           rad_time;
	float           fly_sound;

	float           axhitme;
	float           show_hostile;	// set to time+0.2 whenever a client fires a
	// weapon or takes damage.  Used to alert
	// monsters that otherwise would let the player go
	float           jump_flag;	// player jump flag
	float           swim_flag;	// player swimming sound flag
	float           air_finished;	// when time > air_finished, start drowning
	float           bubble_count;	// keeps track of the number of bubbles
	char           *deathtype;	// keeps track of how the player died
	float           dmgtime;

	th_die_funcref_t   th_die;
	th_pain_funcref_t  th_pain;
    int            old_button0,old_button1,old_button2,old_keys;
	int            isBot;
	int            keys, nextimpulse;
	int            action,oldAction;

} gedict_t;
