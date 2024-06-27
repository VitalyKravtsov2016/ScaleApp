#ifndef EQUIPMENTMANAGER_H
#define EQUIPMENTMANAGER_H

#include <QVariantList>
#include "wm100.h"
#include "constants.h"
#include "externalmessager.h"
#include "weightmanager.h"
#include "printmanager.h"

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
        delete pm;
    }
    void create();
    void start();
    void stop();
    void setSystemDateTime(const QDateTime&);
    void pause(const bool v)
    {
        if (v) {
            wm->stop();
            pm->stop();
        }
        else {
            wm->start();
            pm->start(appManager);
        }
    }

    // Weight Manager:
    void createWM();
    bool isWM() {return (wm != nullptr) && (wm->getStatus().isStarted) && (wm->getMode() != EquipmentMode_None);}
    QString WMversion() const {return wm->getVersion();}
    void setWMParam(const int param) {wm->setParam(param);}
    QString getWMErrorDescription(const int code) const { return wm->getErrorDescription(code); }
    QString getWMDescription() { return wm->getDescription(); }
    ScaleStatus getStatus() { return wm->getStatus(); }

    // Print Manager:
    void createPM();
    bool isPMError() { return pm->isError(); }
    bool isPMDemoMode() { return pm->getMode() == EquipmentMode_Demo;}
    bool isPM(){return (pm != nullptr) && (pm->isStarted()) && (pm->getMode() != EquipmentMode_None);}
    QString PMversion() const { return pm->getVersion(); }
    int print(DataBase* db, const DBRecord& dbr1, const DBRecord&, const QString&, const QString&, const QString&);
    QString getPMErrorDescription(const int e) const { return pm->getErrorDescription(e); }
    void feed() {pm->feed(); };

private:
    QString makeBarcode(const DBRecord&, const QString&, const QString&, const QString&);
    QString parseBarcode(const QString&, const QChar, const QString&);

    // Print Manager:
    int startPM();
    void stopPM();
    bool isPMStateError(uint16_t) const;

    // Weight Manager:
    WeightManager* wm = nullptr;
    // Print Manager:
    PrintManager* pm = nullptr;
    LabelCreator* labelCreator = nullptr;

signals:
    void printed(const DBRecord&);
    void paramChanged(const int, const int);

public slots:
    void wmParamChanged(const int param, const int e);
};

#endif // EQUIPMENTMANAGER_H
