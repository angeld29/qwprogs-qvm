#include "g_local.h"

void            ClientKill();

qboolean 	ClientCommand()
{
	char            cmd_command[1024];

	self = PROG_TO_EDICT( g_globalvars.self );
	trap_CmdArgv( 0, cmd_command, sizeof( cmd_command ) );

	if ( !strcmp( cmd_command, "kill" ) )
	{
		ClientKill();
		return true;
	}
	//G_Printf("ClientCommand %s\n",cmd_command);
	return false;
}

qboolean ClientUserInfoChanged_after(  )
{
	char    key[1024];
	char    value[1024];
	const   char   *sk;
	int     color;

	self = PROG_TO_EDICT( g_globalvars.self );

	trap_CmdArgv( 1, key, sizeof( key ) );
	if ( !strcmp( key, "name" ) )
    {
        trap_CmdArgv( 2, self->s.v.netname, 64 );
    }
    return 0;
}

qboolean ClientUserInfoChanged(  )
{
	char    key[1024];
	char    value[1024];

	self = PROG_TO_EDICT( g_globalvars.self );

	trap_CmdArgv( 1, key, sizeof( key ) );
	trap_CmdArgv( 2, value, sizeof( value ) );

	return 0;

}
