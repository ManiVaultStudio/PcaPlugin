#include "PcaPlugin.h"

#include "PointData.h"
#include "PCA.h"

#include <actions/PluginTriggerAction.h>

#include <QtCore>
#include <QDebug>

Q_PLUGIN_METADATA(IID "nl.BioVault.PCAPlugin")

using namespace hdps;
using namespace hdps::plugin;

/// ////////////////// ///
/// SETTING CONVERSION ///
/// ////////////////// ///

math::PCA_ALG getPcaAlgorithm(size_t index) {
    math::PCA_ALG alg = math::PCA_ALG::COV;

    switch (index)
    {
    case 0:
        alg = math::PCA_ALG::COV;
        break;
    case 1:
        alg = math::PCA_ALG::SVD;
        break;
    }

    return alg;
}

std::ostream& operator<<(std::ostream& o, math::PCA_ALG alg)
{

    if(alg == math::PCA_ALG::COV)
        return o << "COV";;
    if (alg == math::PCA_ALG::SVD)
        return o << "SVD";;

    return o;
}

math::DATA_NORM getDataNorm(size_t index) {
    math::DATA_NORM norm = math::DATA_NORM::NONE;

    switch (index)
    {
    case 0:
        norm = math::DATA_NORM::NONE;
        break;
    case 1:
        norm = math::DATA_NORM::MEAN;
        break;
    case 2:
        norm = math::DATA_NORM::MINMAX;
        break;
    }

    return norm;

}

std::ostream& operator<<(std::ostream& o, math::DATA_NORM norm)
{

    if (norm == math::DATA_NORM::NONE)
        return o << "NONE";;
    if (norm == math::DATA_NORM::MEAN)
        return o << "MEAN";;
    if (norm == math::DATA_NORM::MINMAX)
        return o << "MINMAX";;

    return o;
}


/// ////////// ///
/// PCA WORKER ///
/// ////////// ///
PCAWorker::PCAWorker(std::shared_ptr<std::vector<float>> data, size_t num_dims, size_t num_comps, math::PCA_ALG algorithm, math::DATA_NORM norm, bool std_orient) :
    _data(data), 
    _num_dims(num_dims),
    _num_comps(num_comps), 
    _std_orient(std_orient),
    _algorithm(algorithm),
    _norm(norm)
{
}

void PCAWorker::compute() {
    int32_t pca_status = 0;
    utils::timer([&]() {
        pca_status = math::pca(*_data, /* number of dimension = */ _num_dims, /* transformed PCA data = */ _pca_out, /* number of pca components = */ _num_comps,
                                       /* pca algorithm = */ _algorithm, /* data normalization = */ _norm, /* stdOrientation = */ _std_orient);
        },
        "PCA computation time (ms)");

    emit resultReady(pca_status);
}


/// ////// ///
/// PLUGIN ///
/// ////// ///

PCAPlugin::PCAPlugin(const PluginFactory* factory) :
    AnalysisPlugin(factory),
    _settingsAction(this),
    _dimensionSelectionAction(this),
    _pcaWorker(nullptr),
    _workerThread()
{
}

void PCAPlugin::init()
{
    // Create example output dataset (a points dataset which is derived from the input points dataset) and set the output dataset
    setOutputDataset(_core->createDerivedDataset("PCA", getInputDataset(), getInputDataset()));

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
    outputDataset->addAction(_dimensionSelectionAction);
    
    // Add input dataset to the dimension selection action
    _dimensionSelectionAction.getPickerAction().setPointsDataset(inputDataset);

    // Automatically focus on the PCA action
    outputDataset->getDataHierarchyItem().select();

    const auto numPoints = inputDataset->getNumPoints();

    // Set initial data (default 2 dimensions, all points at (0,0) )
    std::vector<float> initialData;
    const auto numInitialDataDimensions = 2;
    initialData.resize(numPoints * numInitialDataDimensions);
    outputDataset->setData(initialData.data(), inputDataset->getNumPoints(), numInitialDataDimensions);
    events().notifyDatasetChanged(outputDataset);

    if (numPoints > static_cast<uint32_t>(std::numeric_limits<int32_t>::max()))
    {
        std::cerr << "PCA: can only handle data with up to std::numeric_limits<uint32_t>::max() points" << std::endl;
        _settingsAction.getStartAnalysisAction().setDisabled(true);
    }

    // Start the analysis when the user clicks the start analysis push button
    connect(&_settingsAction.getStartAnalysisAction(), &hdps::gui::TriggerAction::triggered, this, &PCAPlugin::computePCA);

    // Publish a copy of the output data set
    connect(&_settingsAction.getPublishNewDataAction(), &hdps::gui::TriggerAction::triggered, this, &PCAPlugin::publishCopy);

    // Update dimension selection with new data
    connect(&inputDataset, &Dataset<Points>::dataChanged, this, [this, inputDataset]() {
        _dimensionSelectionAction.getPickerAction().setPointsDataset(inputDataset);
        });
}

