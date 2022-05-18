#include "SettingsAction.h"

SettingsAction::SettingsAction(QObject* parent) :
    GroupAction(parent, true),
    _pcaAlgorithmAction(this, "PCA alg"),
    _dataNormAction(this, "Data norm"),
    _numberOfComponents(this, "Number of PCA components"),
    _startAnalysisAction(this, "Start analysis")
{
    setText("PCA");

    _pcaAlgorithmAction.setToolTip("Type of PCA algorithm");
    _dataNormAction.setToolTip("Type data normalization");
    _numberOfComponents.setToolTip("Number of PCA components to be used");
    _startAnalysisAction.setToolTip("Start the analysis");

    _pcaAlgorithmAction.initialize(QStringList({ "SVD", "COV" }), "SVD", "SVD");
    _dataNormAction.initialize(QStringList({ "None", "Mean Norm", "Min-Max Norm"}), "None", "None");
    _numberOfComponents.initialize(1, 2, 2, 2);    // default: use 2 PCA components, max is set data-dependent in PcaPlugin.cpp 

}

