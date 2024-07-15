#include "weightmanager_opo.h"

#include "constants.h"
#include "weightmanager.h"
#include "oposcalesdk.h"
#include <QJsonDocument>

bool isEquals(ScaleStatus item1, ScaleStatus item2)
{
    return (item1.weight == item2.weight) &&
           (item1.tare == item2.tare) &&
           (item1.isStarted == item2.isStarted) &&
           (item1.isError == item2.isError) &&
           (item1.isOverloaded == item2.isOverloaded) &&
           (item1.isWeightFixed == item2.isWeightFixed) &&
           (item1.isWeightZero == item2.isWeightZero) &&
           (item1.isTareSet == item2.isTareSet) &&
           (item1.isDemoMode == item2.isDemoMode);

}


int WeightManager_Opo::open(QString path)
{
    Tools::debugLog("@@@@@ WeightManager_Opo::open " + path);
    clearError();
    mode = EquipmentMode_Ok;
    return 0;
}

void WeightManager_Opo::clearError()
{
    errorCode = 0;
    errorText = "Ошибок нет";
}

int WeightManager_Opo::start()
{
    Tools::debugLog("@@@@@ WeightManager_Opo::start");

    clearError();
    if (!started)
    {
        if (!device.Open("/dev/" + deviceName))
        {
            errorCode = -1;
            errorText = "Ошибка открытия устройства";
        } else
        {
            device.ExitTare();
            device.Zero();

            connect(&timer, &QTimer::timeout, this, &WeightManager_Opo::onTimer);
            timer.start(100);
            started = true;
        }
    }
    Tools::debugLog("@@@@@ WeightManager_Opo::start: " + errorText);
    return errorCode;
}

void WeightManager_Opo::onTimer()
{
    Tools::debugLog("@@@@@ WeightManager_Opo::onTimer");
    ScaleStatus nstatus = getStatus();
    if (!isEquals(nstatus, status))
    {
        emit paramChanged(ControlParam_WeightValue, 0);
    }
    status = nstatus;
}

void WeightManager_Opo::onWeightChanged(const ::OnePlusOneAndroidSDK::ScalesOS::WeightInfo& arg1)
{
    Tools::debugLog("@@@@@ WeightManager_Opo::onWeightChanged.0");
    emit paramChanged(ControlParam_WeightValue, 0);
    Tools::debugLog("@@@@@ WeightManager_Opo::onWeightChanged.1");
}

int WeightManager_Opo::stop()
{
    Tools::debugLog("@@@@@ WeightManager_Opo::stop");
    clearError();
    if (started){
        started = false;
        timer.stop();
        device.Close();
    }
    return errorCode;
}

QString WeightManager_Opo::getVersion()
{
    Tools::debugLog("@@@@@ WeightManager_Opo::getVersion");
    return "1.0";
}

QString WeightManager_Opo::getDescription(){
    Tools::debugLog("@@@@@ WeightManager_Opo::getDescription");
    return "OpoScale";
}

ScaleStatus WeightManager_Opo::getStatus()
{
    Tools::debugLog("@@@@@ WeightManager_Opo::getStatus");

    ScaleStatus status;
    QString result = device.GetResult();
    WeightData weight = device.getWeight();

    status.tare = weight.getTareWeight().toDouble();
    status.weight = weight.getNetWeight().toDouble();
    status.grossWeight = weight.getGrossWeight().toDouble();
    status.isError = false;
    status.isStarted = started;
    status.isTareSet = weight.isTareSet();
    status.isWeightFixed = weight.isStable() && (status.grossWeight >= 0.040);
    status.isOverloaded = weight.isOverflow();
    status.isWeightZero = status.grossWeight == 0;
    Tools::debugLog("@@@@@ WeightManager_Opo::getStatus: tare=" + Tools::doubleToString(status.tare, 3) +
                    ", weight=" + Tools::doubleToString(status.weight, 3) +
                    ", isTareSet=" + Tools::boolToString(status.isTareSet) +
                    ", isWeightFixed=" + Tools::boolToString(status.isWeightFixed) +
                    ", isWeightZero=" + Tools::boolToString(status.isWeightZero));
    return status;
}

void WeightManager_Opo::setParam(const int param){
    Tools::debugLog("@@@@@ WeightManager_Opo::setParam " + Tools::intToString(param));

    if (!started) return;

    switch (param)
    {
    case ControlParam_Tare:
        device.Tare();
        break;

    case ControlParam_Zero:
        device.ExitTare();
        device.Zero();
        break;
    default:
        break;
    }
}

int WeightManager_Opo::setDateTime(const QDateTime& dt)
{
    Tools::debugLog("@@@@@ WeightManager_Opo::setDateTime " + dt.toString("dd.MM.yyyy HH:mm:ss"));

    clearError();
    return errorCode;
}

QString WeightManager_Opo::getErrorDescription(const int e)
{
    Tools::debugLog("@@@@@ WeightManager_Opo::getErrorDescription");

    switch (e)
    {
        case -1: return "Ошибка открытия устройства";
        default: return device.getErrorMessage(e);
    }
}


