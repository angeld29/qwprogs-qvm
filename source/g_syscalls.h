
int             trap_GetApiVersion();
qboolean        trap_GetEntityToken( char *token, int size );
void            trap_DPrintf( const char *fmt );
void            trap_BPrint( int level, const char *fmt );
void            trap_SPrint( gedict_t * ed, int level, const char *fmt );
void            trap_CenterPrint( gedict_t * ed, const char *fmt );
void            trap_Error( const char *fmt );
gedict_t       *trap_spawn();
void            trap_remove( gedict_t * ed );
void            trap_precache_sound( char *name );
void            trap_precache_model( char *name );
void            trap_setorigin( gedict_t * ed, float origin_x, float origin_y,
				float origin_z );
void            trap_setsize( gedict_t * ed, float min_x, float min_y, float min_z,
			      float max_x, float max_y, float max_z );
void            trap_setmodel( gedict_t * ed, char *model );
void            trap_ambientsound( float pos_x, float pos_y, float pos_z, char *samp,
				   float vol, float atten );
void            trap_sound( gedict_t * ed, int channel, char *samp, int vol, float att );
void            trap_traceline( float v1_x, float v1_y, float v1_z, float v2_x,
				float v2_y, float v2_z, int nomonst, gedict_t * ed );
gedict_t       *trap_checkclient();
void            trap_stuffcmd( gedict_t * ed, const char *fmt );
void            trap_localcmd( const char *fmt );
float           trap_cvar( const char *var );
void            trap_cvar_set( const char *var, const char *val );
int             trap_walkmove( gedict_t * ed, float yaw, float dist );
int             trap_droptofloor( gedict_t * ed );
void            trap_lightstyle( int style, char *val );
int             trap_checkbottom( gedict_t * ed );
int             trap_pointcontents( float origin_x, float origin_y, float origin_z );
int             trap_nextent( int );
void            trap_makestatic( gedict_t * ed );
void            trap_setspawnparam( gedict_t * ed );
void            trap_changelevel( char *name );
int             trap_multicast( float origin_x, float origin_y, float origin_z, int to );
void            trap_logfrag( gedict_t * killer, gedict_t * killee );
void            trap_infokey( gedict_t * ed, char *key, char *valbuff, int sizebuff );
void            trap_disableupdates( gedict_t * ed, float time );

void            trap_WriteByte   ( int to, int data );
void            trap_WriteChar   ( int to, int data );
void            trap_WriteShort  ( int to, int data );
void            trap_WriteLong   ( int to, int data );
void            trap_WriteAngle  ( int to, float data );
void            trap_WriteCoord  ( int to, float data );
void            trap_WriteString ( int to, char *data );
void            trap_WriteEntity ( int to, gedict_t * ed );
void            trap_FlushSignon ();
