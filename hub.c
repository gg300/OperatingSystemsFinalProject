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
void start_monitor(const hubArguments* arg); // to implement
void calculate_scores(const hubArguments* arg); // to implement

