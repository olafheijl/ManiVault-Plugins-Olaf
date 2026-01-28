// SPDX-License-Identifier: LGPL-3.0-or-later 
// A corresponding LICENSE file is located in the root directory of this source tree 
// Copyright (C) 2023 BioVault (Biomedical Visual Analytics Unit LUMC - TU Delft) 

#include "Volumes.h"

#include "VolumeData.h"
#include "InfoAction.h"

#include <util/Exception.h>
#include <util/Timer.h>

#include <DataHierarchyItem.h>
#include <Dataset.h>
#include <LinkedData.h>

#include <PointData/PointData.h>

#include <cmath>
#include <exception>
#include <limits>
#include <numeric>
#include <stdexcept>

#include <QDebug>


using namespace mv::util;
static int GL_MAX_3D_TEXTURE_SIZE = 2048; // 2048 is the minimum value for OpenGL 3D textures, defined here manually for now to avoid including OpenGL headers

Volumes::Volumes(QString dataName, bool mayUnderive /*= false*/, const QString& guid /*= ""*/) :
    DatasetImpl(dataName, mayUnderive, guid),
    _indices(),
    _volumeData(nullptr),
    _infoAction()
{
    _volumeData = getRawData<VolumeData>();

    setLinkedDataFlags(0);
}

void Volumes::init()
{
    DatasetImpl::init();

    _infoAction = QSharedPointer<InfoAction>::create(nullptr, *this);

    addAction(*_infoAction.get());

    // parent data set must be valid and derived from points
    if (!getDataHierarchyItem().getParent()->getDataset<DatasetImpl>().isValid() ||
        getDataHierarchyItem().getParent()->getDataType() != PointType)
        qCritical() << "Volumes: warning: volume data set must be derived from points.";
}

mv::Dataset<Volumes> Volumes::addVolumeDataset(QString datasetGuiName, const mv::Dataset<Points>& parentDataSet)
{
    mv::Dataset<Volumes> volumes = mv::data().createDataset<Volumes>("Volumes", "volumes", parentDataSet);

    return volumes;
}

Dataset<DatasetImpl> Volumes::createSubsetFromSelection(const QString& guiName, const Dataset<DatasetImpl>& parentDataSet /*= Dataset<DatasetImpl>()*/, const bool& visible /*= true*/) const
{
    return mv::data().createSubsetFromSelection(getSelection(), toSmartPointer(), guiName, parentDataSet, visible);
}

Dataset<DatasetImpl> Volumes::copy() const
{
    auto volumes = new Volumes(getRawDataName());

    volumes->setText(text());

    return volumes;
}

Size3D Volumes::getVolumeSize() const
{
    return _volumeData->getVolumeSize();
}

void Volumes::setVolumeSize(const Size3D& volumeSize)
{
    _volumeData->setVolumeSize(volumeSize);
}

std::uint32_t Volumes::getComponentsPerVoxel() const
{
    return _volumeData->getComponentsPerVoxel();
}

void Volumes::setComponentsPerVoxel(const std::uint32_t& componentsPerVoxel)
{
    _volumeData->setComponentsPerVoxel(componentsPerVoxel);
}

QStringList Volumes::getVolumeFilePaths() const
{
    return _volumeData->getVolumeFilePaths();
}

void Volumes::setVolumeFilePaths(const QStringList& volumeFilePaths)
{
    _volumeData->setVolumeFilePaths(volumeFilePaths);
}

std::uint32_t Volumes::getNumberOfVoxels() const
{
    const auto size = getVolumeSize();
    return size.width() * size.height() * size.depth();
}

//QIcon Volumes::getIcon(const QColor& color /*= Qt::black*/) const 
//{
//    return mv::Application::getIconFont("FontAwesome").getIcon("cube", color);
//}

std::vector<std::uint32_t>& Volumes::getSelectionIndices()
{
    return _indices;
}

void Volumes::setSelectionIndices(const std::vector<std::uint32_t>& indices)
{
}

bool Volumes::canSelect() const
{
    return false;
}

bool Volumes::canSelectAll() const
{
    return false;
}

bool Volumes::canSelectNone() const
{
    return false;
}

bool Volumes::canSelectInvert() const
{
    return false;
}

