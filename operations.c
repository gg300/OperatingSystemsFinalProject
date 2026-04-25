#include "operations.h"

FlagOperation flag_ops[] = {
    {.flag="--role",.func=NULL},
    {.flag="--user",.func=NULL},
    {.flag="--list",.func=list},
    {.flag="--add",.func=add},
    {.flag="--remove_report",.func=remove_report},
    {.flag="--filter",.func=filter},
    {.flag="--update_threshold",.func=update_threshold},
    {.flag="--view",.func=view}
};
file_entry default_files[] = {
    { DEFAULTREPORTNAME, REPORT_PERMISSIONS },
    { DEFAULTCONFIGNAME, CONFIG_PERMISSIONS },
    { DEFAULTLOGSNAME, LOGGED_DISTRICT_PERMISSIONS }
};

void list(const OpsArgument* arg){}
void add(const OpsArgument* arg){}
void remove_report(const OpsArgument* arg){}
void filter(const OpsArgument* arg){}
void update_threshold(const OpsArgument* arg){}
void view(const OpsArgument* arg){}

void commandline_parser(char* argv[]){
    printf("%s", argv[1]);
}

int manage_permissions(const char* operation, const char* role){

    return -1; // operation not permitted
}

void set_permissions(const char* file_type,const char* path) {
    if(strcmp(file_type, "district_directory")==0){
        chmod(path,DISTRICT_PERMISSIONS);
    }
    else if(strcmp(file_type, "reports.dat")==0){
        chmod(path,REPORT_PERMISSIONS);
    }
    else if(strcmp(file_type , "district.cfg") == 0){
        chmod(path,CONFIG_PERMISSIONS);
    }
    else if(strcmp(file_type , "logged_district") == 0){
        chmod(path,LOGGED_DISTRICT_PERMISSIONS);
    }
    else {
        perror("FILE TYPE NOT FOUND, NO PERMISSIONS TO BE ADDED");
    }
}
DIR* setup_default_city_path(){
    struct stat st={0};
    if(stat(DEFAULTFOLDERNAME,&st)==-1){ /// check if the default folder already exists
        if(mkdir(DEFAULTFOLDERNAME,0755)!=0){
            perror("Couldn't create the default cities folder");
            return NULL;
        }
    }
    else if(!S_ISDIR(st.st_mode)){
        perror("NAME EXISTS AND IT\'S NOT A DIRECTORY");
        return NULL;
    }
    DIR *dir = opendir(DEFAULTFOLDERNAME);
    if(dir==NULL){
        perror("ERROR WHILE OPENNING THE DEFAULT DIRECTORY");
        return NULL;
    }
    return dir;
}
DIR* setup_district(const char* district_id){
    // creating the district folder
    char creation_path[DISTRICTNAMESIZE+DEFAULTNAMESIZE+1];
    snprintf(creation_path,sizeof(creation_path),"%s/%s",DEFAULTFOLDERNAME,district_id);
    struct stat st={0};
    if(stat(creation_path,&st)==-1){ /// check if the default folder already exists
        if(mkdir(creation_path,DISTRICT_PERMISSIONS)!=0){   /////////// instead of 0755 should be DISTRICT_PERMISSIONS
            perror("COULDN'T CREATE DISTRICT");
            return NULL;
        }
    }
    else if(!S_ISDIR(st.st_mode)){
        perror("DISTRICT NAME EXISTS AND IT\'S NOT A DIRECTORY");
        return NULL;
    }
    // creating the district default files
    char file_path[sizeof(creation_path) + DEFAULTFILESSIZE];
    int file_count = sizeof(default_files)/sizeof(default_files[0]);
    for(int i=0;i<file_count;i++){ 
        snprintf(file_path,sizeof(file_path),"%s/%s",creation_path,default_files[i].name);
        int fd = open(file_path, O_CREAT | O_WRONLY, default_files[i].permission);
        if (fd == -1) {
            perror("ERROR CREATING FILE");
            return NULL;
        }
        close(fd);
    }


    DIR *dir = opendir(creation_path);
    if(dir==NULL){
        perror("ERROR WHILE OPENNING DISTRICT DIRECTORY");
        return NULL;
    }
    return dir;
}
