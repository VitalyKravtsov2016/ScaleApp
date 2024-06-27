#include "printmanager_opo.h"

#include <QBitmap>
#include <QImage>


PrintManager_Opo::PrintManager_Opo() {}

int PrintManager_Opo::open(QString path){
    Tools::debugLog("@@@@@ PrintManager_Opo::open " + path);
    return 0;
}

int PrintManager_Opo::start(AppManager* appManager)
{
    Tools::debugLog("@@@@@ PrintManager_Opo::start");
    errorCode = 0;
    if (!started)
    {
        if (device.Open())
        {
            started = true;
            errorCode = device.GetStatus();

        } else
        {
            errorCode = -1;
        }
    }
    return errorCode;
}

int PrintManager_Opo::stop()
{
    Tools::debugLog("@@@@@ PrintManager_Opo::stop");
    if (!started)
    {
        started = false;
        device.Close();
    }
    return 0;
}

int PrintManager_Opo::print(QImage image)
{
    Tools::debugLog("@@@@@ PrintManager_Opo::print");
    QBitmap bitmap = QBitmap::fromImage(image, Qt::MonoOnly);
    device.PrintLabelBitmap(bitmap);
    return 0;
}

QString PrintManager_Opo::getVersion()
{
    Tools::debugLog("@@@@@ PrintManager_Opo::getVersion");
    return "1.0";
}

bool PrintManager_Opo::isStateError(uint16_t s)
{
    Tools::debugLog("@@@@@ PrintManager_Opo::getVersion");
    return s != 0;
}

QString PrintManager_Opo::getErrorDescription(const int e)
{
    Tools::debugLog("@@@@@ PrintManager_Opo::getErrorDescription");
    switch (e)
    {
    case -1: return "Ошибка открытия устройства";
    default: return device.getStatusText(e);
    }
}
