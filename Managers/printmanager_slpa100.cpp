#include "printmanager_slpa100.h"
#include "wm100.h"

bool isPMFlag(uint16_t v, int shift) {
    return (v & (0x00000001 << shift)) != 0;
}

PrintManager_Slpa100::PrintManager_Slpa100() {}

int PrintManager_Slpa100::open(QString path)
{
    Wm100 mgr;
    int e2 = mgr.readConnectParam(path, "PrinterUri", uri);
    if(e2 != 0) { return e2;}
    else
    {
        switch (device.checkUri(uri))
        {
        case Slpa100uProtocol::diDemo:
            mode = EquipmentMode_Demo;
            break;

        case Slpa100uProtocol::diNone:
            mode = EquipmentMode_None;
            break;

        default:
            mode = EquipmentMode_Ok;
            break;
        }
    }
}

int PrintManager_Slpa100::start(AppManager* appManager)
{
    Tools::debugLog("@@@@@ PrintManager_Slpa100::start " + uri);
    int e = 0;
    if (!started)
    {
        connect(&device, &Slpa100u::printerErrorChanged, this, &PrintManager_Slpa100::onPMErrorStatusChanged);
        connect(&device, &Slpa100u::printerStatusChanged, this, &PrintManager_Slpa100::onPMStatusChanged);

        e = device.connectDevice(uri);
        started = (e == 0);
        if(started)
        {
            e = device.setBrightness(appManager->settings->getIntValue(SettingCode_PrinterBrightness) + 8);
            if(e >= 0) e = device.setOffset(appManager->settings->getIntValue(SettingCode_PrintOffset) + 8);
            if(e >= 0) e = device.setPaper(appManager->settings->getIntValue(SettingCode_PrintPaper, true) == 0 ?
                                       Slpa100uProtocol::papertype::ptSticker :
                                       Slpa100uProtocol::papertype::ptRibbon);
            if(e >= 0) e = device.setSensor(appManager->settings->getBoolValue(SettingCode_PrintLabelSensor));
        }
    }
    Tools::debugLog("@@@@@ EquipmentManager::startPM error " + QString::number(e));
    return e;
}

int PrintManager_Slpa100::stop()
{
    Tools::debugLog("@@@@@ PrintManager_Slpa100::stop");
    if (started)
    {
        disconnect(&device, &Slpa100u::printerErrorChanged, this, &PrintManager_Slpa100::onPMErrorStatusChanged);
        disconnect(&device, &Slpa100u::printerStatusChanged, this, &PrintManager_Slpa100::onPMStatusChanged);

        device.blockSignals(true);
        device.stopPolling();
        int e = device.disconnectDevice();
        if (e){
            Tools::debugLog("@@@@@ PrintManager_Slpa100::stop error " + QString::number(e));
        }
        started = false;
    }
}


QString PrintManager_Slpa100::getVersion()
{
    return QString::number(device.getPrinterVersion());
}

void PrintManager_Slpa100::feed()
{
    Tools::debugLog("@@@@@ PrintManager_Slpa100::feed");
    device.feed();
}

bool PrintManager_Slpa100::isStateError(uint16_t s)
{
    bool b0 = isPMFlag(s, 0);
    bool b1 = isPMFlag(s, 1);
    bool b2 = isPMFlag(s, 2);
    bool b3 = isPMFlag(s, 3);
    bool b6 = isPMFlag(s, 6);
    bool b8 = isPMFlag(s, 8);
    return !b0 || !b2 || b3 || b8 || (b1 && b6);
}

QString PrintManager_Slpa100::getErrorDescription(const int e)
{
    switch(e)
    {
    case 0: return "Ошибок нет";
    case 1003: return "Нет бумаги! Установите новый рулон!";
    case 1004: return "Закройте головку принтера!";
    case 1005: return "Снимите этикетку!";
    case 1006: return "Этикетка не спозиционирована! Нажмите клавишу промотки!";
    case 1007: return "Ошибка принтера!";
    case 1008: return "Ошибка памяти принтера!";
    case 1009: return "Неверный штрихкод";
    }
    return device.errorDescription(e);
}

int PrintManager_Slpa100::print(QImage image){
    return device.print(image);
}

void PrintManager_Slpa100::onPMStatusChanged(uint16_t s)
{
    Tools::debugLog(QString("@@@@@ EquipmentManager::onPMStatusChanged %1b").arg(QString::number(s, 2)));

    bool b0 = isPMFlag(s, 0);
    bool b1 = isPMFlag(s, 1);
    bool b2 = isPMFlag(s, 2);
    bool b3 = isPMFlag(s, 3);
    bool b6 = isPMFlag(s, 6);
    bool b8 = isPMFlag(s, 8);

    ControlParam param = ControlParam_None;
    int e = 0;
    if(!b0 || b1 || !b2 || b3 || b6 || b8)
    {
        param = ControlParam_PrintError;
        if(!b0 && isPMFlag(status, 0) != b0) e = 1003;
        if(!b2 && isPMFlag(status, 2) != b2) e = 1006;
        if(b3 && isPMFlag(status, 3) != b3) e = 1004;
        if(b8 && isPMFlag(status, 8) != b8) e = 1008;
        if(b1 && b6 && isPMFlag(status, 1) != b1) e = 1005;
    }
    else if(isStateError(status) && errorCode == 0) param = ControlParam_PrintError; // Ошибка исчезла

    status = s;
    if(param != ControlParam_None) emit paramChanged(param, e);
}

void PrintManager_Slpa100::onPMErrorStatusChanged(int e)
{
    Tools::debugLog(QString("@@@@@ EquipmentManager::onPMErrorStatusChanged %1").arg(QString::number(e)));
    if (errorCode != e)
    {
        errorCode = e;
        emit paramChanged(ControlParam_PrintError, e);
    }
}


