/*
 *  QWProgs-TF2003
 *  Copyright (C) 2004  [sd] angel
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *
 *  $Id: g_mod_command.c,v 1.9 2006-11-25 18:54:32 AngelD Exp $
 */

#include "g_local.h"
typedef void (*action_func_ref_t)( int );
typedef struct mod_s{
	char   *command;
	action_func_ref_t Action;
} mod_t;


const static mod_t   mod_cmds[] = {
	{NULL, (action_func_ref_t)(0)}
};



void ModCommand()
{
        char    cmd_command[1024];
        int argc;
        const mod_t  *ucmd;

        argc = trap_CmdArgc();
        if( argc < 2 )
                return;


        trap_CmdArgv( 1, cmd_command, sizeof( cmd_command ) );

	for ( ucmd = mod_cmds; ucmd->command; ucmd++ )
	{
		if ( strcmp( cmd_command, ucmd->command ) )
			continue;
		ucmd->Action( argc );
		return;
	}
	return;

}
