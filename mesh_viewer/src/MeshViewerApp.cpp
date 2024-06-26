#include "MeshViewerApp.hpp"
#include "Vector.hpp"
#include "AutoLOD.hpp"

gShader::gShader(const char* vertcode, const char* fragcode){
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
    //check if user is pressing q or e to rotate the view accordingly
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS){
        viewPitch-=1.0/60.0*2;
    }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS){
        viewPitch+=1.0/60.0*2;
    }

    //calculate look at position
    cgMat4 r1 = rotation(cgVec3(0,1,0),viewPitch);
    cgMat4 r1_inv = rotation(cgVec3(0,1,0),-viewPitch);
    cgMat4 r2 = rotation(r1_inv*cgVec3(1,0,0),viewYaw);
    cgMat4 r2_inv = rotation(r1*cgVec3(1,0,0),-viewYaw);

    cgMat4 viewRot = r1*r2;
    cgMat4 camRot = r1_inv*r2_inv;
    viewDir = camRot*cgVec3(0,0,-1);
    viewX = camRot*cgVec3(1,0,0);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //calculate view matrix
    cgMat4 viewTrans = trans(-this->viewPosition.x,-this->viewPosition.y,-this->viewPosition.z);
    cgMat4 viewMatrix = projectionMatrix*viewRot*viewTrans;

    shader->bind();

    const char* sceneTransform_c = "sceneTransform";
    const char* modelTransform_c = "modelTransform";
    const char* viewPosition_c = "viewPos";
    const char* color_c = "color";

    GLint stloc = glGetUniformLocation(shader->shader, sceneTransform_c);
    glUniformMatrix4fv(stloc, 1, GL_FALSE, viewMatrix.data);

    GLint vploc = glGetUniformLocation(shader->shader, viewPosition_c);
    glUniform3f(vploc, viewPosition.x,viewPosition.y,viewPosition.z);

    for (MeshGPUBuffer* mesh : this->renderBuffers){
        cgMat4 modelTransform = trans(mesh->worldSpacePosition.x,mesh->worldSpacePosition.y,mesh->worldSpacePosition.z);

        GLint mtloc = glGetUniformLocation(shader->shader, modelTransform_c);
        glUniformMatrix4fv(mtloc, 1, GL_FALSE, modelTransform.data);
        mesh->bind();
        //set color
        glUniform3f(glGetUniformLocation(shader->shader, color_c), 251.0/255.0, 247.0/255.0, 226.0/255.0);

        glDrawElements(GL_TRIANGLES, mesh->nIndices, GL_UNSIGNED_INT, nullptr);

        glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
        mesh->bind();
        //set to different color
        glUniform3f(glGetUniformLocation(shader->shader, color_c), 255.0, 0.0, 0.0);

        glDrawElements(GL_TRIANGLES, mesh->nIndices, GL_UNSIGNED_INT, nullptr);

        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    }

    glfwSwapBuffers(window);
    glfwPollEvents();
}

void MeshViewerApp::processInput(){
    cgVec3 delta = cgVec3(0,0,0);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS){
        delta=delta - viewX;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS){
        delta=delta + viewX;
    }
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS){
        delta=delta + viewDir;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS){
        delta=delta - viewDir;
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS){
        delta=delta + cgVec3(0,1,0);
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS){
        delta=delta - cgVec3(0,1,0);
    }
    viewPosition = viewPosition + delta*(1.0/60.0);
}

void MeshViewerApp::moveCursor(double x, double y){
    static double lastX=0;
    static double lastY=0;
    static bool isFirst = true;
    if(isFirst){
        lastX=x;
        lastY=y;
        isFirst=false;
        return;
    }

    double dx = x-lastX;
    double dy = y-lastY;
    lastX=x;
    lastY=y;
    viewPitch+=dx*1.0/60.0;
    viewYaw+=dy*1.0/60.0;
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
        this->renderBuffers[indx]=generateRenderBuffer(item->positions, item->normals, item->indices);
        indx++;
    }

    simplifyMeshes(20,100.0);
}

