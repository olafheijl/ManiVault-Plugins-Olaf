#include "TransferFunctionPlugin.h"

#include "TransferFunctionWidget.h"

#include <Application.h>
#include <DataHierarchyItem.h>

#include <util/PixelSelectionTool.h>
#include <util/Timer.h>

#include <ClusterData/ClusterData.h>
#include <ColorData/ColorData.h>
#include <PointData/PointData.h>

#include <graphics/Vector3f.h>

#include <widgets/DropWidget.h>
#include <widgets/ViewPluginLearningCenterOverlayWidget.h>

#include <actions/PluginTriggerAction.h>

#include <DatasetsMimeData.h>

#include <QAction>
#include <QApplication>
#include <QDebug>
#include <QMenu>
#include <QMetaType>
#include <QtCore>

#include <algorithm>
#include <functional>
#include <vector>
#include <actions/ViewPluginSamplerAction.h>

Q_PLUGIN_METADATA(IID "studio.manivault.TransferFunctionPlugin")

using namespace mv;
using namespace mv::util;

TransferFunctionPlugin::TransferFunctionPlugin(const PluginFactory* factory) :
    ViewPlugin(factory),
    _dropWidget(nullptr),
    _transferFunctionWidget(new TransferFunctionWidget(this)),
    _numPoints(0),
    _settingsAction(this, "Settings"),
	_materialSettings(this, "Material Settings"),
    _primaryToolbarAction(this, "Primary Toolbar")
{
    setObjectName("TransferFunction");

    auto& shortcuts = getShortcuts();

    shortcuts.add({ QKeySequence(Qt::Key_R), "Selection", "Rectangle (default)" });
    shortcuts.add({ QKeySequence(Qt::Key_L), "Selection", "Lasso" });
    shortcuts.add({ QKeySequence(Qt::Key_B), "Selection", "Circular brush (mouse wheel adjusts the radius)" });
    shortcuts.add({ QKeySequence(Qt::SHIFT), "Selection", "Add to selection" });
    shortcuts.add({ QKeySequence(Qt::CTRL), "Selection", "Remove from selection" });

    shortcuts.add({ QKeySequence(Qt::Key_S), "Render", "Scatter mode (default)" });
    
    _dropWidget = new DropWidget(_transferFunctionWidget);

    getWidget().setFocusPolicy(Qt::ClickFocus);

    _primaryToolbarAction.addAction(&_settingsAction.getDatasetsAction());

    _primaryToolbarAction.addAction(&_settingsAction.getSelectionAction());
	_primaryToolbarAction.addAction(&_settingsAction.getPointsAction());

    connect(_transferFunctionWidget, &TransferFunctionWidget::customContextMenuRequested, this, [this](const QPoint& point) {
        if (!_positionDataset.isValid())
            return;

        auto contextMenu = _settingsAction.getContextMenu();

        contextMenu->addSeparator();

        _positionDataset->populateContextMenu(contextMenu);

        contextMenu->exec(getWidget().mapToGlobal(point));
    });

    _dropWidget->setDropIndicatorWidget(new DropWidget::DropIndicatorWidget(&getWidget(), "No data loaded", "Drag an item from the data hierarchy and drop it here to visualize data..."));
    _dropWidget->initialize([this](const QMimeData* mimeData) -> DropWidget::DropRegions {
        DropWidget::DropRegions dropRegions;

        const auto datasetsMimeData = dynamic_cast<const DatasetsMimeData*>(mimeData);

        if (datasetsMimeData == nullptr)
            return dropRegions;

        if (datasetsMimeData->getDatasets().count() > 1)
            return dropRegions;

        const auto& dataset         = datasetsMimeData->getDatasets().first();
        const auto datasetGuiName   = dataset->text();
        const auto datasetId        = dataset->getId();
        const auto dataType         = dataset->getDataType();
        const auto dataTypes        = DataTypes({ PointType , ColorType, ClusterType });

        // Check if the data type can be dropped
        if (!dataTypes.contains(dataType))
            dropRegions << new DropWidget::DropRegion(this, "Incompatible data", "This type of data is not supported", "exclamation-circle", false);

        // Points dataset is about to be dropped
        if (dataType == PointType) {

            // Get points dataset from the core
            auto candidateDataset = mv::data().getDataset<Points>(datasetId);

            // Establish drop region description
            const auto description = QString("Visualize %1 as points or density/contour map").arg(datasetGuiName);

            if (!_positionDataset.isValid()) {

                // Load as point positions when no dataset is currently loaded
                dropRegions << new DropWidget::DropRegion(this, "Point position", description, "map-marker-alt", true, [this, candidateDataset]() {
                    _positionDataset = candidateDataset;
                    });
            }
            else {
                if (_positionDataset != candidateDataset && candidateDataset->getNumDimensions() == 2) {

                    dropRegions << new DropWidget::DropRegion(this, "Point position", description, "map-marker-alt", true, [this, candidateDataset]() {
                        _positionDataset = candidateDataset;
                        });
                }
                else {
					dropRegions << new DropWidget::DropRegion(this, "Incompatible data", "Only 2D point data is supported", "exclamation-circle", false);
                }
            }
        }

        return dropRegions;
    });

    auto& selectionAction = _settingsAction.getSelectionAction();

    /**getSamplerAction().setViewGeneratorFunction([this](const ViewPluginSamplerAction::SampleContext& toolTipContext) -> QString {
        QStringList localPointIndices, globalPointIndices;

        for (const auto& localPointIndex : toolTipContext["LocalPointIndices"].toList())
            localPointIndices << QString::number(localPointIndex.toInt());

        for (const auto& globalPointIndex : toolTipContext["GlobalPointIndices"].toList())
            globalPointIndices << QString::number(globalPointIndex.toInt());

        if (localPointIndices.isEmpty())
            return {};

        return  QString("<table> \
                    <tr> \
                        <td><b>Point ID's: </b></td> \
                        <td>%1</td> \
                    </tr> \
                   </table>").arg(globalPointIndices.join(", "));
    }); */

    //getSamplerAction().setViewingMode(ViewPluginSamplerAction::ViewingMode::Tooltip);
    getSamplerAction().getEnabledAction().setChecked(false);

    getLearningCenterAction().addVideos(QStringList({ "Practitioner", "Developer" }));
}

