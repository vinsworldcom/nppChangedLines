#ifndef RESOURCE_H
#define RESOURCE_H

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

/* VERSION DEFINITIONS */
#define VER_MAJOR   0
#define VER_MINOR   2
#define VER_RELEASE 5
#define VER_BUILD   1
#define VER_STRING  STR(VER_MAJOR) "." STR(VER_MINOR) "." STR(VER_RELEASE) "." STR(VER_BUILD)

#define FILE_DESCRIPTION  "Notepad++ Plugin providing changed line highlighting."
#define INTERNAL_NAME	  "Notepad++ Changed Lines"

#define COMPANY_NAME	  "Vin's World"
#define FILE_VERSION	  VER_STRING
#define LEGAL_COPYRIGHT	  "Copyright (C) VinsWorld. All Rights Reserved."
#define LEGAL_TRADEMARKS  ""
#define ORIGINAL_FILENAME "ChangedLines"
#define PRODUCT_NAME	  "CHANGEDLINES"
#define PRODUCT_VERSION	  VER_STRING

/* ADDITIONAL DEFINITIONS */

#define ENABLE_INDEX 0
#define DOCKABLE_INDEX 1

#define IDB_TB_CL 1001

#endif