void PCAPlugin::computePCA()
{
    std::cout << "PCA Plugin: Setting up..." << std::endl;

    if (_pcaWorker)
        _pcaWorker->deleteLater();

    // Disable actions during analysis
    _settingsAction.getStartAnalysisAction().setEnabled(false);

    // Set the task name and description
    setTaskName("PCA");
    setTaskRunning();
    setTaskDescription("Computing...");

    // Get data 
    std::vector<float> data;
    std::vector<unsigned int> dimensionIndices;
    getDataFromCore(getInputDataset<Points>(), data, dimensionIndices);
    size_t num_comps = _settingsAction.getNumberOfComponents().getValue();

    // Get settings
    math::PCA_ALG alg = getPcaAlgorithm(_settingsAction.getPcaAlgorithmAction().getCurrentIndex());
    math::DATA_NORM norm = getDataNorm(_settingsAction.getDataNormAction().getCurrentIndex());
    bool stdOrientation = _settingsAction.getStdAxisOrientation().isChecked();

    // Compute in different thread
    _pcaWorker = new PCAWorker(std::make_shared<std::vector<float>>(data), dimensionIndices.size(), num_comps, alg, norm, stdOrientation);
    _pcaWorker->moveToThread(&_workerThread);

    // setup pca computation 
    connect(this, &PCAPlugin::startPCA, _pcaWorker, &PCAWorker::compute);               

    // get results from PCA
    connect(_pcaWorker, &PCAWorker::resultReady, this, [&](int32_t pca_status) {
        auto [pca_out, num_comps] = _pcaWorker->getRestuls();

        // Publish pca to core
        setPCADataInCore(getOutputDataset<Points>(), pca_out, num_comps);

        // Flag the analysis task as finished
        if (pca_status == EXIT_SUCCESS)
            setTaskFinished();
        else
        {
            setTaskAborted();
            setTaskDescription("Computation failed");
        }

        _pcaWorker->deleteLater();

        // Enabled action again
        _settingsAction.getStartAnalysisAction().setEnabled(true);

        // Enable users to copy the output to a new data set entry
        _settingsAction.getPublishNewDataAction().setEnabled(true);

        std::cout << "PCA Plugin: Finished." << std::endl;
        });

    std::cout << "PCA Plugin: Starting computing PCA transformation with " << num_comps << " components (settings: alg " << alg << ", norm " << norm << ")" << std::endl;

    // start thread and worker
    _workerThread.start();
    emit startPCA();
}

void PCAPlugin::getDataFromCore(const hdps::Dataset<Points> coreDataset, std::vector<float>& data, std::vector<unsigned int>& dimensionIndices)
{
    // Extract the enabled dimensions from the data
    std::vector<bool> enabledDimensions = _dimensionSelectionAction.getPickerAction().getEnabledDimensions();
    const auto numEnabledDimensions = count_if(enabledDimensions.begin(), enabledDimensions.end(), [](bool b) { return b; });

    // resize outout data
    data.resize((coreDataset->isFull() ? coreDataset->getNumPoints() : coreDataset->indices.size()) * numEnabledDimensions);

    // populate dimensionIndices
    for (uint32_t i = 0; i < coreDataset->getNumDimensions(); i++)
        if (enabledDimensions[i])
            dimensionIndices.push_back(i);

    // populate data
    coreDataset->populateDataForDimensions<std::vector<float>, std::vector<unsigned int>>(data, dimensionIndices);
}

void PCAPlugin::setPCADataInCore(hdps::Dataset<Points> coreDataset, const std::vector<float>& data, size_t num_components)
{
    coreDataset->setData(data.data(), getInputDataset<Points>()->getNumPoints(), num_components);
    events().notifyDatasetChanged(coreDataset);
}

void PCAPlugin::publishCopy()
{
    std::cout << "PCA Plugin: Publish a copy of the output dataset." << std::endl;

    // Create new data set
    auto copyDataset = _core->addDataset("Points", "PCA (copy)", getInputDataset());

    // Get data 
    std::vector<float> data;
    std::vector<unsigned int> dimensionIndices;
    getDataFromCore(getOutputDataset<Points>(), data, dimensionIndices);

    // Set data
    setPCADataInCore(copyDataset, data, dimensionIndices.size());
}

/// ////////////// ///
/// PLUGIN FACTORY ///
/// ////////////// ///

QIcon PCAPluginFactory::getIcon(const QColor& color /*= Qt::black*/) const
{
    return Application::getIconFont("FontAwesome").getIcon("braille");
}

AnalysisPlugin* PCAPluginFactory::produce()
{
    // Return a new instance of the example analysis plugin
    return new PCAPlugin(this);
}

PluginTriggerActions PCAPluginFactory::getPluginTriggerActions(const hdps::Datasets& datasets) const
{
    PluginTriggerActions pluginTriggerActions;

    const auto getPluginInstance = [this](const Dataset<Points>& dataset) -> PCAPlugin* {
        return dynamic_cast<PCAPlugin*>(plugins().requestPlugin(getKind(), { dataset }));
    };

    if (PluginFactory::areAllDatasetsOfTheSameType(datasets, PointType)) {
        if (datasets.count() >= 1) {
            auto pluginTriggerAction = new PluginTriggerAction(const_cast<PCAPluginFactory*>(this), this, "PCA", "Perform a principle component analysis on the selected datasets", getIcon(), [this, getPluginInstance, datasets](PluginTriggerAction& pluginTriggerAction) -> void {
                for (auto dataset : datasets)
                    getPluginInstance(dataset);
                });

            pluginTriggerActions << pluginTriggerAction;
        }
    }

    return pluginTriggerActions;
}
