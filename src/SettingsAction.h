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
    IntegralAction& getNumberOfComponents() { return _numberOfComponents; }
    TriggerAction& getStartAnalysisAction() { return _startAnalysisAction; }

public:
    OptionAction    _pcaAlgorithmAction;            /** PCA algorithm: SVD or Eigenvalues of covariance matrix */
    IntegralAction  _numberOfComponents;            /** Number of components */
    TriggerAction   _startAnalysisAction;           /** Start computation */
};
