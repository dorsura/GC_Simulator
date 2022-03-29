/*
 *	Created by Eyal Lotan and Dor Sura. Based on original design and implementation of Alex Yucovich.
 */

#ifndef FTL_HPP_
#define FTL_HPP_

#include <cstdlib>
#include <cassert>
#include <algorithm>
#include <cmath>
#include <list>
#include <set>
#include <map>
#include <vector>
#include "Auxilaries.h"
#include "main.hpp"

/* Main module for the Flash simulation */

#define NA	-15
#define BLOCK_FULL -71

using std::map;
using std::vector;
using std::set;
using std::pair;

/* forward decleration */
class PhysicalPage;

/* The Logical Page data structure */

class LogicalPage {
public:

	/* pointer to the physical page the logical page is mapped to */

	PhysicalPage* physicalPage;

	/* logical page status: FREE_LOGICAL or USED_LOGICAL */

	LogicalPageStatus status;

	LogicalPage() :
			physicalPage(nullptr), status(FREE_LOGICAL) {
	}

	void clear() {
		physicalPage = nullptr;
		status = FREE_LOGICAL;
	}

};

/* the Physical Page data structure */

class PhysicalPage {
public:

	/* the number of the physical block */

	int blockNo;

	/* the number of the physical page within the block */

	int pageNo;

	/* physical page status: 	FREE_PHYSICAL - Unused page
	 * 							OBSOLETE - used but a logical page is no longer
	 * 							mapped to this page.
	 * 							VALID - a logical page is mapped to this page
	 */

	PhysicalPageStatus status;

	/* pointer to the logical page which is mapped to this physical page */

	LogicalPage* logicalPage;

	PhysicalPage() :
			blockNo(NA), pageNo(NA), status(FREE_PHYSICAL), logicalPage(nullptr) {
	}

	/* obsolete handling */

	void obsolete() {
		status = OBSOLETE;
		logicalPage = nullptr;
	}
};

/* the Physical Block data structure */

class Block {
public:

	/* physical block number */

	int blockNo;

	/* dynamic array of physical pages of histogram_size PAGES_PER_BLOCK */

	PhysicalPage* pages;

	/* number of logical pages mapped to this block */

	int valid;

	/* next free page in the range [0,Z-1] OR NA for full */

	int nextFree;

	Block() :
            blockNo(NA), pages(new PhysicalPage[PAGES_PER_BLOCK]), valid(0), nextFree(
					0) {
		for (int i = 0; i < PAGES_PER_BLOCK; i++) {
			pages[i].pageNo = i;
		}
	}
	~Block() {
		delete[] pages;
	}

	/* obsolete pages update */

	void obsolete(PhysicalPage* page) {
		valid--;
		page->obsolete();
	}


	/* all valid pages are rewritten contiguously from the beginning of the
	 * block
	 */

	void copyValidToTempAndClean(char* data, LogicalPage* logicalPages[],
                                 int* counter) {
		*counter = 0;

		/* read valid data to temp buffer */

		for (int i = 0; i < PAGES_PER_BLOCK; i++) {
			if (pages[i].status == VALID) {
				read(data + (*counter) * PAGE_SIZE, pages[i].logicalPage);
				logicalPages[*counter] = pages[i].logicalPage;
				(*counter)++;
			}
			pages[i].status = FREE_PHYSICAL;
			pages[i].logicalPage = nullptr;
		}

		valid = 0;
		nextFree = 0;
	}

	/* if block is full, perform clean.
	 * write data to one physical page.
	 */

	int write(char* data, LogicalPage* page) {
		PhysicalPage* current = &(pages[nextFree]);
		page->physicalPage = current;
		page->status = USED_LOGICAL;
		current->logicalPage = page;
		current->status = VALID;
		valid++;
		if (nextFree == PAGES_PER_BLOCK - 1) {
			nextFree = BLOCK_FULL;
			return BLOCK_FULL;
		}
		else {
			nextFree++;
			return nextFree;
		}
	}

