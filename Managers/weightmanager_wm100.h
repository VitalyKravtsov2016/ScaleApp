#ifndef WEIGHTMANAGER_WM100_H
#define WEIGHTMANAGER_WM100_H

#include "weightmanager.h"
#include "Wm100Protocol.h"
#include "wm100.h"
#include "constants.h"
#include "externalmessager.h"
#include "appmanager.h"


class WeightManager_wm100: public WeightManager
{
    Q_OBJECT

public:
    WeightManager_wm100(QObject *parent = nullptr)
    {
    };

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
    void clearError();
    void setLastError(int e);
    bool isFlag(Wm100Protocol::channel_status, int);
    bool isStateError(Wm100Protocol::channel_status);

    Wm100 device;
    QString uri;
    int errorCode = 0;
    QString errorText = "";
    bool started = false;
    bool isSystemDateTime = false;
    Wm100Protocol::channel_status status = {0, 0.0, 0.0, 0};
    EquipmentMode mode = EquipmentMode_None;

signals:
    void paramChanged(const int, const int);

public slots:
    void onErrorStatusChanged(int);
    void onStatusChanged(Wm100Protocol::channel_status&);
};

#endif // WEIGHTMANAGER_WM100_H
