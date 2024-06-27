#ifndef PRINTMANAGER_DEMO_H
#define PRINTMANAGER_DEMO_H

#include <QString>
#include "appmanager.h"
#include "printmanager.h"
#include "constants.h"

class PrintManager_Demo: public PrintManager
{
public:
    PrintManager_Demo();

    int open(QString path) { return 0;};
    int start(AppManager* appManager){ started = true; return 0;};
    int stop() { started = false; return 0; };
    bool isError(){ return false; };
    bool isStarted() { return started; };
    QString getVersion() { return "1.0"; };
    void feed() {};
    int print(QImage image) { return 0; };
    EquipmentMode getMode() { return EquipmentMode_Ok; };
    int getErrorCode(){ return 0; };
    QString getErrorText() { return ""; };
    QString getErrorDescription(const int e) { return ""; };
private:
    bool started = false;
};

#endif // PRINTMANAGER_DEMO_H
