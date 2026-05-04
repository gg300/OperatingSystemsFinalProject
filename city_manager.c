#include "includes.h"
#include "operations.h"

int main(int argc, char *argv[]) {
    if(argc < 2){
        perror("NO VALID OPERATIONS TO BE DONE");
        return -1;
    }
    DIR* default_dir = setup_default_city_path();

    commandline_parser(argv,argc);

    operations_handler();
    // printf("\n DEBUG FLAGSIZE: %d\n",DEFAULTFLAGSIZE);
    return 0;
}