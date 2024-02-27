#include "MeshViewerApp.hpp"
#include <iostream>

int main(int argc, char *argv[])
{
    if(argc < 2){
        std::cout << "Args: .obj filename\n"<<
                      "OBJ File must be triangulated!\n";
                    exit(-1);
    }
    std::string filename = std::string(argv[1]);
    MeshViewerApp app = MeshViewerApp(1000,1000,"Mesh Viewer");
    app.loadMesh(filename);
    app.run();

}
