#ifndef WEIGHTMANAGER_DEMO_H
#define WEIGHTMANAGER_DEMO_H

#include <QObject>
#include "weightmanager.h"

class WeightManager_Demo: public WeightManager
{
public:
    WeightManager_Demo(){};

    int open(QString path){ return 0; };
    int start() { started = true; return 0;};
    int stop() { started = false; return 0; };
    int getErrorCode() { return 0; };
    QString getErrorText() { return "Ошибок нет"; };
    ScaleStatus getStatus()
    {
        ScaleStatus status;
        status.weight = 123.45;
        status.tare = 0;
        status.isStarted = started;
        status.isError = false;
        status.isOverloaded = false;
        status.isWeightFixed = true;
        status.isWeightZero = false;
        status.isTareSet = false;
        status.isDemoMode = false;
        return status;
    };
    EquipmentMode getMode() {return EquipmentMode_Ok; }
    QString getVersion() { return "1.0"; };
    QString getDescription() { return "Demo scales"; };
    QString getErrorDescription(const int e) { return "Ошибок нет";};
    int setDateTime(const QDateTime& dt) {};
    void setParam(const int) {};
private:
    bool started = false;
};

#endif // WEIGHTMANAGER_DEMO_H