TransferFunctionPlugin::~TransferFunctionPlugin()
{
}

void TransferFunctionPlugin::init()
{
    auto layout = new QVBoxLayout();

    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(_primaryToolbarAction.createWidget(&getWidget()));
    layout->addWidget(_transferFunctionWidget, 100);

    auto& navigationAction = _transferFunctionWidget->getPointRendererNavigator().getNavigationAction();

    if (auto navigationWidget = navigationAction.createWidget(&getWidget())) {
        layout->addWidget(navigationWidget);
        layout->setAlignment(navigationWidget, Qt::AlignCenter);

        navigationAction.setParent(&_settingsAction);
    }

    getWidget().setLayout(layout);

    addDockingAction(&_materialSettings);

    // Set minimum size for the dock widget
    //getWidget().setMinimumSize(512, 512);

    // Update the data when the scatter plot widget is initialized
    connect(_transferFunctionWidget, &TransferFunctionWidget::initialized, this, &TransferFunctionPlugin::updateVolumeData);

    /*connect(&_transferFunctionWidget->getPixelSelectionTool(), &PixelSelectionTool::areaChanged, [this]() {
        if (_transferFunctionWidget->getPixelSelectionTool().isNotifyDuringSelection()) {
            selectPoints();
        }
        });

    connect(&_transferFunctionWidget->getPixelSelectionTool(), &PixelSelectionTool::ended, [this]() {
        if (_transferFunctionWidget->getPixelSelectionTool().isNotifyDuringSelection())
            return;

        selectPoints();
        });*/

    connect(&_positionDataset, &Dataset<Points>::changed, this, &TransferFunctionPlugin::positionDatasetChanged);
    connect(&_positionDataset, &Dataset<Points>::dataChanged, this, &TransferFunctionPlugin::updateVolumeData);
    connect(&_positionDataset, &Dataset<Points>::dataSelectionChanged, this, &TransferFunctionPlugin::updateSelection);

    connect(&_settingsAction.getPointsAction().getSizeAction(), &DecimalAction::valueChanged, [this](float size) {
		_transferFunctionWidget->setPointSize(size);
        _transferFunctionWidget->update();
    });

	connect(&_settingsAction.getPointsAction().getOpacityAction(), &DecimalAction::valueChanged, [this](float opacity) {
		_transferFunctionWidget->setPointOpacity(opacity);
		_transferFunctionWidget->update();
		});

    _transferFunctionWidget->installEventFilter(this);

    getLearningCenterAction().getViewPluginOverlayWidget()->setTargetWidget(_transferFunctionWidget);

    /*connect(&getTransferFunctionWidget().getPointRendererNavigator().getNavigationAction().getZoomSelectionAction(), &TriggerAction::triggered, this, [this]() -> void {
        if (_selectionBoundaries.isValid())
            _transferFunctionWidget->getPointRendererNavigator().setZoomRectangleWorld(_selectionBoundaries);
        });*/
}