void MeshViewerApp::simplifyMeshes(float compressionfactor, float maxSinTheta){
    
    for (objItem* og_item : data.original_meshes){
        std::vector<geo::Facet> facets = std::vector<geo::Facet>(og_item->indices.size()/3);

        for(int i = 0; i < og_item->indices.size()/3; i++){
            facets[i] = geo::Facet(og_item->indices[i*3+0],og_item->indices[i*3+1],og_item->indices[i*3+2]);
        }

        std::vector<geo::Facet> simplified_facets;
        std::vector<geo::Facet> result_facets;
        std::vector<geo::Facet> result_facets_trash;
        std::vector<cgVec3> result_points;

        int actualSize;
        AutoLOD::genLODMesh(facets,og_item->positions,simplified_facets,compressionfactor,maxSinTheta,actualSize);
        std::cout << "Actual size: "<<actualSize<<"\n";

        geo::remapVertices(simplified_facets,og_item->positions,result_facets,result_points);

        std::vector<int> indices = std::vector<int>(result_facets.size()*3);
        for (int i = 0; i < result_facets.size(); i++){
            indices[i*3+0]=result_facets[i].inds[0];
            indices[i*3+1]=result_facets[i].inds[1];
            indices[i*3+2]=result_facets[i].inds[2];
        }
        //recalculate normals
        std::vector<cgVec3> result_normals = std::vector<cgVec3>(result_points.size());
        for(int i = 0; i < result_normals.size(); i++){
            result_normals[i] = cgVec3(0,0,0);
        }
        for (int i = 0; i < result_facets.size(); i++){
            cgVec3 p0 = result_points[result_facets[i].inds[0]];
            cgVec3 p1 = result_points[result_facets[i].inds[1]];
            cgVec3 p2 = result_points[result_facets[i].inds[2]];
            cgVec3 normal = geo::faceNormal(p0,p1,p2);
            result_normals[result_facets[i].inds[0]] = result_normals[result_facets[i].inds[0]]+normal;
            result_normals[result_facets[i].inds[1]] = result_normals[result_facets[i].inds[1]]+normal;
            result_normals[result_facets[i].inds[2]] = result_normals[result_facets[i].inds[2]]+normal;

        }
        for (int i = 0; i < result_normals.size(); i++){
            result_normals[i].normalize();
        
        }

        MeshGPUBuffer* newbuff = generateRenderBuffer(result_points,result_normals,indices);
        // MeshGPUBuffer* newbuff = generateRenderBuffer(og_item->positions,og_item->normals,indices);
        newbuff->worldSpacePosition = cgVec3(3,0,0);
        this->renderBuffers.push_back(newbuff);
    }
}

MeshGPUBuffer* MeshViewerApp::generateRenderBuffer(std::vector<cgVec3>& pts, std::vector<cgVec3>& normals, std::vector<int>& indices){
    std::vector<gVertex> vertexData = std::vector<gVertex>(pts.size());
    for(int i = 0; i < pts.size(); i++){
        vertexData[i].position = pts[i];
        vertexData[i].normal = normals[i];
    }
    return new MeshGPUBuffer(indices.size(),indices.data(),vertexData.size(),vertexData.data());
}

MeshViewerApp::MeshViewerApp(int width, int height, std::string windowName) : AppWindow(width,height,windowName){
    //just inline the shader here for simplicity
    const char* vertexShader = "#version 330 core "
    "layout (location = 0) in vec3 aPos_i; // the position variable has attribute position 0\n"
    "layout (location = 1) in vec3 aNormal_i; //model corresponding to vertex data\n"
    "uniform mat4 sceneTransform; //camera transform\n"
    "uniform mat4 modelTransform; //model transform\n"
    "out VS_OUT{ //output to frag shader\n"
    "    vec3 fragPos;\n"
    "    flat vec3 fragNormal;\n"
    "} vs_out;\n"
    "void main()\n"
    "{\n"
    "    mat4 transform = sceneTransform*modelTransform;\n"
    "    gl_Position = transform*vec4(aPos_i, 1.0); //position on the screen\n"
    "    vs_out.fragPos = vec3(modelTransform * vec4(aPos_i, 1.0)); //fragment position in world space\n"
    "    vs_out.fragNormal = aNormal_i;//vec3(modelTransform * vec4(aNormal_i, 1.0)); //fragment normal in world space\n"
    "}";

    const char* fragShader = "#version 330 core "
    "in VS_OUT{ //output from frag shader\n"
    "    vec3 fragPos;\n"
    "    flat vec3 fragNormal;\n"
    "} vs_in;\n"
    "out vec4 fragColor;\n"
    "uniform vec3 viewPos;\n"
    "uniform vec3 color;\n"
    "void main()\n"
    "{\n"
    "    vec3 d = normalize(vs_in.fragPos - viewPos);"
    "    float fresnel = pow(1.0-dot(d,-1*vs_in.fragNormal),1);"
    "    fresnel = fresnel + 0.05;"
    "    fresnel = min(1,max(0,fresnel));"
    "    fragColor = vec4(color.x*fresnel, color.y*fresnel, color.z*fresnel,1);\n"
    "}";

    this->shader = new gShader(vertexShader,fragShader);

    projectionMatrix = projectionMatrixSimple(1.4, 1, 0.1, 100);

    glfwSetWindowUserPointer(window,this);
    viewYaw=0;
    viewPitch=0;
}

MeshGPUBuffer::MeshGPUBuffer(int nIndices, int* indices, int nVerts, gVertex* vertices){

    //generate vertex array
    glGenVertexArrays(1, &this->vertexArray);
    glBindVertexArray(this->vertexArray);

    //gen index buffers
    glGenBuffers(1,&this->indexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)*nIndices, indices, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    this->nIndices = nIndices;

    //generate vert buffer (only positions+normals for this stripped down example)
    glGenBuffers(1, &this->vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, this->vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(gVertex)*nVerts, vertices, GL_DYNAMIC_DRAW);
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
