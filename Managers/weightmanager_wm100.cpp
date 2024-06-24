#include "weightmanager_wm100.h"
#include "appmanager.h"


void WeightManager_wm100::clearError(){
    setLastError(0);
}

void WeightManager_wm100::setLastError(int e){
    errorCode = e;
    errorText = getErrorDescription(e);
}

int WeightManager_wm100::open(QString path)
{
    int e = device.readConnectParam(path, "WmUri", uri);
    setLastError(e);
    if(e) {
        return e;
    }

    switch (device.checkUri(uri))
    {
    case Wm100Protocol::diDemo:
        mode = EquipmentMode_Demo;
        break;

    case Wm100Protocol::diNone:
        mode = EquipmentMode_None;
        break;

    default:
        mode = EquipmentMode_Ok;
        break;
    }
    return 0;
}

int WeightManager_wm100::start()
{
    Tools::debugLog("@@@@@ WeightManager_wm100::start " + uri);
    if (!started)
    {
        connect(&device, &Wm100::weightStatusChanged, this, &WeightManager_wm100::onStatusChanged);
        connect(&device, &Wm100::errorStatusChanged, this, &WeightManager_wm100::onErrorStatusChanged);

        int e = device.connectDevice(uri);
        setLastError(e);
        started = (e == 0);
        if (e) {
            Tools::debugLog("@@@@@ WeightManager_wm100::start error: " + errorText);
            return e;
        }

        device.blockSignals(false);
        if(isSystemDateTime)
        {
            Tools::debugLog("@@@@@ WeightManager_wm100::setDateTime");
            device.setDateTime(QDateTime::currentDateTime());
        }
        device.startPolling(200);
        isSystemDateTime = false;
    }
    return 0;
}

int WeightManager_wm100::stop()
{
    Tools::debugLog("@@@@@ WeightManager_wm100::stop");
    if (started)
    {
        disconnect(&device, &Wm100::weightStatusChanged, this, &WeightManager_wm100::onStatusChanged);
        disconnect(&device, &Wm100::errorStatusChanged, this, &WeightManager_wm100::onErrorStatusChanged);

        started = false;
        device.blockSignals(true);
        device.stopPolling();
        int e = device.disconnectDevice();
        if (e){
            Tools::debugLog("@@@@@ WeightManager_wm100::stop error " + Tools::intToString(e));
        }
    }
    return 0;
}

QString WeightManager_wm100::getVersion()
{
    Wm100Protocol::device_metrics dm;
    if(device.getDeviceMetrics(&dm) >= 0) return Tools::intToString(dm.protocol_version);
}

void WeightManager_wm100::setParam(const int param)
{
    Tools::debugLog("@@@@@ WeightManager_wm100::setParam " + Tools::intToString(param));
    if (!started) return;

    switch (param)
    {
    case ControlParam_Tare:
        device.setTare();
        break;

    case ControlParam_Zero:
        device.setZero();
        device.setTare();
        break;
    default:
        break;
    }
}

bool WeightManager_wm100::isFlag(Wm100Protocol::channel_status s, int shift)
{
    return (s.state & (0x00000001 << shift)) != 0;
}

bool WeightManager_wm100::isStateError(Wm100Protocol::channel_status s)
{
    return isFlag(s, 5) || isFlag(s, 6) || isFlag(s, 7) || isFlag(s, 8) || isFlag(s, 9);
}

QString WeightManager_wm100::getErrorDescription(const int e)
{
    switch(e)
    {
    case 0:    return "Ошибок нет";
    case 5003: return "Ошибка автонуля при включении";
    case 5004: return "Перегрузка по весу";
    case 5005: return "Ошибка при получении измерения";
    case 5006: return "Весы недогружены";
    case 5007: return "Ошибка: нет ответа от АЦП";
    case 5008: return "Ошибка: неверное значение даты";

        //default: return "Неизвестная ошибка";
    }
    return device.errorDescription(e);
}

