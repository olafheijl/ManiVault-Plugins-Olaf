// SPDX-License-Identifier: LGPL-3.0-or-later 
// A corresponding LICENSE file is located in the root directory of this source tree 
// Copyright (C) 2023 BioVault (Biomedical Visual Analytics Unit LUMC - TU Delft) 

#include "InfoAction.h"
#include "Application.h"
#include "event/Event.h"

using namespace mv;
using namespace mv::gui;

InfoAction::InfoAction(QObject* parent, Volumes& volumes) :
    GroupAction(parent, "Group", true),
    _volumes(&volumes),
    _typeAction(this, "Volume collection type"),
    _volumeResolutionAction(this, "Volume resolution"),
    _numberOfVoxelsAction(this, "Number of voxels per volume"),
    _componentsPerVoxelAction(this, "Number of components per voxel")
{
    setText("Info");

    addAction(&_typeAction);
    addAction(&_volumeResolutionAction);
    addAction(&_numberOfVoxelsAction);
    addAction(&_componentsPerVoxelAction);

    _typeAction.setEnabled(false);
    _volumeResolutionAction.setEnabled(false);
    _numberOfVoxelsAction.setEnabled(false);
    _componentsPerVoxelAction.setEnabled(false);

    const auto sizeToString = [](const Size3D& size) -> QString {
        return QString("[%1, %2, %3]").arg(QString::number(size.width()), QString::number(size.height()), QString::number(size.depth()));
        };

    const auto updateActions = [this, sizeToString]() -> void {
        if (!_volumes.isValid())
            return;

        const auto volumeSize = _volumes->getVolumeSize();

        _volumeResolutionAction.setString(sizeToString(volumeSize));
        _numberOfVoxelsAction.setString(QString::number(_volumes->getNumberOfVoxels()));
        _componentsPerVoxelAction.setString(QString::number(_volumes->getComponentsPerVoxel()));
        };

        _eventListener.addSupportedEventType(static_cast<std::uint32_t>(EventType::DatasetAdded));
        _eventListener.addSupportedEventType(static_cast<std::uint32_t>(EventType::DatasetDataChanged));
        _eventListener.addSupportedEventType(static_cast<std::uint32_t>(EventType::DatasetDataSelectionChanged));
        _eventListener.registerDataEventByType(VolumeType, [this, updateActions](mv::DatasetEvent* dataEvent) {
        if (!_volumes.isValid())
            return;

        if (dataEvent->getDataset() != _volumes)
            return;

        switch (dataEvent->getType()) {
        case EventType::DatasetAdded:
        case EventType::DatasetDataChanged:
        case EventType::DatasetDataSelectionChanged:
        {
            updateActions();
            break;
        }

        default:
            break;
        }
        });
}

