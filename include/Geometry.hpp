#ifndef GEOMETRY_HPP
#define GEOMETRY_HPP

#include <fstream>
#include "cgVec.hpp"
#include <vector>
#include "HashTable.hpp"
#include <tuple>
#include <map>
#include "Sets.hpp"
#include <array>
#include "UUID.hpp"
#include "Array.hpp"
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
 * @brief Length of line segment formed by 2 points
 * 
 * @param p1 
 * @param p2 
 * @return float 
 */
float segmentLength(cgVec3 p1, cgVec3 p2);

/**
 * @brief Calculates tangent vector from 3 triangle corners and 3 texture coordinates
 * used for generating tbn matrix
 * 
 * @param p0 
 * @param p1 
 * @param p2 
 * @param t0 
 * @param t1 
 * @param t2 
 * @return cgVec3 
 */
cgVec3 calcTangent(cgVec3 p0,cgVec3 p1,cgVec3 p2,cgVec2 t0,cgVec2 t1,cgVec2 t2);

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
 * @brief Computes the signed distance from the point to the plane defined by
 * planeNormal and offset
 * 
 * @param planeNormal plane normal vector
 * @param planeOffset plane offset
 * @param pt point
 * @return float signed distance
 */
float signedDist(cgVec3& planeNormal, float planeOffset, cgVec3& pt);

/**
 * @brief Axis aligned bounding box
 * 
 */
struct aabb
{              // axis aligned bounding box
    cgVec3 c1; // two corners
    cgVec3 c2; //this corner always has the larger x,y,z componenets
    aabb(){}
    aabb(const aabb& bb2){
        c1=bb2.c1; 
        c2=bb2.c2; 
        for(int i  = 0; i < 8;i++){allCorners[i]=bb2.allCorners[i];} 
        }

    aabb(cgVec3 corner1,cgVec3 corner2){
        this->c1=corner1;
        this->c2=corner2;
    }
    // RenderableObj *GenRenderObj(); // generates a renderable object of the aabb
    void print();
    void calcAllCorners();

    float calcSurfaceArea();

    float calcVolume();

    /**
     * @brief Calculates a rotated version of this 
     * 
     * @param transform the matrix transform to be applied to aabb vertices
     * @param c1 target corner 1 - for your transformed aabb
     * @param c2 target corner 2 - for your transformed aabb
     */

    void calcTransformedAABB(cgMat4& transform, cgVec3& c1_t, cgVec3& c2_t);

    cgVec3 allCorners[8]; //used for fast rotations
};

/**
 * @brief Get the axis aligned Bounding Box of the set of points
 * 
 * @param points vector of points
 * @return aabb bounding box
 */
aabb getBoundingBoxOfPtCloud(std::vector<cgVec3>& points);

/**
 * @brief Merges two AABBs into a single AABB representing the bounding box of the two boxes
 * 
 * @param aabb1 
 * @param aabb2 
 * @return aabb 
 */
aabb mergeAABBs(aabb aabb1, aabb aabb2);

/**
 * @brief Merge a set of aabbs into a single aabb
 * 
 * @param aabbs 
 * @return aabb 
 */
aabb mergeAABBs(const std::vector<aabb>& aabbs);

/**
 * @brief Create 6 aabbs which represent the volume in between aabb_outer and aabb_inner
 * 
 * @param aabb_outer 
 * @param aabb_inner
 * @param target ptr to an array of 6 aabbs
 */

void getOnionSkinAABBs(aabb aabb_outer, aabb aabb_inner, aabb* target);

/**
 * @brief returns true if the two aabbs are intersecting
 */
bool areColliding(aabb &aabb1, aabb &aabb2);

/**
 * @brief Checks if line defined by origin and direction intersects with aabb
 * 
 * @param aabb1 aabb
 * @param l_orig line origin 
 * @param l_dir line direction
 * @return true 
 * @return false 
 */
bool checkLineIntersect(aabb &aabb1, cgVec3 l_orig, cgVec3 l_dir);

/**
 * @brief Integer with defined getKey()
 * 
 */
struct Point{
    int val;
    Point(int val){this->val = val;}
    Point(const Point& p){this->val = p.val;}
    Point(){this->val=0;}
    bool operator ==(const Point& p){
        return (val == p.val);
    }
    uint64_t getKey(){return uint64_t(hash(val+69));}
};
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
 * @brief Gets the diagonal length of the aabb
 * 
 * @param pts 
 * @return float 
 */
