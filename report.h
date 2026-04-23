#ifndef REPORT_H
#define REPORT_H

#include <time.h>

#define DESCRIPTION_SIZE 200
#define INSPECTOR_NAME_SIZE 50
#define ISSUE_CATEGORY_SIZE 50

typedef struct Report{
    int id;
    char inspectorName[INSPECTOR_NAME_SIZE];
    double gpsCoordinates[2]; // [latitude, longitude]
    char issueCategory[ISSUE_CATEGORY_SIZE]; //ex: "road", "lighting", "flooding"
    int severityLevel; //1-3
    time_t timestamp;
    char description[DESCRIPTION_SIZE];
} Report;

#endif 