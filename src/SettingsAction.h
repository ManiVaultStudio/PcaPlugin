#pragma once

#include "actions/GroupAction.h"
#include "actions/IntegralAction.h"
#include "actions/OptionAction.h"
#include "actions/ToggleAction.h"
#include "actions/TriggerAction.h"

/** All GUI related classes are in the HDPS Graphical User Interface namespace */
using namespace mv::gui;

/**
 * PCASettingsAction class
 * 
 */
class SettingsAction : public GroupAction
{
public:

    /**
     * Constructor
     * @param parent Pointer to parent object
     */
    SettingsAction(QObject* parent = nullptr);

public: // Action getters

    OptionAction& getPcaAlgorithmAction() { return _pcaAlgorithmAction; }
    OptionAction& getDataNormAction() { return _dataNormAction; }
    IntegralAction& getNumberOfComponents() { return _numberOfComponents; }
    ToggleAction& getStdAxisOrientation() { return _stdAxisOrientation; }
    TriggerAction& getStartAnalysisAction() { return _startAnalysisAction; }
    TriggerAction& getPublishNewDataAction() { return _publishNewDataAction; }

public: // Serialization

    /**
     * Load plugin from variant map
     * @param Variant map representation of the plugin
     */
    void fromVariantMap(const QVariantMap& variantMap) override;

    /**
     * Save plugin to variant map
     * @return Variant map representation of the plugin
     */
    QVariantMap toVariantMap() const override;

public:
    OptionAction    _pcaAlgorithmAction;            /** PCA algorithm action */
    OptionAction    _dataNormAction;                /** data normalization action */
    IntegralAction  _numberOfComponents;            /** Number of components action */
    ToggleAction    _stdAxisOrientation;            /** Enforce standardized axis orientation */
    TriggerAction   _startAnalysisAction;           /** Start computation */
    TriggerAction   _publishNewDataAction;          /** Publish new data set, one that is not derived */
};