void Volumes::selectAll()
{
}

void Volumes::selectNone()
{
}

void Volumes::selectInvert()
{
}

//void Volumes::getVolumeData(const std::uint32_t& dimensionIndex, QVector<float>& scalarData, QPair<float, float>& scalarDataRange)
//{
//    try
//    {
//        const auto numberOfElementsRequired = getNumberOfVoxels();
//
//        if (static_cast<std::uint32_t>(scalarData.count()) < numberOfElementsRequired)
//            throw std::runtime_error("Scalar data vector number of elements is smaller than the number of voxels");
//
//        getScalarDataForVolumeDimension(dimensionIndex, scalarData, scalarDataRange);
//
//        // Initialize scalar data range
//        scalarDataRange = { std::numeric_limits<float>::max(), std::numeric_limits<float>::lowest() };
//
//        // Compute the actual scalar data range
//        for (auto& scalar : scalarData) {
//            scalarDataRange.first = std::min(scalar, scalarDataRange.first);
//            scalarDataRange.second = std::max(scalar, scalarDataRange.second);
//        }
//    }
//    catch (std::exception& e)
//    {
//        exceptionMessageBox("Unable to get scalar data for the given dimension index", e);
//    }
//    catch (...) {
//        exceptionMessageBox("Unable to get scalar data for the given dimension index");
//    }
//}

void Volumes::getVolumeData(const std::vector<std::uint32_t>& dimensionIndices, std::vector<float>& scalarData, QPair<float, float>& scalarDataRange)
{ 
    try
    {
        const auto numberOfVoxels = static_cast<std::int32_t>(getNumberOfVoxels());
        const auto numberOfElementsRequired = dimensionIndices.size() * getNumberOfVoxels();
        const auto numberOfComponentsPerVoxel = static_cast<std::int32_t>(getComponentsPerVoxel());

        if (static_cast<std::uint32_t>(scalarData.capacity()) < numberOfElementsRequired)
            throw std::runtime_error("Scalar data vector number of elements is smaller than (nDimensions * nVoxels)");

        QVector<float> tempScalarData(numberOfElementsRequired);
        QPair<float, float> tempScalarDataRange(std::numeric_limits<float>::max(), std::numeric_limits<float>::lowest());

        std::int32_t componentIndex = 0;

        for (const auto& dimensionIndex : dimensionIndices) {
            getScalarDataForVolumeDimension(dimensionIndex, tempScalarData, tempScalarDataRange);
            for (std::int32_t voxelIndex = 0; voxelIndex < numberOfVoxels; voxelIndex++)
                scalarData[static_cast<size_t>(voxelIndex * numberOfComponentsPerVoxel) + componentIndex] = tempScalarData[voxelIndex];

            componentIndex++;
        }

        scalarDataRange = { std::numeric_limits<float>::max(), std::numeric_limits<float>::lowest() };

        for (auto& scalar : scalarData) {
            scalarDataRange.first = std::min(scalar, scalarDataRange.first);
            scalarDataRange.second = std::max(scalar, scalarDataRange.second);
        }
    }
    catch (std::exception& e)
    {
        exceptionMessageBox("Unable to get scalar data for the given dimension indices", e);
    }
    catch (...) {
        exceptionMessageBox("Unable to get scalar data for the given dimension indices");
    }
}

