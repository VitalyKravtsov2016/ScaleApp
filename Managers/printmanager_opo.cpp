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
            for (int i=1;i<10;i++)
            {
                errorCode = device.GetStatus();
                if (errorCode == 0) break;
            }

        } else
        {
            errorCode = -1;
        }
    }
    Tools::debugLog("@@@@@ PrintManager_Opo::start: " + errorCode);
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
    Tools::debugLog("@@@@@ PrintManager_Opo::stop: 0");
    return 0;
}

int PrintManager_Opo::print(QImage image)
{
    Tools::debugLog("@@@@@ PrintManager_Opo::print");
    image.invertPixels(QImage::InvertRgb);
    QBitmap bitmap = QBitmap::fromImage(image, Qt::MonoOnly);
    device.PrintLabelBitmap(bitmap);
    Tools::debugLog("@@@@@ PrintManager_Opo::print=0");
    return 0;
}

QString PrintManager_Opo::getVersion()
{
    Tools::debugLog("@@@@@ PrintManager_Opo::getVersion");
    return "1.0";
}

bool PrintManager_Opo::isStateError(uint16_t s)
{
    Tools::debugLog("@@@@@ PrintManager_Opo::isStateError=" + s);
    return s != 0;
}

QString PrintManager_Opo::getErrorDescription(const int e)
{

    Tools::debugLog("@@@@@ PrintManager_Opo::getErrorDescription");
    QString text = "";
    switch (e)
    {
    case -1: text = "Ошибка открытия устройства";
    default: text = device.getStatusText(e);
    }
    Tools::debugLog("@@@@@ PrintManager_Opo::getErrorDescription=" + text);
    return text;
}
