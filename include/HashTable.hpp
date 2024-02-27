//
//  HashTable.hpp
//  cGraphics
//
//  Created by Charles callahan on 1/13/22.
//  Copyright © 2022 Charles callahan. All rights reserved.
//

#ifndef HashTable_hpp
#define HashTable_hpp

#include <stdio.h>
#include <iostream>
#include <vector>
#include "UUID.hpp"
#include <assert.h>

struct HashTable{
    HashTable(int log2Bins); //log2bins > # of bins = 2^log2bins
    ~HashTable();
    void add(void* value, uint64_t lookupValue); //lookup value is used to generate the hash to store value
    void* get(uint64_t lookupValue); //gets pointer at lookupValue
    void remove(uint64_t lookupValue); //removes element at lookupValue
    void* pop(uint64_t lookupValue); //gets pointer at lookupValue and removes it (faster than get then remove)
    uint32_t hash(uint64_t value);
    int calcSize(); //calculates the number of elements in the table, dont do this every loop iteration if you care about performance...
    void clear(); //removes all bins from table
    //For iterating through hash table, will have more overhead than iterating through an array or vector
    void iterBegin(); //set iterator to first element
    void* iterGetNext(); //gets next hash table element, returns NULL at the end
    struct element{
        element(void* value, uint64_t lookupValue){this->value = value; this->lookupValue=lookupValue;}
        void* value;
        uint64_t lookupValue;
    };
private:
    int log2Bins;
    int numBins; //calculated from 1<<log2Bins, saved for convinience
    std::vector<element>* bins; //ARRAY of vectors.
    int size; //current size
    //iterators
    int iterBin=0; //index of bin
    int iterInd=0; //index in vector
};

/**
 * @brief Stack allocated hash table with that holds copies of elements of type T
 * 
 * @tparam T type of data to be stored in table
 */
template <class T> 
class HashTable_s{
    struct element{
        element(T value, uint64_t lookupValue){this->value = value; this->lookupValue=lookupValue;}
        T value;
        uint64_t lookupValue;
    };
    public:

    HashTable_s(int log2Bins){
        this->log2Bins = log2Bins;
        this->numBins = 1<<log2Bins;
        for (int i = 0; i < numBins; i++){
            this->bins.push_back(std::vector<element>());
        }
        size=0;
    }

    ~HashTable_s(){};

    void add(T value, uint64_t lookupValue){
        size++;
        uint32_t ind = hash(lookupValue);
        bins[ind].push_back(element(value,lookupValue));
    }

    /**
     * @brief Returns true if the element is in the hash table
     * 
     * @param lookupValue 
     * @return true 
     * @return false 
     */
    bool isIn(uint64_t lookupValue){
        uint32_t ind = hash(lookupValue);
        for(int i = 0; i < bins[ind].size(); i++){
            if (bins[ind][i].lookupValue == lookupValue){
                return true;
            }
        }
        return false;
    }

    /**
     * @brief gets element in table, may break if the element doesnt have a defauly constructor
     * 
     * @param lookupValue 
     * @return T 
     */
    T get(uint64_t lookupValue){
        uint32_t ind = hash(lookupValue);
        for(int i = 0; i < bins[ind].size(); i++){
            if (bins[ind][i].lookupValue == lookupValue){
                return bins[ind][i].value;
            }
        }
        return T();
    }

    T* getPtr(uint64_t lookupValue){
        uint32_t ind = hash(lookupValue);
        for(int i = 0; i < bins[ind].size(); i++){
            if (bins[ind][i].lookupValue == lookupValue){
                return &bins[ind][i].value;
            }
        }
        return NULL;
    }

    void remove(uint64_t lookupValue){
        uint32_t ind = hash(lookupValue);
        for(int i = 0; i < bins[ind].size(); i++){
            if (bins[ind][i].lookupValue == lookupValue){
                bins[ind].erase(bins[ind].begin()+i);
                size--;
                return;
            }
        }
        std::cout << "ERROR: object missing from hash table_s...\n";
        exit(0);
    }

    
    uint32_t hash(uint64_t value){
        uint32_t x = value;
        x = ((x >> 16) ^ x) * 0x45d9f3b;
        x = ((x >> 16) ^ x) * 0x45d9f3b;
        x = (x >> 16) ^ x;
        return (x>>(32-log2Bins));
    }

    int getSize(){
        return size;
    }

    void clear(){
        size=0;
        for (int i = 0; i < numBins; i++){
            this->bins[i].clear();
        }
    }

    //For iterating through hash table, will have more overhead than iterating through an array or vector
    void iterBegin(){
        iterBin=0;
        iterInd=-1;
    }

    /**
     * @brief Iterates to next element in table, sets isFinished to true when
     * the iterator reaches the end of the table
     * 
     * @param isFinished 
     * @return T 
     */

    T iterGetNext(bool& isFinished){
        if (iterInd >= int(bins[iterBin].size())-1 ) { //returned last element in bin
            iterBin++;
            while(iterBin < numBins){
                if (bins[iterBin].size() == 0)
                    iterBin++;
                else
                    break;
            }
            iterInd=0;
        } else {
            iterInd++;
        }

        if (iterBin >= numBins){
            isFinished = true;
            return T();
        }

        element elem = bins[iterBin][iterInd];
        isFinished = false;
        return elem.value;
    }

private:
    int log2Bins;
    int numBins; //calculated from 1<<log2Bins, saved for convinience
    std::vector<std::vector<element>> bins; //vector of vectors.
    int size; //current size
    //iterators
    int iterBin=0; //index of bin
    int iterInd=0; //index in vector
};

