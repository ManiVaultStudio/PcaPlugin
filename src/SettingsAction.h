#pragma once

#include "actions/Actions.h"
#include <actions/GroupAction.h>
#include <DimensionsPickerAction.h>

/** All GUI related classes are in the HDPS Graphical User Interface namespace */
using namespace hdps::gui;

// ////////////// //
// SettingsAction //
// ////////////// //


/**
 * Dimension selection action class
 *
 * Action class for point data dimension selection
 *
 * @author Thomas Kroes
 */
class DimensionSelectionAction : public GroupAction
{
public:

    /**
     * Constructor
     * @param parent Pointer to parent object
     */
    DimensionSelectionAction(QObject* parent);

public: // Action getters

    DimensionsPickerAction& getPickerAction() { return _pickerAction; };

protected:
    DimensionsPickerAction  _pickerAction;    /** Dimension picker action */

    friend class Widget;
};


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
    DimensionSelectionAction& getDimensionSelectionAction() { return _dimensionSelectionAction; }

public:
    OptionAction    _pcaAlgorithmAction;            /** PCA algorithm: SVD or Eigenvalues of covariance matrix */
    IntegralAction  _numberOfComponents;            /** Number of components */
    TriggerAction   _startAnalysisAction;           /** Start computation */
    DimensionSelectionAction          _dimensionSelectionAction;      /** Dimension selection settings action */
};
