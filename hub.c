#include "hub.h"
HubFlagOperation flag_ops[] = {
    {.flag="--start_monitor",.func=start_monitor},
    {.flag="--calculate_scores",.func=calculate_scores}
};

commandline_ops* commands;
short unsigned flag_counter=0;
/// calculate scores can handle a maximum of DEFAULTVALUENO districts found in ops.h

//helpers
static hubArguments* argument_constructor(commandline_ops command){ // to implement filter
        hubArguments* arg = calloc(1, sizeof(hubArguments));
        if(strcmp(command.flag,"--calculate_scores")==0){
            arg->districts[0][0] = '\0';
            for(int i = 0; i < DEFAULTVALUENO && command.value[i][0] != '\0'; i++){ /// probably doesn t work
                strncat(arg->districts[i], command.value[i], DEFAULTARGUMENTSIZE - strlen(arg->districts[i]) - 1);
            }
            return arg;
        }
        return NULL;
}

//handlers
void commandline_parser(char* argv[],int argc); // to implement
void hub_ops_handler(){
    int number_of_commands=flag_counter;
    for(int i=0;i<number_of_commands;i++){
        // printf("DEBUG: %s\n",commands[i].flag);
        hubArguments *arg = argument_constructor(commands[i]);
        if(arg!=NULL){
            for(int j=0;j<DEFAULTFLAGNO;j++){
                if(strcmp(commands[i].flag,flag_ops[j].flag)==0 && flag_ops[j].func!=NULL)
                    flag_ops[j].func(arg);
            }
            free(arg);
        }
    }
}

//ops
void start_monitor(const hubArguments* arg){
    (void)arg; 
    pid_t hub_mon_pid = fork();
    if(hub_mon_pid == -1){
        perror("start_monitor: fork hub_mon");
        return;
    }
 
    if(hub_mon_pid > 0){
        printf("[city_hub] Monitor manager started PID: %d).\n",
               (int)hub_mon_pid);
        fflush(stdout);
        return;
    }
    int pipefd[2];
    if(pipe(pipefd) == -1){
        perror("hub_mon: pipe");
        _exit(1);
    }
    pid_t monitor_pid = fork();
    if(monitor_pid == -1){
        perror("hub_mon: fork monitor_reports");
        close(pipefd[0]);
        close(pipefd[1]);
        _exit(1);
    }
 
    if(monitor_pid == 0){
       close(pipefd[0]); // child does not read from the pipe
 
        if(dup2(pipefd[1], STDOUT_FILENO) == -1){
            perror("monitor child: dup2");
            close(pipefd[1]);
            _exit(1);
        }
        close(pipefd[1]); // original write-end fd no longer needed
        execlp("./monitor_reports", "monitor_reports", NULL);
        fprintf(stdout, "MONITOR_ENDED: exec failed – monitor_reports not found\n");
        fflush(stdout);
        _exit(1);
    }
    close(pipefd[1]);
    FILE *pipe_read = fdopen(pipefd[0], "r");
    if(pipe_read == NULL){
        perror("hub_mon: fdopen");
        close(pipefd[0]);
        waitpid(monitor_pid, NULL, 0);
        _exit(1);
    }
 
    char line[512];
    while(fgets(line, sizeof(line), pipe_read) != NULL){
        size_t len = strlen(line);
        if(len > 0 && line[len-1] == '\n')
            line[len-1] = '\0';
 
        if(strncmp(line, "MONITOR_ENDED:", 14) == 0){
            printf("[hub_mon] Monitor has stopped: %s\n", line + 14);
            fflush(stdout);
            break;
        }
        else if(strncmp(line, "MONITOR_ERROR:", 14) == 0){
            printf("[hub_mon] Monitor could not start: %s\n", line + 14);
            fflush(stdout);
            break;
        }
        else{
            printf("[hub_mon] %s\n", line);
            fflush(stdout);
        }
    }
 
    fclose(pipe_read);
    int status;
    if(waitpid(monitor_pid, &status, 0) != -1){
        if(WIFEXITED(status))
            printf("[hub_mon] monitor_reports exited with code %d.\n",
                   WEXITSTATUS(status));
        else if(WIFSIGNALED(status))
            printf("[hub_mon] monitor_reports terminated by signal %d.\n",
                   WTERMSIG(status));
    }
 
    printf("[hub_mon] Hub monitor manager shutting down.\n");
    fflush(stdout);
    _exit(0);
}
void calculate_scores(const hubArguments* arg); // to implement