	/* read one page. this is a theoretical implementation for now
	 * since we don't use the read function in our simulator.
	 * you can implement this according to your needs.
	 */

	void read(char* buffer, LogicalPage* page) {
	    // read data to buffer...
	}
};

/* the FTL (Flash Transmission Layer) data structure */

class FTL {
public:

	/* Mapping table of the logical pages */

	LogicalPage* mappingTable;

	/* Array of blocks */

	Block** blocks;

	/* List of pointers to free pages */

	std::list<Block*> freeList;

	/* V is an array of sets of integers of histogram_size PAGES_PER_BLOCK+1. each
	 * integer refers to a block number.
	 * set V[i], 0<=i<=PAGES_PER_BLOCK, is a set of all the blocks with i valid
	 * pages.
	 */

	set<int>* V;

	/* Y - the minimum number of valid pages in a block */

	int Y;

	/* total number of block erases */

	int erases;

	/* number of block erases in steady state phase */

	int erases_steady;

	/* logical page writes */

	int logicalPageWrites;
	int logicalPageWritesSteady;

	/* physical page writes */

	int physicalPageWrites;
    int physicalPageWritesSteady;

	/* blocks for writing pages by generation, used for generational GC algorithm */
	map<int, Block*> gen_blocks;

	/* indicator for printing mode. if turned on, the V array will be printed with every block erasure,
	 * along with the number of logical page writes and Y
	 */
	bool print_mode;

    /* optimized parameters for the given OP that we are currently running with.
     * This is a pair (n,num_of_generations) where:
     * n - the denominator's power in the block score function
     * num_of_generations - best number of generations for generational algorithm
     * */
    std::pair<int,int> optimized_params;

	explicit FTL() :
            mappingTable(
					new LogicalPage[LOGICAL_BLOCK_NUMBER * PAGES_PER_BLOCK]), blocks(
					new Block*[PHYSICAL_BLOCK_NUMBER]), V(
					new set<int> [PAGES_PER_BLOCK + 1]), Y(0), erases(0), erases_steady(0), logicalPageWrites(
					0), logicalPageWritesSteady(0), physicalPageWrites(0), physicalPageWritesSteady(0),
            print_mode(false) {
		for (int i = 0; i < PHYSICAL_BLOCK_NUMBER; i++) {
			blocks[i] = new Block;
			blocks[i]->blockNo = i;
			freeList.push_back(blocks[i]);
			for (int j = 0; j < PAGES_PER_BLOCK; j++) {
				blocks[i]->pages[j].blockNo = i;
			}
		}
        optimized_params.first = getOptimizedAlphaValParam();
		optimized_params.second = std::max((int)min(LOGICAL_BLOCK_NUMBER/OVER_LOADING_FACTOR, PHYSICAL_BLOCK_NUMBER-LOGICAL_BLOCK_NUMBER), 1);
    }

	~FTL() {
		delete[] mappingTable;
		for (int i = 0; i < PHYSICAL_BLOCK_NUMBER; i++) {
			delete blocks[i];
		}
		delete[] blocks;
		delete[] V;
	}

	void printHeader() {
		cout << "Erases\t\tLogical Writes\tY\t";
		for (int i = 0; i < PAGES_PER_BLOCK + 1; i++) {
			cout << "V[" << i << "]\t";
		}
		cout << endl;
	}


	// not including blocks in freelist
	int getNumberOfValidPages(){
	    int counter = 0;
	    for (int i=0 ; i < PAGES_PER_BLOCK+1 ; i++){
	        counter = counter + (V[i].size() * i);
	    }
	    for (auto block : freeList){
	        counter += block->valid;
	    }
	    return counter;
	}

