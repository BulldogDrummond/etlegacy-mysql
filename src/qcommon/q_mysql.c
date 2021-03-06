/*
 * MySQL Functions
 *
 */

#ifdef FEATURE_MYSQL

#include "q_mysql.h"
#include <stdio.h>
#include <strings.h>

// Cvars
cvar_t *sv_dbEnable;
cvar_t *sv_dbHostname;
cvar_t *sv_dbDatabase;
cvar_t *sv_dbUsername;
cvar_t *sv_dbPassword;

int Com_DB_Ready()
{
	int db_ready;

	cvar_t *dbTest = Cvar_Get("sv_dbReady", "0", CVAR_SERVERINFO | CVAR_ROM);
	db_ready = dbTest->integer;

	return db_ready;
}

void Com_DB_SetMap(char *server)
{
	sv_dbHostname = Cvar_Get("db_hostname", "",  CVAR_TEMP);
	sv_dbDatabase = Cvar_Get("db_database", "",  CVAR_TEMP);
	sv_dbUsername = Cvar_Get("db_username", "",  CVAR_TEMP);
	sv_dbPassword = Cvar_Get("db_password", "",  CVAR_TEMP);

	char *dbhost;
	char *dbname;
	char *dbuser;
	char *dbpass;

	dbhost = sv_dbHostname->string;
	dbname = sv_dbDatabase->string;
	dbuser = sv_dbUsername->string;
	dbpass = sv_dbPassword->string;

	MYSQL *conn;
	MYSQL_RES *res;

	char query[100];

	Com_Printf("Setting Map to %s in database.\n",server);
	sprintf(query,"UPDATE server_status SET ss_val = \"%s\" WHERE ss_key = \"Current_Map\"",server);

	conn=mysql_init(NULL);

	if(!mysql_real_connect(conn,dbhost,dbuser,dbpass,dbname,0,NULL,0)) {
		Com_Printf("Database Error: %s\n",mysql_error(conn));
	} else {
		if (mysql_query(conn, query)) {
			Com_Printf("Database Error: %s\n", mysql_error(conn));
		} else {
			mysql_close(conn);
		}
	}
	return;
}

void Com_DB_ResetMap()
{
	sv_dbHostname = Cvar_Get("db_hostname", "",  CVAR_TEMP);
	sv_dbDatabase = Cvar_Get("db_database", "",  CVAR_TEMP);
	sv_dbUsername = Cvar_Get("db_username", "",  CVAR_TEMP);
	sv_dbPassword = Cvar_Get("db_password", "",  CVAR_TEMP);

	char *dbhost;
	char *dbname;
	char *dbuser;
	char *dbpass;

	dbhost = sv_dbHostname->string;
	dbname = sv_dbDatabase->string;
	dbuser = sv_dbUsername->string;
	dbpass = sv_dbPassword->string;

	MYSQL *conn;
	MYSQL_RES *res;

	char query[100];

	Com_Printf("Resetting Map in database.\n");
	sprintf(query,"UPDATE server_status SET ss_val = \"None\" WHERE ss_key = \"Current_Map\"");

	conn=mysql_init(NULL);

	if(!mysql_real_connect(conn,dbhost,dbuser,dbpass,dbname,0,NULL,0)) {
		Com_Printf("Database Error: %s\n",mysql_error(conn));
	} else {
		if (mysql_query(conn, query)) {
			Com_Printf("Database Error: %s\n", mysql_error(conn));
		} else {
			mysql_close(conn);
		}
	}
	return;
}

void Com_DB_InitGameTest()
{
	Com_Printf("------ Database Test ------\n");

	sv_dbEnable   = Cvar_Get("db_enable",   "0", CVAR_TEMP);
	sv_dbHostname = Cvar_Get("db_hostname", "",  CVAR_TEMP);
	sv_dbDatabase = Cvar_Get("db_database", "",  CVAR_TEMP);
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

	MYSQL *conn;
	MYSQL_RES *res;
	MYSQL_ROW row;

	/* This tests that the database can be reached and login succeeds */
	if ( use_db == 1 ) {
		conn=mysql_init(NULL);
		if(!mysql_real_connect(conn,dbhost,dbuser,dbpass,dbname,0,NULL,0)) {
			Com_Printf("Database Initialization Failed.\n");
			Com_Printf("Database Error: %s\n",mysql_error(conn));
		} else{
			Com_Printf("Database Client Version: %s\n",mysql_get_client_info());
			Com_Printf("Database Server Version: %s\n",mysql_get_server_info(conn));
			// Read from table
			if (mysql_query(conn, "SELECT ss_val FROM server_status WHERE ss_key = \"DB_Test\"")) {
				Com_Printf("Database Status: %s\n", mysql_error(conn));
				Cvar_Set("sv_dbReady","0");
			} else {
				res = mysql_use_result(conn);
				while ((row = mysql_fetch_row(res)) != NULL)
					Com_Printf("Database Query Result: %s \n", row[0]);
				mysql_free_result(res);
				mysql_close(conn);
				Com_Printf("Database Initialized.\n");
				Cvar_Set("sv_dbReady","1");
			}
		}
	} else {
		Com_Printf("Database Disabled in Server Configuration.\n");
	}

	Com_Printf("Database Test Complete.\n");
	return;
}

#endif /* FEATURE_MYSQL */

