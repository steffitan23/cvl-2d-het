/*
 * mat.hpp
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

#ifndef MAT_HPP
#define MAT_HPP

#include <cmath>

float mat_radians(float put);
float mat_degrees(float put);


// ####################                            ####################
// #################### the strucs                 ####################


struct int4 {
    int a, b, c, d;

    int4(): a(0), b(0), c(0), d(0) {};
    int4(int s): a(s), b(s), c(s), d(s) {};
    int4(int u, int v, int w): a(u), b(u), c(v), d(w) {};
    int4(int u, int v, int w, int x): a(u), b(v), c(w), d(x) {};
};


struct vect2 {
    double a, b;

    vect2(): a(0), b(0) {};
    vect2(double s): a(s), b(s) {};
    vect2(double x, double y): a(x), b(y) {};

    // set operations
    void    set(const vect2& put);                  // set vector with new values
    void    add(const vect2& put);                  // add vector with existing values
    void    zero();                                 // set vector to zero

    // transform operations
    vect2&  normalize();
    double  norm() const;
    double  area(const vect2& put) const;           // calculate spanned area
    double  dot(const vect2& put) const;            // mult vector

    // operators
    vect2   operator-() const;
    vect2   operator+(const vect2& put) const;      // add put
    vect2   operator-(const vect2& put) const;      // sub put
    vect2   operator*(const double put) const;      // mult double
    vect2   operator/(const double put) const;      // divide double
};


struct vect3 {
    float a, b, c;

    vect3(): a(0), b(0), c(0) {};
    vect3(float s): a(s), b(s), c(s) {};
    vect3(float x, float y, float z) : a(x), b(y), c(z) {};

    // transform operations
    vect3&  normalize();
    float   dot(const vect3& put) const;            // dot product
    vect3   cross(const vect3& put) const;          // cross product

    // operators
    vect3   operator-() const;
    vect3   operator+(const vect3& put) const;      // add put
    vect3   operator-(const vect3& put) const;      // sub put
    vect3   operator*(const float put) const;      // mult put
};


struct matr4 {
    // constructors
    matr4();
    matr4(const float src[16]);
    matr4(float m00, float m01, float m02, float m03, // 1st column
          float m04, float m05, float m06, float m07, // 2nd column
          float m08, float m09, float m10, float m11,
          float m12, float m13, float m14, float m15);

    void set(const float src[16]);
    void set(float m00, float m01, float m02, float m03, // 1st column
             float m04, float m05, float m06, float m07, // 2nd column
             float m08, float m09, float m10, float m11,
             float m12, float m13, float m14, float m15);

    const float* get() const;
    matr4& identity();

    // transform matrix
    matr4& translate(float x, float y, float z);    // translation by (x,y,z)
    matr4& rotate(float angle, float x, float y, float z);    // rotation by (x,y,z) degrees
    matr4& rotateX(float angle);                    // rotate on X-axis with degree
    matr4& rotateY(float angle);                    // rotate on Y-axis with degree
    matr4& rotateZ(float angle);                    // rotate on Z-axis with degree
    matr4& scale(float sc);                         // uniform scale
    matr4& scale(float x, float y, float z);
    matr4& lookAt(const vect3& from, const vect3& target, const vect3& head);

    // operators
    matr4   operator*(const matr4& put) const;
    float   operator[](int index) const;            // subscript operator v[0], v[1]
    float&  operator[](int index);                  // subscript operator v[0], v[1]

private:
    float m[16];
};


// view creation functions
matr4 setOrtho(float l, float r, float b, float t, float n, float f);
matr4 setPerspective(float fovY, float aspect, float front, float back);



// ####################                            ####################
// #################### the vect2 inlines          ####################


inline vect2& vect2::normalize()
{
    double ab2 = a*a + b*b, invLength = 1.0 / std::sqrt(ab2);
    a *= invLength;
    b *= invLength;

    return *this;
}

inline void vect2::set(const vect2& put)
{
    a = put.a;
    b = put.b;
}

inline void vect2::add(const vect2& put)
{
    a = a + put.a;
    b = b + put.b;
}

inline void vect2::zero()
{
    a = b = 0;
}

inline double vect2::norm() const
{
    return std::sqrt(a*a + b*b);
}

inline double vect2::area(const vect2& put) const
{
    return std::abs(a * put.b - b * put.a) * 0.5;
}

inline double vect2::dot(const vect2& put) const
{
    return a * put.a + b * put.b;
}

inline vect2 vect2::operator-() const
{
    return vect2(-a, -b);
}

inline vect2 vect2::operator+(const vect2& put) const
{
    return vect2(a + put.a, b + put.b);
}

inline vect2 vect2::operator-(const vect2& put) const
{
    return vect2(a - put.a, b - put.b);
}

inline vect2 vect2::operator*(const double put) const
{
    return vect2(a * put, b * put);
}

inline vect2 vect2::operator/(const double put) const
{
    return vect2(a / put, b / put);
}


// ####################                            ####################
// #################### the vect3 inlines          ####################


inline vect3& vect3::normalize()
{
    float abc2 = a*a + b*b + c*c;

    float invLength = 1.0f / sqrtf(abc2);
    a *= invLength;
    b *= invLength;
    c *= invLength;

    return *this;
}

inline float vect3::dot(const vect3& put) const
{
    return (a * put.a + b * put.b + c * put.c);
}

inline vect3 vect3::cross(const vect3& put) const
{
    return vect3(b * put.c - c * put.b, c * put.a - a * put.c, a * put.b - b * put.a);
}


inline vect3 vect3::operator-() const
{
    return vect3(-a, -b, -c);
}

inline vect3 vect3::operator+(const vect3& put) const
{
    return vect3(a + put.a, b + put.b, c + put.c);
}

inline vect3 vect3::operator-(const vect3& put) const
{
    return vect3(a - put.a, b - put.b, c - put.c);
}

inline vect3 vect3::operator*(const float put) const
{
    return vect3(a * put, b * put, c * put);
}


// ####################                            ####################
// #################### the matr4 inlines          ####################


inline matr4::matr4()
{
    identity();
}

inline matr4::matr4(const float src[16])
{
    set(src);
}

inline matr4::matr4(float m00, float m01, float m02, float m03,
                        float m04, float m05, float m06, float m07,
                        float m08, float m09, float m10, float m11,
                        float m12, float m13, float m14, float m15)
{
    set(m00, m01, m02, m03,  m04, m05, m06, m07,  m08, m09, m10, m11,  m12, m13, m14, m15);
}

inline void matr4::set(const float src[16])
{
    m[0] = src[0];  m[1] = src[1];  m[2] = src[2];  m[3] = src[3];
    m[4] = src[4];  m[5] = src[5];  m[6] = src[6];  m[7] = src[7];
    m[8] = src[8];  m[9] = src[9];  m[10]= src[10]; m[11]= src[11];
    m[12]= src[12]; m[13]= src[13]; m[14]= src[14]; m[15]= src[15];
}

inline void matr4::set(float m00, float m01, float m02, float m03,
                         float m04, float m05, float m06, float m07,
                         float m08, float m09, float m10, float m11,
                         float m12, float m13, float m14, float m15)
{
    m[0] = m00;  m[1] = m01;  m[2] = m02;  m[3] = m03;
    m[4] = m04;  m[5] = m05;  m[6] = m06;  m[7] = m07;
    m[8] = m08;  m[9] = m09;  m[10]= m10;  m[11]= m11;
    m[12]= m12;  m[13]= m13;  m[14]= m14;  m[15]= m15;
}


inline const float* matr4::get() const
{
    return m;
}

inline matr4& matr4::identity()
{
    m[0] = m[5] = m[10] = m[15] = 1.0f;
    m[1] = m[2] = m[3] = m[4] = m[6] = m[7] = m[8] = m[9] = m[11] = m[12] = m[13] = m[14] = 0.0f;
    return *this;
}



inline matr4 matr4::operator*(const matr4& put) const
{
    return matr4(
        m[0]*put[0]  + m[4]*put[1]  + m[8]*put[2]   + m[12]*put[3],
        m[1]*put[0]  + m[5]*put[1]  + m[9]*put[2]   + m[13]*put[3],
        m[2]*put[0]  + m[6]*put[1]  + m[10]*put[2]  + m[14]*put[3],
        m[3]*put[0]  + m[7]*put[1]  + m[11]*put[2]  + m[15]*put[3],

        m[0]*put[4]  + m[4]*put[5]  + m[8]*put[6]   + m[12]*put[7],
        m[1]*put[4]  + m[5]*put[5]  + m[9]*put[6]   + m[13]*put[7],
        m[2]*put[4]  + m[6]*put[5]  + m[10]*put[6]  + m[14]*put[7],
        m[3]*put[4]  + m[7]*put[5]  + m[11]*put[6]  + m[15]*put[7],

        m[0]*put[8]  + m[4]*put[9]  + m[8]*put[10]  + m[12]*put[11],
        m[1]*put[8]  + m[5]*put[9]  + m[9]*put[10]  + m[13]*put[11],
        m[2]*put[8]  + m[6]*put[9]  + m[10]*put[10] + m[14]*put[11],
        m[3]*put[8]  + m[7]*put[9]  + m[11]*put[10] + m[15]*put[11],

        m[0]*put[12] + m[4]*put[13] + m[8]*put[14]  + m[12]*put[15],
        m[1]*put[12] + m[5]*put[13] + m[9]*put[14]  + m[13]*put[15],
        m[2]*put[12] + m[6]*put[13] + m[10]*put[14] + m[14]*put[15],
        m[3]*put[12] + m[7]*put[13] + m[11]*put[14] + m[15]*put[15]);
}


inline float matr4::operator[](int index) const
{
    return m[index];
}


inline float& matr4::operator[](int index)
{
    return m[index];
}


#endif /* MAT_HPP */
