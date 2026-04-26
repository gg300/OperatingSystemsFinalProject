#ifndef OPERATIONS_H
#define OPERATIONS_H

#include "includes.h"

/// Define permission levels for different file types

#define DISTRICT_PERMISSIONS 0750
#define REPORT_PERMISSIONS 0664
#define CONFIG_PERMISSIONS 0640
#define LOGGED_DISTRICT_PERMISSIONS 0644

#define DEFAULTREPORTNAME "reports.dat"
#define DEFAULTCONFIGNAME "district.cfg"
#define DEFAULTLOGSNAME "logged_district"
#define DEFAULTFOLDERNAME "cities"

#define DEFAULTFLAGNO 8
#define DEFAULTFLAGSIZE 20
#define DISTRICTNAMESIZE 200
#define DEFAULTNAMESIZE (sizeof(DEFAULTFOLDERNAME)-1) //
#define DEFAULTFILESSIZE 17

#define DEFAULTARGUMENTSIZE 200
#define DEFAULTVALUENO 10

typedef struct{ // created just for easier name -> permission matching
    const char* name;
    mode_t permission;
}file_entry;

typedef struct OpsArgument{
    int value,report_id;
    char condition[DEFAULTARGUMENTSIZE],user[DEFAULTARGUMENTSIZE],role[DEFAULTARGUMENTSIZE],district_id[DEFAULTARGUMENTSIZE];
}OpsArgument; // i intent to simplify my function arguments to make it a little more abstract and easier to scale

typedef void (*operation_function)(const OpsArgument* arg);

typedef struct {
    const char* flag;
    operation_function func;
}FlagOperation;

typedef struct commandline_ops{
        char flag[DEFAULTFILESSIZE];
        char value[DEFAULTVALUENO][DEFAULTARGUMENTSIZE];
}commandline_ops;

DIR* setup_default_city_path();

void commandline_parser(char* argv[],int argc); // will parse the command line and register the operations needed using flagops and ops args

int manage_permissions(const char* operation, const char* role); // will handle the permissions of the current operation

void set_permissions(const char* file_type, const char* path);

/// operations
void add(const OpsArgument* arg); 
void list(const OpsArgument* arg);
void view(const OpsArgument* arg);
void remove_report(const OpsArgument* arg); // manager only 
void update_threshold(const OpsArgument* arg); // manager only , call stat() , extract permissions from info.st_mode
void filter(const OpsArgument* arg); // manager only 
DIR* find_district(const char* district_id);
DIR* setup_district(const char* district_id);

void operations_handler();

#endif 