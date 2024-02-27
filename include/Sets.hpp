//Tools for manipulating sets
#include "HashTable.hpp"
#include <vector>

/**
 * @brief Finds the union between the two sets, with each unique element enumerated exactly once
 * T must have a getKey() method to work.
 * This function will efficiently remove duplicate keys if you pass an empty set as the 1st or 
 * 
 * @param set1 
 * @param set2 
 * @param target 
 */
template <class T>
void setUnion(std::vector<T>& set1, std::vector<T>& set2, std::vector<T>& target){
    int tableSz = std::max(2,int(log2(float(set1.size() + set2.size()))));
    HashTable_s<T> table = HashTable_s<T>(tableSz);

    for (T curr : set1){
        if(!table.isIn(curr.getKey())){
            table.add(curr,curr.getKey());
        }
    }
    for (T curr : set2){
        if(!table.isIn(curr.getKey())){
            table.add(curr,curr.getKey());
        }
    }
    table.iterBegin();
    bool isFinished;
    while(1){
        T curr = table.iterGetNext(isFinished);
        if(isFinished){
            return;
        } else {
            target.push_back(curr);
        }
    }
}

/**
 * @brief Gets the difference between set1 and set2 (set1-set2)
 * The sets shouldnt have any duplicates 
 * 
 * @param set1 
 * @param set2 
 * @param target 
 */
template <class T>
void setDiff(std::vector<T>& set1, std::vector<T>& set2, std::vector<T>& target){
    int tableSz = std::max(2,int(log2(float(set1.size() + set2.size()))));
    HashTable_s<T> table = HashTable_s<T>(tableSz);
    for(T curr : set1){
        table.add(curr, curr.getKey());
    }
    for(T curr : set2){
        if(table.isIn(curr.getKey())){
            table.remove(curr.getKey());
        }
    }
    
    table.iterBegin();
    bool isFinished;
    while(1){
        T curr = table.iterGetNext(isFinished);
        if(isFinished){
            return;
        } else {
            target.push_back(curr);
        }
    }
}