#ifndef PRINTMANAGER_OPO_H
#define PRINTMANAGER_OPO_H

#include <QString>
#include "appmanager.h"
#include "printmanager.h"
#include "constants.h"
#include "opolabelprinter.h"

class PrintManager_Opo: public PrintManager
{
    Q_OBJECT
public:
    PrintManager_Opo();

    int open(QString path);
    int start(AppManager* appManager);
    int stop();
    void feed() {};
    bool isError() { return errorCode != 0; }
    bool isStarted() { return started;};
    int print(QImage image);
    QString getVersion();
    EquipmentMode getMode() { return mode; };
    QString getErrorDescription(const int e);
    int getErrorCode() { return errorCode; }
    QString getErrorText() { return getErrorDescription(errorCode); }

private:
    bool isStateError(uint16_t s);
    OpoLabelPrinter device;
    bool started = false;
    int errorCode = 0;
    int status = 0;
    QString uri;
    EquipmentMode mode = EquipmentMode_Ok;
public:

signals:
    void paramChanged(const int, const int);
};

#endif // PRINTMANAGER_OPO_H