float AABBDiagLen(std::vector<Facet>& facets, std::vector<cgVec3>& pts);

/**
 * @brief plane described by normal & offset
 * 
 */
struct Plane{
    Plane(cgVec3 normal, float offset){
        this->normal = normal;
        this->offset = offset;
    }

    /**
     * @brief Create plane defined by 3 points, the order of the points sets the normal direction,
     * according to the definition of
     * cgVec3 faceNormal(cgVec3& p1,cgVec3& p2,cgVec3& p3);
     */
    Plane(cgVec3& pt1, cgVec3& pt2, cgVec3& pt3){
        this->normal = faceNormal(pt1,pt2,pt3);
        this->offset = -dot(pt1,normal);
    }

    Plane(const Plane& p){
        this->normal=p.normal;
        this->offset=p.offset;
    }
    
    Plane(){}
    cgVec3 normal = cgVec3(0,0,0);
    float offset=0;
};

/**
 * @brief Frustum 
 * 
 */
struct Frustum{
    Frustum(const Frustum& fr){       
        this->t = fr.t;
        this->b = fr.b;
        this->l = fr.l;
        this->r = fr.r;
        this->n = fr.n;
        this->f = fr.f;
    }
    Frustum(){}

    /**
     * @brief Returns true if the box is contained by the frustum
     * //checks if all points are outside of a single plane and returns false
     *  else returns true, can return true even if the box is outside the frustum if it is near a corner.
     * 
     * @param box 
     * @return true 
     * @return false 
     */
    bool contains(aabb& box);

    Plane t; //top
    Plane b; //bottom
    Plane l; //left
    Plane r; //right
    Plane n; //near
    Plane f; //far

};

/**
 * @brief Get teh frustum of an orthographic projection 
 * 
 * @param viewDir 
 * @param left 
 * @param right 
 * @param bottom 
 * @param top 
 * @param zNear 
 * @param zFar 
 * @return Frustum 
 */
Frustum getOrthoFrustum(cgVec3 viewDir, cgVec3 upDir, float left, float right, float bottom, float top, float zNear, float zFar);

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

//QUICK HULL - CONVEX HULL CALCULATION
/**
 * @brief Computes the convex hull of points, puts convex hull into target triangle mesh
 * 
 * @returns false if failed
 * @param points points to compute convex hull of
 * @param target target (empty vector of facets)
 * 
 */
bool quickHull(std::vector<cgVec3>& points, std::vector<Facet>& target);

/**
 * @brief Facet struct used in quickhull algo
 * contains this facet along with neigboring facets
 * 
 */
struct qhFacet{
    qhFacet(){}
    void calcNormalOffset(std::vector<cgVec3>& points){fNormal = faceNormal(points[facet.inds[0]],points[facet.inds[1]],points[facet.inds[2]]); offset = dot(points[facet.inds[0]],fNormal)*(-1);};
    
    Facet facet;
    Facet neighbors[3];
    //Edge neighborEdges[3]; //the edges connecting to each respective neighbor
    cgVec3 fNormal;
    float offset; //signed offset = -(p_0)*normal
    std::vector<int> outsideSet; //set of outside points
};

/**
 * @brief Get the Horizon Edges of the set of facets
 * The horizon edges are defined as all edges that are touching only a single
 * facet
 * @param facets facets to find the horizon of
 * @param target location to store edges
 * @param neighbors facet neigbor adjacent to the facet containing the respective edge (set by this function)
 */
void getHorizonEdges(std::vector<qhFacet*>& facets, std::vector<Edge>& target, std::vector<Facet>& neighbors);

/**
 * @brief Gets the furthest point in dir direction
 * 
 * @param points set of points
 * @param dir direction
 * @param targetInd target index in points vector
 * @return cgVec3 furthest point along dir
 */
cgVec3 support(std::vector<cgVec3>& points, cgVec3 dir, int& targetInd);

/**
 * @brief Adds facets to target from the 3 simplex formed by the s1-4. The faces are defined such 
 * that their normals point outwards from the simplex volume. The simplex points must be ordered
 * so that s4 is along the direction of faceNormal(s1,s2,s3) for this to work correctly.
 * This only adds the 3 faces excluding the s0-s1-s2 face.
 * 
 * @param s0 
 * @param s1 
 * @param s2 
 * @param s3 
 * @param target 
 */
