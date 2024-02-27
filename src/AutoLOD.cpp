#include "AutoLOD.hpp"
#include <limits>
#include <set>

float AutoLOD::AutoLODGraphNode::getLoss(std::vector<cgVec3>& points){
    float loss = 0.0;
    float area = 0.0;
    std::vector<cgVec3> normals = std::vector<cgVec3>(facets.size());
    int i = 0;
    for(geo::Facet f : facets){
        normals[i] = geo::faceNormal(points[f.inds[0]],points[f.inds[1]],points[f.inds[2]]);
        area += geo::triArea(points[f.inds[0]],points[f.inds[1]],points[f.inds[2]]);
        i++;
    }
    cgVec3 meanNormal = {0,0,0};
    for(cgVec3 n : normals){
        meanNormal = meanNormal + n;
    }
    meanNormal = meanNormal/float(i);
    cgVec3 stdevNormal = {0,0,0};

    for(cgVec3 n : normals){
        cgVec3 diff = meanNormal - n;
        stdevNormal = stdevNormal + diff*diff;
    }
    stdevNormal = stdevNormal/float(i);
    return area*stdevNormal.norm();
}

float AutoLOD::AutoLODGraphNode::getEcolLoss(std::vector<cgVec3>& points, int neighborNode){
    float loss = 0.0;
    float area = 0.0;

    geo::Edge coll_edge = geo::Edge(this->vertInd,neighborNode); //collapsing edge
    //get facets to be removed - there should always be 2:
    geo::Facet coll_facets [2]; //collapsing faces
    int temp = 0;
    for(geo::Facet f : this->facets){
        if(f.contains(coll_edge)){
            coll_facets[temp] = f;
            temp++;
        }
    }
    // assert(temp == 2);
    if(temp != 2){
        return -1.0;
    }
    cgVec3 n0 = geo::faceNormal(points[coll_facets[0].inds[0]],points[coll_facets[0].inds[1]],points[coll_facets[0].inds[2]]);
    cgVec3 n1 = geo::faceNormal(points[coll_facets[1].inds[0]],points[coll_facets[1].inds[1]],points[coll_facets[1].inds[2]]);
    float a0 = geo::triArea(points[coll_facets[0].inds[0]],points[coll_facets[0].inds[1]],points[coll_facets[0].inds[2]]);
    float a1 = geo::triArea(points[coll_facets[1].inds[0]],points[coll_facets[1].inds[1]],points[coll_facets[1].inds[2]]);

    return cross(n0,n1).norm()*(a0+a1);
}

AutoLOD::AutoLODGraph::AutoLODGraph(std::vector<geo::Facet>& facets, std::vector<cgVec3>& points){
    int nPts = int(points.size());
    double logPts = log2(float(nPts));
    int nBins = int(logPts);
    if(nBins < 1){
        nBins = 1;
    }

    this->nodes = new HashTable128(nBins);

    ptsCopy = points;

    for (geo::Facet f : facets){
        
        for (int i = 0 ; i < 3; i++){
            
            AutoLODGraphNode* thisNode = (AutoLODGraphNode*)this->nodes->get(uuid128(f.inds[i]));
            
            if(thisNode == NULL){ //make a new node
                thisNode = new AutoLODGraphNode(f.inds[i]);
                this->nodes->add((void*)thisNode,uuid128(f.inds[i]));
            }

            thisNode->facets.insert(f);

            for(int j = 0; j < 3; j++){ //add adjacent nodes and edges related to this facet
                if(i != j){
                    thisNode->adjacentNodes.insert(f.inds[j]);
                }
            }

        }
    }

    std::vector<geo::Edge> horizonEdgeVec;
    geo::getHorizonEdges(facets,horizonEdgeVec);

    // std::cout << "horizon edges: \n";
    for(geo::Edge e : horizonEdgeVec){
        // e.print();
        this->horizonEdges.insert(e);
        horizonVerts.insert(e.inds[0]);
        horizonVerts.insert(e.inds[1]);
    }

    // exit(-1);
}

