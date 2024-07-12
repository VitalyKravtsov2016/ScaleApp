#include "weightmanager_opo.h"

#include "constants.h"
#include "weightmanager.h"
#include "oposcalesdk.h"
#include <QJsonDocument>

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
    if (nstatus.weight != status.weight){
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
    status.isError = false;
    status.isStarted = started;
    status.isTareSet = weight.isTareSet();
    status.isWeightFixed = weight.isStable();
    status.isOverloaded = weight.isOverflow();
    status.isWeightZero = weight.getGrossWeight().toDouble() == 0;
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
        device.Zero();
        device.Tare();
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


