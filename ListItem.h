/*
 *	Created by Eyal Lotan and Dor Sura.
 */


/*
 *	ListItem is a key-value object that contains a logical page number and a vector
 *	of all locations (indexes) in the writing sequence where the logical page number 'page_number' appears
 */

#ifndef FLASHGC_LISTITEM_H
#define FLASHGC_LISTITEM_H

#include <cstdlib>
#include <cassert>
#include <cmath>
#include <vector>
#include <set>
#include "main.hpp"

#define NOT_EXIST -3
using std::vector;

class ListItem{
public:
    /* the logical page number */
    unsigned int page_number;

    /* locations (indexes) in the writing sequence where the logical page number 'page_number' appears */
    vector<unsigned long long> location_list;

    ListItem(unsigned int page_number, unsigned long long location) : page_number(page_number), location_list(vector<unsigned long long>({location})) {}

    /* add a location (index) to the ListItem */
    void addLocation(unsigned long long location) {
        location_list.push_back(location);
    }

    /* get the location of the SECOND write of the logical page.
     * we use this to order pages by the time they are owerwritten
     */
    long long getPageLocation(){
        if (location_list.size() <= 1){
            return NOT_EXIST;
        }
        return location_list[1];
    }

    /* erase the smallest index from location_list */
    void updateLocationList(){
        location_list.erase(location_list.begin());
    }

    /* get the first location of the page*/
    unsigned long long getFirstLocationInList(){
        return location_list.front();
    }

    /* get the last location of the page */
    unsigned long long getLastLocationInList(){
        return location_list.back();
    }

    unsigned long long getFirstLocationAfterIndex(unsigned long long page_index){
        for (auto i : location_list){
            if (i > page_index){
                return i;
            }
        }
        return NUMBER_OF_PAGES;
    }

    vector<unsigned long long> getLocationList() const {
        return location_list;
    }

};




#endif //FLASHGC_LISTITEM_H
