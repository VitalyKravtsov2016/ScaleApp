#include "printmanager_opo.h"

#include <QBitmap>
#include <QImage>



PrintManager_Opo::PrintManager_Opo() {}

int PrintManager_Opo::open(QString path){
    return 0;
}

int PrintManager_Opo::start(AppManager* appManager)
{
    if (!started)
    {
        if (!device.Open()){
            return -1;
        }
        errorCode = device.GetStatus();
        return errorCode;
    }
    return 0;
}

int PrintManager_Opo::stop(){
    if (!started)
    {
        device.Close();
    }
    return 0;
}

int PrintManager_Opo::print(QImage image)
{
    QBitmap bitmap;
    device.PrintLabelBitmap(bitmap);

}

QString PrintManager_Opo::getVersion(){
    return "1.0";
}

bool PrintManager_Opo::isStateError(uint16_t s)
{
    return s != 0;
}

QString PrintManager_Opo::getErrorDescription(const int e)
{
    device.getStatusText(e);
}
