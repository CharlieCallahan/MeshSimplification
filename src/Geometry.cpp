#include "Geometry.hpp"
#include <functional>

//COMMON
uint64_t geo::hash(uint64_t value){
    int64_t x = value+123123;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = (x >> 16) ^ x;
    return x;
}

cgVec3 geo::faceNormal(cgVec3& p1,cgVec3& p2,cgVec3& p3){
    cgVec3 A12 = p2-p1;
    cgVec3 A13 = p3-p1;
    return cross(A12,A13).normalized();
}

geo::Edge geo::getSharedEdge(Facet& f1, Facet& f2){
    Edge e1[3];
    e1[0] = Edge(f1.inds[0],f1.inds[1]);
    e1[1] = Edge(f1.inds[1],f1.inds[2]);
    e1[2] = Edge(f1.inds[2],f1.inds[0]);

    Edge e2[3];
    e2[0] = Edge(f2.inds[0],f2.inds[1]);
    e2[1] = Edge(f2.inds[1],f2.inds[2]);
    e2[2] = Edge(f2.inds[2],f2.inds[0]);

    for(int i = 0; i < 3; i++){
        for(int j=0; j < 3; j++){
            if(e1[i]==e2[j]){
                return e1[i];
            }
        }
    }
    return Edge(-1,-1);
}

void geo::getHorizonEdges(std::vector<Facet>& facets, std::vector<Edge>& target){

    //table of edge and true/false to track if this edge is unique or not
    int nBins = std::max(2, int(log2(int(facets.size())/3 )));
    HashTable_s<std::tuple<Edge,bool>> edgeTable = HashTable_s<std::tuple<Edge,bool>>(nBins);

    //enumerate every edge
    for( Facet f: facets){
        bool wasFound = false; //tracks if this element was found in the table already

        Edge e = Edge(f.inds[0],f.inds[1]);
        // edgeTable.get(e.getKey(),wasFound);
        wasFound = edgeTable.isIn(e.getKey());
        if (!wasFound){
            edgeTable.add(std::make_tuple(e,true),e.getKey());
        } else {
            //already exists in table
            edgeTable.remove(e.getKey());
            edgeTable.add(std::make_tuple(e,false),e.getKey());
        }

        e = Edge(f.inds[1],f.inds[2]);
        wasFound = edgeTable.isIn(e.getKey());
        if (!wasFound){
            edgeTable.add(std::make_tuple(e,true),e.getKey());
        } else {
            //already exists in table
            edgeTable.remove(e.getKey());
            edgeTable.add(std::make_tuple(e,false),e.getKey());
        }

        e = Edge(f.inds[2],f.inds[0]);
        wasFound = edgeTable.isIn(e.getKey());
        if (!wasFound){
            edgeTable.add(std::make_tuple(e,true),e.getKey());
        } else {
            //already exists in table
            edgeTable.remove(e.getKey());
            edgeTable.add(std::make_tuple(e,false),e.getKey());
        }
    }

    edgeTable.iterBegin();
    bool isFinished = false;
    while(1){
        std::tuple<Edge,bool> t = edgeTable.iterGetNext(isFinished);
        if(isFinished){
            return;
        } else {
            if( std::get<1>(t) ){ //is unique
                Edge e = std::get<0>(t); 
                target.push_back(e);
            }
        }
    }
}

float geo::triArea(cgVec3 p1,cgVec3 p2,cgVec3 p3){
    return 0.5*(cross(p2-p1,p3-p1).norm());
};


bool geo::Facet::operator == (const geo::Facet& f1) const {
    return 
    (inds[0] == f1.inds[0] || inds[0] == f1.inds[1] || inds[0] == f1.inds[2]) && //contains ind1
    (inds[1] == f1.inds[0] || inds[1] == f1.inds[1] || inds[1] == f1.inds[2]) && //contains ind2
    (inds[2] == f1.inds[0] || inds[2] == f1.inds[1] || inds[2] == f1.inds[2]);   //contains ind3
}

bool geo::Facet::replace(int oldIndex, int newIndex){
    int i=-1;
    if(inds[0] == oldIndex)
        i=0;
    if(inds[1] == oldIndex)
        i=1;
    if(inds[2] == oldIndex)
        i=2;
    if(i < 0)
        return false;
    else
        inds[i] = newIndex;
        return true;
}

bool geo::Facet::contains(Edge& e){
    return (e==Edge(inds[0],inds[1])) || (e==Edge(inds[1],inds[2])) ||(e==Edge(inds[2],inds[0]));
}

bool geo::Facet::sharesEdge(Facet& f2){
    Edge e = getSharedEdge(*this, f2);
    if(e.inds[0] < 0){
        return false;
    } else {
        return true;
    }
}
