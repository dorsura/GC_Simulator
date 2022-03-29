/*
 *	Created by Alex Yucovich. Edited and expanded by Eyal Lotan and Dor Sura.
 */

/*
 *	Custom random number generator. Adapted from KISS() algorithm by George Marsaglia.
*/

/*
 * The need for the custom generator emerged from the fact that the STL rand()
 * returns only 32768 different numbers (between 0 and 32767), less than the
 * range needed for this simulation. Marsaglia's algorithm is fast and performs
 * very well on randomness tests. The STL rand() is used for seeding.
 */

/* USAGE:
 * seed() - once before using KISS().
 * KISS().
 */

#ifndef MYRAND_H_
#define MYRAND_H_

#include <iostream>
#include <ctime>
#include <cstdlib>
#include <cmath>
#include <random>
#include "main.hpp"
#include "Auxilaries.h"


using namespace std;

static unsigned int x = 123456789, y = 362436000, z = 521288629, c = 7654321; /* Seed variables */

unsigned int KISS() {
	unsigned long long t, a = 698769069ULL;

	x = 69069 * x + 12345;
	y ^= (y << 13);
	y ^= (y >> 17);
	y ^= (y << 5); /* y must never be set to zero! */
	t = a * z + c;
	c = (t >> 32); /* Also avoid setting z=c=0! */

	return x + y + (z = t);
}
void seed() {
	srand(time(nullptr));
	do {
		x = time(nullptr);
		y = pow(rand() % 1621, 3);
		z = pow(rand() % 251, 4);
		c = 104729 * rand();
	} while (y == 0 || z == 0 || c == 0);
}

static std::uniform_int_distribution<int> num_generator_hot;
std::default_random_engine gen_hot;
static std::uniform_int_distribution<int> num_generator_cold;
std::default_random_engine gen_cold;
static std::uniform_int_distribution<int> num_generator_toss;
std::default_random_engine gen_toss;


void setUniformDistributionGenerator(int range_from, int range_to, RandVariable indicator){
    std::default_random_engine generator(time(nullptr));
    std::uniform_int_distribution<int>  distr(range_from, range_to);

    switch (indicator) {
        case HOT:
            num_generator_hot = distr;
            gen_hot = generator;
        case COLD:
            num_generator_cold = distr;
            gen_cold = generator;
        case COIN_TOSS:
            num_generator_toss = distr;
            gen_toss = generator;
    }

}

/* function for generation a number from one of the uniform engines */

int getNumber(RandVariable indicator){
    switch (indicator) {
        case HOT:
            return num_generator_hot(gen_hot);
        case COLD:
            return num_generator_cold(gen_cold);
        case COIN_TOSS:
            return num_generator_toss(gen_toss);
    }
    return -1; // error //
}

/* generate a uniformly distributed writing sequence of length NUMBER_OF_PAGES.
 * the sequence is generated using the KISS generator, and using mod function to
 * get a logical page number between 0 and LOGICAL_PAGE_NUMBER*PAGES_PER_BLOCK - 1
 */

unsigned int* generateUniformlyDistributedWriteSequence(){
    unsigned int* writing_sequence = new unsigned int[NUMBER_OF_PAGES];
    for (unsigned long long i = 0; i < NUMBER_OF_PAGES; ++i) {
        writing_sequence[i] = KISS() % (LOGICAL_BLOCK_NUMBER*PAGES_PER_BLOCK);
    }
    return writing_sequence;
}

/* generate a Hot & Cold pages writing sequence of length NUMBER_OF_PAGES.
 * the sequence is generated using a uniform distribution generator.
 * @param hot_page_percentage is the percentage of hot pages out of total number of logical pages.
 * @p_hot is the probability for a given write to be a hot page write.
 * Note: within each area (Hot/Cold areas) the pages are picked uniformly.
 */

unsigned int* generateHotColdWriteSequence(double hot_page_percentage, double p_hot){
    unsigned int* writing_sequence = new unsigned int[NUMBER_OF_PAGES];

    /* set Hot and Cold distribution engines. The selection of pages within every memory area is uniform */
    setUniformDistributionGenerator(0,(LOGICAL_BLOCK_NUMBER*PAGES_PER_BLOCK)*(double)(hot_page_percentage / 100) , HOT);
    setUniformDistributionGenerator((LOGICAL_BLOCK_NUMBER*PAGES_PER_BLOCK)*(double)(hot_page_percentage / 100) + 1,(LOGICAL_BLOCK_NUMBER*PAGES_PER_BLOCK) - 1 ,COLD);

    /* this is a simple uniform distribution variable to represent a coin toss with probability p */
    setUniformDistributionGenerator(1,10,COIN_TOSS);

    for (unsigned int i = 0; i < NUMBER_OF_PAGES; ++i) {
        int coin_toss = getNumber(COIN_TOSS);
        if (coin_toss <= p_hot*10){
            writing_sequence[i] = getNumber(HOT);
        }
        else {
            writing_sequence[i] = getNumber(COLD);
        }
    }
    return writing_sequence;
}

#endif /* MYRAND_H_ */
