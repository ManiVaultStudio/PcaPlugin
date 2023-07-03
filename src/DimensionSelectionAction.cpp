#include "DimensionSelectionAction.h"

DimensionSelectionAction::DimensionSelectionAction(QObject* parent) :
    GroupAction(parent, "DimensionSelectionAction"),
    _pickerAction(this, "DimensionPicker")
{
    setText("Dimensions");
    setPopupSizeHint(QSize(400, 0));
}
