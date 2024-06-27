#include <QDebug>
#include <QTimer>
#include <QDateTime>
#include "tools.h"
#include "equipmentmanager.h"
#include "transactiondbtable.h"
#include "productdbtable.h"
#include "database.h"
#include "users.h"
#include "Label/labelcreator.h"
#include "appmanager.h"
#include "weightmanager.h"
#include "weightmanager_demo.h"
#include "weightmanager_wm100.h"
#include "weightmanager_opo.h"
#include "printmanager.h"
#include "printmanager_opo.h"
#include "printmanager_demo.h"
#include "printmanager_slpa100.h"

EquipmentManager::EquipmentManager(AppManager *parent) : ExternalMessager(parent)
{
    Tools::debugLog("@@@@@ EquipmentManager::EquipmentManager ");
}

void EquipmentManager::create()
{
#define WM_MESSAGE_DEMO "\nДемо-режим весового модуля"
#define WM_MESSAGE_NONE "\nВесовой модуль не подключен"
#define PM_MESSAGE_DEMO "\nДемо-режим принтера"
#define PM_MESSAGE_NONE "\nПринтер не подключен"
#define WMPM_MESSAGE_NONE "\nОборудование не подключено"

    const QString path = EQUIPMENT_CONFIG_FILE;
    Tools::debugLog("@@@@@ EquipmentManager::create " + path);

    if(!QFile::exists(path))
    {
        showAttention(QString("Конфиг.файл %1 не найден. %2").arg(path, WMPM_MESSAGE_NONE));
        return;
    }
    QFile file(path);
    if(file.size() == 0)
    {
        showAttention(QString("Конфиг.файл %1 имеет размер 0. %2").arg(path, WMPM_MESSAGE_NONE));
        return;
    }

    createWM();
    int e1 = wm->open(path);
    if(e1 != 0) showAttention(wm->getErrorText());

    switch (wm->getMode())
    {
        case EquipmentMode_Demo:
        showAttention(WM_MESSAGE_DEMO);
        break;

        case EquipmentMode_None:
        showAttention(WMPM_MESSAGE_NONE);
        break;
    }

    createPM();
    int e2 = pm->open(path);
    if(e2 != 0) showAttention(pm->getErrorText());
    switch (pm->getMode())
    {
    case EquipmentMode_Demo:
        showAttention(WM_MESSAGE_DEMO);
        break;

    case EquipmentMode_None:
        showAttention(WMPM_MESSAGE_NONE);
        break;
    }
}

void EquipmentManager::createWM()
{
    Tools::debugLog("@@@@@ EquipmentManager::createWM ");
    if(wm == nullptr)
    {
        ScaleModel scaleModel = ScaleModel_Opo;
        // TODO: Add read scaleModel from settings

        switch (scaleModel)
        {
        case ScaleModel_Wm100: {
            WeightManager_wm100* wm100 = new WeightManager_wm100(nullptr);
            connect(wm100, &WeightManager_wm100::paramChanged, this, &EquipmentManager::wmParamChanged);
            wm = (WeightManager*) wm100;
            break;
        }

        case ScaleModel_Opo:
        {
            WeightManager_Opo* wm_opo = new WeightManager_Opo(nullptr);
            connect(wm_opo, &WeightManager_Opo::paramChanged, this, &EquipmentManager::wmParamChanged);
            wm = (WeightManager*) wm_opo;
            break;
        }
        default:
        {
            wm = new WeightManager_Demo();
        }
        }
    }
}

void EquipmentManager::createPM()
{
    Tools::debugLog("@@@@@ EquipmentManager::createPM ");
    if(pm == nullptr)
    {
        PrinterModel printerModel = PrinterModel_Opo;
        // TODO: Add read scaleModel from settings

        switch (printerModel){
        case PrinterModel_Slpa100:
        {
            PrintManager_Slpa100* pm1 = new PrintManager_Slpa100();
            connect(pm1, &PrintManager_Slpa100::paramChanged, this, &EquipmentManager::wmParamChanged);
            pm = pm1;
            break;
        }

        case PrinterModel_Opo:
        {
            PrintManager_Opo* pm2 = new PrintManager_Opo();
            connect(pm2, &PrintManager_Opo::paramChanged, this, &EquipmentManager::wmParamChanged);
            pm = pm2;
            break;
        }

        default:
            pm = new PrintManager_Demo();
        }
    }
}

void EquipmentManager::start()
{
    Tools::debugLog("@@@@@ EquipmentManager::start ");

    int e = wm->start();
    if(e) appManager->showAttention(QString("\nОшибка весового модуля %1: %2").arg(
            Tools::intToString(e), wm->getErrorDescription(e)));

    e = pm->start(appManager);
    if(e) appManager->showAttention(QString("\nОшибка принтера %1: %2").arg(
        Tools::intToString(e), pm->getErrorDescription(e)));
}

void EquipmentManager::stop()
{
    Tools::debugLog("@@@@@ EquipmentManager::stop ");
    wm->stop();
    pm->stop();
    Tools::debugLog("@@@@@ EquipmentManager::stop: OK");
}

