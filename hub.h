#include "operations.h"

typedef struct hubArguments{
    char districts[DEFAULTARGUMENTSIZE][DISTRICTNAMESIZE];
}hubArguments;

typedef void (*hubfunction)(const hubArguments* arg);

typedef struct {
    const char* flag;
    hubfunction func;
}HubFlagOperation;

//helpers


//handlers
void hub_ops_handler();

//ops
void start_monitor(const hubArguments* arg);
void calculate_scores(const hubArguments* arg);
