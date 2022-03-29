/*
 *	Created by Eyal Lotan and Dor Sura.
 */

#ifndef FLASHGC_AUXILARIES_H
#define FLASHGC_AUXILARIES_H

#include <cstring>

typedef enum {
    FREE_LOGICAL, USED_LOGICAL
} LogicalPageStatus;

typedef enum {
    FREE_PHYSICAL, OBSOLETE, VALID
} PhysicalPageStatus;

typedef enum {
    UNIFORM, HOT_COLD, INVALID_DIST
} PageDistribution;

typedef enum {
    GREEDY, GREEDY_LOOKAHEAD, GENERATIONAL, WRITING_ASSIGNMENT, INVALID_ALGO
} Algorithm;

typedef enum {
    HOT, COLD, COIN_TOSS
} RandVariable;

typedef enum {
    WINDOW_SIZE_ON, WINDOW_SIZE_OFF, INVALID_WINDOW_SIZE_FLAG
}WindowSizeFlag;

Algorithm algoStringToEnum(const char* string);

PageDistribution distributionStringToEnum(const char* string);

WindowSizeFlag windowSizeFlagToEnum(const char* string);

unsigned int min(unsigned int a,unsigned int b);

#endif //FLASHGC_AUXILARIES_H
