#ifndef EQUIPMENTMANAGER_H
#define EQUIPMENTMANAGER_H

#include <QVariantList>
#include "wm100.h"
#include "constants.h"
#include "externalmessager.h"
#include "weightmanager.h"

class AppManager;
class DataBase;
class Slpa100u;
class LabelCreator;

class EquipmentManager : public ExternalMessager
{
    Q_OBJECT

public:
    EquipmentManager(AppManager*);
    ~EquipmentManager() {
        delete wm;
        removePM();
    }
    void create();
    void start();
    void stop();
    void setSystemDateTime(const QDateTime&);
    void pause(const bool v)
    {
        if (v) wm->stop(); else wm->start();
        pausePM(v);
    }

    // Weight Manager:
    bool isWM() {return (wm != nullptr) && (wm->getStatus().isStarted) && (wm->getMode() != EquipmentMode_None);}
    QString WMversion() const {return wm->getVersion();}
    void setWMParam(const int param) {wm->setParam(param);}
    QString getWMErrorDescription(const int code) const { return wm->getErrorDescription(code); }
    QString getWMDescription() { return wm->getDescription(); }
    ScaleStatus getStatus() { return wm->getStatus(); }

    // Print Manager:
    QString PMversion() const;
    int print(DataBase*, const DBRecord&, const DBRecord&, const QString&, const QString&, const QString&);
    bool isPMError() const { return PMErrorCode != 0 || isPMStateError(PMStatus); }
    bool isPMDemoMode() const { return PMMode == EquipmentMode_Demo; }
    QString getPMErrorDescription(const int) const;
    bool isPM();
    void feed();

private:
    QString makeBarcode(const DBRecord&, const QString&, const QString&, const QString&);
    QString parseBarcode(const QString&, const QChar, const QString&);
    QString getWMDescriptionNow();

    // Weight Manager:
    void createWM();
    //void removeWM();
    void stopWM();
    int startWM();
    void pauseWM(const bool);
    bool isWMFlag(Wm100Protocol::channel_status, int) const;
    bool isWMStateError(Wm100Protocol::channel_status) const;

    // Print Manager:
    void createPM();
    void removePM();
    int startPM();
    void stopPM();
    void pausePM(const bool);
    bool isPMFlag(uint16_t v, int shift) const { return (v & (0x00000001 << shift)) != 0; }
    bool isPMStateError(uint16_t) const;

    // Weight Manager:
    WeightManager* wm = nullptr;

    // Print Manager:
    Slpa100u* slpa = nullptr;
    LabelCreator* labelCreator = nullptr;
    bool isPMStarted = false;
    int PMErrorCode = 0;
    uint16_t PMStatus = 0;
    QString PMUri;
    EquipmentMode PMMode = EquipmentMode_None;

signals:
    void printed(const DBRecord&);
    void paramChanged(const int, const int);

public slots:
    void onPMStatusChanged(uint16_t);
    void onPMErrorStatusChanged(int);
    void wmParamChanged(const int param, const int e);
};

#endif // EQUIPMENTMANAGER_H
