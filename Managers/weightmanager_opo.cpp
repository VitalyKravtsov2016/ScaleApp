#include "weightmanager_opo.h"

#include "constants.h"
#include "weightmanager.h"
#include "oposcalesdk.h"
#include <QJsonDocument>

int WeightManager_Opo::readConnectParam(const QString &filename, const QString &param, QString &uri)
{
    Tools::debugLog("@@@@@ WeightManager_Opo::readConnectParam");

    clearError();

    QFile file(filename);
    QByteArray data;
    if (!file.exists())
    {
        errorCode = -21;
        errorText = "Файл не найден";
        return errorCode;
    }

    if (!file.open(QIODevice::ReadOnly))
    {
        if (file.error() == QFileDevice::PermissionsError)
        {
            errorCode = -22;
            errorText = "Нет разрешений на чтение файла";
            return errorCode;
        }
        else
        {
            errorCode = -23;
            errorText = "Ошибка открытия файла";
            return errorCode;
        }
    }

    data = file.readAll();
    if (file.error() != QFileDevice::NoError)
    {
        errorCode = -24;
        errorText = "Ошибка чтения файла";
        file.close();
        return errorCode;
    }

    if (data.isEmpty()) {
        errorCode = -25;
        errorText = "Пустой файл";
        file.close();
        return errorCode;
    }

    QJsonDocument jsonDocument = QJsonDocument::fromJson(data);
    QJsonObject   jsonObject = jsonDocument.object();
    QJsonValue    jsonValue = jsonObject[param];
    if (jsonValue != QJsonValue::Null) uri = jsonValue.toString();
    else
    {
        errorCode = -26;
        errorText = "Параметр не найден";
        file.close();
        return errorCode;
    }
    file.close();
    return errorCode;
}

// "/dev/ttyS0";
// "WmUri":"serial:/dev/ttyS0",

int WeightManager_Opo::open(QString path)
{
    Tools::debugLog("@@@@@ WeightManager_Opo::open " + path);
    clearError();
    QString uri;
    int e = readConnectParam(path, "WmUri", uri);
    if(e)
    {
        return e;
    }

    if (uri.startsWith("demo:")){
        mode = EquipmentMode_Demo;
        return 0;
    }

    if (uri.startsWith("serial:")){
        static QString SerialRegexStr("^serial:\\/\\/w+");
        QRegularExpression reg(SerialRegexStr);
        QRegularExpressionMatch match = reg.match(uri);
        deviceName = match.captured(0);
        mode = EquipmentMode_Ok;
        return 0;
    }
    mode = EquipmentMode_None;
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
            started = true;
        }
    }
    Tools::debugLog("@@@@@ WeightManager_Opo::start: " + errorText);
    return errorCode;
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

    QString result = device.GetResult();
    WeightData weight = device.getWeight();

    status.tare = weight.getTareWeight().toDouble();
    status.weight = weight.getNetWeight().toDouble();
    status.isError = false;
    status.isOverloaded = false;
    status.isStarted = started;
    status.isTareSet = weight.isTareSet();
    status.isWeightFixed = weight.isStable();
    status.isWeightZero = status.weight == 0;
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


