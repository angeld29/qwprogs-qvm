project('qwprogs', 'c')

sources = [
    'source/buttons.c',
    'source/client.c',
    'source/combat.c',
    'source/doors.c',
    'source/g_cmd.c',
    'source/g_main.c',
    'source/g_mem.c',
    'source/g_mod_command.c',
    'source/g_spawn.c',
    'source/g_syscalls.c',
    'source/g_utils.c',
    'source/items.c',
    'source/mathlib.c',
    'source/misc.c',
    'source/plats.c',
    'source/player.c',
    'source/q_shared.c',
    'source/server.c',
    'source/spectate.c',
    'source/subs.c',
    'source/triggers.c',
    'source/weapons.c',
    'source/world.c',
]

library('qwprogs', sources,
	#include_directories : include_directories('include'),
	c_args : '-DBOT_SUPPORT=1',
	dependencies : [
		meson.get_compiler('c').find_library('m', required : false)
	],
	name_prefix : ''
)
