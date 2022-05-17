#pragma once

#include <AnalysisPlugin.h>

#include <memory>
#include <tuple>

#include "SettingsAction.h"
#include "DimensionSelectionAction.h"

#include "graphics/Vector2f.h"

/** All plugin related classes are in the HDPS plugin namespace */
using namespace hdps::plugin;

/** Vector classes used in this plugin are in the HDPS namespace */
using namespace hdps;

namespace math {
    enum class PCA_ALG;
    enum class DATA_NORM;
}

/// ////////// ///
/// PCA WORKER ///
/// ////////// ///

class PCAWorker : public QObject
{
    Q_OBJECT

public:
    PCAWorker();
    PCAWorker(std::shared_ptr<std::vector<float>> data, size_t num_dims, size_t num_comps, math::PCA_ALG algorithm, math::DATA_NORM norm);

    void setup(std::shared_ptr<std::vector<float>> data, size_t num_dims, size_t num_comps, math::PCA_ALG algorithm, math::DATA_NORM norm);

    std::tuple<std::vector<float>&, size_t> getRestuls() { return { _pca_out, _num_comps }; }

signals:
    void resultReady();

public slots:
    void compute();

private:
    std::shared_ptr<std::vector<float>> _data;
    size_t _num_dims;
    size_t _num_comps;
    std::vector<float> _pca_out;
    math::PCA_ALG _algorithm;
    math::DATA_NORM _norm;
};


/// ////////// ///
/// PCA PLUGIN ///
/// ////////// ///
class PCAPlugin : public AnalysisPlugin
{
Q_OBJECT

public:

    /**
     * Constructor
     * @param factory Pointer to the plugin factory
     */
    PCAPlugin(const PluginFactory* factory);

    /** Destructor */
    ~PCAPlugin() override = default;

    /**
     * This function is called by the core after the analysis plugin has been created
     *
     * Typical implementations of this function focus on the generation of output data
     * and responding to events which are sent by the core.
    */
    void init() override;

    /**
     * Invoked when a points data event occurs
     * @param dataEvent Data event which occurred
     */
    void onDataEvent(hdps::DataEvent* dataEvent);

signals:
    void startPCA();

private:
    void computePCA();
    void getDataFromCore(std::vector<float>& data, std::vector<unsigned int>& indices);
    void setPCADataInCore(std::vector<float>& data, size_t num_components);

private:
    SettingsAction              _settingsAction;            /** General PCA settings */
    DimensionSelectionAction    _dimensionSelectionAction;  /** Dimension selection */

    PCAWorker                   _pcaWorker;
    QThread*                    _workerThread;
};

/// ////////////// ///
/// PLUGIN FACTORY ///
/// ////////////// ///
class PCAPluginFactory : public AnalysisPluginFactory
{
    Q_INTERFACES(hdps::plugin::AnalysisPluginFactory hdps::plugin::PluginFactory)
    Q_OBJECT
    Q_PLUGIN_METADATA(IID   "nl.BioVault.PCAPlugin"
                      FILE  "PCAPlugin.json")

public:

    /** Default constructor */
    PCAPluginFactory() {}

    /** Destructor */
    ~PCAPluginFactory() override {}

    /** Returns the plugin icon */
    QIcon getIcon() const override;

    /** Creates an instance of the example analysis plugin */
    AnalysisPlugin* produce() override;

    /** Returns the data types that are supported by the example analysis plugin */
    hdps::DataTypes supportedDataTypes() const override;
};
