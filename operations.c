#include "operations.h"
#include "record.h"
#include "includes.h"

//global variables
FlagOperation flag_ops[] = {
    {.flag="--role",.func=NULL},
    {.flag="--user",.func=NULL},
    {.flag="--list",.func=list},
    {.flag="--add",.func=add},
    {.flag="--remove_report",.func=remove_report},
    {.flag="--filter",.func=filter},
    {.flag="--update_threshold",.func=update_threshold},
    {.flag="--view",.func=view},
    {.flag="--remove_district",.func=remove_district}
};
file_entry default_files[] = {
    { DEFAULTREPORTNAME, REPORT_PERMISSIONS },
    { DEFAULTCONFIGNAME, CONFIG_PERMISSIONS },
    { DEFAULTLOGSNAME, LOGGED_DISTRICT_PERMISSIONS }
};
commandline_ops *commands = NULL;
short unsigned flag_counter=0;


//helpers
static void mode_to_str(mode_t mode, char *buf) {
    buf[0] = (mode & S_IRUSR) ? 'r' : '-';
    buf[1] = (mode & S_IWUSR) ? 'w' : '-';
    buf[2] = (mode & S_IXUSR) ? 'x' : '-';
    buf[3] = (mode & S_IRGRP) ? 'r' : '-';
    buf[4] = (mode & S_IWGRP) ? 'w' : '-';
    buf[5] = (mode & S_IXGRP) ? 'x' : '-';
    buf[6] = (mode & S_IROTH) ? 'r' : '-';
    buf[7] = (mode & S_IWOTH) ? 'w' : '-';
    buf[8] = (mode & S_IXOTH) ? 'x' : '-';
    buf[9] = '\0';
}
static OpsArgument* argument_constructor(commandline_ops command){ // to implement filter
        OpsArgument* arg = calloc(1, sizeof(OpsArgument));
        if(strcmp(command.flag,"--list")==0){
            strcpy(arg->district_id, command.value[0]);
            return arg;
        }
        else if(strcmp(command.flag,"--add")==0){
            strcpy(arg->district_id, command.value[0]);
            return arg;
        }
        else if(strcmp(command.flag,"--remove_report")==0){
            strcpy(arg->district_id, command.value[0]);
            if (command.value[1][0]!='\0') {
                arg->report_id = atoi(command.value[1]);
            }
            return arg;
        }
        else if(strcmp(command.flag,"--update_threshold")==0){
            strcpy(arg->district_id, command.value[0]);
            if (command.value[1][0]!='\0') {
                arg->value = atoi(command.value[1]);
            }
            return arg;
        }
        else if (strcmp(command.flag, "--remove_district") == 0) {
            strcpy(arg->district_id, command.value[0]);
            return arg;
        }
        else if(strcmp(command.flag,"--view")==0){
            strcpy(arg->district_id, command.value[0]);
            if (command.value[1][0]!='\0') {
                arg->report_id = atoi(command.value[1]);
            }
            return arg;
        }
        else if(strcmp(command.flag,"--filter")==0){
            strcpy(arg->district_id, command.value[0]);
            arg->condition[0] = '\0';
            for(int i = 1; i < DEFAULTVALUENO && command.value[i][0] != '\0'; i++){
                if(i > 1) strncat(arg->condition, " ", DEFAULTARGUMENTSIZE - strlen(arg->condition) - 1);
                strncat(arg->condition, command.value[i], DEFAULTARGUMENTSIZE - strlen(arg->condition) - 1);
            }
            return arg;
        }
        return NULL;
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
int manage_permissions(const char* operation, const char* role, mode_t mode){
    int permitted = 0;
    if (strcmp(role, "manager") == 0) {
        if(strcmp(operation, "read")  == 0) 
            permitted= (mode & S_IRUSR) != 0;
        else if(strcmp(operation, "write") == 0)
            permitted= (mode & S_IWUSR) != 0;
    }
    else{
        if (strcmp(role, "inspector") == 0) {
            if (strcmp(operation, "read") == 0)
                permitted = (mode & S_IRGRP) != 0 || (mode & S_IROTH) != 0;
            else if (strcmp(operation, "write") == 0)
                permitted = (mode & S_IWGRP) != 0; // only 664 has this    
        }
        else{
            perror("Unknown role");
            return -1;
        }
    }
    if(!permitted)
        return -1; 
    return 0;    
}
int parse_condition(const char *input, char *field, char *op, char *value) {
    const char *first_colon = strchr(input, ':');
    if (!first_colon) 
        return 0;
    const char *second_colon = strchr(first_colon + 1, ':');
    if (!second_colon) 
        return 0;
    size_t field_len = first_colon - input;
    strncpy(field, input, field_len);
    field[field_len] = '\0';
    size_t op_len = second_colon - (first_colon + 1);
    strncpy(op, first_colon + 1, op_len);
    op[op_len] = '\0';
    strcpy(value, second_colon + 1);

    return (field[0] != '\0' && op[0] != '\0' && value[0] != '\0');
}
int match_condition(Record *r, const char *field, const char *op, const char *value) {
    /// numeric comparison helper (using w macro-like pattern)
#define NUMCMP(actual, opstr, rhs)                  \
    (strcmp(opstr,"==") == 0 ? (actual) == (rhs) :  \
     strcmp(opstr,"!=") == 0 ? (actual) != (rhs) :  \
     strcmp(opstr,"<")  == 0 ? (actual) <  (rhs) :  \
     strcmp(opstr,"<=") == 0 ? (actual) <= (rhs) :  \
     strcmp(opstr,">")  == 0 ? (actual) >  (rhs) :  \
     strcmp(opstr,">=") == 0 ? (actual) >= (rhs) : 0)

    if (strcmp(field, "severity") == 0) {
        int rhs = atoi(value);
        return NUMCMP(r->severityLevel, op, rhs);
    }
    else if (strcmp(field, "timestamp") == 0) {
        long rhs = atol(value);
        return NUMCMP((long)r->timestamp, op, rhs);
    }
    else if (strcmp(field, "category") == 0) {
        int cmp = strcmp(r->issueCategory, value);
        return NUMCMP(cmp, op, 0);
    }
    else if (strcmp(field, "inspector") == 0) {
        int cmp = strcmp(r->inspectorName, value);
        return NUMCMP(cmp, op, 0);
    }

#undef NUMCMP

    fprintf(stderr, "match_condition: unknown field '%s'\n", field);
    return 0;
}
static void notify_monitor(const char *district_id, int report_id) {
    char log_path[DISTRICTNAMESIZE + DEFAULTNAMESIZE + DEFAULTFILESSIZE + 2];
    snprintf(log_path, sizeof log_path, "%s/%s/%s", DEFAULTFOLDERNAME, district_id, DEFAULTLOGSNAME);

    int log_fd = open(log_path, O_WRONLY | O_APPEND | O_CREAT, LOGGED_DISTRICT_PERMISSIONS);
    if (log_fd == -1) {
        perror("notify_monitor: open log");
        return;
    }
    /// Read the monitor PID from .monitor_pid
    int pid_fd = open(MONITOR_PID_FILE, O_RDONLY);
    if (pid_fd == -1) {
        const char *msg = "[monitor] WARN: .monitor_pid not found — monitor could not be informed of the new report\n";
        write(log_fd, msg, strlen(msg));
        close(log_fd);
        return;
    }
    char pid_buf[32];
    memset(pid_buf, 0, sizeof pid_buf);
    ssize_t n = read(pid_fd, pid_buf, sizeof pid_buf - 1);
    close(pid_fd);
    if (n <= 0) {
        const char *msg = "[monitor] WARN: .monitor_pid is empty — monitor could not be informed of the new report\n";
        write(log_fd, msg, strlen(msg));
        close(log_fd);
        return;
    }
    pid_t monitor_pid = (pid_t)atoi(pid_buf);
    if (monitor_pid <= 0) {
        const char *msg = "[monitor] WARN: invalid PID in .monitor_pid — monitor could not be informed of the new report\n";
        write(log_fd, msg, strlen(msg));
        close(log_fd);
        return;
    }
    if (kill(monitor_pid, SIGUSR1) == -1) {
        char msg[256];
        int len = snprintf(msg, sizeof msg, "[monitor] WARN: kill(SIGUSR1) failed for PID %d (errno %d) — monitor could not be informed of report %d\n", (int)monitor_pid, errno, report_id);
        write(log_fd, msg, len);
    } 
    else {
        char msg[256];
        int len = snprintf(msg, sizeof msg, "[monitor] INFO: SIGUSR1 sent to monitor (PID %d) — report %d added to district '%s'\n",(int)monitor_pid, report_id, district_id);
        write(log_fd, msg, len);
        printf("[monitor] Notified monitor (PID %d) of new report %d.\n",(int)monitor_pid, report_id);
    }
    close(log_fd);
}

//handlers
void commandline_parser(char* argv[],int argc){ 
    commands = calloc(argc, sizeof(*commands));
    flag_counter=0;
    int value_counter=0;
    for(int i=1;i<argc;i++){
        if(argv[i][0]=='-' && argv[i][1]=='-' && strlen(argv[i])>1){
            strcpy(commands[flag_counter].flag,argv[i]);
            // printf("%s\n",commands[flag_counter].flag);     for Debugging purposes
            value_counter=0;
            flag_counter++;
        }
        else{
            strcpy(commands[flag_counter-1].value[value_counter],argv[i]);
            // printf("%s\n",commands[flag_counter-1].value[value_counter]);     for Debugging purposes
            value_counter++;
        }
    }
}
void operations_handler(){
    int number_of_commands=flag_counter;
    char role[DEFAULTARGUMENTSIZE],user[DEFAULTARGUMENTSIZE];
    for(int i=0;i<number_of_commands;i++){
        // printf("DEBUG: %s\n",commands[i].flag);
        OpsArgument *arg = argument_constructor(commands[i]);
        if(strcmp(commands[i].flag,"--role")==0){
            strcpy(role,commands[i].value[0]);
        }
        else if(strcmp(commands[i].flag,"--user")==0){
           strcpy(user, commands[i].value[0]);
        }
        if(arg!=NULL){
            strcpy(arg->role,role);
            strcpy(arg->user,user);
            for(int j=0;j<DEFAULTFLAGNO;j++){
                if(strcmp(commands[i].flag,flag_ops[j].flag)==0 && flag_ops[j].func!=NULL)
                    flag_ops[j].func(arg);
            }
            free(arg);
        }
    }
}

//operations
void list(const OpsArgument* arg) {
    char reports_path[DISTRICTNAMESIZE+DEFAULTNAMESIZE+DEFAULTFILESSIZE+2];
    snprintf(reports_path, sizeof reports_path,
             "%s/%s/%s", DEFAULTFOLDERNAME, arg->district_id, DEFAULTREPORTNAME);
 
    struct stat st;
    if (stat(reports_path, &st) != 0) { perror("list: stat"); return; }
 
    if (strcmp(arg->role, "inspector") == 0 && !(st.st_mode & S_IRGRP)) {
        fprintf(stderr, "PERMISSION DENIED: inspector cannot read reports.dat\n");
        return;
    }
 
    char perms[10], mtime[32];
    mode_to_str(st.st_mode, perms);
    strftime(mtime, sizeof mtime, "%Y-%m-%d %H:%M:%S", localtime(&st.st_mtime));
    printf("reports.dat  %s  %lld bytes  %s\n", perms, (long long)st.st_size, mtime);
 
    int fd = open(reports_path, O_RDONLY);
    if (fd == -1) { perror("list: open"); return; }
 
    Record r;
    int count = 0;
    while (read(fd, &r, sizeof r) == (ssize_t)sizeof r) {
        count++;
        char ts[32];
        strftime(ts, sizeof ts, "%Y-%m-%d %H:%M:%S", localtime(&r.timestamp));
        printf("[%d] inspector=%-20s cat=%-12s sev=%d  %s  (%.4f,%.4f)  %s\n",
               r.id, r.inspectorName, r.issueCategory,
               r.severityLevel, ts,
               r.gpsCoordinates[0], r.gpsCoordinates[1],
               r.description);
    }
    close(fd);
 
    if (count == 0) printf("No reports in district '%s'.\n", arg->district_id);
    else            printf("Total: %d report(s).\n", count);
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
    notify_monitor(arg->district_id, r.id);
}
void remove_report(const OpsArgument* arg) {
    if (strcmp(arg->role, "manager") != 0) {
        fprintf(stderr, "PERMISSION DENIED: only manager can remove reports\n");
        return;
    }

    char reports_path[DISTRICTNAMESIZE + DEFAULTNAMESIZE + DEFAULTFILESSIZE + 2];
    snprintf(reports_path, sizeof reports_path,
             "%s/%s/%s", DEFAULTFOLDERNAME, arg->district_id, DEFAULTREPORTNAME);

    struct stat st;
    if (stat(reports_path, &st) == -1) { perror("remove_report: stat"); return; }
    if (manage_permissions("write", arg->role, st.st_mode) != 0) return;

    int fd = open(reports_path, O_RDWR);
    if (fd == -1) { perror("remove_report: open"); return; }

    Record r;
    off_t target_offset = -1;
    off_t current_offset = 0;

    while (read(fd, &r, sizeof r) == (ssize_t)sizeof r) {
    if (r.id == arg->report_id) {
        target_offset = current_offset;
        break;
    }
    current_offset += sizeof r;
}

    if (target_offset == -1) {
        fprintf(stderr, "remove_report: report %d not found in district '%s'\n",
                arg->report_id, arg->district_id);
        close(fd);
        return;
    }

    off_t read_offset  = target_offset + sizeof r;
    off_t write_offset = target_offset;

    while (1) {
        lseek(fd, read_offset, SEEK_SET);
        if (read(fd, &r, sizeof r) != (ssize_t)sizeof r) break;
        lseek(fd, write_offset, SEEK_SET);
        if (write(fd, &r, sizeof r) != (ssize_t)sizeof r) {
            perror("remove_report: write");
            close(fd);
            return;
        }
        read_offset  += sizeof r;
        write_offset += sizeof r;
    }

    if (ftruncate(fd, write_offset) == -1)
        perror("remove_report: ftruncate");
    else
        printf("Report %d removed from district '%s'.\n", arg->report_id, arg->district_id);

    close(fd);
}
void update_threshold(const OpsArgument* arg) {
    // Manager only
    if (strcmp(arg->role, "manager") != 0) {
        fprintf(stderr, "PERMISSION DENIED: only manager can update threshold\n");
        return;
    }

    char cfg_path[DISTRICTNAMESIZE + DEFAULTNAMESIZE + DEFAULTFILESSIZE + 2];
    snprintf(cfg_path, sizeof cfg_path,
             "%s/%s/%s", DEFAULTFOLDERNAME, arg->district_id, DEFAULTCONFIGNAME);

    // Spec: call stat(), verify bits are exactly 640 before writing
    struct stat st;
    if (stat(cfg_path, &st) == -1) { perror("update_threshold: stat"); return; }

    if ((st.st_mode & 0777) != CONFIG_PERMISSIONS) {
        fprintf(stderr, "update_threshold: district.cfg permissions have been altered "
                "(expected 640, got %03o) — refusing to write\n",
                st.st_mode & 0777);
        return;
    }

    if (manage_permissions("write", arg->role, st.st_mode) != 0) return;

    int fd = open(cfg_path, O_WRONLY | O_TRUNC);
    if (fd == -1) { perror("update_threshold: open"); return; }

    char buf[32];
    int len = snprintf(buf, sizeof buf, "threshold=%d\n", arg->value);
    if (write(fd, buf, len) != len)
        perror("update_threshold: write");
    else
        printf("Threshold updated to %d in district '%s'.\n", arg->value, arg->district_id);

    close(fd);
}
void view(const OpsArgument* arg) {
    char reports_path[DISTRICTNAMESIZE + DEFAULTNAMESIZE + DEFAULTFILESSIZE + 2];
    snprintf(reports_path, sizeof reports_path,
             "%s/%s/%s", DEFAULTFOLDERNAME, arg->district_id, DEFAULTREPORTNAME);

    struct stat st;
    if (stat(reports_path, &st) == -1) { perror("view: stat"); return; }
    if (manage_permissions("read", arg->role, st.st_mode) != 0) return;

    int fd = open(reports_path, O_RDONLY);
    if (fd == -1) { perror("view: open"); return; }

    Record r;
    int found = 0;
    while (read(fd, &r, sizeof r) == (ssize_t)sizeof r) {
        if (r.id == arg->report_id) {
            found = 1;
            break;
        }
    }
    close(fd);

    if (!found) {
        fprintf(stderr, "view: report %d not found in district '%s'\n",
                arg->report_id, arg->district_id);
        return;
    }

    char ts[32];
    strftime(ts, sizeof ts, "%Y-%m-%d %H:%M:%S", localtime(&r.timestamp));
    printf("ID: %d\n", r.id);
    printf("Inspector: %s\n", r.inspectorName);
    printf("Timestamp: %s\n", ts);
    printf("GPS: (%.6f, %.6f)\n", r.gpsCoordinates[0], r.gpsCoordinates[1]);
    printf("Category: %s\n", r.issueCategory);
    printf("Severity: %d\n", r.severityLevel);
    printf("Description: %s\n", r.description);
}
void filter(const OpsArgument* arg) {
    char reports_path[DISTRICTNAMESIZE + DEFAULTNAMESIZE + DEFAULTFILESSIZE + 2];
    snprintf(reports_path, sizeof reports_path,
             "%s/%s/%s", DEFAULTFOLDERNAME, arg->district_id, DEFAULTREPORTNAME);

    struct stat st;
    if (stat(reports_path, &st) == -1) { perror("filter: stat"); return; }
    if (manage_permissions("read", arg->role, st.st_mode) != 0) return;
    char cond_buf[DEFAULTARGUMENTSIZE];
    strncpy(cond_buf, arg->condition, DEFAULTARGUMENTSIZE - 1);
    cond_buf[DEFAULTARGUMENTSIZE - 1] = '\0';

    char *tokens[DEFAULTVALUENO];
    int   token_count = 0;
    char *tok = strtok(cond_buf, " ");
    while (tok && token_count < DEFAULTVALUENO) {
        tokens[token_count++] = tok;
        tok = strtok(NULL, " ");
    }

    if (token_count == 0) {
        fprintf(stderr, "filter: no conditions provided\n");
        return;
    }
    char fields[DEFAULTVALUENO][64];
    char ops   [DEFAULTVALUENO][4];
    char values[DEFAULTVALUENO][DEFAULTARGUMENTSIZE];
    for (int i = 0; i < token_count; i++) {
        if (!parse_condition(tokens[i], fields[i], ops[i], values[i])) {
            fprintf(stderr, "filter: malformed condition '%s' "
                    "(expected field:op:value)\n", tokens[i]);
            return;
        }
    }
    int fd = open(reports_path, O_RDONLY);
    if (fd == -1) { perror("filter: open"); return; }
    Record r;
    int matched = 0;
    while (read(fd, &r, sizeof r) == (ssize_t)sizeof r) {
        int pass = 1;
        for (int i = 0; i < token_count && pass; i++)
            if (!match_condition(&r, fields[i], ops[i], values[i]))
                pass = 0;
        if (pass) {
            matched++;
            char ts[32];
            strftime(ts, sizeof ts, "%Y-%m-%d %H:%M:%S", localtime(&r.timestamp));
            printf("[%d] inspector=%-20s cat=%-12s sev=%d  %s  (%.4f,%.4f)  %s\n",
                   r.id, r.inspectorName, r.issueCategory,r.severityLevel, 
                   ts,r.gpsCoordinates[0], r.gpsCoordinates[1], r.description);
        }
    }
    close(fd);
    if (matched == 0)
        printf("No reports matched the given condition(s) in district '%s'.\n",
               arg->district_id);
    else
        printf("Total matched: %d report(s).\n", matched);
}
void remove_district(const OpsArgument* arg) {
    if (strcmp(arg->role, "manager") != 0) {
        fprintf(stderr, "PERMISSION DENIED: only manager can remove a district\n");
        return;
    }
    char district_path[DISTRICTNAMESIZE + DEFAULTNAMESIZE + 2];
    snprintf(district_path, sizeof district_path,
             "%s/%s", DEFAULTFOLDERNAME, arg->district_id);
    struct stat st;
    if (stat(district_path, &st) == -1) {
        fprintf(stderr, "remove_district: district '%s' not found\n",
                arg->district_id);
        return;
    }
    if (!S_ISDIR(st.st_mode)) {
        fprintf(stderr, "remove_district: '%s' exists but is not a directory\n",
                district_path);
        return;
    }
    pid_t pid = fork();
    if (pid == -1) {
        perror("remove_district: fork");
        return;
    }
    if (pid == 0) {
        /*  Child process  */
        char *args[] = { "rm", "-rf", district_path, NULL };
        execv("/bin/rm", args);
        perror("remove_district: execv");
        _exit(1); 
    }

    //  Parent process: wait for the child to finish
    int status;
    if (waitpid(pid, &status, 0) == -1) {
        perror("remove_district: waitpid");
        return;
    }

    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        fprintf(stderr,
                "remove_district: 'rm -rf %s' failed (exit code %d)\n",
                district_path, WEXITSTATUS(status));
        return;
    }
    printf("District '%s' deleted.\n", arg->district_id);

    ///  Remove the active_reports symlink
    char link_name[DISTRICTNAMESIZE + 32];
    snprintf(link_name, sizeof link_name,
             "active_reports-%s", arg->district_id);

    struct stat lst;
    if (lstat(link_name, &lst) == 0) {          /* symlink (or anything) exists */
        if (unlink(link_name) == -1)
            perror("remove_district: unlink symlink");
        else
            printf("Symlink '%s' removed.\n", link_name);
    } else {
        fprintf(stderr,
                "remove_district: symlink '%s' not found — skipping\n",
                link_name);
    }
}
