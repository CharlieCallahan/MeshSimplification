#include "MeshViewerApp.hpp"
#include "Vector.hpp"

gShader::gShader(char* vertcode, char* fragcode){
    // compile shaders
    unsigned int vertex, fragment;
    // vertex shader
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vertcode, NULL);
    glCompileShader(vertex);
    checkCompileErrors(vertex, "VERTEX");
    // fragment Shader
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fragcode, NULL);
    glCompileShader(fragment);
    checkCompileErrors(fragment, "FRAGMENT");
    // shader Program
    shader = glCreateProgram();
    glAttachShader(shader, vertex);
    glAttachShader(shader, fragment);
    glLinkProgram(shader);
    checkCompileErrors(shader, "PROGRAM");
    // delete the shaders as they're linked into our program now and no longer necessary
    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

void gShader::bind(){
    glUseProgram(shader);
}

void gShader::checkCompileErrors(unsigned int shader, std::string type)
{
    int success;
    char infoLog[1024];
    if (type != "PROGRAM")
    {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            exit(-1);
        }
    }
    else
    {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            exit(-1);
        }
    }
}

void MeshViewerApp::updateState(){
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //calculate view matrix
    this->viewPosition = cgVec3(0,0,2);
    cgMat4 viewTrans = trans(-this->viewPosition.x,-this->viewPosition.y,-this->viewPosition.z);

    cgMat4 viewMatrix = projectionMatrix*viewTrans;

    shader->bind();
    // std::cout << "A"; logOpenGLErrors();

    const char* sceneTransform_c = "sceneTransform";
    const char* modelTransform_c = "modelTransform";

    GLint stloc = glGetUniformLocation(shader->shader, sceneTransform_c);
    glUniformMatrix4fv(stloc, 1, GL_FALSE, viewMatrix.data);

    for (MeshGPUBuffer* mesh : this->renderBuffers){
        cgMat4 modelTransform = trans(mesh->worldSpacePosition.x,mesh->worldSpacePosition.y,mesh->worldSpacePosition.z);

        GLint mtloc = glGetUniformLocation(shader->shader, modelTransform_c);
        glUniformMatrix4fv(mtloc, 1, GL_FALSE, modelTransform.data);
        mesh->bind();
        glDrawElements(GL_TRIANGLES, mesh->nIndices, GL_UNSIGNED_INT, nullptr);
    }

    glfwSwapBuffers(window);
    glfwPollEvents();
}

void MeshViewerApp::loadMesh(std::string objFilename){
    std::string objID = "MeshDecimation";
    loadOBJFile(objFilename, this->data.original_meshes, objID);
    std::cout << "Loaded "<<data.original_meshes.size() << " Meshes\n";
    for(objItem* item : data.original_meshes){
        std::cout << item->name<<"\n";
        std::cout << "num verts: "<<item->positions.size()<<"\n";
        std::cout << "num tris: "<<item->indices.size()/3<<"\n";
    }

    this->renderBuffers = std::vector<MeshGPUBuffer*>(data.original_meshes.size());

    int indx=0;

    for(objItem* item : data.original_meshes){
        std::cout << item->positions.size()<<"\n";
        std::vector<gVertex> vertexData = std::vector<gVertex>(item->positions.size());

        for(int i = 0; i < item->positions.size(); i++){
            vertexData[i].position = item->positions[i];
            vertexData[i].normal = item->normals[i];
        }
        this->renderBuffers[indx]=new MeshGPUBuffer(item->indices.size(),item->indices.data(),vertexData.size(),vertexData.data());
        indx++;
    }

}

MeshViewerApp::MeshViewerApp(int width, int height, std::string windowName) : AppWindow(width,height,windowName){
    //just inline the shader here for simplicity
    char* vertexShader = "#version 330 core "
    "layout (location = 0) in vec3 aPos_i; // the position variable has attribute position 0\n"
    "layout (location = 1) in vec3 aNormal_i; //model corresponding to vertex data\n"
    "uniform mat4 sceneTransform; //camera transform\n"
    "uniform mat4 modelTransform; //model transform\n"
    "out VS_OUT{ //output to frag shader\n"
    "    vec3 fragPos;\n"
    "    vec3 fragNormal;\n"
    "} vs_out;\n"
    "void main()\n"
    "{\n"
    "    mat4 transform = sceneTransform*modelTransform;\n"
    "    gl_Position = transform*vec4(aPos_i, 1.0); //position on the screen\n"
    "    vs_out.fragPos = vec3(modelTransform * vec4(aPos_i, 1.0)); //fragment position in world space\n"
    "    vs_out.fragNormal = vec3(modelTransform * vec4(aNormal_i, 1.0)); //fragment normal in world space\n"
    "}";

    char* fragShader = "#version 330 core "
    "in VS_OUT{ //output from frag shader\n"
    "    vec3 fragPos;\n"
    "    vec3 fragNormal;\n"
    "} vs_in;\n"
    "out vec4 fragColor;\n"
    "void main()\n"
    "{\n"
    "    fragColor = vec4(vs_in.fragNormal,1);\n"
    "}";

    this->shader = new gShader(vertexShader,fragShader);

    projectionMatrix = projectionMatrixSimple(1.4, 1, 0.1, 100);
}

MeshGPUBuffer::MeshGPUBuffer(int nIndices, int* indices, int nVerts, gVertex* vertices){

    //generate vertex array
    glGenVertexArrays(1, &this->vertexArray);
    glBindVertexArray(this->vertexArray);

    //gen index buffer
    glGenBuffers(1,&this->indexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)*nIndices, indices, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    this->nIndices = nIndices;

    //generate vert buffer (only positions+normals for this stripped down example)
    glGenBuffers(1, &this->vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, this->vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices)*nVerts, vertices, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(this->vertexArray);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,this->indexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER,this->vertexBuffer);

    //position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)(int64_t(0)));

    //normal
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)(int64_t(3*sizeof(float))));

    worldSpacePosition = cgVec3(0,0,0);
    // setVertexAttrPointer(0, 3, GL_FLOAT, 7*sizeof(float), 0); //pos
    // setVertexAttrPointer(1, 4, GL_FLOAT, 7*sizeof(float), 3*sizeof(float)); //color
}

void MeshGPUBuffer::bind(){
    glBindVertexArray(this->vertexArray);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,this->indexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER,this->vertexBuffer);
}

MeshGPUBuffer::~MeshGPUBuffer(){
    glDeleteBuffers(1,&this->vertexBuffer);
    glDeleteBuffers(1,&this->indexBuffer);
    glDeleteVertexArrays(1,&this->vertexArray);
}

void logOpenGLErrors(){
    GLenum err;
    while((err = glGetError()) != GL_NO_ERROR)
    {
        std::cout << "OpenGL Error Code: " << err << std::endl;
    }
}