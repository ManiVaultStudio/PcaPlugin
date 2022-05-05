#pragma once

#include <AnalysisPlugin.h>

#include "SettingsAction.h"
#include "DimensionSelectionAction.h"

#include "graphics/Vector2f.h"

#include <QRandomGenerator>
#include <QtMath>

/** All plugin related classes are in the HDPS plugin namespace */
using namespace hdps::plugin;

/** Vector classes used in this plugin are in the HDPS namespace */
using namespace hdps;

/**
 * PCA Plugin
 */
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

private:
    void computePCA();
    void getDataFromCore(std::vector<float>& data, std::vector<unsigned int>& indices);
    void setPCADataInCore(std::vector<float>& data, size_t num_components);

private:
    SettingsAction              _settingsAction;            /** General PCA settings */
    DimensionSelectionAction    _dimensionSelectionAction;  /** Dimension selection */
};

/**
 * Example analysis plugin factory class
 *
 * Note: Factory does not need to be altered (merely responsible for generating new plugins when requested)
 */
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

    /** Creates an instance of the example analysis plugin */
    AnalysisPlugin* produce() override;

    /** Returns the data types that are supported by the example analysis plugin */
    hdps::DataTypes supportedDataTypes() const override;
};
