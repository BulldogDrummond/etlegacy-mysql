/*
 * MySQL Functions
 *
 */

#ifdef FEATURE_MYSQL

#include "q_mysql.h"

// Cvars
cvar_t *sv_dbEnable;
cvar_t *sv_dbHostname;
cvar_t *sv_dbDatabase;
cvar_t *sv_dbUsername;
cvar_t *sv_dbPassword;

void Com_MysqlHook_InitGame()
{
	Com_Printf("------ Database Test ------\n");

	sv_dbEnable   = Cvar_Get("db_enable",   "0", CVAR_TEMP);
	sv_dbHostname = Cvar_Get("db_hostname", "",  CVAR_TEMP);
	sv_Database   = Cvar_Get("db_database", "",  CVAR_TEMP);
	sv_dbUsername = Cvar_Get("db_username", "",  CVAR_TEMP);
	sv_dbPassword = Cvar_Get("db_password", "",  CVAR_TEMP);

	Cvar_Set("sv_dbReady","0");

	int use_db;
	char *dbhost;
	char *dbname;
	char *dbuser;
	char *dbpass;

	use_db = sv_dbEnable->integer;

	dbhost = sv_dbHostname->string;
	dbname = sv_dbDatabase->string;
	dbuser = sv_dbUsername->string;
	dbpass = sv_dbPassword->string;

	/* This tests that the database can be reached and login succeeds */
	if ( use_db == 1 ) {
		MYSQL *connect;
		connect=mysql_init(NULL);
		if(!connect)
		{
			Com_Printf("Database Initialization Failed.\n");
			Cvar_Set("sv_dbReady","0");
		} else{
			/* TODO: Add login */
			Com_Printf("Database Initialized.\n");
			Cvar_Set("sv_dbReady","1");
		}
	} else {
		Com_Printf("Database Disabled in Server Configuration.\n");
		Cvar_Set("sv_dbReady","0");
	}

	Com_Printf("Database Test Complete.\n");
	return;
}

#endif /* FEATURE_MYSQL */

