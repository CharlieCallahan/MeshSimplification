#ifndef MESHVIEWERAPP
#define MESHVIEWERAPP
#include "AppWindow.hpp"
#include "MeshLoader.hpp"

// vertex data passed to the shader
#pragma pack(1)
struct gVertex{
    cgVec3 position;
    cgVec3 normal;
};
#pragma pack(pop)

//glsl shader
struct gShader{
    public:
    gShader(char* vertcode, char* fragcode);

    void bind();

    GLuint shader;
    private:
    void checkCompileErrors(unsigned int shader, std::string type);
};

struct MeshGPUBuffer{

    MeshGPUBuffer(int nIndices, int* indices, int nVerts, gVertex* vertices);

    void bind();

    ~MeshGPUBuffer();

    cgVec3 worldSpacePosition = cgVec3(0,0,0);

    GLuint vertexBuffer;
    GLuint indexBuffer;
    GLuint vertexArray;
    int nIndices;
};

struct MeshViewerData{
    std::vector<objItem*> original_meshes;
};

class MeshViewerApp : public AppWindow{
    public:
    MeshViewerApp(int width, int height, std::string windowName);

    void loadMesh(std::string objFilename);
    
    void updateState() override;

    MeshViewerData data;
    gShader* shader=nullptr;
    cgMat4 projectionMatrix;
    cgVec3 viewPosition=cgVec3(0,-2,0);
    std::vector<MeshGPUBuffer*> renderBuffers;
};

void logOpenGLErrors();

#endif /* MESHVIEWERAPP */
