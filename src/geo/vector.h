/*
 * Taken from https://github.com/Digitelektro/MeteorDemod
 * Its license is as follows:
 * 
 * MIT License
 * 
 * Copyright (c) 2020 Digitelektro
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef VECTOR_H
#define VECTOR_H

struct Vector {
    double x;
    double y;
    double z;
    double w;

    public:
        Vector()
            : x(0.0),
              y(0.0),
              z(0.0),
              w(0.0) { }

        Vector(double _x, double _y, double _z)
            : x(_x),
              y(_y),
              z(_z),
              w(0.0) { }

        double Magnitude() {
            return sqrt(x * x + y * y + z * z);
        }

        Vector operator-(const Vector& rhs) {
            return Vector(x - rhs.x,
                          y - rhs.y,
                          z - rhs.z);
        }
};

struct Vector3 : Vector {

    Vector3()
        : Vector() {
    }

    Vector3(const Vector v)
        : Vector(v) {
    }

    Vector &operator -= (const Vector& v) {
        x -= v.x;
        y -= v.y;
        z -= v.z;
        return *this;
    }

    Vector3 operator * (double factor) const {
        Vector3 r(*this);
        r.x *= factor;
        r.y *= factor;
        r.z *= factor;
        return r;
    }

    Vector3 &operator *= (double factor) {
        x *= factor;
        y *= factor;
        z *= factor;
        return *this;
    }

    Vector3 operator / (double factor) const {
        Vector3 r(*this);
        r.x /= factor;
        r.y /= factor;
        r.z /= factor;
        return r;
    }

    Vector3 &operator /= (double factor) {
        x /= factor;
        y /= factor;
        z /= factor;
        return *this;
    }

    Vector3 Cross(const Vector& v) const {
        Vector3 r;
        r.x = y * v.z - z * v.y;
        r.y = z * v.x - x * v.z;
        r.z = x * v.y - y * v.x;
        return r;
    }

    Vector3 &Normalize() {
        double m = Magnitude();
        if(m > 0) {
            return (*this) /= m;
        }
        return *this;
    }

    double DistanceSquared() const {
        return (x * x + y * y + z * z);
    }
};

#endif // VECTOR_H
