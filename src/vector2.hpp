#ifndef __VECTOR2_HPP
#define __VECTOR2_HPP

template <typename T>
class Vector2 {
public:
    T x, y;

    Vector2() {}
    Vector2(T x, T y) : x(x), y(y) {}

    template <typename U>
    Vector2(U x, U y) : x(static_cast<T>(x)), y(static_cast<T>(y)) {}
};

typedef Vector2<int> Vector2i;
typedef Vector2<float> Vector2f;

#endif /* __VECTOR2_HPP */