void AutoLOD::AutoLODGraph::debugCheckGraphLegality(){
    this->nodes->iterBegin();
    AutoLODGraphNode* node;
    while(1){
        node = (AutoLODGraphNode*)this->nodes->iterGetNext();
        if(!node){
            break;
        }

        //check mutual adjacency 
        for(int adjIndex : node->adjacentNodes){
            AutoLODGraphNode* adjNode = (AutoLODGraphNode*)nodes->get(uuid128(adjIndex));
            if(!adjNode){
                std::cout << "Missing adjacent node!";
                std::cout << "Central Node: "<<node->vertInd<<"\n";
                std::cout << "Adjacent Node: "<<adjNode->vertInd<<"\n";
                assert(0);
            }
            bool hasThisNode = false;
            for(int n : adjNode->adjacentNodes){
                if(n==node->vertInd)
                    hasThisNode = true;
            }
            if(!hasThisNode){
                std::cout << "Mutual Adjacency Violation: \n";
                std::cout << "Central Node: "<<node->vertInd<<"\n";
                std::cout << "Adjacent Node: "<<adjNode->vertInd<<"\n";
                std::cout << "Central Node Neighbors:\n";
                for(int n : node->adjacentNodes){
                    std::cout << "["<<n <<"]"<< "\n";
                }
                std::cout << "Adjacent Node Neighbors:\n";
                for(int n : adjNode->adjacentNodes){
                    std::cout << "["<<n <<"]"<< "\n";
                }
                assert(0);
            }
        }

        //check that facet inds exist in adjacent node lists
        for(geo::Facet f : node->facets){

            int err = -1;
            if(f.inds[0] != node->vertInd){
                if(!node->adjacentNodes.count(f.inds[0]))
                    err = 0;
            }
            if(f.inds[1] != node->vertInd){
                if(!node->adjacentNodes.count(f.inds[1]))
                    err = 1;
            }
            if(f.inds[2] != node->vertInd){
                if(!node->adjacentNodes.count(f.inds[2]))
                    err = 2;
            }

            if(err > 0){
                std::cout << "Facet index missing from adjacency list:\n";
                std::cout << "Facets: \n";
                for(geo::Facet fl : node->facets){
                    fl.print();
                }
                std::cout << "Offending facet:\n";
                f.print();
                std::cout << "Central Node Neighbors:\n";
                for(int n : node->adjacentNodes){
                    std::cout << "["<<n <<"]"<< "\n";
                }
                assert(0);
            }
        }
    }
    std::cout << "Graph in legal state\n";
}

bool AutoLOD::AutoLODGraph::ecolIsLegal(int v_keep, int v_remove){
    if(horizonVerts.count(v_remove)){
        return false;
    }
    AutoLODGraphNode* keepNode = (AutoLODGraphNode*)this->nodes->get(uuid128(v_keep));
    AutoLODGraphNode* removeNode = (AutoLODGraphNode*)this->nodes->get(uuid128(v_remove));
    geo::Edge coll_edge = geo::Edge(v_keep,v_remove); //collapsing edge

    //get facets to be removed - there should always be 2:
    geo::Facet coll_facets [2]; //collapsing faces
    int temp = 0;
    for(geo::Facet f : keepNode->facets){
        if(f.contains(coll_edge)){
            if(temp > 1){

                std::cout << "Problems detected...\n";
                std::cout << "COLL EDGE: ";
                coll_edge.print();

                for(geo::Facet f : keepNode->facets){
                    f.print();
                }
                exit(-1);
            }
            coll_facets[temp] = f;
            temp++;
        }
    }

    //check that there are exactly 2 shared neighbors
    int nShared = 0;
    for(int neighbor : keepNode->adjacentNodes){
        nShared+=int(removeNode->adjacentNodes.count(neighbor));
    }
    if(nShared == 2){

    } else {
        return false;
    }

    //check that triangle normals arent going to flip when vertex is replaced

    //check affected facets:
    for(geo::Facet f : removeNode->facets){

        if(f == coll_facets[0] || f == coll_facets[1]){
            continue;
        }

        geo::Facet newFacet = geo::Facet(f);
        newFacet.replace(v_remove,v_keep);

        cgVec3 origNorm = geo::faceNormal(ptsCopy[f.inds[0]],ptsCopy[f.inds[1]],ptsCopy[f.inds[2]]);
        cgVec3 newNorm = geo::faceNormal(ptsCopy[newFacet.inds[0]],ptsCopy[newFacet.inds[1]],ptsCopy[newFacet.inds[2]]);
        if(dot(newNorm,origNorm) < 0.0){

            return false;
        }
    }
    return true;
}

