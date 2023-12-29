#include "DimensionSelectionAction.h"

DimensionSelectionAction::DimensionSelectionAction(QObject* parent) :
    GroupAction(parent, "DimensionSelectionAction"),
    _pickerAction(this, "DimensionPicker")
{
    qDebug() << "DimensionSelectionAction::DimensionSelectionAction";

    setText("PCA Input Dimensions");
    setSerializationName("DimensionSelectionAction");

    setPopupSizeHint(QSize(400, 0));

    addAction(&_pickerAction);
}

void DimensionSelectionAction::fromVariantMap(const QVariantMap& variantMap)
{
    qDebug() << "DimensionSelectionAction::fromVariantMap";

    GroupAction::fromVariantMap(variantMap);

    _pickerAction.fromParentVariantMap(variantMap);
}

QVariantMap DimensionSelectionAction::toVariantMap() const
{
    qDebug() << "DimensionSelectionAction::toVariantMap";

    QVariantMap variantMap = GroupAction::toVariantMap();

    _pickerAction.insertIntoVariantMap(variantMap);

    return variantMap;
}