	/* calculate window size auxiliary for writing assignment algorithm.
	 * full window size calculation is provided in the written report.
	 */
	unsigned int windowSizeAux(){
        int minValid = updateMinValid();

        int counter = 0;
        if (minValid <= PAGES_PER_BLOCK){
            counter = minValid * V[minValid].size();
        }

        for (int i = minValid+1 ; i < PAGES_PER_BLOCK+1 ; i++){
            counter += V[i].size() * PAGES_PER_BLOCK;
        }

        for (auto block : freeList){
            counter += block->nextFree;
        }

        return counter;
	}

	/* brute force find of the block with minimum number of valid pages. for
	 * testing only.
	 */

	Block* choseMinValidOld() {
		Block* chosen1 = NULL;
		int temp1;
		int minValid1 = PAGES_PER_BLOCK + 1;
		for (int i = 0; i < PHYSICAL_BLOCK_NUMBER; i++) {
			temp1 = blocks[i]->valid;
			if (temp1 < minValid1 && (blocks[i]->nextFree == BLOCK_FULL)) {
				chosen1 = blocks[i];
				minValid1 = temp1;
			}
		}

		return chosen1;
	}

	/* finding the block with minimum number of valid pages algorithm:
	 * go over V1 and V2 from i=0 to the top and find the first i for which
	 * V1[i] or V2[i] is not empty.
	 * complexity: update of data structures is O(log(PHYSICAL_BLOCK_NUMBER))
	 * (the complexity of inserting and deleting from the sets) for each write.
	 * retrieval of the block with minimum valid pages is O(PAGES_PER_BLOCK).
	 * now, since when memory is full, number of writes between erases is
	 * O(PAGES_PER_BLOCK) we get a very fast O(PAGES_PER_BLOCK*log(PHYSICAL_BLOCK_NUMBER)) algorithm.
	 */

	/* returns a pointer to the minimum block on 1st write.
	 * performs this simply by going over N1 and finding the minimal i for
	 * which V[i] is not empty.
	 */

	Block* minBlock() {
		updateMinValid();
		return blocks[*(V[Y].begin())];
	}

	/* given a LogicalPage object, find the logical page number */
    int getLogicalPageNumber(LogicalPage* logical_page) const{
        for (int lpn = 0 ; lpn < LOGICAL_BLOCK_NUMBER * PAGES_PER_BLOCK; ++lpn) {
            if (&mappingTable[lpn] == logical_page){
                return lpn;
            }
        }
        return -1 ; // error
	}

	/* function to calculate a block score given a writing sequence and base index to
	 * search from. the block score is calculated by taking into account the age of all
	 * pages in the block. for full description of the function logic, parameter adjustment
	 * experiments and graph results - please see written report
	 */
	double getBlockScore(int block_num, unsigned long long base_index, unsigned int* writing_sequence) const{
        assert(block_num >= 0);
	    Block* curr_block = blocks[block_num];
        set<int> pages_in_block;
        for (int i = 0; i < PAGES_PER_BLOCK; ++i) {
            if (curr_block->pages[i].logicalPage){
                pages_in_block.insert(getLogicalPageNumber(curr_block->pages[i].logicalPage));
            }
        }

        double block_score = 0;
        //TODO: should we scan until i < NUMBER_OF_PAGES or until i < base_index + PAGES_PER_BLOCK*LOGICAL_BLOCK_NUMBER ?
        for (unsigned long long i = base_index ; i < base_index + PAGES_PER_BLOCK*PHYSICAL_BLOCK_NUMBER && i < NUMBER_OF_PAGES ; i++){
            if (pages_in_block.find(writing_sequence[i]) != pages_in_block.end()){
                pages_in_block.erase(writing_sequence[i]);
                if (pages_in_block.empty()){
                    return block_score;
                }
            }
            // TODO: adjust the block score function.
            long long div_value = i - base_index;
            block_score += div_value > 0 ? (pages_in_block.size()/(double)pow(div_value,optimized_params.first)) : pages_in_block.size();
        }
        return block_score;
	}

