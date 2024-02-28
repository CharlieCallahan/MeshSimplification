#ifndef MESHVIEWERAPP
#define MESHVIEWERAPP
#include "AppWindow.hpp"
#include "MeshLoader.hpp"

// vertex data passed to the shader
// #pragma pack(1)
struct gVertex{
    cgVec3 position;
    cgVec3 normal;
};
// #pragma pack(pop)

//glsl shader
struct gShader{
    public:
    gShader(const char* vertcode, const char* fragcode);

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
    std::vector<objItem*> processed_meshes;
};

class MeshViewerApp : public AppWindow{
    public:
    MeshViewerApp(int width, int height, std::string windowName);

    void loadMesh(std::string objFilename);
    
    void simplifyMeshes(float compressionfactor, float maxSinTheta);

    void updateState() override;

    void processInput() override; 

    void moveCursor(double x, double y);

    MeshViewerData data;
    gShader* shader=nullptr;
    cgMat4 projectionMatrix;
    cgVec3 viewPosition=cgVec3(0,0,2);
    cgVec3 viewDir = cgVec3(0,0,-1);
    cgVec3 viewX = cgVec3(1,0,0);
    float viewYaw, viewPitch;
    std::vector<MeshGPUBuffer*> renderBuffers;

    private:
    MeshGPUBuffer* generateRenderBuffer(std::vector<cgVec3>& pts, std::vector<cgVec3>& normals, std::vector<int>& indices);
};

void logOpenGLErrors();

void mouse_callback(GLFWwindow* window, double xpos, double ypos);

#endif /* MESHVIEWERAPP */
