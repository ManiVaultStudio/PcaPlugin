#include "PcaPlugin.h"

#include "PointData.h"
#include "PCA.h"

#include <QtCore>
#include <QDebug>

Q_PLUGIN_METADATA(IID "nl.BioVault.PCAPlugin")

using namespace hdps;
using namespace hdps::plugin;


PCAPlugin::PCAPlugin(const PluginFactory* factory) :
    AnalysisPlugin(factory),
    _settingsAction()
{
}

void PCAPlugin::init()
{
    // Create example output dataset (a points dataset which is derived from the input points dataset) and set the output dataset
    setOutputDataset(_core->createDerivedDataset("Output Data", getInputDataset()));

    // Retrieve the input dataset for our specific data type (in our case points)
    // The HDPS core sets the input dataset reference when the plugin is created
    const auto inputDataset = getInputDataset<Points>();

    // Set maximum number of PCA components in GUI
    _settingsAction.getNumberOfComponents().setMaximum(inputDataset->getNumDimensions());

    // Retrieve the output dataset for our specific data type (in our case points)
    auto outputDataset = getOutputDataset<Points>();

    // Inject the settings action in the output points dataset 
    // By doing so, the settings user interface will be accessible though the data properties widget
    outputDataset->addAction(_settingsAction);
    
    // Add input dataset to the dimension selection action
    _settingsAction.getDimensionSelectionAction().getPickerAction().setPointsDataset(inputDataset);

    // Automatically focus on the PCA action
    outputDataset->getDataHierarchyItem().select();

    // Start the analysis when the user clicks the start analysis push button
    connect(&_settingsAction.getStartAnalysisAction(), &hdps::gui::TriggerAction::triggered, this, [&]() {
        computePCA();
    });

    // Register for points datasets events using a custom callback function
    registerDataEventByType(PointType, std::bind(&PCAPlugin::onDataEvent, this, std::placeholders::_1));
}

void PCAPlugin::onDataEvent(hdps::DataEvent* dataEvent)
{

    if (dataEvent->getType() == EventType::DataChanged)
    {
        if (dataEvent->getDataset() == getInputDataset())
            _settingsAction.getDimensionSelectionAction().getPickerAction().setPointsDataset(dataEvent->getDataset<Points>());
    }

}

void PCAPlugin::computePCA()
{
    // Disable actions during analysis
    _settingsAction.getStartAnalysisAction().setEnabled(false);

    // Set the task name as it will appear in the data hierarchy viewer
    setTaskName("PCA");

    // In order to report progress the task status has to be set to running
    setTaskRunning();

    // Zero progress at the start
    setTaskProgress(0.0f);

    // Get data 
    std::vector<float> data;
    std::vector<unsigned int> dimensionIndices;
    getDataFromCore(data, dimensionIndices);
    size_t num_comps = _settingsAction.getNumberOfComponents().getValue();

    // Do computation
    setTaskDescription("Computing...");
    std::vector<float> PCA;
    math::pca(data, /* numer of dimension = */ dimensionIndices.size(), /* transformed PCA data = */ PCA, /* number of pca components = */ num_comps);

    // Publish pca to core
    setPCADataInCore(PCA, num_comps);

    // Flag the analysis task as finished
    setTaskFinished();

    // Enabled action again
    _settingsAction.getStartAnalysisAction().setEnabled(true);
}

void PCAPlugin::getDataFromCore(std::vector<float>& data, std::vector<unsigned int>& dimensionIndices)
{
    auto inputPoints = getInputDataset<Points>();

    // Extract the enabled dimensions from the data
    std::vector<bool> enabledDimensions = _settingsAction.getDimensionSelectionAction().getPickerAction().getEnabledDimensions();

    const auto numEnabledDimensions = count_if(enabledDimensions.begin(), enabledDimensions.end(), [](bool b) { return b; });

    data.resize((inputPoints->isFull() ? inputPoints->getNumPoints() : inputPoints->indices.size()) * numEnabledDimensions);

    for (int i = 0; i < inputPoints->getNumDimensions(); i++)
        if (enabledDimensions[i])
            dimensionIndices.push_back(i);

    inputPoints->populateDataForDimensions<std::vector<float>, std::vector<unsigned int>>(data, dimensionIndices);
}

void PCAPlugin::setPCADataInCore(std::vector<float>& data, size_t num_components)
{
    auto outputDataset = getOutputDataset<Points>();

    outputDataset->setData(data, num_components);

    _core->notifyDatasetChanged(outputDataset);
}

AnalysisPlugin* PCAPluginFactory::produce()
{
    // Return a new instance of the example analysis plugin
    return new PCAPlugin(this);
}

hdps::DataTypes PCAPluginFactory::supportedDataTypes() const
{
    DataTypes supportedTypes;

    // This example analysis plugin is compatible with points datasets
    supportedTypes.append(PointType);

    return supportedTypes;
}