    #define X(lower_bound, upper_bound, i_val) \
        if (OP > lower_bound && OP <= upper_bound){     \
            return i_val;     \
        }
        /**
         * Get the optimized parameters for running the different algorithms on the memory
         * configuration based on the over-provisioning factor.
         * These parameters were found by running empiric experiments.
         * */
        int getOptimizedAlphaValParam()
        {
            float OP = (float)(PHYSICAL_BLOCK_NUMBER-LOGICAL_BLOCK_NUMBER)/LOGICAL_BLOCK_NUMBER;
            ALGO_PARAMS_TABLE
            return -1; // shouldn't get here
        }

    #undef X


    Block* getBestBlockToEvict(unsigned int* writing_sequence, long long base_index) const {

        vector<pair<int, double>> block_scores;

        // TOOD: adjust k
        /* the k parameter is adjustable and will decide the number of blocks to examine for each GC */
        for (int k = Y; k <= Y and k < PAGES_PER_BLOCK; k++) {
            for (int block_num : V[k]){
                assert(block_num >= 0);
                double score = getBlockScore(block_num, base_index, writing_sequence);
                block_scores.emplace_back(pair<int, double>{block_num, score});
            }
        }

        /* sort blocks in descending order by block score */
        std::sort(block_scores.begin(),block_scores.end(),[] (const pair<int,double>& l_val, const pair<int,double>& r_val) {
            return l_val.second > r_val.second;
        });
	    return blocks[block_scores.front().first];
	}

	int updateMinValid(){
        int minValid = 0;
        while (V[minValid].size() == 0 && minValid <= PAGES_PER_BLOCK) {
            minValid++;
        }

        if (minValid > PAGES_PER_BLOCK) {
            return NA;
        }
        Y = minValid;
        return minValid;
	}

	Block* minBlockWithLookAhead(unsigned int* writing_sequence, long long base_index){
        updateMinValid();
        /* if we have blocks with no valid pages, pick one at random (all are
         * equally good)
         */
        if (Y == 0){
            return blocks[*(V[Y].begin())];
        }
        return getBestBlockToEvict(writing_sequence, base_index);
	}

	void updateObsolete(Block* block) const {
		if (block->nextFree == BLOCK_FULL) {
            int valid = block->valid;
            V[valid + 1].erase(block->blockNo);
            V[valid].insert(block->blockNo);
        }

	}

	void copyValidToNewPlace(char* data, LogicalPage* logicalPages[],
                             int counter, Block* to) {
		Block* current = to;
		int result;
		for (int i = 0; i < counter; i++) {
			logicalPages[i]->clear();
			result = current->write(data + i * PAGE_SIZE, logicalPages[i]);
			physicalPageWrites++;
			if (result == BLOCK_FULL) {
				V[current->valid].insert(current->blockNo);
				freeList.pop_front();
				current = freeList.front();
			}
		}
	}

	void blockClean(Block* block) {
		char tempData[PAGES_PER_BLOCK * PAGE_SIZE];
		LogicalPage* logicalPages[PAGES_PER_BLOCK];
		int counter;
		Block* current = freeList.front();
		Block temp;

		block->copyValidToTempAndClean(tempData, logicalPages, &counter);
		copyValidToNewPlace(tempData, logicalPages, counter, current);
	}

	void print() {
		cout << erases << "\t\t" << logicalPageWrites << "\t\t" << Y << "\t";
		for (int i = 0; i < PAGES_PER_BLOCK + 1; i++) {
			cout << V[i].size() << "\t";
		}

		cout << endl;
	}

	void GC() {

		Block* min = minBlock();
		assert(min);

//		assert(min->valid == choseMinValidOld()->valid);

		erases++;
		if (print_mode){
            print();
        }

		freeList.push_back(min);
		assert(!freeList.empty());
		V[min->valid].erase(min->blockNo);
		blockClean(min);

	}

	void printV() {
	    cout<<"blocks status:"<<endl;
        for (int i = 0; i < PAGES_PER_BLOCK+1; i++) {
            cout<<"V["<<i<<"]: ";
            for (int j : V[i]){
                cout<<j<<" ";
            }
            cout<<endl;
        }
	}

