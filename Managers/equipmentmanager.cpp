#include <QDebug>
#include <QTimer>
#include <QDateTime>
#include "tools.h"
#include "equipmentmanager.h"
#include "transactiondbtable.h"
#include "productdbtable.h"
#include "database.h"
#include "users.h"
#include "Slpa100u.h"
#include "Label/labelcreator.h"
#include "appmanager.h"
#include "weightmanager.h"
#include "weightmanager_wm100.h"
#include "weightmanager_oposcale.h"

EquipmentManager::EquipmentManager(AppManager *parent) : ExternalMessager(parent)
{
    Tools::debugLog("@@@@@ EquipmentManager::EquipmentManager ");
}

bool EquipmentManager::isPM()
{
    return slpa != nullptr && isPMStarted && PMMode != EquipmentMode_None;
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

    PMMode = EquipmentMode_None;
    removePM();
    /*
    if(!Tools::checkPermission("android.permission.READ_EXTERNAL_STORAGE"))
    {
        showAttention(QString("Нет разрешения для чтения конфиг.файла. %1").arg(WMPM_MESSAGE_NONE));
        return;
    }
    */
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
    Wm100 mgr;
    int e2 = mgr.readConnectParam(path, "PrinterUri", PMUri);
    if(e2 != 0) showAttention(getPMErrorDescription(e2));
    else
    {
        switch (slpa->checkUri(PMUri))
        {
        case Slpa100uProtocol::diDemo:
            PMMode = EquipmentMode_Demo;
            showAttention(PM_MESSAGE_DEMO);
            break;
        case Slpa100uProtocol::diNone:
            showAttention(PM_MESSAGE_NONE);
            break;
        default:
            PMMode = EquipmentMode_Ok;
            break;
        }
    }
    if(PMMode == EquipmentMode_None) removePM();
}

void EquipmentManager::start()
{
    Tools::debugLog("@@@@@ EquipmentManager::start ");
    int e = wm->start();
    if(e) appManager->showAttention(QString("\nОшибка весового модуля %1: %2").arg(
            Tools::intToString(e), wm->getErrorText()));

    startPM();
}

void EquipmentManager::stop()
{
    Tools::debugLog("@@@@@ EquipmentManager::stop ");
    wm->stop();
    stopPM();
}

void EquipmentManager::createWM()
{
    if(wm == nullptr)
    {
        Tools::debugLog("@@@@@ EquipmentManager::createWM ");
        if (false)
        {
            WeightManager_wm100* wm100 = new WeightManager_wm100(nullptr);
            connect(wm100, &WeightManager_wm100::paramChanged, this, &EquipmentManager::wmParamChanged);
            wm = (WeightManager*) wm100;
        } else
        {
            WeightManager_OpoScale* opoScale = new WeightManager_OpoScale(nullptr);
            connect(opoScale, &WeightManager_OpoScale::paramChanged, this, &EquipmentManager::wmParamChanged);
            wm = (WeightManager*) opoScale;
        }
    }
 }

void EquipmentManager::wmParamChanged(const int param, const int e)
{
    emit paramChanged(param, e);
}

void EquipmentManager::createPM()
{
    if(slpa == nullptr)
    {
        Tools::debugLog("@@@@@ EquipmentManager::createPM ");
        slpa = new Slpa100u(this);
        labelCreator = new LabelCreator(this);
        connect(slpa, &Slpa100u::printerErrorChanged, this, &EquipmentManager::onPMErrorStatusChanged);
        connect(slpa, &Slpa100u::printerStatusChanged, this, &EquipmentManager::onPMStatusChanged);
    }
}

int EquipmentManager::startPM()
{
    Tools::debugLog("@@@@@ EquipmentManager::startPM " + PMUri);
    createPM();
    int e = 0;
    if (slpa != nullptr && labelCreator != nullptr && !isPMStarted)
    {
        e = slpa->connectDevice(PMUri);
        isPMStarted = (e == 0);
        //slpa->blockSignals(!isPMStarted);
        if(isPMStarted)
        {
            //slpa->startPolling(200);
            e = slpa->setBrightness(appManager->settings->getIntValue(SettingCode_PrinterBrightness) + 8);
            if(e >= 0) e = slpa->setOffset(appManager->settings->getIntValue(SettingCode_PrintOffset) + 8);
            if(e >= 0) e = slpa->setPaper(appManager->settings->getIntValue(SettingCode_PrintPaper, true) == 0 ?
                                          Slpa100uProtocol::papertype::ptSticker :
                                          Slpa100uProtocol::papertype::ptRibbon);
            if(e >= 0) e = slpa->setSensor(appManager->settings->getBoolValue(SettingCode_PrintLabelSensor));
        }
    }
    if(e) showAttention(QString("\nОшибка принтера %1: %2").arg(
                Tools::intToString(e), getPMErrorDescription(e)));
    Tools::debugLog("@@@@@ EquipmentManager::startPM error " + QString::number(e));
    return e;
}

