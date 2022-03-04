/*
 * mat.cpp
 *
 * Copyright 2005 Song Ho Ahn, 2017 Tung Thai simpified <>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 *
 *
 */

#include "matr.h"


const float DEG2RAD = 3.141592653589793f / 180.0f;
const float RAD2DEG = 180.0f / 3.141592653589793f;


float mat_radians(float put)
{
    return (put * DEG2RAD);
}

float mat_degrees(float put)
{
    return (put * RAD2DEG);
}



matr4& matr4::translate(float x, float y, float z)
{
    m[12] += m[0]*x + m[4]*y + m[8]*z;
    m[13] += m[1]*x + m[5]*y + m[9]*z;
    m[14] += m[2]*x + m[6]*y + m[10]*z;
    m[15] += m[3]*x + m[7]*y + m[11]*z;
    return *this;
}

matr4& matr4::rotate(float angle, float x, float y, float z)
{
    float c = cosf(angle * DEG2RAD);    // cosine
    float s = sinf(angle * DEG2RAD);    // sine
    float c1 = 1.0f - c;                // 1 - c
    float m0 = m[0], m4 = m[4], m8  = m[8],  m12 = m[12],
          m1 = m[1], m5 = m[5], m9  = m[9],  m13 = m[13],
          m2 = m[2], m6 = m[6], m10 = m[10], m14 = m[14];

    float
        r0  = x * x * c1 + c,
        r1  = x * y * c1 + z * s,
        r2  = x * z * c1 - y * s,
        r4  = x * y * c1 - z * s,
        r5  = y * y * c1 + c,
        r6  = y * z * c1 + x * s,
        r8  = x * z * c1 + y * s,
        r9  = y * z * c1 - x * s,
        r10 = z * z * c1 + c;

    m[0] = r0 * m0  + r4 * m1  + r8  * m2;
    m[1] = r1 * m0  + r5 * m1  + r9  * m2;
    m[2] = r2 * m0  + r6 * m1  + r10 * m2;
    m[4] = r0 * m4  + r4 * m5  + r8  * m6;
    m[5] = r1 * m4  + r5 * m5  + r9  * m6;
    m[6] = r2 * m4  + r6 * m5  + r10 * m6;
    m[8] = r0 * m8  + r4 * m9  + r8  * m10;
    m[9] = r1 * m8  + r5 * m9  + r9  * m10;
    m[10]= r2 * m8  + r6 * m9  + r10 * m10;
    m[12]= r0 * m12 + r4 * m13 + r8  * m14;
    m[13]= r1 * m12 + r5 * m13 + r9  * m14;
    m[14]= r2 * m12 + r6 * m13 + r10 * m14;

    return *this;
}

matr4& matr4::rotateX(float angle)
{
    float c = cosf(angle * DEG2RAD);
    float s = sinf(angle * DEG2RAD);
    float m1  = m[1],  m2  = m[2],
          m5  = m[5],  m6  = m[6],
          m9  = m[9],  m10 = m[10],
          m13 = m[13], m14 = m[14];

    m[1]  = m1  * c - m2  * s;
    m[2]  = m1  * s + m2  * c;
    m[5]  = m5  * c - m6  * s;
    m[6]  = m5  * s + m6  * c;
    m[9]  = m9  * c - m10 * s;
    m[10] = m9  * s + m10 * c;
    m[13] = m13 * c - m14 * s;
    m[14] = m13 * s + m14 * c;

    return *this;
}

matr4& matr4::rotateY(float angle)
{
    float c = cosf(angle * DEG2RAD);
    float s = sinf(angle * DEG2RAD);
    float m0  = m[0],  m2  = m[2],
          m4  = m[4],  m6  = m[6],
          m8  = m[8],  m10 = m[10],
          m12 = m[12], m14 = m[14];

    m[0]  = m2  * s + m0  * c;
    m[2]  = m2  * c - m0  * s;
    m[4]  = m6  * s + m4  * c;
    m[6]  = m6  * c - m4  * s;
    m[8]  = m10 * s + m8  * c;
    m[10] = m10 * c - m8  * s;
    m[12] = m14 * s + m12 * c;
    m[14] = m14 * c - m12 * s;

    return *this;
}

matr4& matr4::rotateZ(float angle)
{
    float c = cosf(angle * DEG2RAD);
    float s = sinf(angle * DEG2RAD);
    float m0  = m[0],  m1  = m[1],
          m4  = m[4],  m5  = m[5],
          m8  = m[8],  m9  = m[9],
          m12 = m[12], m13 = m[13];

    m[0] = m0  * c - m1  * s;
    m[1] = m0  * s + m1  * c;
    m[4] = m4  * c - m5  * s;
    m[5] = m4  * s + m5  * c;
    m[8] = m8  * c - m9  * s;
    m[9] = m8  * s + m9  * c;
    m[12]= m12 * c - m13 * s;
    m[13]= m12 * s + m13 * c;

    return *this;
}


matr4& matr4::scale(float x, float y, float z)
{
    m[0] *= x;  m[4] *= y;  m[8] *= z;
    m[1] *= x;  m[5] *= y;  m[9] *= z;
    m[2] *= x;  m[6] *= y;  m[10]*= z;
    m[3] *= x;  m[7] *= y;  m[11]*= z;
    return *this;
}


matr4& matr4::scale(float sc)
{
    return scale(sc, sc, sc);
}


matr4& matr4::lookAt(const vect3& from, const vect3& target, const vect3& head)
{
    // compute forward vector and normalize
    vect3 forward = from - target;
    forward.normalize();

    // compute left vector
    vect3 left = head.cross(forward);
    left.normalize();

    // compute orthonormal up vector
    vect3 up = forward.cross(left);
    up.normalize();

    m[0] = left.a;
    m[1] = up.a;
    m[2] = forward.a;
    m[3] = 0;

    m[4] = left.b;
    m[5] = up.b;
    m[6] = forward.b;
    m[7] = 0;

    m[8]  = left.c;
    m[9]  = up.c;
    m[10] = forward.c;
    m[11] = 0;

    m[12] = -left.dot(from);;
    m[13] = -up.dot(from);
    m[14] = -forward.dot(from);
    m[15] = 1.0f;

    return *this;
}


// view creation functions

matr4 setOrtho(float l, float r, float b, float t, float n, float f)
{
    matr4 mat;
    mat[0]  = 2 / (r - l);
    mat[5]  = 2 / (t - b);
    mat[10] = -2 / (f - n);
    mat[12] = -(r + l) / (r - l);
    mat[13] = -(t + b) / (t - b);
    mat[14] = -(f + n) / (f - n);

    return mat;
}


matr4 setPerspective(float fovY, float aspect, float front, float back)
{
    float tangent = tanf(fovY/2 * DEG2RAD); // tangent of half fovY
    float height = front * tangent;         // half height of near plane
    float width = height * aspect;          // half width of near plane

    matr4 mat;
    mat[0]  = front / width;
    mat[5]  = front / height;
    //mat[8]  = (width - width) / (width + width);
    //mat[9]  = (height - height) / (height + height);
    mat[10] = -(back + front) / (back - front);
    mat[11] = -1;
    mat[14] = -(2 * back * front) / (back - front);
    mat[15] = 0;

    return mat;
}