    void GCWithLookAhead(unsigned int* writing_sequence, unsigned int base_index) {

        Block* min = minBlockWithLookAhead(writing_sequence, base_index);

        assert(min);

//		assert(min->valid == choseMinValidOld()->valid);

        erases++;
        if (print_mode){
            print();
        }

        freeList.push_back(min);
        assert(!freeList.empty());
        V[min->valid].erase(min->blockNo);
        blockClean(min);
    }


    void updateMappingTable(unsigned int lpn, Block* current) const{
        Block *obsoletePlace =
                blocks[mappingTable[lpn].physicalPage->blockNo];
        obsoletePlace->obsolete(mappingTable[lpn].physicalPage);
        if (obsoletePlace != current) {
            updateObsolete(obsoletePlace);
        }
        mappingTable[lpn].clear();
	}

	void write(char* data, unsigned int lpn , Algorithm algorithm , unsigned int* writing_sequence = nullptr,unsigned long long base_index = NA ) {
        if (freeList.empty()){
            if (algorithm == GREEDY){
                GC();
            }
            else {
                GCWithLookAhead(writing_sequence,base_index);
            }
        }
        Block *current = freeList.front();

        if (mappingTable[lpn].status != FREE_LOGICAL) {
            updateMappingTable(lpn,current);
        }

        int result = current->write(data, &(mappingTable[lpn]));
        physicalPageWrites++;
        assert(current->valid<= PAGES_PER_BLOCK);

        if (result == BLOCK_FULL) {
            V[current->valid].insert(current->blockNo);
            freeList.pop_front();
        }

		logicalPageWrites++;
	}


    Block* getGenerationalBlock(int generation) const{
		return gen_blocks.at(generation);
	}

    void updateGenBlock(int generation, Block* block_to_assign){
		gen_blocks.at(generation) = block_to_assign;
	}

    void writeGenerational(char* data, unsigned int lpn, int generation, unsigned int* writing_sequence, unsigned long long base_index) {
        Block* gen_block = getGenerationalBlock(generation);
        if (!gen_block){
            if (freeList.empty()){
                GCWithLookAhead(writing_sequence,base_index);
            }
            gen_block = freeList.front();
            updateGenBlock(generation,gen_block);
            freeList.pop_front();
        }
        if (mappingTable[lpn].status != FREE_LOGICAL) {
            updateMappingTable(lpn, gen_block);
        }
        int result = gen_block->write(data, &(mappingTable[lpn]));
        physicalPageWrites++;
        assert(gen_block->valid <= PAGES_PER_BLOCK);

        if (result == BLOCK_FULL) {
            V[gen_block->valid].insert(gen_block->blockNo);
            updateGenBlock(generation,nullptr);
        }
        logicalPageWrites++;
    }

    /* this block clean function is quite similar to the original block clean function implemented above,
     * but here we specifically clean the block and then rewrite all valid pages to the same block.
     * This implementation better fits the theoretical model of the GC as learned in class
     */
    void NewBlockClean(Block* block) {
        char data[PAGE_SIZE];
        LogicalPage *logicalPages[PAGES_PER_BLOCK];
        int counter = 0;
        for (int i = 0; i < PAGES_PER_BLOCK; i++) {
            if (block->pages[i].status == VALID) {
                //read(data + (*counter) * PAGE_SIZE, pages[i].logicalPage);
                logicalPages[counter] = block->pages[i].logicalPage;
                counter++;
            }
            block->pages[i].status = FREE_PHYSICAL;
            block->pages[i].logicalPage = nullptr;
        }
        block->valid = 0;
        block->nextFree = 0;

        /* rewrite valid pages to block */
        for (int i = 0; i < counter; i++) {
            logicalPages[i]->clear();
            block->write(data, logicalPages[i]);
            physicalPageWrites++;
        }
    }

