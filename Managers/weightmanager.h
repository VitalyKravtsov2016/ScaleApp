#ifndef WEIGHTMANAGER_H
#define WEIGHTMANAGER_H

#include <QString>
#include <QObject>
#include "appmanager.h"

#define WM_MESSAGE_DEMO "\nДемо-режим весового модуля"
#define WM_MESSAGE_NONE "\nВесовой модуль не подключен"
#define WMPM_MESSAGE_NONE "\nОборудование не подключено"


class WeightManager: public QObject
{
public:
    WeightManager(QObject *parent = nullptr){};

    virtual int open(QString path) = 0;
    virtual int start() = 0;
    virtual int stop() = 0;
    virtual int getErrorCode() = 0;
    virtual QString getErrorText() = 0;
    virtual ScaleStatus getStatus() = 0;
    virtual EquipmentMode getMode() = 0;
    virtual QString getVersion() = 0;
    virtual QString getDescription() = 0;
    virtual QString getErrorDescription(const int e) = 0;
    virtual int setDateTime(const QDateTime& dt) = 0;
    virtual void setParam(const int) = 0;
};

#endif // WEIGHTMANAGER_H
