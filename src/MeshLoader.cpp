#include "MeshLoader.hpp"
#include "Sets.hpp"

void loadOBJFile(std::string filename, std::vector<objItem*>& target, std::string object_id){
    std::string pathSeparator = "/";
    
#if defined(WIN32) || defined(WIN64)
    pathSeparator = "/";
#endif
    
    int endLoc = filename.rfind(pathSeparator); //find loc of last separator
    std::string rootDirPath = filename.substr(0,endLoc); //directory of obj file

    FILE * file = fopen(filename.c_str(), "r");
    if( file == NULL ){
        std::cout << "Failed to open OBJ file: " << filename << "\n";
        exit(-1);
    }
    
    std::vector<cgVec3> tempVertPositions;
    std::vector<cgVec3> tempVertNormals;
    std::vector<cgVec2> tempTextPos;
    
    std::vector<int> vertexPosIndices;
    std::vector<int> textPosIndices;
    std::vector<int> normalIndices;
    //first load all of the vertex data into vectors
    std::string mtllib;

    bool isFirstObject = true;
    bool onFirstObject = false;

    while(1){
        
        char lineHeader[128];
        // read the first word of the line
        int res = fscanf(file, "%s", lineHeader);           
            //"o" is object name, 
        char objName[128]; //current object name
        if ( strcmp( lineHeader, "o" ) == 0 || strcmp( lineHeader, "g" ) == 0 || res == EOF ){ //new object, constructs previous object
            if(res!=EOF){
                fscanf(file, "%s",objName);
            }

            if(isFirstObject){ //if its the first object in the file, we dont want to assemble the previous mesh since there isnt one...
                target.push_back(new objItem());
                // tempObjs.push_back(this); //set first object to this
                target[target.size()-1]->name = std::string(objName);
                target[target.size()-1]->filename = filename;
                isFirstObject = false;
                onFirstObject = true;
                continue;
            }
            
            //assemble last mesh
            if(target.size()>0){
                if(onFirstObject){
                    onFirstObject = false;
                } else {
                    target[target.size()-1]->name = std::string(objName);
                    target[target.size()-1]->filename = filename;
                }
            
                //assemble mesh for this object here
                std::vector<vertIndices> vIndices; //using Facets since they are 3 integers with a getKey() method
                                                // then the set union can be used to get rid of duplicates
                for(int i = 0; i < vertexPosIndices.size(); i++){
                    vIndices.push_back(vertIndices(vertexPosIndices[i]-1,textPosIndices[i]-1,normalIndices[i]-1));
                }
                std::vector<vertIndices> nullSet;
                std::vector<vertIndices> setNoDupes; 
                //taking the union with an empty set efficiently removes duplicates
                setUnion<vertIndices>(vIndices,nullSet, setNoDupes);

                //this will map a set of indices to a single index so we can find the indices that 
                //vIndices map to in the obj target arrays.

                HashTable_s128<int> indexMap = HashTable_s128<int>(10);
                //get actual vertex data
                int ind = 0;
                for(vertIndices f : setNoDupes){
                    target[target.size()-1]->positions.push_back(tempVertPositions[f.inds[0]]);
                    target[target.size()-1]->textCoords.push_back(tempTextPos[f.inds[1]]);
                    target[target.size()-1]->normals.push_back(tempVertNormals[f.inds[2]]);
                    indexMap.add(ind,f.getKey128());
                    ind++;
                }

                //index to the compiled vertex data
                for(vertIndices f : vIndices){
                    target[target.size()-1]->indices.push_back(indexMap.get(f.getKey128()));
                }
                target[target.size()-1]->mtllib = mtllib;
                //delete indices so that they are fresh for the next object
                vertexPosIndices.clear();
                textPosIndices.clear();
                normalIndices.clear();
            }
            if(res==EOF){ //end of file, break before making an empty mesh
                break;
            }
            //generate next object...
            target.push_back(new objItem());
        }else if ( strcmp( lineHeader, "mtllib" ) == 0 ){ //mtl file specification, load mtl data
            char mtlfilename[128];
            fscanf(file, "%s", mtlfilename); //read filename
            std::string fullMTLFn = rootDirPath+pathSeparator+mtlfilename;
            mtllib = mtlfilename;
            //objname
            continue;
            
        }else if ( strcmp( lineHeader, "usemtl" ) == 0 ){ //adds material to object
            char mtlID[128];
            fscanf(file, "%s", mtlID);
            bool check=false;
            target[target.size()-1]->matName = mtlID;
            
        }else if ( strcmp( lineHeader, "g" ) == 0 ){ //group name (not sure what this even does)
            char group[128];
            fscanf(file, group);
            target[target.size()-1]->name = std::string(objName);
            
        }else if ( strcmp( lineHeader, "v" ) == 0 ){
            cgVec3 vertPos;
            fscanf(file, "%f %f %f\n", &vertPos.x, &vertPos.y, &vertPos.z );
            tempVertPositions.push_back(vertPos);
            
        }else if ( strcmp( lineHeader, "vt" ) == 0 ){
            cgVec2 textPos;
            fscanf(file, "%f %f\n", &textPos.x, &textPos.y );
            tempTextPos.push_back(textPos);
            
        }else if ( strcmp( lineHeader, "vn" ) == 0 ){
            cgVec3 normal;
            fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z );
            tempVertNormals.push_back(normal);
            
        }else if ( strcmp( lineHeader, "f" ) == 0 ){
            unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
            int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2] );
            if (matches != 9){
                std::cout << lineHeader<<"\n";
                char line[1024];
                std::cout << fgets(line,1024,file);
                printf("OBJ File can't be read, Make sure that file is Triangulated \n");
                exit(-1);
            }
            vertexPosIndices.push_back(vertexIndex[0]);
            vertexPosIndices.push_back(vertexIndex[1]);
            vertexPosIndices.push_back(vertexIndex[2]);
            textPosIndices  .push_back(uvIndex[0]);
            textPosIndices  .push_back(uvIndex[1]);
            textPosIndices  .push_back(uvIndex[2]);
            normalIndices.push_back(normalIndex[0]);
            normalIndices.push_back(normalIndex[1]);
            normalIndices.push_back(normalIndex[2]);
        }
    }
    fclose(file);
}