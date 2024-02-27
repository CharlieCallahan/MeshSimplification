#include "UUID.hpp"

/**
 * @brief initialize the unique seeds!
 * 
 */
uint64_t uuid128::uniqueSeed0 = 0x696969;
uint64_t uuid128::uniqueSeed1 = 0x420420;

std::string uuid128::getHexStr(){
    char temp[3];
    std::string out;
    snprintf(temp,3,"0x");
    // sprintf(temp, "0x");
    out+=temp;
    for(int i = 0; i < 16; i++){
        snprintf(temp,3, "%X",uint8_t( ((char*)(dat))[i] ));
        out = out + temp;
    }
    return out;
}

uuid128::uuid128(const uuid128& b){
    dat[0] = b.dat[0];
    dat[1] = b.dat[1];
}

uuid128::uuid128(){
    dat[0]=0;
    dat[1]=0;
}

uuid128::uuid128(const void* msgSeed, size_t msgLen){
    dat[0] = 0;
    dat[1] = 0;
    SpookyHash::Hash128(msgSeed,msgLen,dat+0,dat+1);
}

uuid128::uuid128(const void* ptr){
    dat[0] = 0;
    dat[1] = 0;
    int sz = sizeof(void*);
    const void* temp = ptr;
    SpookyHash::Hash128(&temp,sz,dat+0,dat+1);
}

uuid128::uuid128(const std::string& keyString){
    dat[0] = 0;
    dat[1] = 0;
    SpookyHash::Hash128(keyString.c_str(),keyString.size(),dat+0,dat+1);
}

void uuid128::setUnique(){
    uint64_t msg[2] = {uniqueSeed0,uniqueSeed1};
    SpookyHash::Hash128((void*)msg,sizeof(uint64_t)*2,dat+0,dat+1);
    uniqueSeed0 = dat[0];
    uniqueSeed1 = dat[1];
}

bool uuid128::operator==(const uuid128& b) const{
    //compare 1st and 2nd 64 bit segments, 2 clock cycles on x64
    return (dat[0]==b.dat[0]) && (dat[1]==b.dat[1]);
}

uuid128 uuid128::operator^(const uuid128& b){
    uuid128 out;
    out.dat[0]=dat[0]^b.dat[0];
    out.dat[1]=dat[1]^b.dat[1];
    return out;
}