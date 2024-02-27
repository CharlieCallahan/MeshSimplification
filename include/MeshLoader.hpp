#ifndef MESHLOADER
#define MESHLOADER
#include <vector>
#include <string>
#include "Vector.hpp"
#include "UUID.hpp"

struct vertIndices{
    int inds[3];
    vertIndices(){inds[0]=0;inds[1]=0;inds[2]=0;}
    vertIndices(int ind1, int ind2, int ind3){inds[0]=ind1;inds[1]=ind2;inds[2]=ind3;}

    vertIndices(const vertIndices& v){inds[0]=v.inds[0];inds[1]=v.inds[1];inds[2]=v.inds[2];}
    uint64_t getKey(){
        uuid128 h0 = uuid128((void*)&inds[0], sizeof(int));
        h0.dat[0]+=inds[1];
        uuid128 h1 = uuid128((void*)&h0, sizeof(uuid128));
        h1.dat[0]+=inds[2];
        uuid128 h2 = uuid128((void*)&h1, sizeof(uuid128));
        return h2.dat[0]; }

    uuid128 getKey128(){
        uuid128 h0 = uuid128((void*)&inds[0], sizeof(int));
        h0.dat[0]+=inds[1];
        uuid128 h1 = uuid128((void*)&h0, sizeof(uuid128));
        h1.dat[0]+=inds[2];
        uuid128 h2 = uuid128((void*)&h1, sizeof(uuid128));
        return h2;
    }
};

struct objItem{
    ~objItem(){

    }
    std::vector<cgVec3> positions;
    std::vector<cgVec3> normals;
    std::vector<cgVec3> tangents;
    bool hasTangents=false;
    std::vector<cgVec2> textCoords;
    std::vector<int> indices;

    std::string matName; //material name for lookup in assetDict
    std::string name;
    std::string filename;
    std::string mtllib;
    
    void print(){
        std::cout << "OBJ Load target:\n";
        std::cout << "name: "<<name<<"\n";
        std::cout << "num Verts: "<<indices.size()<<"\n";
        std::cout << "Material: "<<matName<<"\n";
    }
};

/**
 * @brief Loads data from a .obj file into target vectors
 * vertex targets = a vector of vectors (one vector per object )containing positions,
 * normals and texture coordinates describing the object.
 * each vertex is a unique (per object) set of positions normals and texture coords
 * 
 * the indextargets is filled with a vector per object, describing the 
 * 
 * @param filename 
 * @param target
 * @param objName name used to tag textures
 */

void loadOBJFile(std::string filename, 
                 std::vector<objItem*>& target, std::string object_id);


#endif /* MESHLOADER */
