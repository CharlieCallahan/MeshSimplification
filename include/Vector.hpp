#ifndef VECTOR
#define VECTOR
#include <iostream>

/**
 * @brief A pared down vector library
 * 
 */

struct cgVec3{
    float x;
    float y;
    float z;
    cgVec3(){this->x = 0;this->y = 0;this->z = 0;};
    cgVec3(float x,float y, float z){this->x = x;this->y = y;this->z = z;};
    cgVec3(const cgVec3& vec1){x = vec1.x;y = vec1.y;z = vec1.z;}
    float at(int index){return *(((float*)this)+index);};
    void set(int index, float to){ *(((float*)this)+index) = to;};
    void print(){std::cout <<"("<< x <<", " << y << ", " << z << ", " << " )\n";}
    void add(cgVec3 vec){x = x+vec.x;y = y+vec.y;z = z+vec.z;}
    cgVec3 operator+(const cgVec3& vector) {return cgVec3(vector.x+x,vector.y+y,vector.z+z);}
    cgVec3 operator-(const cgVec3& vector) {return cgVec3(x-vector.x,y-vector.y,z-vector.z);}
    //cgVec3 operator-(const cgVec3& vector) {return cgVec3(vector.x-x,vector.y-y,vector.z-z);}
    cgVec3 operator*(const float scalar) {return cgVec3(x*scalar,y*scalar,z*scalar);}
    cgVec3 operator/(const float scalar) {return cgVec3(x/scalar,y/scalar,z/scalar);}
    void operator=(const cgVec3& v){x=v.x;y=v.y;z=v.z;}

    //element wise multiplication
    cgVec3 operator*(const cgVec3& v){return {v.x*x,v.y*y,v.z*z}; }
    
    cgVec3 scale(float scalar){return cgVec3(x*scalar,y*scalar,z*scalar);}
    cgVec3 crossProd(const cgVec3& vector);
    float dot(const cgVec3& v){return v.x*x + v.y*y + v.z*z;}
    void normalize();
    cgVec3 normalized(){float n = norm(); return cgVec3(x/n,y/n,z/n);}
    float norm(); 

};

struct cgVec2{
    float x;
    float y;
    cgVec2(){};
    cgVec2(float x,float y);
    void print(){std::cout <<"("<< x <<", " << y << " )\n";}
    cgVec2(const cgVec2& vector); //Copy constructor
    cgVec2 operator+(const cgVec2& vector);
    cgVec2 operator-(const cgVec2& vector);
    cgVec2 operator*(const float& scalar);
    float innerProd(const cgVec2& vector);
    float mag(){return sqrt(x*x+y*y);}
    cgVec2 normalized();
};


struct cgMat4{
    float data[16];
    cgMat4(float xx,float yx,float zx,float wx,
           float xy,float yy,float zy,float wy,
           float xz,float yz,float zz,float wz,
           float xw,float yw,float zw,float ww);
    cgMat4(){for(int i = 0; i < 16; i++){data[i] = 0;}}
    cgMat4(const cgMat4& mat){for(int i = 0; i < 16; i++){data[i] = mat.data[i];}}
    inline float at(int r,int c) const {return data[4*c + r];}
    inline float* colPtr(int c) {return data + 4*c;}
    inline const float* colPtr_c(int c) const {return data + 4*c;}
    // inline const float* colPtr_c(int c) const {return data + 4*c;}
    inline void set(int r,int c, float to){data[4*c + r] = to;}
    void zeros(){for(int i = 0; i < 16; i++){data[i] = 0;}} 
    void toIdentity(){this->zeros(); this->set(0,0,1);this->set(1,1,1);this->set(2,2,1);this->set(3,3,1);}
    cgMat4 operator*(const cgMat4& mat) const;

};
cgMat4 trans(float Tx, float Ty, float Tz);

cgMat4 projectionMatrixSimple(float aspect, float fov, float n, float f);

cgMat4 lookAt(cgVec3 eye, cgVec3 center, cgVec3 up);

cgVec3 cross(const cgVec3& v1, const cgVec3& v2);

float dot(cgVec3 v1,cgVec3 v2);

#endif /* VECTOR */