void TransferFunctionPlugin::loadData(const Datasets& datasets)
{
    // Exit if there is nothing to load
    if (datasets.isEmpty())
        return;

    // Load the first dataset
    _positionDataset = datasets.first();
}

void TransferFunctionPlugin::createSubset(const bool& fromSourceData /*= false*/, const QString& name /*= ""*/)
{
    // Create the subset
    mv::Dataset<DatasetImpl> subset;

    // Avoid making a bigger subset than the current data by restricting the selection to the current data
    subset = _positionDataset->createSubsetFromVisibleSelection(name, _positionDataset);

    subset->getDataHierarchyItem().select();
}

//void TransferFunctionPlugin::selectPoints()
//{
//
//    auto& pixelSelectionTool = _transferFunctionWidget->getPixelSelectionTool();
//
//    auto& renderer = _transferFunctionWidget->getPointRenderer();
//    auto& navigator = _transferFunctionWidget->getPointRendererNavigator();
//
//    // Only proceed with a valid points position dataset and when the pixel selection tool is active
//    if (!_positionDataset.isValid()) // || !pixelSelectionTool.isActive() || navigator.isNavigating() || !pixelSelectionTool.isEnabled())
//        return;
//
//    auto selectionAreaImage = pixelSelectionTool.getAreaPixmap().toImage();
//    auto selectionSet = _positionDataset->getSelection<Points>();
//
//    std::vector<std::uint32_t> targetSelectionIndices;
//
//    targetSelectionIndices.reserve(_positionDataset->getNumPoints());
//
//    std::vector<std::uint32_t> localGlobalIndices;
//
//    _positionDataset->getGlobalIndices(localGlobalIndices);
//
//    const auto zoomRectangleWorld = navigator.getZoomRectangleWorld();
//    const auto screenRectangle = QRect(QPoint(), renderer.getRenderSize()); 
//
//    float boundaries[4]{
//        std::numeric_limits<float>::max(),
//        std::numeric_limits<float>::lowest(),
//        std::numeric_limits<float>::max(),
//        std::numeric_limits<float>::lowest()
//    };
//
//    // Go over all points in the dataset to see if they are selected
//    for (std::uint32_t localPointIndex = 0; localPointIndex < _positions.size(); localPointIndex++) {
//        const auto& point = _positions[localPointIndex];
//
//        // Compute the offset of the point in the world space
//        const auto pointOffsetWorld = QPointF(point.x - zoomRectangleWorld.left(), point.y - zoomRectangleWorld.top());
//
//        // Normalize it 
//        const auto pointOffsetWorldNormalized = QPointF(pointOffsetWorld.x() / zoomRectangleWorld.width(), pointOffsetWorld.y() / zoomRectangleWorld.height());
//
//        // Convert it to screen space
//        const auto pointOffsetScreen = QPoint(pointOffsetWorldNormalized.x() * screenRectangle.width(), screenRectangle.height() - pointOffsetWorldNormalized.y() * screenRectangle.height());
//
//        // Continue to next point if the point is outside the screen
//        if (!screenRectangle.contains(pointOffsetScreen))
//            continue;
//
//        // If the corresponding pixel is not transparent, add the point to the selection
//        if (selectionAreaImage.pixelColor(pointOffsetScreen).alpha() > 0) {
//            targetSelectionIndices.push_back(localGlobalIndices[localPointIndex]);
//
//            boundaries[0] = std::min(boundaries[0], point.x);
//            boundaries[1] = std::max(boundaries[1], point.x);
//            boundaries[2] = std::min(boundaries[2], point.y);
//            boundaries[3] = std::max(boundaries[3], point.y);
//        }
//    }
//
//    _selectionBoundaries = QRectF(boundaries[0], boundaries[2], boundaries[1] - boundaries[0], boundaries[3] - boundaries[2]);
//
//    switch (const auto selectionModifier = pixelSelectionTool.isAborted() ? PixelSelectionModifierType::Subtract : pixelSelectionTool.getModifier())
//    {
//    case PixelSelectionModifierType::Replace:
//        break;
//
//    case PixelSelectionModifierType::Add:
//    case PixelSelectionModifierType::Subtract:
//    {
//        // Get reference to the indices of the selection set
//        auto& selectionSetIndices = selectionSet->indices;
//
//        // Create a set from the selection set indices
//        QSet<std::uint32_t> set(selectionSetIndices.begin(), selectionSetIndices.end());
//
//        switch (selectionModifier)
//        {
//            // Add points to the current selection
//        case PixelSelectionModifierType::Add:
//        {
//            // Add indices to the set 
//            for (const auto& targetIndex : targetSelectionIndices)
//                set.insert(targetIndex);
//
//            break;
//        }
//
//        // Remove points from the current selection
//        case PixelSelectionModifierType::Subtract:
//        {
//            // Remove indices from the set 
//            for (const auto& targetIndex : targetSelectionIndices)
//                set.remove(targetIndex);
//
//            break;
//        }
//
//        case PixelSelectionModifierType::Replace:
//            break;
//        }
//
//        targetSelectionIndices = std::vector<std::uint32_t>(set.begin(), set.end());
//
//        break;
//    }
//    }
//
//    auto& navigationAction = navigator.getNavigationAction();
//
//    navigationAction.getZoomSelectionAction().setEnabled(!targetSelectionIndices.empty());
//
//    _positionDataset->setSelectionIndices(targetSelectionIndices);
//
//    events().notifyDatasetDataSelectionChanged(_positionDataset->getSourceDataset<Points>());
//}


