#ifndef WEIGHTMANAGER_OPO_H
#define WEIGHTMANAGER_OPO_H

#include <QTimer>
#include "constants.h"
#include "weightmanager.h"
#include "oposcalesdk.h"

class WeightManager_Opo: public WeightManager
{
    Q_OBJECT

public:
    WeightManager_Opo(QObject *parent = nullptr){};

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
    QTimer timer;
    OpoScaleSDK device;
    ScaleStatus status;
    QString deviceName = "ttyS0";
    int errorCode = 0;
    QString errorText = "";
    bool started = false;
    EquipmentMode mode = EquipmentMode_None;

    void clearError();
    int readConnectParam(const QString &filename, const QString &param, QString &uri);
    void onWeightChanged(const ::OnePlusOneAndroidSDK::ScalesOS::WeightInfo& arg1);

public slots:
    void onTimer();

signals:
    void paramChanged(const int, const int);

};


#endif // WEIGHTMANAGER_OPO_H
