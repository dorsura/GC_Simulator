//
// Created by eyall on 4/16/2021.
//


#include "Auxilaries.h"
#include <iostream>

using namespace std;

unsigned int min(unsigned int a,unsigned int b){
    if (a <= b) return a;
    return b;
}

Algorithm algoStringToEnum(const char* string){
    if (strcmp(string,"greedy") == 0){
        return GREEDY;
    }
    if (strcmp(string,"greedy_lookahead") == 0){
        return GREEDY_LOOKAHEAD;
    }
    if (strcmp(string,"generational") == 0){
        return GENERATIONAL;
    }
    if (strcmp(string,"writing_assignment") == 0){
        return WRITING_ASSIGNMENT;
    }
    return INVALID_ALGO;
}

PageDistribution distributionStringToEnum(const char* string){
    if (strcmp(string,"uniform") == 0){
        return UNIFORM;
    }
    if (strcmp(string,"hot_cold") == 0){
        return HOT_COLD;
    }
    return INVALID_DIST;
}

WindowSizeFlag windowSizeFlagToEnum(const char* string){
    if(strcmp(string, "window_on") == 0)
        return WINDOW_SIZE_ON;
    if(strcmp(string, "window_off") == 0)
        return WINDOW_SIZE_OFF;
    return INVALID_WINDOW_SIZE_FLAG;
}

