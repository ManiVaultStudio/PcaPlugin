#pragma once

#include <AnalysisPlugin.h>

#include "SettingsAction.h"
#include "DimensionSelectionAction.h"

#include <tuple>

#include <QThread>
#include <QPointer>

/** All plugin related classes are in the HDPS plugin namespace */
using namespace mv::plugin;

/** Vector classes used in this plugin are in the HDPS namespace */
using namespace mv;

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
    PCAWorker(std::shared_ptr<std::vector<float>> data, size_t num_dims, size_t num_comps, math::PCA_ALG algorithm, math::DATA_NORM norm, bool std_orient);

    std::tuple<std::vector<float>&, size_t> getRestuls() { return { _pca_out, _num_comps }; }

signals:
    void resultReady(int32_t pca_status);

public slots:
    void compute();

private:
    std::shared_ptr<std::vector<float>> _data;
    size_t _num_dims;
    size_t _num_comps;
    bool _std_orient;
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

    /* This function is called by the core after the analysis plugin has been created, sets up init data */
    void init() override;

signals:
    void startPCA();

private:
    void computePCA();
    void getDataFromCore(const mv::Dataset<Points> coreDataset, std::vector<float>& data, std::vector<unsigned int>& indices);
    void setPCADataInCore(mv::Dataset<Points> coreDataset, const std::vector<float>& data, const size_t num_components);
    void publishCopy();

private:
    SettingsAction              _settingsAction;            /** General PCA settings */
    DimensionSelectionAction    _dimensionSelectionAction;  /** Dimension selection */

    QPointer<PCAWorker>         _pcaWorker;                 /** Worker that computes PCA in another thread */
    QThread                     _workerThread;              /** Thread for PCA computation */
};

/// ////////////// ///
/// PLUGIN FACTORY ///
/// ////////////// ///
class PCAPluginFactory : public AnalysisPluginFactory
{
    Q_INTERFACES(mv::plugin::AnalysisPluginFactory mv::plugin::PluginFactory)
    Q_OBJECT
    Q_PLUGIN_METADATA(IID   "nl.BioVault.PCAPlugin"
                      FILE  "PcaPlugin.json")

public:

    /** Default constructor */
    PCAPluginFactory() {}

    /** Destructor */
    ~PCAPluginFactory() override {}

    /** Returns the plugin icon */
    QIcon getIcon(const QColor& color = Qt::black) const override;

    /** Creates an instance of the example analysis plugin */
    AnalysisPlugin* produce() override;

    /**
     * Get plugin trigger actions given \p datasets
     * @param datasets Vector of input datasets
     * @return Vector of plugin trigger actions
     */
    PluginTriggerActions getPluginTriggerActions(const mv::Datasets& datasets) const override;
};