void EquipmentManager::stopPM()
{
    if (slpa != nullptr && isPMStarted)
    {
        Tools::debugLog("@@@@@ EquipmentManager::stopPM");
        slpa->blockSignals(true);
        slpa->stopPolling();
        int e = slpa->disconnectDevice();
        Tools::debugLog("@@@@@ EquipmentManager::stopPM error " + QString::number(e));
        isPMStarted = false;
    }
}

void EquipmentManager::removePM()
{
    Tools::debugLog("@@@@@ EquipmentManager::removePM");
    stopPM();
    if (slpa != nullptr)
    {
        disconnect(slpa, &Slpa100u::printerErrorChanged, this, &EquipmentManager::onPMErrorStatusChanged);
        disconnect(slpa, &Slpa100u::printerStatusChanged, this, &EquipmentManager::onPMStatusChanged);
        delete labelCreator;
        labelCreator = nullptr;
        delete slpa;
        slpa = nullptr;
    }
}

void EquipmentManager::pausePM(const bool v)
{
    Tools::debugLog("@@@@@ EquipmentManager::pausePM " + Tools::boolToIntString(v));
    if(slpa != nullptr && isPMStarted)
    {
        if(v) slpa->stopPolling();
        else slpa->startPolling(EQUIPMENT_POLLING_INTERVAL);
        slpa->blockSignals(v);
    }
}

void EquipmentManager::setSystemDateTime(const QDateTime& dt)
{
    wm->setDateTime(dt);
}

QString EquipmentManager::PMversion() const
{
    return (slpa == nullptr) ? "-" : QString::number(slpa->getPrinterVersion());
}

void EquipmentManager::feed()
{
    Tools::debugLog("@@@@@ EquipmentManager::feed");
    if(!isPMStarted || slpa == nullptr) return;
    slpa->feed();
}

bool EquipmentManager::isPMStateError(uint16_t s) const
{
    if(slpa == nullptr) return false;
    bool b0 = isPMFlag(s, 0);
    bool b1 = isPMFlag(s, 1);
    bool b2 = isPMFlag(s, 2);
    bool b3 = isPMFlag(s, 3);
    bool b6 = isPMFlag(s, 6);
    bool b8 = isPMFlag(s, 8);
    return !b0 || !b2 || b3 || b8 || (b1 && b6);
}

QString EquipmentManager::getPMErrorDescription(const int e) const
{
    switch(e)
    {
    case 0: return "Ошибок нет";
    case 1003: return "Нет бумаги! Установите новый рулон!";
    case 1004: return "Закройте головку принтера!";
    case 1005: return "Снимите этикетку!";
    case 1006: return "Этикетка не спозиционирована! Нажмите клавишу промотки!";
    case 1007: return "Ошибка принтера!";
    case 1008: return "Ошибка памяти принтера!";
    case 1009: return "Неверный штрихкод";
    }
    return slpa == nullptr ? "" : slpa->errorDescription(e);
}

void EquipmentManager::onPMStatusChanged(uint16_t s)
{
    Tools::debugLog(QString("@@@@@ EquipmentManager::onPMStatusChanged %1b").arg(QString::number(s, 2)));
    if(slpa == nullptr)
    {
        PMStatus = 0;
        return;
    }

    bool b0 = isPMFlag(s, 0);
    bool b1 = isPMFlag(s, 1);
    bool b2 = isPMFlag(s, 2);
    bool b3 = isPMFlag(s, 3);
    bool b6 = isPMFlag(s, 6);
    bool b8 = isPMFlag(s, 8);

    ControlParam param = ControlParam_None;
    int e = 0;
    if(!b0 || b1 || !b2 || b3 || b6 || b8)
    {
        param = ControlParam_PrintError;
        if(!b0 && isPMFlag(PMStatus, 0) != b0) e = 1003;
        if(!b2 && isPMFlag(PMStatus, 2) != b2) e = 1006;
        if(b3 && isPMFlag(PMStatus, 3) != b3) e = 1004;
        if(b8 && isPMFlag(PMStatus, 8) != b8) e = 1008;
        if(b1 && b6 && isPMFlag(PMStatus, 1) != b1) e = 1005;
    }
    else if(isPMStateError(PMStatus) && PMErrorCode == 0) param = ControlParam_PrintError; // Ошибка исчезла

    PMStatus = s;
    if(param != ControlParam_None) emit paramChanged(param, e);
}

void EquipmentManager::onPMErrorStatusChanged(int e)
{
    Tools::debugLog(QString("@@@@@ EquipmentManager::onPMErrorStatusChanged %1").arg(QString::number(e)));
    if(slpa != nullptr && PMErrorCode != e)
    {
        PMErrorCode = e;
        emit paramChanged(ControlParam_PrintError, e);
    }
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
    if(!isPMStarted || slpa == nullptr) return 1007;

    QString barcode = makeBarcode(product, quantity, price, amount);
    if (barcode.isEmpty()) return 1009;

    quint64 dateTime = Tools::nowMsec();
    int labelNumber = 0; // todo
    int e = labelCreator->loadLabel(":/Labels/60x40.lpr"); // todo
    //int e = slpa->printTest(100);
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
        e = slpa->print(p);
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
