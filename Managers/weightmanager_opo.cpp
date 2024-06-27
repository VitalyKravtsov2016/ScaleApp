#include "weightmanager_opo.h"

#include "constants.h"
#include "weightmanager.h"
#include "oposcalesdk.h"
#include <QJsonDocument>

int WeightManager_Opo::readConnectParam(const QString &filename, const QString &param, QString &uri)
{
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
    return errorCode;
}

int WeightManager_Opo::stop()
{
    clearError();
    if (started){
        device.Close();
    }
    return errorCode;
}

QString WeightManager_Opo::getVersion()
{
    return "1.0";
}

QString WeightManager_Opo::getDescription(){
    return "OpoScale";
}

ScaleStatus WeightManager_Opo::getStatus()
{
    QString result = device.GetResult();
    WeightData weight = device.getWeight();

    status.tare = weight.getTareWeight().toDouble();
    status.weight = weight.getNetWeight().toDouble();
    status.isError = false;
    status.isOverloaded = false;
    status.isStarted = started;
    status.isTareSet = weight.isTareSet();
    status.isWeightFixed = weight.isStable();
    status.isWeightZero = weight.isZero();
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

QString WeightManager_Opo::getErrorDescription(const int e){
    return device.getErrorMessage(e);
}


