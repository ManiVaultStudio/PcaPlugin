#include "SettingsAction.h"

SettingsAction::SettingsAction(QObject* parent) :
    GroupAction(parent, "SettingsAction", true),
    _pcaAlgorithmAction(this, "PCA alg"),
    _dataNormAction(this, "Data norm"),
    _numberOfComponents(this, "Number of PCA components"),
    _stdAxisOrientation(this, "Std. axis orientation"),
    _startAnalysisAction(this, "Start analysis"),
    _publishNewDataAction(this, "Copy to new data set")
{
    setText("PCA");

    _pcaAlgorithmAction.setToolTip("Type of PCA algorithm");
    _dataNormAction.setToolTip("Type data normalization");
    _numberOfComponents.setToolTip("Number of PCA components to be used");
    _stdAxisOrientation.setToolTip("Enforce standardized axis orientation");
    _startAnalysisAction.setToolTip("Start the analysis");
    _publishNewDataAction.setToolTip("Published a copy of the output");

    _publishNewDataAction.setEnabled(false); // only enable once an analysis is done

    _pcaAlgorithmAction.initialize(QStringList({ "COV", "SVD" }), "COV");
    _dataNormAction.initialize(QStringList({ "None", "Mean Norm", "Min-Max Norm"}), "None");
    _stdAxisOrientation.setChecked(true);
    _numberOfComponents.initialize(1, 2, 2);    // default: use 2 PCA components, max is set data-dependent in PcaPlugin.cpp 
}

