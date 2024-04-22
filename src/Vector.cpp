#include "Vector.hpp"

cgVec3 cgVec3::crossProd(const cgVec3 &vector)
{
    cgVec3 output = cgVec3(y * vector.z - z * vector.y, z * vector.x - x * vector.z, x * vector.y - y * vector.x);
    return output;
}

void cgVec3::normalize()
{
    float r = norm();
    x = x / r;
    y = y / r;
    z = z / r;
}

float cgVec3::norm()
{
    return sqrt(x * x + y * y + z * z);
}

cgVec2::cgVec2(float x, float y)
{
    this->x = x;
    this->y = y;
}

cgVec2::cgVec2(const cgVec2 &vector)
{
    x = vector.x;
    y = vector.y;
}

cgVec2 cgVec2::operator+(const cgVec2 &vector)
{
    return cgVec2(this->x + vector.x, this->y + vector.y);
}

cgVec2 cgVec2::operator-(const cgVec2 &vector)
{
    return cgVec2(this->x - vector.x, this->y - vector.y);
}

cgVec2 cgVec2::operator*(const float &scalar)
{
    return cgVec2(this->x * scalar, this->y * scalar);
}

float cgVec2::innerProd(const cgVec2 &vector)
{
    return this->x * vector.x + this->y * vector.y;
}

cgVec2 cgVec2::normalized()
{
    float norm = sqrt(x * x + y * y);
    return cgVec2(x / norm, y / norm);
}

float dot(cgVec2 v1, cgVec2 v2)
{
    return v1.x * v2.x + v1.y * v2.y;
}

cgMat4::cgMat4(float xx, float yx, float zx, float wx,
               float xy, float yy, float zy, float wy,
               float xz, float yz, float zz, float wz,
               float xw, float yw, float zw, float ww)
{
    data[0] = xx; // 00
    data[1] = xy; // 10
    data[2] = xz; // 20
    data[3] = xw; // 30

    data[4] = yx;
    data[5] = yy; // 11
    data[6] = yz;
    data[7] = yw;

    data[8] = zx;
    data[9] = zy;
    data[10] = zz;
    data[11] = zw;

    data[12] = wx;
    data[13] = wy;
    data[14] = wz;
    data[15] = ww;
}

cgMat4 trans(float Tx, float Ty, float Tz)
{
    return cgMat4(1, 0, 0, Tx,
                  0, 1, 0, Ty,
                  0, 0, 1, Tz,
                  0, 0, 0, 1);
}

cgMat4 projectionMatrixSimple(float aspect, float fov, float n, float f)
{
    float f_p = 1.0/(tan(fov/2));
    return cgMat4(f_p/aspect, 0, 0, 0,
                0, f_p, 0, 0,
                0, 0, (f+n)/(n-f), (2*f*n)/(n-f),
                0, 0, -1, 0);
}

cgMat4 lookAt(cgVec3 eye, cgVec3 center, cgVec3 up)
{

    cgVec3 zaxis = (center - eye).normalized();
    cgVec3 xaxis = (cross(zaxis, up)).normalized();
    cgVec3 yaxis = cross(xaxis, zaxis);
    zaxis = zaxis * -1;
    cgMat4 result = {
        xaxis.x, xaxis.y, xaxis.z, -dot(xaxis, eye),
        yaxis.x, yaxis.y, yaxis.z, -dot(yaxis, eye),
        zaxis.x, zaxis.y, zaxis.z, -dot(zaxis, eye),
        0, 0, 0, 1};

    return result;
}

cgVec3 cross(const cgVec3 &v1, const cgVec3 &v2)
{
    cgVec3 out = cgVec3();
    out.x = v1.y * v2.z - v1.z * v2.y;
    out.y = v1.z * v2.x - v1.x * v2.z;
    out.z = v1.x * v2.y - v1.y * v2.x;
    return out;
}

float dot(cgVec3 v1, cgVec3 v2)
{
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

cgMat4 cgMat4::operator*(const cgMat4 &mat) const
{
    cgMat4 output = cgMat4();
    float sum;
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            sum = 0;
            for (int k = 0; k < 4; k++)
            {
                sum = sum + at(i, k) * mat.at(k, j);
            }
            output.set(i, j, sum);
        }
    }
    return output;
}

cgVec3 cgMat4::operator*(const cgVec3& vec) const{
    return cgVec3(  data[0]*vec.x+data[4]*vec.y+data[8]*vec.z+data[12]*1,
                    data[1]*vec.x+data[5]*vec.y+data[9]*vec.z+data[13]*1,
                    data[2]*vec.x+data[6]*vec.y+data[10]*vec.z+data[14]*1
                    );
}

cgMat4 rotation(cgVec3 axis, float theta)
{
    return cgMat4(cos(theta) + axis.x * axis.x * (1 - cos(theta)), axis.x * axis.y * (1 - cos(theta)) - axis.z * sin(theta), axis.x * axis.z * (1 - cos(theta)) + axis.y * sin(theta), 0,
                  axis.y * axis.x * (1 - cos(theta)) + axis.z * sin(theta), cos(theta) + axis.y * axis.y * (1 - cos(theta)), axis.y * axis.z * (1 - cos(theta)) - axis.x * sin(theta), 0,
                  axis.z * axis.x * (1 - cos(theta)) - axis.y * sin(theta), axis.z * axis.y * (1 - cos(theta)) + axis.x * sin(theta), cos(theta) + axis.z * axis.z * (1 - cos(theta)), 0,
                  0, 0, 0, 1);
}