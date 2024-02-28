#ifndef AUTOLOD_HPP
#define AUTOLOD_HPP

#include "Geometry.hpp"
#include <unordered_set>

namespace AutoLOD{

    struct AutoLODGraphNode{

        AutoLODGraphNode(int vertInd){
            this->vertInd = vertInd;
        }

        int vertInd; //index of the vertex that this node represents
        std::unordered_set<int> adjacentNodes; //adjacent nodes
        std::unordered_set<geo::Facet, geo::Facet::HashFunctionUnordered> facets; //facets touching this node
        bool wasAffected = false; //this gets set to true indicating when this is involved in an ecol indicating that its loss value is out of date
        
        /**
         * @brief metric describing how much topological information is contained in this vertex
         * basically increases when facets arent parallel, and increases for larger facets.
         * 
         * @return float 
         */
        float getLoss(std::vector<cgVec3>& points);

    };

    class AutoLODGraph{

        public:
        AutoLODGraph(std::vector<geo::Facet>& facets, std::vector<cgVec3>& points);

        /**
         * @brief Half edge collapse - remove edge between v_keep and v_remove by removing v_remove
         * 
         * @param v_keep 
         * @param v_remove 
         */
        void ecol(int v_keep, int v_remove);

        /**
         * @brief How much loss would be generated by edge collapsing neighborNode into this node
         * 
         * @param points 
         * @param neighborNode neighboring node - must exist in adjacentNodes
         * @return float 
         */
        float getEcolLoss(std::vector<cgVec3>& points, int thisnode, int neighborNode, float maxSinTheta);

        /**
         * @brief Checks to see if edge collapse is legal: 
         *        - doesnt produce a non-manifold mesh (duplicates faces)
         *        - doest flip faces
         * 
         * @param v_keep 
         * @param v_remove 
         * @return true 
         * @return false 
         */
        bool ecolIsLegal(int v_keep, int v_remove);

        ~AutoLODGraph();

        void print(){
            std::cout << "AutoLODGraph Info:"<<"\n";
            std::cout << "nNodes = "<<nodes->calcSize()<<"\n";
        }

        /**
         * @brief Checks every single graph node to make sure it is in a legal configuration
         * checks that adjacency is shared between nodes.
         * checks that every facet index exists as a node.
         * 
         */
        void debugCheckGraphLegality();

        HashTable128* nodes;
        std::vector<cgVec3> ptsCopy;
        std::unordered_set<geo::Edge, geo::Edge::HashFunction> horizonEdges;
        std::unordered_set<int> horizonVerts;
    };

    /**
     * @brief Generate a coarser mesh with 1/compressionFactor vertices from basemesh
     * 
     * @param meshFacets base mesh facets
     * @param meshPoints base mesh points
     * @param targetFacets resulting facets
     * @param compressionFactor 
     * @param maxSinTheta a smaller value makes the algorithm try to preserve sharp edges over 
     * keeping the triangle aspect ratio close to 1.
     * @param actualSize actual number of vetices in resulting mesh
     */
    void genLODMesh(std::vector<geo::Facet>& meshFacets, 
                 std::vector<cgVec3>& meshPoints,
                 std::vector<geo::Facet>& targetFacets,
                 float compressionFactor,float maxSinTheta, int& actualSize );
    
};

#endif