mv::Vector3f Volumes::getVolumeAtlasData(const std::vector<std::uint32_t>& dimensionIndices, std::vector<float>& scalarData, QPair<float, float>& scalarDataRange, int textureBlockDimensions /*default value = 4 (RGBA)*/)
{
    qDebug() << "amount of dimensions: " << dimensionIndices.size();
    try
    {
        const std::int32_t numberOfVoxels = getNumberOfVoxels();
        const std::int32_t numberOfElementsRequired = std::ceil(float(dimensionIndices.size()) / float(textureBlockDimensions)) * textureBlockDimensions * getNumberOfVoxels();

        int brickAmount = std::ceil(float(dimensionIndices.size()) / float(textureBlockDimensions));

        // These are currently not intresting, but later on we might add borders to the blocks to avoid interpolation artifacts
        int width = getVolumeSize().width();
        int height = getVolumeSize().height();
        int depth = getVolumeSize().depth();

        // Here we figure out the optimal way we can arrange the atlas bricks such that we minimize the amount of wasted space in the texture
        mv::Vector3f maxDimsInBricks = mv::Vector3f(GL_MAX_3D_TEXTURE_SIZE / width, GL_MAX_3D_TEXTURE_SIZE / height, GL_MAX_3D_TEXTURE_SIZE / depth);
        mv::Vector3f brickLayout = findOptimalDimensions(brickAmount, maxDimsInBricks);


        QVector<float> tempScalarData(numberOfElementsRequired);
        qDebug() << "Size of temp vector: " << tempScalarData.size() << " " << numberOfElementsRequired;
        QPair<float, float> tempScalarDataRange(std::numeric_limits<float>::max(), std::numeric_limits<float>::lowest());

        int trueWidth = width * brickLayout.x;
        int trueHeight = height * brickLayout.y;
        int trueDepth = depth * brickLayout.z;
        
        int trueVolume = trueWidth * trueHeight * trueDepth * textureBlockDimensions;
        if (trueVolume >= scalarData.size()) {
            scalarData.resize(trueVolume);
            qDebug() << "Resized scalar data to: " << trueVolume;
        }

        // Loop over the dimensions and fill the scalar data
        std::int32_t componentIndex = 0;
        for (const auto& dimensionIndex : dimensionIndices) {
            getScalarDataForVolumeDimension(dimensionIndex, tempScalarData, tempScalarDataRange);
            int brickIndex = std::floor(float(componentIndex) / float(textureBlockDimensions));

            int brickX = brickIndex % int(brickLayout.x);
            int brickY = int(std::floor(float(brickIndex) / float(brickLayout.x))) % int(brickLayout.y);
            int brickZ = std::floor(float(brickIndex) / float(brickLayout.x * brickLayout.y));
            for (std::int32_t voxelIndex = 0; voxelIndex < numberOfVoxels; voxelIndex++) {
                mv::Vector3f voxelCoordinate = getVoxelCoordinateFromVoxelIndex(voxelIndex);
                // The voxel coordinate detimenes the position of the voxel in the volume, not accounting for (for example) the rgba format yet inside the brick itself
                std::uint32_t voxelBaseIndex = (std::uint32_t(voxelCoordinate.x + (brickX * width))
                        + std::uint32_t(voxelCoordinate.y + (brickY * height)) * trueWidth
                        + std::uint32_t(voxelCoordinate.z + (brickZ * depth)) * trueWidth * trueHeight)
                    * textureBlockDimensions;

                // Here, we add the offset of the channel inside the brick
                std::uint32_t channelOffset = componentIndex % textureBlockDimensions;
                std::uint32_t valueIndex = voxelBaseIndex + channelOffset;

                float value = tempScalarData[voxelIndex];
                if(valueIndex > scalarData.size())
                    qCritical() << "Index out of bounds: " << valueIndex << " " << scalarData.size();
                scalarData[valueIndex] = value;
            }

            componentIndex++;
        }

        scalarDataRange = { std::numeric_limits<float>::max(), std::numeric_limits<float>::lowest() };

        for (auto& scalar : scalarData) {
            scalarDataRange.first = std::min(scalar, scalarDataRange.first);
            scalarDataRange.second = std::max(scalar, scalarDataRange.second);
        }

        return mv::Vector3f(trueWidth, trueHeight, trueDepth);
    }
    catch (std::exception& e)
    {
        exceptionMessageBox("Unable to get scalar data for the given dimension indices", e);
    }
    catch (...) {
        exceptionMessageBox("Unable to get scalar data for the given dimension indices");
    }
}

