#ifndef GEOMETRY
#define GEOMETRY

#include <fstream>
#include "Vector.hpp"
#include <vector>
#include "HashTable.hpp"
#include <tuple>
#include <map>
#include "Sets.hpp"
#include <array>
#include "UUID.hpp"
#include <math.h>

#define MIN_DIST 0.0001

#ifdef _WIN64
#define M_PI 3.14159265358979323846
#include <assert.h>
#include <float.h>
#define MAXFLOAT FLT_MAX
#endif

namespace geo{

//COMMON

/**
 * @brief Calculate area of triangle formed by 3 points
 * 
 * @param p1 
 * @param p2 
 * @param p3 
 * @return float 
 */
float triArea(cgVec3 p1,cgVec3 p2,cgVec3 p3);

/**
 * @brief Get the aspect ratio of the triangle - aspect ratio is larger for a more
 * skinny triangle
 * 
 * @param p1 
 * @param p2 
 * @param p3 
 * @return float 
 */
float triAspectRatio(cgVec3 p1,cgVec3 p2,cgVec3 p3);

uint64_t hash(uint64_t value);

/**
 * @brief Gets normal from face formed by 3 points, normal points along direction defined by
 * clockwise right hand rule for p1,p2,p3 
 * 
 *     p3 *---------* p2
 *         \       /
 *          \     /
 *           \   /
 *            \ /
 *             * p1
 * 
 *          Normal points out of the screen in this case
 * 
 * @param p1 
 * @param p2 
 * @param p3 
 * @return cgVec3 face normal
 */
cgVec3 faceNormal(cgVec3& p1,cgVec3& p2,cgVec3& p3);

/**
 * @brief Defines the edge of a facet ( a line segment between 2 points)
 * 
 */
struct Edge{
    int inds[2];
    Edge(int i1, int i2){inds[0]=i1; inds[1]=i2;}
    Edge(const Edge& e2){inds[0]=e2.inds[0];inds[1]=e2.inds[1];}
    Edge(){inds[0]=0;inds[1]=0;}
    bool operator == (const Edge& e2) const {return (inds[0]==e2.inds[0] && inds[1]==e2.inds[1]) ||  (inds[0]==e2.inds[1] && inds[1]==e2.inds[0]);}
    void print(){std::cout << "{ "<<inds[0]<<", "<<inds[1]<<" }\n";}
    bool contains(int pt){return pt==inds[0] || pt==inds[1];}
    uint64_t getKey(){return hash(inds[0])+hash(inds[1]);}

    struct HashFunction{
    	size_t operator()(const Edge& edge) const
	    {
	    	return hash(edge.inds[0])^hash(edge.inds[1]);
	    }
    };
};

/**
 * @brief Defines a face relative to a set of points
 * 
 */
struct Facet{
    int inds[3]={0,0,0};
    Facet(int i1, int i2, int i3){inds[0]=i1;inds[1]=i2;inds[2]=i3;}
    Facet(const Facet& f){inds[0] = f.inds[0];inds[1] = f.inds[1];inds[2] = f.inds[2];}
    Facet(){inds[0]=0;inds[1]=0;inds[2]=0;}
    bool operator == (const Facet& f1) const;
    void print(){std::cout << "["<<inds[0] <<", "<<inds[1] <<", "<<inds[2] <<"]\n";}
    /**
     * @brief Returns true if this face contains the index ind
     * 
     * @param ind 
     * @return true 
     * @return false 
     */
    bool contains(int ind){return (ind==inds[0])||(ind==inds[1])||(ind==inds[2]); }

    /**
     * @brief Replace old index in facet with new index
     * 
     * @param oldIndex index to replace
     * @param newIndex index to replace oldindex with
     * @return true oldIndex exists and was replaced
     * @return false oldIndex did not exist
     */
    bool replace(int oldIndex, int newIndex);
    
    /**
     * @brief Returns true if the edge is contained by the facet
     * 
     * @param e 
     * @return true 
     * @return false 
     */
    bool contains(Edge& e);
    uint64_t getKey(){return hash(inds[0])+hash(inds[1])+hash(inds[2]);} //ordering of indices shouldnt matter

    /**
     * @brief Returns true if the two facets share an edge
     * 
     * @param f2 
     * @return true 
     * @return false 
     */
    bool sharesEdge(Facet& f2);

    /**
     * @brief returns a unique key - depends on the ordering of the facets
     * 
     * @return uuid128 
     */
    uuid128 getKey128_unique(){ //hash in a chain
        uuid128 h0 = uuid128((void*)&inds[0], sizeof(int));
        h0.dat[0]+=inds[1];
        uuid128 h1 = uuid128((void*)&h0, sizeof(uuid128));
        h1.dat[0]+=inds[2];
        uuid128 h2 = uuid128((void*)&h1, sizeof(uuid128));
        return h2;
        }

    /**
     * @brief Get the Key128 object - ordering of points doesnt change key
     * 
     * @return uuid128 
     */
    uuid128 getKey128(){
        uuid128 h0 = uuid128((void*)&inds[0], sizeof(int));
        uuid128 h1 = uuid128((void*)&inds[1], sizeof(int));
        uuid128 h2 = uuid128((void*)&inds[2], sizeof(int));
        return h0^h1^h2;
    }

    struct HashFunctionUnordered{ //used for unordered_set, does not depend on order of indices
    	size_t operator()(const Facet& facet) const
	    {
            uint64_t h0 = hash(facet.inds[0]);
            uint64_t h1 = hash(facet.inds[1]);
            uint64_t h2 = hash(facet.inds[2]);
	    	return (h0^h1^h2);
	    }
    };

    struct HashFunctionOrdered{ //used for unordered_set, depends on order of indices
    	size_t operator()(const Facet& facet) const
	    {
            uint64_t h0 = hash(facet.inds[0]);
            uint64_t h1 = hash(facet.inds[1]^h0);
            uint64_t h2 = hash(facet.inds[2]^h1);
	    	return h2;
	    }
    };
};

/**
 * @brief Gets any shared edges between the 2 facets, if none are found, returns an edge with negative indices
 * 
 * @param f1 
 * @param f2 
 * @return Edge 
 */
Edge getSharedEdge(Facet& f1, Facet& f2);

/**
 * @brief Get the Horizon Edges of the set of facets
 * The horizon edges are defined as all edges that are touching only a single
 * facet
 * 
 * @param facets 
 * @param target edges will be appended into this vector
 */
void getHorizonEdges(std::vector<Facet>& facets, std::vector<Edge>& target);

/**
 * @brief removes unused vertices and remaps the vertex indices to map to the new set of verts
 * 
 * @param og_facets 
 * @param og_vertices 
 * @param new_facets 
 * @param new_vertices 
 */
void remapVertices(std::vector<Facet>& og_facets, std::vector<cgVec3>& og_vertices, std::vector<Facet>& new_facets, std::vector<cgVec3>& new_vertices);

}
#endif /* GEOMETRY */
