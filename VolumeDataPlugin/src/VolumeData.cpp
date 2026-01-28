// SPDX-License-Identifier: LGPL-3.0-or-later 
// A corresponding LICENSE file is located in the root directory of this source tree 
// Copyright (C) 2023 BioVault (Biomedical Visual Analytics Unit LUMC - TU Delft) 

#include "VolumeData.h"
#include "Volumes.h"
#include "Application.h"

#include "DataHierarchyItem.h"
#include "PointData/PointData.h"

#include "util/Exception.h"

#include <QDebug>

#include <stdexcept>

Q_PLUGIN_METADATA(IID "studio.manivault.VolumeData")

using namespace mv;
using namespace mv::util;

VolumeData::VolumeData(const mv::plugin::PluginFactory* factory) :
    mv::plugin::RawData(factory, VolumeType),
    _volumeSize(),
    _componentsPerVoxel(0),
    _volumeFilePaths(),
    _dimensionNames()
{
}

void VolumeData::init()
{
}

Size3D VolumeData::getVolumeSize() const
{
    return _volumeSize;
}

void VolumeData::setVolumeSize(const Size3D& volumeSize)
{
    _volumeSize = volumeSize;
}

std::uint32_t VolumeData::getComponentsPerVoxel() const
{
    return _componentsPerVoxel;
}

void VolumeData::setComponentsPerVoxel(const std::uint32_t& componentsPerVoxel)
{
    _componentsPerVoxel = componentsPerVoxel;
}

QStringList VolumeData::getVolumeFilePaths() const
{
    return _volumeFilePaths;
}

void VolumeData::setVolumeFilePaths(const QStringList& volumeFilePaths)
{
    _volumeFilePaths = volumeFilePaths;
}

Dataset<DatasetImpl> VolumeData::createDataSet(const QString& guid /*= ""*/) const
{
    return Dataset<DatasetImpl>(new Volumes(getName(), false, guid));
}

mv::plugin::RawData* VolumeDataFactory::produce()
{
    return new VolumeData(this);
}

// =============================================================================
// Extra features for the VolumeData plugin that have not been updated for volume data TODO
// =============================================================================
//QIcon VolumeDataFactory::getIcon(const QColor& color /*= Qt::black*/) const 
//{
//    return mv::Application::getIconFont("FontAwesome").getIcon("cube", color);
//}

QUrl VolumeDataFactory::getReadmeMarkdownUrl() const
{
#ifdef ON_LEARNING_CENTER_FEATURE_BRANCH
    return QUrl("");
#else
    return QUrl("");
#endif
}

QUrl VolumeDataFactory::getRepositoryUrl() const
{
    return QUrl("https://github.com/ManiVaultStudio/core");
}