void AutoLOD::AutoLODGraph::ecol(int v_keep, int v_remove){
    // std::cout << "keep: "<<v_keep<<" v_remove: "<<v_remove<<"\n";
    AutoLODGraphNode* keepNode = (AutoLODGraphNode*)this->nodes->get(uuid128(v_keep));
    AutoLODGraphNode* removeNode = (AutoLODGraphNode*)this->nodes->get(uuid128(v_remove));

    assert(keepNode);
    assert(removeNode);
    assert(keepNode->adjacentNodes.count(v_remove));

    //collapsing edge
    geo::Edge coll_edge = geo::Edge(v_keep,v_remove);
    assert(!horizonEdges.count(coll_edge) );

    //get facets to be removed - there should always be 2:
    geo::Facet coll_facets [2];
    int temp = 0;
    for(geo::Facet f : keepNode->facets){
        if(f.contains(coll_edge)){
            coll_facets[temp] = f;
            temp++;
        }
    }
    // std::cout << "COLL FACETS:\n";
    // coll_facets[0].print();
    // coll_facets[1].print();

    assert(temp == 2);
    
    //collect affected nodes
    std::unordered_set<int> affectedNodes;
    for(int adjNode : keepNode->adjacentNodes){
        affectedNodes.insert(adjNode);
    }
    for(int adjNode : removeNode->adjacentNodes){
        affectedNodes.insert(adjNode);
    }

    affectedNodes.erase(v_remove); //dont modify this one because it will be deleted soon

    for(geo::Facet f : removeNode->facets){ //add remove node facets to keep node
        // std::cout <<"adding to keepNode: \n";
        // f.print();
        keepNode->facets.insert(f);
    };

    assert(affectedNodes.count(v_keep));

    //remove coll_facets from facet list
    //for facets containing v_remove, replace v_remove with v_keep
    //and remove v_remove from adjacentNodes
    //and add v_keep to adjacentNodes
    for(int nnode : affectedNodes){
        AutoLODGraphNode* node = (AutoLODGraphNode*)this->nodes->get(uuid128(nnode));
        node->wasAffected = true;

        assert(node);

        //erase removed facets, if they exist
        node->facets.erase(coll_facets[0]);
        node->facets.erase(coll_facets[1]);
       
        //get facets containing v_remove, to replace v_remove with v_keep
        std::vector<geo::Facet> facetsToUpdate;
        for(geo::Facet f : node->facets){
            if(f.contains(v_remove)){
                facetsToUpdate.push_back(f);
            }
        }

        for(geo::Facet f : facetsToUpdate){
            geo::Facet newFacet = geo::Facet(f.inds[0],f.inds[1],f.inds[2]);
            bool check = newFacet.replace(v_remove,v_keep);
            assert(check);
            node->facets.erase(f);
            node->facets.insert(newFacet);
        }
        
        //remove v_remove from adjacent nodes
        node->adjacentNodes.erase(v_remove);
        //add v_keep to adjacent nodes
        if(node->vertInd != v_keep){
            node->adjacentNodes.insert(v_keep);
        }

    }

    //add new neighbors to v_keep
    for(int newNeighbor : removeNode->adjacentNodes){
        if(newNeighbor != keepNode->vertInd)
            keepNode->adjacentNodes.insert(newNeighbor);
    }

    this->nodes->remove(uuid128(v_remove));
    delete removeNode;
}

