#include "Size3D.h"

Size3D::Size3D() : _width(0), _height(0), _depth(0) {}

Size3D::Size3D(int width, int height, int depth) : _width(width), _height(height), _depth(depth) {}

bool Size3D::isEmpty() const
{
    return _width <= 0 || _height <= 0 || _depth <= 0;
}

bool Size3D::isNull() const
{
    return _width == 0 && _height == 0 && _depth == 0;
}

int Size3D::width() const
{
    return _width;
}

int Size3D::height() const
{
    return _height;
}

int Size3D::depth() const
{
    return _depth;
}

void Size3D::setWidth(int width)
{
    _width = width;
}

void Size3D::setHeight(int height)
{
    _height = height;
}

void Size3D::setDepth(int depth)
{
    _depth = depth;
}

mv::Vector3f Size3D::toVector3f() const
{
    return mv::Vector3f(static_cast<float>(_width), static_cast<float>(_height), static_cast<float>(_depth));
}

bool Size3D::operator==(const Size3D &other) const
{
    return _width == other._width && _height == other._height && _depth == other._depth;
}

bool Size3D::operator!=(const Size3D &other) const
{
    return !(*this == other);
}

QDebug operator<<(QDebug dbg, const Size3D &size)
{
    dbg.nospace() << "Size3D(" << size.width() << ", " << size.height() << ", " << size.depth() << ")";
    return dbg.space();
}
