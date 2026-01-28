// SPDX-License-Identifier: LGPL-3.0-or-later 
// A corresponding LICENSE file is located in the root directory of this source tree 
// Copyright (C) 2023 BioVault (Biomedical Visual Analytics Unit LUMC - TU Delft) 

#include "Volume.h"

Volume::Volume(const Size3D& size, const std::uint32_t& noComponents, const QString& volumeFilePath) :
    _data(),
    _size(size),
    _noComponents(noComponents),
    _volumeFilePath(volumeFilePath),
    _dimensionName()
{
    setSize(size);
}

std::uint16_t* Volume::data()
{
    return _data.data();
}

Size3D Volume::size() const
{
    return _size;
}

void Volume::setSize(const Size3D& size)
{
    if (size == _size)
        return;

    _size = size;

    _data.resize(noElements());
}

std::uint32_t Volume::width() const
{
    return _size.width();
}

std::uint32_t Volume::height() const
{
    return _size.height();
}

std::uint32_t Volume::depth() const
{
    return _size.depth();
}

std::uint32_t Volume::noComponents() const
{
    return _noComponents;
}

std::uint32_t Volume::noVoxels() const
{
    return _size.width() * _size.height() * _size.depth();
}

std::uint32_t Volume::noElements() const
{
    return noVoxels() * _noComponents;
}

std::uint32_t Volume::voxelIndex(const std::uint32_t& x, const std::uint32_t& y, const std::uint32_t& z) const
{
    return (z * _size.width() * _size.height()) + (y * _size.width()) + x;
}

void Volume::getVoxel(const std::uint32_t& x, const std::uint32_t& y, const std::uint32_t& z, std::uint16_t* voxel) const
{
    const auto startVoxelIndex = voxelIndex(x, y, z) * _noComponents;

    for (std::uint32_t c = 0; c < _noComponents; c++)
    {
        voxel[c] = static_cast<std::uint16_t>(_data[startVoxelIndex + c]);
    }
}

void Volume::setVoxel(const std::uint32_t& x, const std::uint32_t& y, const std::uint32_t& z, const std::uint16_t* voxel)
{
    const auto startVoxelIndex = voxelIndex(x, y, z) * _noComponents;

    for (std::uint32_t c = 0; c < _noComponents; c++)
    {
        _data[startVoxelIndex + c] = voxel[c];
    }
}

void Volume::toFloatVector(std::vector<float>& voxels) const
{
    voxels.reserve(noElements());

    for (std::uint32_t i = 0; i < noElements(); i++)
    {
        voxels.push_back(static_cast<float>(_data[i]));
    }
}

QString Volume::volumeFilePath() const
{
    return _volumeFilePath;
}

QString Volume::dimensionName() const
{
    return _dimensionName;
}

void Volume::setDimensionName(const QString& dimensionName)
{
    _dimensionName = dimensionName;
}
