#include "weightdata.h"

#include <QString>
#include <QStringList>

// Mode constants
#define ModeNet         "N"
#define ModeTare        "T"
#define ModeTareSet     "P"

// Status constants
#define StatusFlow      "F"
#define StatusStable    "S"
#define StatusUnstable  "U"

WeightData::WeightData() {}

WeightData WeightData::parse(QString data)
{
    WeightData result;
    QStringList list = data.split(",");
    if (list.length() < 7) return result;

    result.sMode = list[0];
    result.sStatus = list[1];
    result.sZero = list[2];
    result.sUnit = list[3];
    result.sNetWeight = list[4];
    result.sTareWeight = list[5];
    result.sGrossWeight = list[6];
    return result;
}

QString WeightData::getMode(){
    return sMode;
}

QString WeightData::getStatus(){
    return sStatus;
}

QString WeightData::getZero(){
    return sZero;
}

QString WeightData::getUnit(){
    return sUnit;
}

QString WeightData::getNetWeight(){
    return sNetWeight;
}

QString WeightData::getTareWeight(){
    return sTareWeight;
}

QString WeightData::getGrossWeight(){
    return sGrossWeight;
}

bool WeightData::isTareSet(){
    return (sMode == ModeTare)||(sMode == ModeTareSet);
}

bool WeightData::isStable(){
    return sStatus == StatusStable;
}
