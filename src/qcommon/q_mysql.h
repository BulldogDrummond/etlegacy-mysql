/*
 * ET <-> Mysql interface header file.
 */
#ifndef _Q_MYSQL_H
#define _Q_MYSQL_H

#ifdef FEATURE_MYSQL

#include "q_shared.h"
#include "qcommon.h"

// TODO: Test windows build
#ifdef WIN32
#include <windows.h>
#include <winsock.h>
#pragma warning (disable: 4514 4786)
#pragma warning( push, 3 )
#endif

#include "mysql/my_global.h"
#include "mysql/mysql.h"

#ifndef WIN32
#include <unistd.h>
#endif

// Database connection cvars
extern cvar_t *sv_dbEnable;
extern cvar_t *sv_dbHostname;
extern cvar_t *sv_dbDatabase;
extern cvar_t *sv_dbUsername;
extern cvar_t *sv_dbPassword;

void Com_DB_InitGameTest();

#endif /* ifdef FEATURE_MYSQL */

#endif /* ifndef _Q_MYSQL_H */

