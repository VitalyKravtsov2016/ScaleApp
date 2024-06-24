#ifndef WEIGHTMANAGER_OPOSCALE_H
#define WEIGHTMANAGER_OPOSCALE_H

#include "constants.h"
#include "weightmanager.h"
#include "oposcalesdk.h"

class WeightManager_OpoScale: public WeightManager
{
    Q_OBJECT

public:
    WeightManager_OpoScale(QObject *parent = nullptr){};

    int open(QString path);
    int start();
    int stop();
    QString getVersion();
    QString getDescription();
    ScaleStatus getStatus();
    void setParam(const int);
    int setDateTime(const QDateTime& dt);
    EquipmentMode getMode(){ return mode; }
    int getErrorCode() { return errorCode; }
    QString getErrorText() { return errorText;}
    QString getErrorDescription(const int e);
private:
    OpoScaleSDK device;
    ScaleStatus status;
    QString deviceName = "/dev/ttyS0";
    int errorCode = 0;
    QString errorText = "";
    bool started = false;
    EquipmentMode mode = EquipmentMode_None;

    void clearError();
    int readConnectParam(const QString &filename, const QString &param, QString &uri);

signals:
    void paramChanged(const int, const int);

};


#endif // WEIGHTMANAGER_OPOSCALE_H
