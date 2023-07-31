#include "DimensionSelectionAction.h"

DimensionSelectionAction::DimensionSelectionAction(QObject* parent) :
    GroupAction(parent, "DimensionSelectionAction"),
    _pickerAction(this, "DimensionPicker")
{
    setText("PCA Input Dimensions");
    setPopupSizeHint(QSize(400, 0));

    addAction(&_pickerAction);
}
