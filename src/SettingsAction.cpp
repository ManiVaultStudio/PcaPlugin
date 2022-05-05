#include "SettingsAction.h"

// //////////////////////// //
// DimensionSelectionAction //
// //////////////////////// //
DimensionSelectionAction::DimensionSelectionAction(QObject* parent) :
    GroupAction(parent, true),
    _pickerAction(this)
{
    setText("Dimensions");
}

// ////////////// //
// SettingsAction //
// ////////////// //
SettingsAction::SettingsAction(QObject* parent) :
    GroupAction(parent, true),
    _pcaAlgorithmAction(this, "PCA alg"),
    _numberOfComponents(this, "Number of iterations"),
    _startAnalysisAction(this, "Start analysis"),
    _dimensionSelectionAction(this)
{
    setText("PCA");

    _pcaAlgorithmAction.setToolTip("Type of PCA algorithm");
    _numberOfComponents.setToolTip("Number of PCA components to be used");
    _startAnalysisAction.setToolTip("Start the analysis");

    _pcaAlgorithmAction.initialize(QStringList({ "SVD", "COV" }), "SVD", "SVD");
    _numberOfComponents.initialize(1, 2, 2, 2);    // default: use 2 PCA components

}