Dataset<Points>& TransferFunctionPlugin::getPositionDataset()
{
    return _positionDataset;
}

void TransferFunctionPlugin::positionDatasetChanged()
{
    _dropWidget->setShowDropIndicator(!_positionDataset.isValid());
    _transferFunctionWidget->getPixelSelectionTool().setEnabled(_positionDataset.isValid());

    if (!_positionDataset.isValid())
        return;

    _numPoints = _positionDataset->getNumPoints();

    _transferFunctionWidget->getPointRendererNavigator().resetView(true);
    
    updateVolumeData();
}

TransferFunctionWidget& TransferFunctionPlugin::getTransferFunctionWidget()
{
    return *_transferFunctionWidget;
}

void TransferFunctionPlugin::updateVolumeData()
{
    // Check if the scatter plot is initialized, if not, don't do anything
    if (!_transferFunctionWidget->isInitialized())
        return;

    // If no dataset has been selected, don't do anything
    if (_positionDataset.isValid()) {

        // Determine number of points depending on if its a full dataset or a subset
        _numPoints = _positionDataset->getNumPoints();

        // Extract 2-dimensional points from the data set based on the selected dimensions
        _positionDataset->extractDataForDimensions(_positions, 0, 1);

        // Pass the 2D points to the scatter plot widget
        _transferFunctionWidget->setData(&_positions);

        updateSelection();
    }
    else {
        _numPoints = 0;
        _positions.clear();
        _transferFunctionWidget->setData(&_positions);
    }
}