ScaleStatus WeightManager_wm100::getStatus()
{
    ScaleStatus scaleStatus;
    scaleStatus.weight = status.weight;
    scaleStatus.tare = status.tare;
    scaleStatus.isError = errorCode != 0 || isStateError(status);
    scaleStatus.isDemoMode = mode == EquipmentMode_Demo;
    scaleStatus.isOverloaded = isFlag(status, 6);
    scaleStatus.isTareSet = isFlag(status, 3);
    scaleStatus.isWeightFixed = isFlag(status, 0);
    scaleStatus.isWeightZero = isFlag(status, 1);
    scaleStatus.isStarted = started;
    return scaleStatus;
}


void WeightManager_wm100::onStatusChanged(Wm100Protocol::channel_status &s)
{
    if(DEBUG_WEIGHT_STATUS)
        Tools::debugLog(QString("@@@@@WeightManager_wm100::onWMStatusChanged state=%1b weight=%2 tare=%3").arg(
            QString::number(s.state, 2), QString::number(s.weight), QString::number(s.tare)));

    //bool b0 = isFlag(s, 0); // признак фиксации веса
    //bool b1 = isFlag(s, 1); // признак работы автонуляmain()
    //bool b2 = isFlag(s, 2); // "0"- канал выключен, "1"- канал включен
    //bool b3 = isFlag(s, 3); // признак тары
    //bool b4 = isFlag(s, 4); // признак успокоения веса
    bool b5 = isFlag(s, 5); // ошибка автонуля при включении
    bool b6 = isFlag(s, 6); // перегрузка по весу
    bool b7 = isFlag(s, 7); // ошибка при получении измерения (нет градуировки весов или она не правильная)
    bool b8 = isFlag(s, 8); // весы недогружены
    bool b9 = isFlag(s, 9); // ошибка: нет ответа от АЦП

    ControlParam param = ControlParam_WeightValue;
    int e = 0;
    if(b5 || b6 || b7 || b8 || b9) // Ошибка состояния
    {
        param = ControlParam_WeightError;
        if(b5 && isFlag(status, 5) != b5) e = 5003;
        if(b6 && isFlag(status, 6) != b6) e = 5004;
        if(b7 && isFlag(status, 7) != b7) e = 5005;
        if(b8 && isFlag(status, 8) != b8) e = 5006;
        if(b9 && isFlag(status, 9) != b9) e = 5007;
    }
    else if(isStateError(status) && errorCode == 0) param = ControlParam_WeightError; // Ошибка исчезла

    status.weight = s.weight;
    status.tare = s.tare;
    status.state = s.state;
    emit paramChanged(param, e);
}

void WeightManager_wm100::onErrorStatusChanged(int e)
{
    Tools::debugLog("@@@@@ WeightManager_wm100::onErrorStatusChanged " + QString::number(e));
    if(errorCode != e)
    {
        errorCode = e;
        errorText = getErrorDescription(e);
        emit paramChanged(ControlParam_WeightError, e);
    }
}

int WeightManager_wm100::setDateTime(const QDateTime& dt)
{
    Tools::debugLog("@@@@@ WeightManager_wm100::setDateTime " + dt.toString("dd.MM.yyyy HH:mm:ss"));
    if (!dt.isValid()){
        errorCode =  5008;
        errorText = "Ошибка: неверное значение даты";
        return errorCode;
    }

    int e = device.setDateTime(dt);
    setLastError(e);
    return e;
}

QString WeightManager_wm100::getDescription()
{
    Wm100Protocol::channel_specs cp;
    device.getChannelParam(&cp);
    QString res, max, min, dis, tar;
    max = "Max ";
    int i = 0;
    while (i < 3 && cp.ranges[i])
    {
        max += QString("%1/").arg(cp.ranges[i]);
        ++i;
    }
    max += QString("%1 кг").arg(cp.max);
    min = QString("Min %1 г").arg(cp.min*1000);
    dis = "e=";
    i = 0;
    while (i < 4 && cp.discreteness[i])
    {
        if (i) dis += "/";
        dis += QString("%1").arg(cp.discreteness[i]*1000);
        ++i;
    }
    dis += " г";
    tar = QString("T=-%1 кг").arg(cp.tare);
    res = QString("%1   %2   %3   %4").arg(max, min, dis, tar);
    return res;
}

// SIGNAL 0
void WeightManager_wm100::paramChanged(const int _t1, const int _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