void addSimplexFaces(int s0, int s1, int s2, int s3, std::vector<Facet>& target);

//ACD APPROXIMATE CONVEX DECOMPOSITION

/**
 * @brief Decompose a mesh into a set of approximately convex meshes
 * 
 * @param facets facets in mesh
 * @param pts points comprising mesh
 * @param target empty vector of vectors of facets
 * @param alpha parameter to tune the contribution of the aspect ratio to the objective function
 * @param numBodies how many bodies you want to decompose the mesh into
 */
void ACD(std::vector<Facet>& facets, std::vector<cgVec3>& pts, std::vector<std::vector<Facet>>& target, float alpha, int numBodies);

/**
 * @brief Decompose a mesh into a set of approximately convex meshes
 *        This version keeps merging facets until the loss function begins to increase, 
 *        This results in the lowest number of sub meshes which minimizes the loss
 *        Smaller Alpha values tend to result in a decomposition with more submeshes and 
 *        lower concavity
 *        This algorithm is based on:
 *        K. Mamou and F. Ghorbel, "A simple and efficient approach for 3D mesh approximate convex decomposition"
 * 
 * @param facets facets in mesh
 * @param pts points comprising mesh
 * @param target empty vector of vectors of facets
 * @param alpha parameter to tune the contribution of the aspect ratio to the objective function
 * @param numBodies how many bodies you want to decompose the mesh into
 */
void ACD(std::vector<Facet>& facets, std::vector<cgVec3>& pts, std::vector<std::vector<Facet>>& target, float alpha);

/**
 * @brief Node of the dual graph, each node contains a facet and a set of neighboring facets that share an edge
 * to this node. 
 */
struct dgNode{
    dgNode(Facet facet){this->f = facet;a.push_back(f);}
    ~dgNode(){}
    //original facet that this node represents
    Facet f;

    //total surface area of the surface formed by this collection of triangles
    float area;
    //perimeter of the surface formed by the edges surrounding this area
    float perim;

    //ancestors - facets which make up this node
    std::vector<Facet> a;
    //Neighboring nodes
    std::vector<Facet> n;

    uint64_t getKey(){return f.getKey();}
    float loss;
};

/**
 * @brief Describes an edge on the dual graph
 * 
 */
struct dgEdge{
    Facet facets[2];
    dgEdge(){};
    dgEdge(Facet f1, Facet f2){facets[0]=f1;facets[1]=f2;};
    dgEdge(const dgEdge& e1){facets[0]=e1.facets[0];facets[1]=e1.facets[1];}
    uint64_t getKey(){return facets[0].getKey() + facets[1].getKey();}
    /**
     * @brief Compares two edges, order of facets doesnt matter
     * 
     * @param e1 
     * @return true 
     * @return false 
     */
    bool operator == (const dgEdge& e1){
        return (facets[0] == e1.facets[0] && facets[1] == e1.facets[1]) || (facets[0] == e1.facets[1] && facets[1] == e1.facets[0]);
    };
};

/**
 * @brief describes the connection an edge makes between 2 facets
 * 
 */
struct edgeConn{
    edgeConn(){}
    Edge edge;
    std::vector<Facet> facets;
    uint64_t getKey(){return edge.getKey();}
};

/**
 * @brief Generates a table of edgeConn struct pointers (make sure to delete these when finished)
 * from a list of facets
 * 
 * @param facets 
 * @return HashTable* hashtable of edgeConn structs - indexed by edge.getKey()
 */
HashTable* getGraphConnections(std::vector<Facet>& facets);

/**
 * @brief Removes facets to make every edge only connect 2 facets
 * 
 * @param facets 
 * @param target 
 */
void removeTripleEdges(std::vector<Facet>& facets, std::vector<Facet>& target);

struct dualGraph {
    public:
    /**
     * @brief table of nodes that this graph contains
     * 
     */
    HashTable* nodes;

    /**
     * @brief list of current edges on dual graph, updated when hecol is applied
     * 
     */
    HashTable_s<dgEdge>* edges;

    /**
     * @brief Construct a new dual Graph object from a list of facets
     * 
     * @param infacets 
     */
    dualGraph(std::vector<Facet>& facets, std::vector<cgVec3>& points);

