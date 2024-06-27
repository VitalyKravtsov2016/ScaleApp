#ifndef PRINTMANAGER_SLPA100_H
#define PRINTMANAGER_SLPA100_H

#include <QString>
#include "Slpa100u.h"
#include "appmanager.h"
#include "printmanager.h"

class PrintManager_Slpa100: public PrintManager
{
    Q_OBJECT
public:
    PrintManager_Slpa100();

    int open(QString path);
    int start(AppManager* appManager);
    int stop();
    void feed();
    bool isError() { return errorCode != 0 || isStateError(status); }
    bool isStarted() { return started;};
    int print(QImage image);
    QString getVersion();
    EquipmentMode getMode() { return mode; };
    bool isStateError(uint16_t s);
    QString getErrorDescription(const int e);
    int getErrorCode() { return errorCode; }
    QString getErrorText() { return getErrorDescription(errorCode); }

private:
    Slpa100u device;
    bool started = false;
    int errorCode = 0;
    int status = 0;
    QString uri;
    EquipmentMode mode = EquipmentMode_None;
public:

signals:
    void paramChanged(const int, const int);

public slots:
    void onPMStatusChanged(uint16_t);
    void onPMErrorStatusChanged(int);
};

#endif // PRINTMANAGER_SLPA100_H
