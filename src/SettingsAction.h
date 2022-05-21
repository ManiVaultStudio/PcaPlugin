#pragma once

#include "actions/Actions.h"

/** All GUI related classes are in the HDPS Graphical User Interface namespace */
using namespace hdps::gui;

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

public:
    OptionAction    _pcaAlgorithmAction;            /** PCA algorithm action */
    OptionAction    _dataNormAction;                /** data normalization action */
    IntegralAction  _numberOfComponents;            /** Number of components action */
    ToggleAction    _stdAxisOrientation;            /** Enforce standardized axis orientation */
    TriggerAction   _startAnalysisAction;           /** Start computation */
};
