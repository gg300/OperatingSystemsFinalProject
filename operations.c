#include "operations.h"

FlagOperation flag_ops = {
    {"--role",NULL},
    {"--user",NULL},
    {"--list",list},
    {"--add",add},
    { "--remove_report",remove_report},
    {"--filter",filter},
    {"--update_threshold",update_threshold},
    {"--view",view}
};

void commandline_parser(char* argv[]){
    printf("%s", argv[1]);
}

void manage_permissions(const char* operation, const char* role){

}

void set_permissions(const char* file_type,const char* path) {
    if(strcmp(file_type, "district_directory")==0){
        chmod(path,DIRECTORY_PERMISSIONS);
    }
    else if(strcmp(file_type, "reports.dat")==0){
        chmod(path,REPORT_PERMISSIONS);
    }
    else if(strcmp(file_type , "district.cfg") == 0){
        chmod(path,DISTRICT_PERMISSIONS);
    }
    else if(strcmp(file_type , "logged_district") == 0){
        chmod(path,LOGGED_DISTRICT_PERMISSIONS);
    }
    else {
        perror("FILE TYPE NOT FOUND, NO PERMISSIONS TO BE ADDED");
    }
}
