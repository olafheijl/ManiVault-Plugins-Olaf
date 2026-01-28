// SPDX-License-Identifier: LGPL-3.0-or-later 
// A corresponding LICENSE file is located in the root directory of this source tree 
// Copyright (C) 2023 BioVault (Biomedical Visual Analytics Unit LUMC - TU Delft) 

#pragma once

#include "VolumeDataPlugin_export.h"
#include "Volume.h"
#include "VolumeData.h"

#include <Set.h>

#include <QColor>
#include <QRect>
#include <QString>

#include <tuple>
#include <vector>
#include "graphics/Vector3f.h"

using namespace mv::plugin;

class InfoAction;

/**
 * Volumes dataset class
 * Class for dealing with high-dimensional volume data
 */
class VOLUMEDATAPLUGIN_EXPORT Volumes : public mv::DatasetImpl
{
public: // Construction

    /**
     * Constructor
     * @param dataName Name of the dataset
     * @param mayUnderive Whether the dataset may be un-derived, if not it should always co-exist with its source
     * @param guid Globally unique dataset identifier (use only for deserialization)
     */
    Volumes(QString dataName, bool mayUnderive = false, const QString& guid = "");

    /** Initializes the dataset */
    void init() override;

    /**
     * Recieves a dataset and derives a volume dataset from it:
     *
     *      auto [pointDataset, volumeDataset] = Volumes::addVolumeDataset("My Volume", parent);
     *
     * This function sends the core event notifications "notifyDatasetAdded" for both pointDataset and volumeDataset
     *
     * @param datasetGuiName Name of the added dataset in the GUI
     * @param parentDataSet Smart pointer to the parent dataset in the data hierarchy
     */
    static mv::Dataset<Volumes> addVolumeDataset(QString datasetGuiName, const mv::Dataset<Points>& parentDataSet);

public: // Subsets

    /**
     * Create subset from the current selection and specify where the subset will be placed in the data hierarchy
     * @param guiName Name of the subset in the GUI
     * @param parentDataSet Smart pointer to parent dataset in the data hierarchy (default is below the set)
     * @param visible Whether the subset will be visible in the UI
     * @return Smart pointer to the created subset
     */
    mv::Dataset<mv::DatasetImpl> createSubsetFromSelection(const QString& guiName, const mv::Dataset<mv::DatasetImpl>& parentDataSet = mv::Dataset<mv::DatasetImpl>(), const bool& visible = true) const override;


public: // Volume retrieval functions

    /** Obtain a copy of this dataset */
    mv::Dataset<mv::DatasetImpl> copy() const override;

    /** Gets the size of the volumes */
    Size3D getVolumeSize() const;

    /**
     * Set the volume size
     * @param volumeSize Size of the volume(s)
     */
    void setVolumeSize(const Size3D& volumeSize);

    /** Gets the number of values per voxel */
    std::uint32_t getComponentsPerVoxel() const;

    /**
     * Sets the number of values per voxel
     * @param ValuesPerVoxel Number of values per voxel
     */
    void setComponentsPerVoxel(const std::uint32_t& numberOfComponentsPerVoxel);


    /** Get the volume file paths */
    QStringList getVolumeFilePaths() const;

    /**
     * Sets the volume file paths
     * @param volumeFilePaths Volume file paths
     */
    void setVolumeFilePaths(const QStringList& volumeFilePaths);

    /** Returns the number of voxels in total */
    std::uint32_t getNumberOfVoxels() const;

    /**
     * Get plugin icon
     * @param color Icon color for flat (font) icons
     * @return Icon
     */
    QIcon getIcon(const QColor& color = Qt::black) const;
public: // Selection

    /**
     * Get selection indices
     * @return Selection indices
     */
    std::vector<std::uint32_t>& getSelectionIndices() override;

    /**
     * Select by indices
     * @param indices Selection indices
     */
    void setSelectionIndices(const std::vector<std::uint32_t>& indices) override;

    /** Determines whether items can be selected */
    bool canSelect() const override;

    /** Determines whether all items can be selected */
    bool canSelectAll() const override;

    /** Determines whether there are any items which can be deselected */
    bool canSelectNone() const override;

    /** Determines whether the item selection can be inverted (more than one) */
    bool canSelectInvert() const override;

    /** Select all items */
    void selectAll() override;

    /** Deselect all items */
    void selectNone() override;

    /** Invert item selection */
    void selectInvert() override;


public:

    /**
     * Get scalar volume data for dimensionIndices, populates scalarData as if populating a n-dimensional texture according to the specified type and establishes the scalarDataRange
     * @param dimensionIndices Dimension indices to retrieve the scalar data for
     * @param scalarData Scalar data for the specified dimension (assumes enough elements are allocated by the caller)
     * @param scalarDataRange Scalar data range
     * @param returnType Return format
     */
    void getVolumeData(const std::vector<std::uint32_t>& dimensionIndices, std::vector<float>& scalarData, QPair<float, float>& scalarDataRange);

    /**
     * Get scalar volume data for dimensionIndices, populates scalarData as if populating a n-dimensional texture according to the specified type and establishes the scalarDataRange
     * @param dimensionIndices Dimension indices to retrieve the scalar data for
     * @param scalarData Scalar data for the specified dimension (assumes enough elements are allocated by the caller)
     * @param scalarDataRange Scalar data range
     * @param textureBlockDimensions Texture block dimensions per voxel (default value = 4 (RGBA))
     * returns the dimensions of the volume atlas
     */
    mv::Vector3f getVolumeAtlasData(const std::vector<std::uint32_t>& dimensionIndices, std::vector<float>& scalarData, QPair<float, float>& scalarDataRange, int textureBlockDimensions = 4);

protected:

    mv::Vector3f findOptimalDimensions(int N, mv::Vector3f maxDims);

    /**
    * Get scalar data for a single volume dimension
    * @param dimensionIndex Dimension index to retrieve the scalar data for
    * @param scalarData Scalar data for the specified dimension (assumes enough elements are allocated by the caller)
    * @param scalarDataRange Scalar data range
    */
    void getScalarDataForVolumeDimension(const std::uint32_t& dimensionIndex, QVector<float>& scalarData, QPair<float, float>& scalarDataRange);

    /**
     * Get voxel coordinate from voxel index, doesn't take into account valueDimensions
     * @param voxelIndex Voxel index
     */
    mv::Vector3f getVoxelCoordinateFromVoxelIndex(const std::int32_t& voxelIndex) const;

    /**
     * Get voxel index from voxel coordinate, doesn't take into account valueDimensions
     * @param voxelCoordinate Voxel coordinate (0-1 range)
     * @return Voxel index
     */
    std::int32_t getVoxelIndexFromVoxelCoordinate(const mv::Vector3f& voxelCoordinate) const;

public: // Serialization

    /**
     * Load widget action from variant
     * @param Variant representation of the widget action
     */
    void fromVariantMap(const QVariantMap& variantMap) override;

    /**
     * Save widget action to variant
     * @return Variant representation of the widget action
     */
    QVariantMap toVariantMap() const override;


private:
    std::vector<std::uint32_t>      _indices;               /** Selection indices */
    VolumeData*                     _volumeData;            /** Pointer to raw volume data */
    QSharedPointer<InfoAction>      _infoAction;            /** Shared pointer to info action */
};