void TransferFunctionPlugin::updateSelection()
{
    if (!_positionDataset.isValid())
        return;

    //Timer timer(__FUNCTION__);

    auto selection = _positionDataset->getSelection<Points>();

    std::vector<bool> selected;
    std::vector<char> highlights;

    _positionDataset->selectedLocalIndices(selection->indices, selected);

    highlights.resize(_positionDataset->getNumPoints(), 0);

    for (std::size_t i = 0; i < selected.size(); i++)
        highlights[i] = selected[i] ? 1 : 0;

    _transferFunctionWidget->setHighlights(highlights, static_cast<std::int32_t>(selection->indices.size()));

    if (getSamplerAction().getSamplingMode() == ViewPluginSamplerAction::SamplingMode::Selection) {
        std::vector<std::uint32_t> localGlobalIndices;

        _positionDataset->getGlobalIndices(localGlobalIndices);

        std::vector<std::uint32_t> sampledPoints;

        sampledPoints.reserve(_positions.size());

        for (auto selectionIndex : selection->indices)
            sampledPoints.push_back(selectionIndex);

        std::int32_t numberOfPoints = 0;

        QVariantList localPointIndices, globalPointIndices;

        const auto numberOfSelectedPoints = selection->indices.size();

        localPointIndices.reserve(static_cast<std::int32_t>(numberOfSelectedPoints));
        globalPointIndices.reserve(static_cast<std::int32_t>(numberOfSelectedPoints));

        for (const auto& sampledPoint : sampledPoints) {
            if (getSamplerAction().getRestrictNumberOfElementsAction().isChecked() && numberOfPoints >= getSamplerAction().getMaximumNumberOfElementsAction().getValue())
                break;

            const auto& localPointIndex = sampledPoint;
            const auto& globalPointIndex = localGlobalIndices[localPointIndex];

            localPointIndices << localPointIndex;
            globalPointIndices << globalPointIndex;

            numberOfPoints++;
        }

        _transferFunctionWidget->update();

        getSamplerAction().setSampleContext({
            { "PositionDatasetID", _positionDataset.getDatasetId() },
            { "LocalPointIndices", localPointIndices },
            { "GlobalPointIndices", globalPointIndices },
            { "Distances", QVariantList()}
		});
    }
}

void TransferFunctionPlugin::fromVariantMap(const QVariantMap& variantMap)
{
    ViewPlugin::fromVariantMap(variantMap);

    variantMapMustContain(variantMap, "Settings");

    _primaryToolbarAction.fromParentVariantMap(variantMap);
    _settingsAction.fromParentVariantMap(variantMap);

    auto& pointRenderer = const_cast<PointRenderer&>(_transferFunctionWidget->getPointRenderer());

    if (pointRenderer.getNavigator().getNavigationAction().getSerializationCountFrom() == 0) {
        qDebug() << "Resetting view";
        pointRenderer.getNavigator().resetView(true);

        _transferFunctionWidget->update();
    }
}

QVariantMap TransferFunctionPlugin::toVariantMap() const
{
    QVariantMap variantMap = ViewPlugin::toVariantMap();

    _primaryToolbarAction.insertIntoVariantMap(variantMap);
    _settingsAction.insertIntoVariantMap(variantMap);

    return variantMap;
}

std::uint32_t TransferFunctionPlugin::getNumberOfPoints() const
{
    if (!_positionDataset.isValid())
        return 0;

    return _positionDataset->getNumPoints();
}

TransferFunctionPluginFactory::TransferFunctionPluginFactory()
{
}

/**QIcon TransferFunctionPluginFactory::getIcon(const QColor& color /*= Qt::black*) const
{
    return Application::getIconFont("FontAwesome").getIcon("braille", color);
}*/

ViewPlugin* TransferFunctionPluginFactory::produce()
{
    return new TransferFunctionPlugin(this);
}

PluginTriggerActions TransferFunctionPluginFactory::getPluginTriggerActions(const mv::Datasets& datasets) const
{
    PluginTriggerActions pluginTriggerActions;

    const auto getInstance = [this]() -> TransferFunctionPlugin* {
        return dynamic_cast<TransferFunctionPlugin*>(Application::core()->getPluginManager().requestViewPlugin(getKind()));
    };

    const auto numberOfDatasets = datasets.count();

    if (PluginFactory::areAllDatasetsOfTheSameType(datasets, PointType)) {
        //auto& fontAwesome = Application::getIconFont("FontAwesome");

        if (numberOfDatasets >= 1) {
            auto pluginTriggerAction = new PluginTriggerAction(const_cast<TransferFunctionPluginFactory*>(this), this, "TransferFunction", "View selected datasets side-by-side in separate scatter plot viewers", Application::windowIcon(), [this, getInstance, datasets](PluginTriggerAction& pluginTriggerAction) -> void {
                for (const auto& dataset : datasets)
                    getInstance()->loadData(Datasets({ dataset }));
            });

            pluginTriggerActions << pluginTriggerAction;
        }
    }

    return pluginTriggerActions;
}

QUrl TransferFunctionPluginFactory::getRepositoryUrl() const
{
    return QUrl("https://github.com/ManiVaultStudio/TransferFunction");
}
