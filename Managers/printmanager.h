#ifndef PRINTMANAGER_H
#define PRINTMANAGER_H

#include <QObject>
#include "constants.h"
#include "appmanager.h"


class PrintManager : public QObject
{
    Q_OBJECT
public:
    explicit PrintManager(QObject *parent = nullptr);

    virtual int open(QString path) = 0;
    virtual int start(AppManager* appManager) = 0;
    virtual int stop() = 0;
    virtual bool isError() = 0;
    virtual bool isStarted() = 0;
    virtual QString getVersion() = 0;
    virtual void feed() = 0;
    virtual int print(QImage image) = 0;
    virtual EquipmentMode getMode() = 0;
    virtual int getErrorCode() = 0;
    virtual QString getErrorText() = 0;
    virtual QString getErrorDescription(const int e) = 0;


signals:
};

#endif // PRINTMANAGER_H
