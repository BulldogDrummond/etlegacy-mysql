#
# Find the MySQL client includes and library
# Based on this post: http://www.cmake.org/pipermail/cmake/2010-July/037934.html 
#

# This module defines
# MYSQL_INCLUDE_DIR, where to find mysql.h
# MYSQL_FOUND, If false, you cannot build anything that requires MySQL.
# MYSQL_LIBRARY, where to find the MySQL library.

SET( MYSQL_FOUND 0 )

FIND_PATH(MYSQL_INCLUDE_DIR mysql.h
	/usr/include
	/usr/include/mysql
	/usr/local/include
	/usr/local/include/mysql
	/usr/local/mysql/include
	/usr/local/mysql/include/mysql
	"C:/Program Files/MySQL/include"
	"C:/Program Files/MySQL/MySQL Server 5.1/include"
	"C:/Program Files (x86)/MySQL/MySQL Server 5.1/include"
	"C:/MySQL/include"
	"[HKEY_LOCAL_MACHINE\\SOFTWARE\\MySQL AB\\MySQL Server 5.1;Location]/include"
	"[HKEY_LOCAL_MACHINE\\SOFTWARE\\Wow6432Node\\MySQL AB\\MySQL Server 5.1;Location]/include"
	DOC "Directory containing mysql.h."
)

FIND_LIBRARY( MYSQL_LIBRARY 
	NAMES mysql libmysql mysqlclient
	PATHS
	/usr/lib
	/usr/lib64
	/usr/lib/mysql
	/usr/lib64/mysql
	/usr/local/lib
	/usr/local/lib/mysql
	/usr/local/mysql/lib
	/usr/local/mysql/lib/mysql
	"C:/Program Files/MySQL/lib"
	"C:/Program Files/MySQL/MySQL Server 5.1/lib/opt"
	"C:/MySQL/lib/opt"
	"[HKEY_LOCAL_MACHINE\\SOFTWARE\\MySQL AB\\MySQL Server 5.1;Location]/lib/opt"
	"[HKEY_LOCAL_MACHINE\\SOFTWARE\\Wow6432Node\\MySQL AB\\MySQL Server 5.1;Location]/lib/opt"
	DOC "Directory containing mysql library."
)

IF (NOT WIN32)
	FIND_LIBRARY( MYSQL_EXTRA_LIBRARIES
		NAMES z zlib
		PATHS /usr/lib /usr/lib64 /usr/local/lib 
		DOC "Specify additional required libs here.")
ELSE (NOT WIN32)
		# Windows doesn't usually need this
		SET( MYSQL_EXTRA_LIBRARIES "" )
ENDIF (NOT WIN32)


IF (MYSQL_LIBRARY)
	IF (MYSQL_INCLUDE_DIR)
		SET( MYSQL_FOUND 1 )
	ENDIF (MYSQL_INCLUDE_DIR)
ENDIF (MYSQL_LIBRARY)

MARK_AS_ADVANCED( MYSQL_FOUND MYSQL_LIBRARY MYSQL_EXTRA_LIBRARIES MYSQL_INCLUDE_DIR )