void EquipmentManager::wmParamChanged(const int param, const int e)
{
    emit paramChanged(param, e);
}

void EquipmentManager::setSystemDateTime(const QDateTime& dt)
{
    wm->setDateTime(dt);
}

QString EquipmentManager::parseBarcode(const QString& barcodeTemplate, const QChar c, const QString& value)
{
    QString result;
    int n = 0;
    for(int i = 0; i < barcodeTemplate.length(); i++) if(barcodeTemplate.at(i) == c) n++;
    if(n > 0)
    {
        QString v = value;
        v = v.replace(QString("."), QString("")).replace(QString(","), QString(""));
        if(n >= v.length())
        {
            for(int i = 0; i < n - v.length(); i++) result += "0";
            result += v;
        }
    }
    Tools::debugLog(QString("@@@@@ EquipmentManager::parseBarcode %1 %2 %3 %4").arg(barcodeTemplate, value, result, Tools::intToString(n)));
    return result;
}

QString EquipmentManager::makeBarcode(const DBRecord& product, const QString& quantity, const QString& price, const QString& amount)
{
    QString result;
    QString barcodeTemplate = ProductDBTable::isPiece(product) ?
                appManager->settings->getStringValue(SettingCode_PrintLabelBarcodePiece) :
                appManager->settings->getStringValue(SettingCode_PrintLabelBarcodeWeight);
    if(barcodeTemplate.contains("P"))
    {
        result = ProductDBTable::isPiece(product) ?
                 appManager->settings->getStringValue(SettingCode_PrintLabelPrefixPiece) :
                 appManager->settings->getStringValue(SettingCode_PrintLabelPrefixWeight);
    }
    result += parseBarcode(barcodeTemplate, 'C', product[ProductDBTable::Code].toString()) +
              parseBarcode(barcodeTemplate, 'T', amount) +
              parseBarcode(barcodeTemplate, 'B', product[ProductDBTable::Barcode].toString()) +
              parseBarcode(barcodeTemplate, 'W', quantity);
    Tools::debugLog(QString("@@@@@ EquipmentManager::makeBarcode %1 %2").arg(barcodeTemplate, result));
    return result.length() != barcodeTemplate.length() ? "" : result;
}

int EquipmentManager::print(DataBase* db, const DBRecord& user, const DBRecord& product,
                         const QString& quantity, const QString& price, const QString& amount)
{
    Tools::debugLog("@@@@@ EquipmentManager::print");
    if (!pm->isStarted()) return 1007;

    QString barcode = makeBarcode(product, quantity, price, amount);
    if (barcode.isEmpty()) return 1009;

    quint64 dateTime = Tools::nowMsec();
    int labelNumber = 0; // todo
    int e = labelCreator->loadLabel(":/Labels/60x40.lpr"); // todo
    if (e == 0)
    {
        PrintData pd;
        pd.weight = quantity;
        pd.price = price;
        pd.cost = amount;
        pd.tare = product[ProductDBTable::Tare].toString();
        pd.barcode = barcode;
        pd.itemcode = product[ProductDBTable::Code].toString();
        pd.name = product[ProductDBTable::Name].toString();
        pd.shelflife = product[ProductDBTable::Shelflife].toString();
        pd.validity = ""; // todo
        pd.price2 = product[ProductDBTable::Price2].toString();
        pd.certificate = product[ProductDBTable::Certificate].toString();
        pd.message = db->getProductMessageById(product[ProductDBTable::MessageCode].toString());
        pd.shop = appManager->settings->getStringValue(SettingCode_ShopName);
        pd.operatorcode =  Tools::intToString(Users::getCode(user));
        pd.operatorname = Users::getName(user);
        pd.date = Tools::dateFromUInt(dateTime, DATE_FORMAT);
        pd.time = Tools::timeFromUInt(dateTime, TIME_FORMAT);
        pd.labelnumber = QString::number(labelNumber);
        pd.scalesnumber = appManager->settings->getStringValue(SettingCode_ScalesNumber),
        pd.picturefile = ""; // todo
        pd.textfile = ""; // todo
        QImage p = labelCreator->createImage(pd);
        e = pm->print(p);
    }
    if(e == 0)
    {
        TransactionDBTable* t = (TransactionDBTable*)(db->getTable(DBTABLENAME_TRANSACTIONS));
        if(t != nullptr)
        {
            DBRecord r = t->createRecord(
                        dateTime,
                        Users::getCode(user),
                        Tools::stringToInt(product[ProductDBTable::Code]),
                        labelNumber,
                        Tools::stringToDouble(quantity),
                        Tools::stringToInt(price),
                        Tools::stringToInt(amount));
            emit printed(r);
        }
        else Tools::debugLog("@@@@@ EquipmentManager::print ERROR (get Transactions Table)");
    }
    if(e != 0) showAttention("Ошибка печати " + getPMErrorDescription(e));
    else if(isPMDemoMode()) showAttention("Демо-печать");
    return e;
}