    ~dualGraph();

    /**
     * @brief Merges facet w into v, v and w must be neighbors!
     * 
     * @param v 
     * @param w 
     */
    void hecol(Facet v, Facet w, std::vector<cgVec3>& points);

    /**
     * @brief Gets the loss function that would be produced from the half edge collapse of v and w
     *        used for ACD
     * 
     * @param v 
     * @param w 
     * @param points 
     * @return float 
     */
    float getLossHecol(Facet v, Facet w, std::vector<cgVec3>& points, float alpha);

};

/**
 * @brief Determines if ray intersects triangle.
 * 
 * @param rayOrigin 
 * @param rayVector 
 * @param tri0 triangle vertices
 * @param tri1 
 * @param tri2 
 * @param outIntersectionPoint intersection point (set to 0,0,0 if there is no intersection)
 * @return true - ray intersects
 * @return false - ray doesnt intersect
 */
bool rayIntersectsTriangle(cgVec3& rayOrigin, 
                           cgVec3& rayVector, 
                           cgVec3& tri0,
                           cgVec3& tri1,
                           cgVec3& tri2,
                           cgVec3& outIntersectionPoint);

/**
 * @brief Gets point where ray intersects the surface - assumes that ray starts on the interior of the surface
 * 
 * @param S surface with outward facing normals
 * @param points surface points
 * @param rayOrigin 
 * @param rayVector 
 * @param success  set to false if the projection failed
 * @return cgVec3 
 */
cgVec3 surfaceProjectionInt(std::vector<Facet>& S, std::vector<cgVec3>& points,cgVec3& rayOrigin,cgVec3& rayVector, bool& success);

/**
 * @brief Saves acd output to a .mmc (multimeshcollider) file
 *        File layout (in binary):
 *        
 *        uint64_t numberOfVectors = n,
 *        float,float,float vec_1,
 *        ...               vec_n,
 * 
 *        uint64_t numberOfVectors = m
 *        float,float,float vec_1,
 *        ...               vec_m,
 *        ...
 * 
 * @param acdOut 
 * @param pts 
 */
void saveMMCFile(std::vector<std::vector<Facet>>& acdOut, std::vector<cgVec3>& meshPts, std::string filename);

void loadMMCFile(std::string filename, std::vector<std::vector<cgVec3>>& target);

/**
 * @brief Converts ACD output (vector of facet sets) to vector of sets of cgVec3s (one for each body)
 * 
 * @param acdOut 
 * @param meshPts set of points from the original mesh.
 */
void ACDOutToPoints(std::vector<std::vector<geo::Facet>>& acdOut, std::vector<cgVec3>& meshPts, std::vector<std::vector<cgVec3>>& target);

/**
 * @brief Takes a set of points and a set of facet sets indexed to the set of points and decomposes it into a 
 *        vector of point sets and a vector of facet sets indexed to the new point set.
 *        Basically used to take the ACD result and convert it into a form that can be used to create a multimesh body.
 * 
 * @param origPtCloud original set of all points
 * @param facetSets submesh facets indexed to origPtCloud
 * @param newPts pts for each submesh
 * @param newFacets facets for each submesh indexed to new points
 */
void subMeshDecompose(std::vector<cgVec3>& origPtCloud,
                      std::vector<std::vector<Facet>>& facetSets,
                      std::vector<std::vector<cgVec3>>& newPts,
                      std::vector<std::vector<Facet>>& newFacets);

/**
 * @brief Finds every voxel that a line intersects with.
 * 
 * @param orig start point of the line
 * @param dir line direction
 * @param maxDist maxmimum distance to traverse
 * @param vxlSz edge length of the voxels
 * @param target vector of voxel indices
 */
void voxelizeLine(cgVec3 orig, cgVec3 dir, float maxDist, float vxlSz, std::vector<std::array<int, 3>>& target);

/**
 * @brief Finds the point that the line defined by l_orig and l_dir intersects the plane defined by a normal and point
 * 
 * @param l_orig line origin
 * @param l_dir line direction
 * @param p_n plane normal
 * @param p_pt plane point
 * @param isParallel set to true if line is parallel to the plane
 * @return cgVec3 
 */
cgVec3 linePlaneIntersectionPt(cgVec3 l_orig, cgVec3 l_dir, cgVec3 p_n, cgVec3 p_pt, bool& isParallel);