/**
 * @brief Stack allocated hash table with that holds copies of elements of type T
 * and 128 bit keys
 * 
 * @tparam T type of data to be stored in table
 */
template <class T> 
class HashTable_s128{
    struct element{
        element(T value, uuid128 lookupValue){this->value = value; this->lookupValue=lookupValue;}
        T value;
        uuid128 lookupValue;
    };
    
    public:

    HashTable_s128(){}

    HashTable_s128(int log2Bins){
        this->log2Bins = log2Bins;
        this->numBins = 1<<log2Bins;
        for (int i = 0; i < numBins; i++){
            this->bins.push_back(std::vector<element>());
        }
        size=0;
    }

    ~HashTable_s128(){};

    void add(T value, uuid128 lookupValue){
        size++;
        uint32_t ind = hash(lookupValue);
        bins[ind].push_back(element(value,lookupValue));
    }

    /**
     * @brief Returns true if the element is in the hash table
     * 
     * @param lookupValue 
     * @return true 
     * @return false 
     */
    bool isIn(uuid128 lookupValue){
        uint32_t ind = hash(lookupValue);
        for(int i = 0; i < bins[ind].size(); i++){
            if (bins[ind][i].lookupValue == lookupValue){
                return true;
            }
        }
        return false;
    }

    /**
     * @brief gets element in table, may break if the element doesnt have a defauly constructor
     * 
     * @param lookupValue 
     * @return T 
     */
    T get(uuid128 lookupValue){
        uint32_t ind = hash(lookupValue);
        for(int i = 0; i < bins[ind].size(); i++){
            if (bins[ind][i].lookupValue == lookupValue){
                return bins[ind][i].value;
            }
        }
        
        return T();
    }

    T* getPtr(uuid128 lookupValue){
        uint32_t ind = hash(lookupValue);
        for(int i = 0; i < bins[ind].size(); i++){
            if (bins[ind][i].lookupValue == lookupValue){
                return &bins[ind][i].value;
            }
        }
        return NULL;
    }

    void remove(uuid128 lookupValue){
        uint32_t ind = hash(lookupValue);
        for(int i = 0; i < bins[ind].size(); i++){
            if (bins[ind][i].lookupValue == lookupValue){
                bins[ind].erase(bins[ind].begin()+i);
                size--;
                return;
            }
        }
        std::cout << "ERROR: object missing from hash table_s...\n";
        assert(false);
        // exit(0);
    }

    
    uint32_t hash(uuid128 value){
        uint32_t x = uint32_t(value.dat[0] ^ value.dat[1]);
        x = ((x >> 16) ^ x) * 0x45d9f3b;
        x = ((x >> 16) ^ x) * 0x45d9f3b;
        x = (x >> 16) ^ x;
        return (x>>(32-log2Bins));
    }

    int getSize(){
        return size;
    }

    void clear(){
        size=0;
        for (int i = 0; i < numBins; i++){
            this->bins[i].clear();
        }
    }

    //For iterating through hash table, will have more overhead than iterating through an array or vector
    void iterBegin(){
        iterBin=0;
        iterInd=-1;
    }

    /**
     * @brief Iterates to next element in table, sets isFinished to true when
     * the iterator reaches the end of the table
     * 
     * @param isFinished 
     * @return T 
     */

    T iterGetNext(bool& isFinished){
        if (iterInd >= int(bins[iterBin].size())-1 ) { //returned last element in bin
            iterBin++;
            while(iterBin < numBins){
                if (bins[iterBin].size() == 0)
                    iterBin++;
                else
                    break;
            }
            iterInd=0;
        } else {
            iterInd++;
        }

        if (iterBin >= numBins){
            isFinished = true;
            return T();
        }

        element elem = bins[iterBin][iterInd];
        isFinished = false;
        return elem.value;
    }

private:
    int log2Bins;
    int numBins; //calculated from 1<<log2Bins, saved for convinience
    std::vector<std::vector<element>> bins; //vector of vectors.
    int size; //current size
    //iterators
    int iterBin=0; //index of bin
    int iterInd=0; //index in vector
};

/**
 * @brief Hash table with 128 bit keys, makes collisions less likely
 * 
 */
struct HashTable128{
    HashTable128(int log2Bins); //log2bins > # of bins = 2^log2bins
    ~HashTable128();
    void add(void* value, uuid128 lookupValue); //lookup value is used to generate the hash to store value
    void* get(uuid128 lookupValue); //gets pointer at lookupValue
    void remove(uuid128 lookupValue); //removes element at lookupValue
    void* pop(uuid128 lookupValue); //gets pointer at lookupValue and removes it (faster than get then remove)
    uint32_t hash(uuid128 value); //this hashes the value to get a bin to put it into
    int calcSize(); //calculates the number of elements in the table, dont do this every loop iteration if you care about performance...
    void clear(); //removes all bins from table
    //For iterating through hash table, will have more overhead than iterating through an array or vector
    void iterBegin(); //set iterator to first element
    void* iterGetNext(); //gets next hash table element, returns NULL at the end
    struct element{
        element(void* value, uuid128 lookupValue){this->value = value; this->lookupValue=lookupValue;}
        void* value;
        uuid128 lookupValue;
    };
private:
    int log2Bins;
    int numBins; //calculated from 1<<log2Bins, saved for convinience
    std::vector<element>* bins; //ARRAY of vectors.
    int size; //current size
    //iterators
    int iterBin=0; //index of bin
    int iterInd=0; //index in vector
};


#endif /* HashTable_hpp */
