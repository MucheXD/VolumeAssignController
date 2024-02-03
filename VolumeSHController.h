#pragma once

#include <QtWidgets/QMainWindow>
#include <QFile>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QProcess>
#include <QRegularExpression>
#include "ui_VolumeSHController.h"

struct ConfigData
{
    QString configName;
    QString useChar;
    int32_t diskIndex;
    int32_t volumeIndex;
};

class VolumeSHController : public QMainWindow
{
    Q_OBJECT

public:
    VolumeSHController(QWidget *parent = nullptr);
    ~VolumeSHController();

private:
    std::vector<ConfigData> configs;
    int32_t defaultConfigIndex;
    bool disableWarning = false;
    QProcess* diskpartProcess;

    void readConfig();
    void refreshUi();
    void updateIndexs();
    void buildDefaultConfig();
    void saveConfig();
    void changeConfig();
    void openConfig();

    void runDiskpart();
    void readOutput();
    void diskpartFinished();
    void diskpartStarted();
    void assignVolume();
    void checkAssignment();
    void setIsAssign(bool isAssigned);

private:
    Ui::VolumeSHControllerClass ui;
};