/**
 * @brief Returns true if the point lies inside the mesh, works by checking signed distance against every face
 * 
 * @param pts mesh points
 * @param facets mesh facets
 * @param point point to check
 * @return true 
 * @return false 
 */
bool meshContainsPt(std::vector<cgVec3>& pts, std::vector<Facet>& facets, cgVec3 point);

/**
 * @brief Creates a 3d grid of voxels from the list of meshes and facets, creates grid over the AABB of the meshes with the size of 
 * each grid element given by voxelSize. The number of voxels is quantized so that an integer number of voxels comprise each grid dimension.
 * The resulting grid contains an integer at each grid position describing which mesh contains that grid point. Grid elements with a 0 value
 * indicates no mesh at that location.
 * 
 * @param pts 
 * @param facets 
 * @param voxelSize 
 * @param origin origin calculated from bounding box (bottom left corner)
 * @return Array3d<uint16_t>* 
 */
Array3d<uint16_t>* voxelizeMeshes(std::vector<std::vector<cgVec3>>& pts, std::vector<std::vector<Facet>>& facets, float voxelSize, cgVec3& origin);

/**
 * @brief Creates a mesh from a heightmap from (startX,startY) to (startX+strideX,startY+strideY)
 * 
 * @param target_pts target mesh points
 * @param target_facets target mesh indicies
 * @param in_heightmap input heightmap
 * @param xEdgeLength edge length of a single heightmap element in x direction
 * @param yEdgeLength edge length of a single heightmap element in y direction
 * @param startX start position in array
 * @param startY
 * @param strideX stride through array
 * @param strideY
 * 
 */
void tesselateHeightMap(std::vector<cgVec3>& target_pts, 
                        std::vector<Facet>& target_facets,
                        Array2d<float>& in_heightmap,
                        float xEdgeLength,
                        float yEdgeLength,
                        int startX,
                        int startY,
                        int strideX,
                        int strideY
                        );

/**
 * @brief Returns true if the triangle contains the point, if the pt is on the edge, it will be nudged slightly into the triangle
 * 
 * @param tri1 
 * @param tri2 
 * @param tri3 
 * @param pt 
 * @return true 
 * @return false 
 */
bool triangleContainsPt(cgVec2 tri1, cgVec2 tri2, cgVec2 tri3, cgVec2& pt);

/**
 * @brief Generate a triangulation of the pts array that consists of delaunay triangles. Vertices defining the
 * four corners of the rectangle defined by xExtents and yExtents will be added to the pts vector
 * 
 * @param pts input points
 * @param targetFacets output facets
 * @param xExtents minX,maxX
 * @param yExtents minY,maxY
 */
void DelaunayTriangulateBroken(std::vector<cgVec2>& pts, 
                         std::vector<Facet>& targetFacets,
                         std::tuple<float, float> xExtents,
                         std::tuple<float, float> yExtents);

/**
 * @brief for each point in the facet list, collect all of the facets that share that point
 * 
 * @param facetList list of facets 
 * @param target list of tuples (point, list of facets)
 */
void collectFacetsSharingPoint(std::vector<Facet>& facetList, std::vector<std::tuple<int,std::vector<Facet>>>& target);

/**
 * @brief Smoothes the mesh using the laplace smoothing method - just averages adjacent points
 * 
 * @param points 
 * @param facets 
 * @param numIters number of times to smooth
 */
void laplaceSmooth(std::vector<cgVec3>& points, std::vector<Facet>& facets, int numIters);

/**
 * @brief Smooths mesh faster by iterating over every facet and incrementally moving vertices closer together.
 * This is the fastest implementation - but it does not always produce the exact same result since it smooths verts in place
 * 
 * @param points 
 * @param facets 
 * @param numIters 
 */
void fastLaplaceSmooth_inplace(std::vector<cgVec3>& points, std::vector<Facet>& facets, int numIters);

/**
 * @brief Smooths mesh faster by iterating over every facet and incrementally moving vertices closer together.
 * This should always produce the same smoothed mesh - doesnt depend on the ordering of the verts
 * 
 * @param points 
 * @param facets 
 * @param numIters 
 */
void fastLaplaceSmooth(std::vector<cgVec3>& points, std::vector<Facet>& facets, int numIters);

}
#endif