// Finds the optimal dimensions for the volume cube when given a certain amount of cubes
mv::Vector3f Volumes::findOptimalDimensions(int N, mv::Vector3f maxDims) {
    int maxX = 1;
    int maxY = 1;
    int maxZ = 1;

    mv::Vector3f bestDims(1, 1, 1);
    int bestVolume = std::numeric_limits<int>::max();
    if (N <= maxDims.x) {
        return mv::Vector3f(N, 1, 1);
    }
    else if (N <= maxDims.x * maxDims.y) {
        maxX = std::min(int(std::ceil(std::sqrt(N))), int(std::floor(maxDims.x)));
        maxY = std::min(int(std::ceil(std::sqrt(N))), int(std::floor(maxDims.y)));
    }
    else if (N <= maxDims.x * maxDims.y * maxDims.z) {
        maxX = std::min(int(std::ceil(std::cbrt(N))), int(std::floor(maxDims.x)));
        maxY = std::min(int(std::ceil(std::cbrt(N))), int(std::floor(maxDims.y)));
        maxZ = std::min(int(std::ceil(std::cbrt(N))), int(std::floor(maxDims.z)));
    }
    else {
        qCritical() << "Volume too large to fit in texture";
    }


    for (int z = 1; z <= maxZ; ++z) {
        float remaining = std::ceil(N / float(z));
        for (int y = 1; y <= maxY; ++y) {
            int x = std::ceil(remaining / float(y));
            if (x <= maxX) {
                int volume = x * y * z;
                if (volume == N) {
                    return mv::Vector3f(x, y, z); // Exact match found
                }
                if (volume >= N && volume < bestVolume) {
                    bestVolume = volume;
                    bestDims = mv::Vector3f(x, y, z); // best match so far
                }
            }
        }
    }

    return bestDims; // Return the closest combination found
}

void Volumes::getScalarDataForVolumeDimension(const std::uint32_t& dimensionIndex, QVector<float>& scalarData, QPair<float, float>& scalarDataRange)
{
    auto parent = getParent();

    if (parent->getDataType() == PointType) {
        auto points = Dataset<Points>(parent);

        std::vector<std::uint32_t> globalIndices;

        points->getGlobalIndices(globalIndices); //TODO this seems very inneficient to call this every time

        points->visitData([this, dimensionIndex, &globalIndices, &scalarData](auto pointData) {
            for (std::uint32_t pointIndex = 0; pointIndex < pointData.size(); pointIndex++) {
                scalarData[globalIndices[pointIndex]] = pointData[pointIndex][dimensionIndex];
            }
        });
    }
    else {
        qCritical() << "Volumes: warning: volume data set must be derived from points."; // This is already checked during creation so this should never happen
    }
}

mv::Vector3f Volumes::getVoxelCoordinateFromVoxelIndex(const std::int32_t& voxelIndex) const
{
    const auto size = getVolumeSize();
    return mv::Vector3f(voxelIndex % size.width(), (voxelIndex / size.width()) % size.height(), voxelIndex / (size.width() * size.height()));
}

std::int32_t Volumes::getVoxelIndexFromVoxelCoordinate(const mv::Vector3f& voxelCoordinate) const
{
    const auto size = getVolumeSize();
    return voxelCoordinate.z * size.width() * size.height() + voxelCoordinate.y * size.width() + voxelCoordinate.x;
}

void Volumes::fromVariantMap(const QVariantMap& variantMap)
{
    DatasetImpl::fromVariantMap(variantMap);

    auto volumeData = getRawData<VolumeData>();

    if (variantMap.contains("VolumeSize")) {
        const auto volumeSize = variantMap["VolumeSize"].toMap();

        setVolumeSize(Size3D(volumeSize["Width"].toInt(), volumeSize["Height"].toInt(), volumeSize["Depth"].toInt()));
    }

    if (variantMap.contains("NumberOfComponentsPerVoxel"))
        setComponentsPerVoxel(variantMap["NumberOfComponentsPerVoxel"].toInt());

    if (variantMap.contains("VolumeFilePaths"))
        setVolumeFilePaths(variantMap["VolumeFilePaths"].toStringList());

    events().notifyDatasetDataChanged(this);
}

QVariantMap Volumes::toVariantMap() const
{
    auto variantMap = DatasetImpl::toVariantMap();


    variantMap["VolumeSize"] = QVariantMap({ { "Width", getVolumeSize().width() }, { "Height", getVolumeSize().height() }, { "Depth", getVolumeSize().depth() } });
    variantMap["NumberOfComponentsPerVoxel"] = getComponentsPerVoxel();
    variantMap["VolumeFilePaths"] = getVolumeFilePaths();

    return variantMap;
}

