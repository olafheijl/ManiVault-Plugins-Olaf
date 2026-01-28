// SPDX-License-Identifier: LGPL-3.0-or-later 
// A corresponding LICENSE file is located in the root directory of this source tree 
// Copyright (C) 2023 BioVault (Biomedical Visual Analytics Unit LUMC - TU Delft) 

#pragma once

#include "VolumeDataPlugin_export.h"
#include "Size3D.h"

#include <QString>
#include <QDebug>
#include <QImage>

#include <vector>

class VOLUMEDATAPLUGIN_EXPORT Volume
{
public:
    Volume(const Size3D& size, const std::uint32_t& noComponents, const QString& volumeFilePath);

    std::uint16_t* data();
    Size3D size() const;
    void setSize(const Size3D& size);
    std::uint32_t width() const;
    std::uint32_t height() const;
    std::uint32_t depth() const;
    std::uint32_t noComponents() const;
    QString volumeFilePath() const;
    QString dimensionName() const;
    void setDimensionName(const QString& dimensionName);

    std::uint32_t noVoxels() const;
    std::uint32_t noElements() const;
    std::uint32_t voxelIndex(const std::uint32_t& x, const std::uint32_t& y, const std::uint32_t& z) const;

    void getVoxel(const std::uint32_t& x, const std::uint32_t& y, const std::uint32_t& z, std::uint16_t* voxel) const;
    void setVoxel(const std::uint32_t& x, const std::uint32_t& y, const std::uint32_t& z, const std::uint16_t* voxel);

    void toFloatVector(std::vector<float>& voxels) const;

private:
    std::vector<std::uint16_t>	_data;
    Size3D                      _size;
    std::uint32_t				_noComponents;
    QString						_volumeFilePath;
    QString						_dimensionName;
};

inline QDebug operator<<(QDebug dbg, Volume& volume)
{
    dbg << QString("Volume %1x%2x%3, %4 components").arg(QString::number(volume.width()), QString::number(volume.height()), QString::number(volume.depth()), QString::number(volume.noComponents()));

    return dbg;
}
