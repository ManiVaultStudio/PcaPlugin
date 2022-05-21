#include "PcaPlugin.h"

#include "PointData.h"
#include "PCA.h"
#include "Utils.h" 

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
        alg = math::PCA_ALG::SVD;
        break;
    case 1:
        alg = math::PCA_ALG::COV;
        break;
    }

    return alg;
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


/// ////////// ///
/// PCA WORKER ///
/// ////////// ///
PCAWorker::PCAWorker() :
    _data(nullptr),
    _num_dims(0),
    _num_comps(0),
    _std_orient(true),
    _algorithm(math::PCA_ALG::COV),
    _norm(math::DATA_NORM::NONE)
{
}

PCAWorker::PCAWorker(std::shared_ptr<std::vector<float>> data, size_t num_dims, size_t num_comps, math::PCA_ALG algorithm, math::DATA_NORM norm, bool std_orient) :
    _data(data), 
    _num_dims(num_dims),
    _num_comps(num_comps), 
    _std_orient(std_orient),
    _algorithm(algorithm),
    _norm(norm)
{
}

void PCAWorker::setup(std::shared_ptr<std::vector<float>> data, size_t num_dims, size_t num_comps, math::PCA_ALG algorithm, math::DATA_NORM norm, bool std_orient) {
    _data = data;
    _num_dims = num_dims;
    _num_comps = num_comps;
    _std_orient = std_orient;
    _algorithm = algorithm;
    _norm = norm;
}

void PCAWorker::compute() {
    utils::timer([&]() {
        math::pca(*_data, /* number of dimension = */ _num_dims, /* transformed PCA data = */ _pca_out, /* number of pca components = */ _num_comps,
            /* pca algorithm = */ _algorithm, /* data normalization = */ _norm, /* stdOrientation = */ _std_orient);
        },
        "PCA computation time (ms)");

    emit resultReady();
}


/// ////// ///
/// PLUGIN ///
/// ////// ///

PCAPlugin::PCAPlugin(const PluginFactory* factory) :
    AnalysisPlugin(factory),
    _settingsAction(this),
    _dimensionSelectionAction(this),
    _pcaWorker(),
    _workerThread(nullptr)
{
    qRegisterMetaType<std::vector<float>>();
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

    // Set initial data (default 2 dimensions, all points at (0,0) )
    std::vector<float> initialData;
    const auto numInitialDataDimensions = 2;
    initialData.resize(inputDataset->getNumPoints() * numInitialDataDimensions);
    outputDataset->setData(initialData.data(), inputDataset->getNumPoints(), numInitialDataDimensions);

    // Start the analysis when the user clicks the start analysis push button
    connect(&_settingsAction.getStartAnalysisAction(), &hdps::gui::TriggerAction::triggered, this, [&]() {
        computePCA();
    });

    connect(&_pcaWorker, &PCAWorker::resultReady, this, [&]() {
        auto [pca_out, num_comps] = _pcaWorker.getRestuls();

        // Publish pca to core
        setPCADataInCore(pca_out, num_comps);

        // Flag the analysis task as finished
        setTaskFinished();

        // Enabled action again
        _settingsAction.getStartAnalysisAction().setEnabled(true);

        std::cout << "PCA Plugin: Finished." << std::endl;
    });

    // Register for points datasets events using a custom callback function
    _eventListener.setEventCore(Application::core());

    _eventListener.addSupportedEventType(static_cast<std::uint32_t>(EventType::DataChanged));
    _eventListener.registerDataEventByType(PointType, std::bind(&PCAPlugin::onDataEvent, this, std::placeholders::_1));
}

void PCAPlugin::onDataEvent(hdps::DataEvent* dataEvent)
{

    if (dataEvent->getType() == EventType::DataChanged)
    {
        if (dataEvent->getDataset() == getInputDataset())
            _dimensionSelectionAction.getPickerAction().setPointsDataset(dataEvent->getDataset<Points>());
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

    // Get settings
    math::PCA_ALG alg = getPcaAlgorithm(_settingsAction.getPcaAlgorithmAction().getCurrentIndex());
    math::DATA_NORM norm = getDataNorm(_settingsAction.getDataNormAction().getCurrentIndex());
    bool stdOrientation = _settingsAction.getStdAxisOrientation().isChecked();

    // Computat in different thread
    setTaskDescription("Computing...");

    _pcaWorker.setup(std::make_shared<std::vector<float>>(data), dimensionIndices.size(), num_comps, alg, norm, stdOrientation);

    _workerThread = new QThread();
    _pcaWorker.moveToThread(_workerThread);

    connect(_workerThread, &QThread::finished, _workerThread, &QObject::deleteLater);   // delete thread after work is done
    connect(this, &PCAPlugin::startPCA, &_pcaWorker, &PCAWorker::compute);              // setup pca computation 

    std::cout << "PCA Plugin: Starting computing PCA transformation with " << num_comps << " components (settings: alg " << static_cast<int>(alg) << ", norm " << static_cast<int>(norm) << ")" << std::endl;
    _workerThread->start();
    // once finished _pcaWorker will call a lamda defined in this->init() that publishes the PCA
    emit startPCA();
}

void PCAPlugin::getDataFromCore(std::vector<float>& data, std::vector<unsigned int>& dimensionIndices)
{
    auto inputPoints = getInputDataset<Points>();

    // Extract the enabled dimensions from the data
    std::vector<bool> enabledDimensions = _dimensionSelectionAction.getPickerAction().getEnabledDimensions();

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

QIcon PCAPluginFactory::getIcon() const
{
    return Application::getIconFont("FontAwesome").getIcon("braille");
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
