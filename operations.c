#include "operations.h"
#include "record.h"
#include "includes.h"
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


commandline_ops *commands = NULL;


void commandline_parser(char* argv[],int argc){ 
    
    commands = malloc(argc * sizeof(commandline_ops));
    if (!commands) {
        perror("malloc failed");
        exit(EXIT_FAILURE);
    }

    memset(commands,0,argc*sizeof(commands));
    int flag_counter=0;
    int value_counter=0;
    for(int i=1;i<argc;i++){
        if(argv[i][0]=='-' && argv[i][1]=='-' && strlen(argv[i])>1){
            strcpy(commands[flag_counter].flag,argv[i]);
            printf("%s\n",commands[flag_counter].flag);    
            value_counter=0;
            flag_counter++;
        }
        else{
            strcpy(commands[flag_counter-1].value[value_counter],argv[i]);
            printf("%s\n",commands[flag_counter-1].value[value_counter]);
            value_counter++;
        }
    }
    // OpsArgument args = {.condition="",.district_id="",.report_id="",.role="",.user="",.value=0,.report_id=0};
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


void list(const OpsArgument* arg){
    
}
static int next_id(const char *path) {
    int fd = open(path, O_RDONLY);
    if (fd == -1) return 1;
    int max = 0;
    Record r;
    while (read(fd, &r, sizeof r) == (ssize_t)sizeof r)
        if (r.id > max) max = r.id;
    close(fd);
    return max + 1;
}

void add(const OpsArgument* arg) {
    DIR* district = setup_district(arg->district_id);
    if (!district) return;
    closedir(district);
 
    char reports_path[DISTRICTNAMESIZE+DEFAULTNAMESIZE+DEFAULTFILESSIZE+2];
    snprintf(reports_path, sizeof reports_path,
             "%s/%s/%s", DEFAULTFOLDERNAME, arg->district_id, DEFAULTREPORTNAME);
 
    struct stat st;
    if (stat(reports_path, &st) == 0) {
        if (strcmp(arg->role, "manager")   == 0 && !(st.st_mode & S_IWUSR)) {
            fprintf(stderr, "PERMISSION DENIED: manager cannot write reports.dat\n");
            return;
        }
        if (strcmp(arg->role, "inspector") == 0 && !(st.st_mode & S_IWGRP)) {
            fprintf(stderr, "PERMISSION DENIED: inspector cannot write reports.dat\n");
            return;
        }
    }
 
    int fd = open(reports_path, O_WRONLY|O_APPEND|O_CREAT, REPORT_PERMISSIONS);
    if (fd == -1) { perror("add: open"); return; }
    chmod(reports_path, REPORT_PERMISSIONS);
 
    Record r;
    memset(&r, 0, sizeof r);
    r.id        = next_id(reports_path);
    r.timestamp = time(NULL);
    strncpy(r.inspectorName, arg->user, INSPECTOR_NAME_SIZE - 1);
 
    printf("X:  "); scanf("%lf", &r.gpsCoordinates[0]);
    printf("Y:  "); scanf("%lf", &r.gpsCoordinates[1]);
    printf("Category (road,lightning,flooding,other):  "); scanf("%49s", r.issueCategory);
    printf("Severity (1-3):  "); scanf("%d",  &r.severityLevel);
    getchar();
    printf("Description:  "); fgets(r.description, DESCRIPTION_SIZE, stdin);
    r.description[strcspn(r.description, "\n")] = '\0';
 
    if (write(fd, &r, sizeof r) != (ssize_t)sizeof r)
        perror("add: write");
    else
        printf("Report %d added to district '%s'.\n", r.id, arg->district_id);
    close(fd);
 
    char link[DISTRICTNAMESIZE+32];
    snprintf(link, sizeof link, "active_reports-%s", arg->district_id);
    struct stat lst;
    if (lstat(link, &lst) == 0) {
        if (S_ISLNK(lst.st_mode) && stat(link, &st) != 0)
            fprintf(stderr, "WARNING: dangling symlink %s\n", link);
    } else {
        symlink(reports_path, link);
    }
}
void remove_report(const OpsArgument* arg){}
void filter(const OpsArgument* arg){}
void update_threshold(const OpsArgument* arg){}
void view(const OpsArgument* arg){}
