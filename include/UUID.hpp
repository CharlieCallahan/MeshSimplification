#ifndef UUID_HPP
#define UUID_HPP
#include <string>
#include "SpookyV2.hpp"

/**
 * @brief Universally unique identifiers UUID
 * 
 */


/**
 * @brief 128 bit uuid
 * 
 */
struct uuid128{
    uint64_t dat[2];

    uuid128(const uuid128& b);

    /**
     * @brief Construct a new uuid object by hashing a piece of data
     * 
     * @param msgSeed data to hash
     * @param msgLen length of data in bytes
     */
    uuid128(const void* msgSeed, size_t msgLen);

    /**
     * @brief Construct a new uuid128 object by hashing a string
     * 
     * @param keyString 
     */
    uuid128(const std::string& keyString);

    /**
     * @brief Construct a new uuid128 object by hashing a pointer
     * 
     * @param ptr 
     */
    uuid128(const void* ptr);

    uuid128(int integer){
        this->dat[0] = integer;
        this->dat[1] = 0;
    }

    uuid128();

    /**
     * @brief Give the uuid a unique value by hashing the unique seed
     * this is not thread safe!
     */
    void setUnique();

    bool operator==(const uuid128& b) const;

    //bitwise XOR - commutative
    uuid128 operator^(const uuid128& b);
    
    std::string getHexStr();

    //used for storing in std::unordered_set 
    struct HashFunction
    {
        size_t operator()(const uuid128& uuid) const
        {
            return uuid.dat[0] ^ uuid.dat[1];
        }
    };

    static uint64_t uniqueSeed0; //just a unique integer to be incremented every time a uniqe uuid is generated
    static uint64_t uniqueSeed1; 
};


#endif