    /* this write function writes a logical page to a specific block.
     * if the block is full we preform a block clean and then write the page
     */
    void writeToBlock(char* data, int lpn, int block_number){
	    Block* write_to = blocks[block_number];
	    while (write_to->nextFree == BLOCK_FULL && write_to->valid == PAGES_PER_BLOCK){
	        // error - should not get here. but if we got here we resort to greedy lookahead algorithm.
	        cout<<"block full! wanted to write page number "<<lpn<<" to block: "<<block_number<<endl;
	        write(data,lpn,GREEDY_LOOKAHEAD);
	        return;
	    }

	    if (write_to->nextFree == BLOCK_FULL){
            erases++;
            if (print_mode){
                print();
            }

            freeList.push_back(write_to); // after cleaning this block will have free pages
            V[write_to->valid].erase(write_to->blockNo);
            NewBlockClean(write_to);
	    }

        if (mappingTable[lpn].status != FREE_LOGICAL) {
            updateMappingTable(lpn,write_to);
        }

        int result = write_to->write(data, &(mappingTable[lpn]));
        physicalPageWrites++;

        if (result == BLOCK_FULL) {
            V[write_to->valid].insert(write_to->blockNo);
            freeList.remove(write_to); // delete block from freelist (must be there)
        }

        logicalPageWrites++;
    }

    /* deletes all blocks with Z invalid pages, i.e all the block is invalid. */
    void sweepFullBlocks(){
        for (int i = 0 ; i < PHYSICAL_BLOCK_NUMBER ; i++){
            if (blocks[i]->nextFree == BLOCK_FULL && blocks[i]->valid == 0){
                erases++;
                V[blocks[i]->valid].erase(blocks[i]->blockNo);
                freeList.push_front(blocks[i]);
                NewBlockClean(blocks[i]);
            }
        }
    }

    /* this should used for debugging purposes only. use with small block numbers */
    void printMemoryLayout() const{
        cout<<"       ";
        for (int i = 0; i < PHYSICAL_BLOCK_NUMBER; ++i) {
            cout<<i<<"    "; // block number
        }
        cout<<endl;
        cout<<"     ";
        for (int i = 0; i < PHYSICAL_BLOCK_NUMBER; ++i) {
            cout<<"-----";
        }
        cout<<endl;

        for (int i = 0; i < PAGES_PER_BLOCK; ++i) {
            cout<<i<<"   |"; // page number
            for (int j = 0; j < PHYSICAL_BLOCK_NUMBER; ++j) {
                if(blocks[j]->pages[i].status == OBSOLETE){
                    cout<<" X  |";
                }
                if(blocks[j]->pages[i].status == FREE_PHYSICAL){
                    cout<<"    |";
                }
                if(blocks[j]->pages[i].status == VALID){
                    LogicalPage* logical_page = blocks[j]->pages[i].logicalPage;
                    int k = getLogicalPageNumber(logical_page);
                    if (k/10 == 0){
                        cout<<"  "<<k<<" |";
                    }
                    else {
                        cout<<" "<<k<<" |";
                    }
                }
            }
            cout<<endl<<"     ";
            for (int j = 0; j < PHYSICAL_BLOCK_NUMBER; ++j) {
                cout<<"-----";
            }
            cout<<endl;
        }
	}

	/* get the number of valid page writes in a given block */
    int getValidWritesInBlock(int block_num) const{
        Block* curr_block = blocks[block_num];
        int valid_writes = 0;
        for (int i = 0; i < PAGES_PER_BLOCK; ++i) {
            if (curr_block->pages[i].logicalPage){
                valid_writes++;
            }
        }
        return valid_writes;
	}


	void read(char* buffer, int lpn) {
		if (mappingTable[lpn].status == FREE_LOGICAL) {
			return;
		}
		blocks[mappingTable[lpn].physicalPage->blockNo]->read(buffer,
				&mappingTable[lpn]);
	}


};

#endif /* FTL_HPP_ */