AutoLOD::AutoLODGraph::~AutoLODGraph(){
    AutoLODGraphNode* node;
    this->nodes->iterBegin();

    while(1){
        node = (AutoLODGraphNode*)this->nodes->iterGetNext();
        if(node){
            delete node;
        } else {
            break;
        }
    }
    delete nodes;
}

void AutoLOD::genLODMesh(std::vector<geo::Facet>& meshFacets, 
                 std::vector<cgVec3>& meshPoints,
                 std::vector<geo::Facet>& targetFacets,
                 float compressionFactor, int& actualSize )
{
    std::cout << "num points: "<<meshPoints.size();
    AutoLODGraph graph = AutoLODGraph(meshFacets, meshPoints);
    std::cout << "graph size: "<<graph.nodes->calcSize();
    graph.debugCheckGraphLegality();
    
    int size = graph.nodes->calcSize();
    int baseSize = size;
    int targetSize = int(float(baseSize)/float(compressionFactor));

    while(size > targetSize){

        //find lowest loss ecol:
        graph.nodes->iterBegin();

        //min loss over each possible ecol operation  {loss, v_k, v_rem} - sorted by loss
        std::set<std::tuple<float, int, int>> lossHierarchy;

        while(1){ //sort ecol ops
            AutoLODGraphNode* node = (AutoLODGraphNode*)graph.nodes->iterGetNext();
            if(!node)
                break;
            node->wasAffected = false;

            for(int adjNode : node->adjacentNodes){
                if(graph.horizonEdges.count(geo::Edge(node->vertInd,adjNode))){
                    continue;
                }
                if(graph.ecolIsLegal(node->vertInd,adjNode)){
                    float loss = node->getEcolLoss(graph.ptsCopy,adjNode);
                    if(loss < 0){
                        continue;
                    }
                    lossHierarchy.insert(std::make_tuple(loss,node->vertInd,adjNode));
                    
                }
            }
        }

        if(lossHierarchy.size() == 0 ){
            std::cout << "No legal ecol operations, exiting\n";
            break;
        }

        // std::cout << "merge nodes: " << nodeToMergeInto << " " << minNode->vertInd<<"\n";
        // assert(! (graph.horizonEdges.count(geo::Edge(nodeToMergeInto,minNode->vertInd))));
        int numEcols = 0;
        int maxEcols = lossHierarchy.size();
        for(auto ecolOp : lossHierarchy){
            if(numEcols > maxEcols/2){
                break;
            }
            AutoLODGraphNode* keepNode  = (AutoLODGraphNode*)graph.nodes->get(uuid128(std::get<1>(ecolOp)));
            AutoLODGraphNode* RemNode  = (AutoLODGraphNode*)graph.nodes->get(uuid128(std::get<2>(ecolOp)));
            if(keepNode==NULL || RemNode == NULL){
                continue;
            }

            if(keepNode->wasAffected || RemNode->wasAffected){
                continue;
            }
            // std::cout << "loss: "<<std::get<0>(ecolOp)<<"\n";
            graph.ecol(std::get<1>(ecolOp),std::get<2>(ecolOp));
            numEcols++;

            size--;
        }
        // graph.debugCheckGraphLegality();
        std::cout << "size: "<<size<<"\n";
    }

    //finished
    //collect resulting facets
    std::unordered_set<geo::Facet,geo::Facet::HashFunctionUnordered> resultingFacets;
    graph.nodes->iterBegin();
    while(1){ //
        AutoLODGraphNode* node = (AutoLODGraphNode*)graph.nodes->iterGetNext();
        if(!node)
            break;
        
        for(geo::Facet f : node->facets){
            resultingFacets.insert(f);
        }
    }

    for(geo::Facet f : resultingFacets){
        targetFacets.push_back(f);
    }

    actualSize = size;
}