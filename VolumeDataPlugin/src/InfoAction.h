// SPDX-License-Identifier: LGPL-3.0-or-later 
// A corresponding LICENSE file is located in the root directory of this source tree 
// Copyright (C) 2023 BioVault (Biomedical Visual Analytics Unit LUMC - TU Delft) 

#pragma once

#include "actions/StringAction.h"
#include "event/EventListener.h"

#include "Volumes.h"

using namespace mv;
using namespace mv::gui;
using namespace mv::util;

/**
 * Info action class
 *
 * Action class for displaying basic volume info
 */
class InfoAction : public GroupAction
{
    Q_OBJECT

public:

    /**
     * Constructor
     * @param parent Pointer to parent object
     * @param volumes Reference to volumes dataset
     */
    InfoAction(QObject* parent, Volumes& volumes);

protected:
    Dataset<Volumes>        _volumes;                            /** Volumes dataset reference */
    StringAction            _typeAction;                         /** Volume collection type action */
    StringAction            _volumeResolutionAction;             /** Volume resolution action */
    StringAction            _numberOfVoxelsAction;               /** Number of voxels per volume action */
    StringAction            _componentsPerVoxelAction;           /** Number of components per voxel action */
    mv::EventListener       _eventListener;                      /** Listen to ManiniVault events */
};

