#include "includes.h"
#include "operations.h"

int main(int argc, char *argv[]) {
    DIR* default_dir = setup_default_city_path();
    setup_district("test_district");
    setup_district("teest2district");
    commandline_parser(argv);
    return 0;
}