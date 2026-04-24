#ifndef OPERATIONS_H
#define OPERATIONS_H

#include "includes.h"

/// Define permission levels for different file types

#define DIRECTORY_PERMISSIONS 750
#define REPORT_PERMISSIONS 664
#define DISTRICT_PERMISSIONS 640
#define LOGGED_DISTRICT_PERMISSIONS 644

extern const char* flags[];

void commandline_parser(char* argv[]); // will parse the command line and register the operations needed

void manage_permissions(const char* operation, const char* role); // will handle the permissions of the current operation

void set_permissions(const char* file_type, const char* path);

void add(int district_id, const char* user); // user will be unused but I'll put it in all operations as a convention
void list(int district_id, const char* user);
void view(int district_id,int report_id, const char* user);
void remove_report(int district_id, int report_id, const char* user); // manager only 
void update_threshold(int district_id, int value, const char* user); // manager only 
void filter(int district_id, const char* condition, const char* user); // manager only 

#endif