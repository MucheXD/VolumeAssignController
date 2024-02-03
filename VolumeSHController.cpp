#include "VolumeSHController.h"

VolumeSHController::VolumeSHController(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    connect(ui.cb_configChoose, &QComboBox::activated,
        this, &VolumeSHController::changeConfig);
    connect(ui.btn_assign, &QPushButton::clicked,
        this, &VolumeSHController::assignVolume);
    connect(ui.btn_editConfig, &QPushButton::clicked,
        this, &VolumeSHController::openConfig);
    readConfig();
    refreshUi();
    runDiskpart();
}

VolumeSHController::~VolumeSHController()
{
    diskpartProcess->kill();
    diskpartProcess->close();
}

void VolumeSHController::readConfig()
{
    QFile configFile;
    configFile.setFileName("config.mtd");
    if (!configFile.open(QIODevice::ReadWrite))
    {
        QMessageBox::critical(this, "ERROR", "Cannot open config file");
        exit(-1);
    }
    QJsonDocument jsonDoc;
    QJsonParseError jsonErr;
    jsonDoc = jsonDoc.fromJson(configFile.readAll(), &jsonErr);
    configFile.close();
    if (jsonErr.error != QJsonParseError::NoError)
    {
        if (QMessageBox::critical(this, "ERROR", "Illegal config, create new?",
            QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
        {
            buildDefaultConfig();
            saveConfig();
        }
        exit(-1);
    }
    defaultConfigIndex = jsonDoc.object().value("default").toInt();
    disableWarning = jsonDoc.object().value("disableWarning").toBool(false);
    QJsonArray configArray = jsonDoc.object().value("configs").toArray();

	for (auto nConfig : configArray)
	{
		const QJsonObject nObj = nConfig.toObject();
		ConfigData cfgData;
		cfgData.configName = nObj.value("name").toString();
        cfgData.useChar = nObj.value("char").toString();
        if (cfgData.useChar.isEmpty() || !cfgData.useChar.at(0).isLetter())
            cfgData.useChar = "";
		cfgData.diskIndex = nObj.value("disk").toInt();
		cfgData.volumeIndex = nObj.value("volume").toInt();
		configs.push_back(cfgData);
	}
}

void VolumeSHController::refreshUi()
{
    for (auto n : configs)
    {
        ui.cb_configChoose->addItem(n.configName);
    }
    if (defaultConfigIndex >= ui.cb_configChoose->count())
    {
        QMessageBox::warning(this, "WARNING", "Illegal default config, set to 0.");
        defaultConfigIndex = 0;
    }
    ui.cb_configChoose->setCurrentIndex(defaultConfigIndex);
    updateIndexs();
}

void VolumeSHController::updateIndexs()
{
    int a  = ui.cb_configChoose->currentIndex();
    ConfigData currentConfig = configs.at(ui.cb_configChoose->currentIndex());
    ui.l_diskIndex->setText(QString("%1").arg(currentConfig.diskIndex));
    ui.l_VolumeIndex->setText(QString("%1").arg(currentConfig.volumeIndex));
    if(!currentConfig.useChar.isEmpty())
        ui.l_diskLetter->setText(QString("%1").arg(currentConfig.useChar.at(0)));
    else
        ui.l_diskLetter->setText("×");
}

void VolumeSHController::buildDefaultConfig()
{
    configs.clear();
    ConfigData cfgData;
    cfgData.configName = "default";
    cfgData.diskIndex = 0;
    cfgData.volumeIndex = 0;
    defaultConfigIndex = 0;
    configs.push_back(cfgData);
}

void VolumeSHController::saveConfig()
{
    QFile configFile;
    configFile.setFileName("config.mtd");
    configFile.open(QIODevice::ReadWrite);
    QJsonArray configArray;
    for (auto n : configs)
    {
        QJsonObject nObj;
        nObj.insert("name",n.configName);
        nObj.insert("disk", n.diskIndex);
        nObj.insert("volume", n.volumeIndex);
        configArray.append(nObj);
    }
    QJsonObject rootObj;
    rootObj.insert("configs", configArray);
    rootObj.insert("default", defaultConfigIndex);
    QJsonDocument jsonDoc;
    jsonDoc.setObject(rootObj);
    configFile.write(jsonDoc.toJson());
    configFile.close();
}

void VolumeSHController::changeConfig()
{
    if (ui.cb_configChoose->count() == 0)
        return;
    ConfigData currentConfig = configs.at(ui.cb_configChoose->currentIndex());
    diskpartProcess->write(QString("select disk %1\n").arg(currentConfig.diskIndex).toStdString().c_str());
    diskpartProcess->write(QString("select volume %1\n").arg(currentConfig.volumeIndex).toStdString().c_str());
    ui.lb_isAssigned->setProperty("isValid", false);
    ui.lb_isAssigned->setText("?");
    ui.lb_isAssigned->setStyleSheet(ui.lb_isAssigned->styleSheet());
    updateIndexs();
    checkAssignment();
}

void VolumeSHController::openConfig()
{
    if (ui.btn_editConfig->text() == "Edit Config")
    {
        QProcess explorerProcess;
        explorerProcess.setWorkingDirectory(QApplication::applicationDirPath());
        explorerProcess.startDetached("explorer.exe", QStringList("config.mtd"));
        ui.btn_editConfig->setProperty("isHighlighted",true);
        ui.btn_editConfig->setStyleSheet(ui.lb_isAssigned->styleSheet());
        ui.btn_editConfig->setText("Reload Config");
    }
    else
    {
        configs.clear();
        ui.cb_configChoose->clear();
        readConfig();
        refreshUi();
        ui.btn_editConfig->setProperty("isHighlighted", false);
        ui.btn_editConfig->setStyleSheet(ui.lb_isAssigned->styleSheet());
        ui.btn_editConfig->setText("Edit Config");
    }
}

void VolumeSHController::runDiskpart()
{
    diskpartProcess = new QProcess(this);
    connect(diskpartProcess, &QProcess::readyReadStandardOutput,
        this, &VolumeSHController::readOutput);
    connect(diskpartProcess, &QProcess::finished,
        this, &VolumeSHController::diskpartFinished);
    connect(diskpartProcess, &QProcess::started,
        this, &VolumeSHController::diskpartStarted);
    diskpartProcess->start("diskpart.exe");
}

void VolumeSHController::readOutput()
{
    QString input = QString::fromLocal8Bit(diskpartProcess->readAllStandardOutput());
    qDebug() << input;
	QRegularExpression regExp("(?<=\\s{5})[0-9]\\s{5}");
	QRegularExpressionMatch regExpMatch = regExp.match(input);
    if (!regExpMatch.hasMatch())
    {
        return;
    }
    regExp.setPattern("(?<=\\s{5}[0-9]\\s{5})[A-Z]");
    regExpMatch = regExp.match(input);
	if (regExpMatch.hasMatch())
	{
		setIsAssign(true);
		ui.lb_isAssigned->setText(regExpMatch.captured());
	}
	else
	{
		setIsAssign(false);
		ui.lb_isAssigned->setText("×");
	}
}

void VolumeSHController::diskpartFinished()
{
    ui.statusBar->setProperty("isConnected", false);
    ui.statusBar->showMessage("Diskpart disconnected");
}

void VolumeSHController::diskpartStarted()
{
    ui.statusBar->setProperty("isConnected", true);
    ui.statusBar->showMessage("Diskpart connected");
    changeConfig();
}

void VolumeSHController::assignVolume()
{
    if (ui.btn_assign->text() == "Assign")
    {
        ConfigData currentConfig = configs.at(ui.cb_configChoose->currentIndex());
        ui.statusBar->showMessage(QString("Assign to volume %1").arg(currentConfig.volumeIndex));
        if (!currentConfig.useChar.isEmpty())
            diskpartProcess->write(QString("assign LETTER=%1\n").arg(currentConfig.useChar.at(0)).toLocal8Bit());
        else
            diskpartProcess->write("assign\n");
        checkAssignment();
    }
    else
    {
        if (ui.lb_isAssigned->text() == "C" && !disableWarning)
        {
            QMessageBox::warning(this, "Dangerous operation",
                "Most people use \"C\" as system partition.\nUnassigning system partition may be dangerous.\n\nUse [\"disableWarning\": true] in config root to prevent this warning.");
            return;
        }
        ConfigData currentConfig = configs.at(ui.cb_configChoose->currentIndex());
        ui.statusBar->showMessage(QString("Remove volume %1").arg(currentConfig.volumeIndex));
        diskpartProcess->write("remove\n");
        checkAssignment();
    }
}
    
void VolumeSHController::checkAssignment()
{
    diskpartProcess->write(QString("detail PARTITION\n").toStdString().c_str());
}

void VolumeSHController::setIsAssign(bool isAssigned)
{
    ui.lb_isAssigned->setProperty("isValid", true);
    ui.lb_isAssigned->setProperty("isTrue", isAssigned);
    ui.lb_isAssigned->setStyleSheet(ui.lb_isAssigned->styleSheet());
    if (isAssigned)
    {
        ui.btn_assign->setText("Unassign");
    }
    else
    {
        ui.btn_assign->setText("Assign");
    }
}
