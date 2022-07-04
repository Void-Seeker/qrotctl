#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <qwt_dial_needle.h>
#include <settingsdialog.h>
#include <QFile>
#include <QMessageBox>
MainWindow::MainWindow(const QString &conffile, QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), configfile(conffile),
      rotator(QFile::exists(conffile)?new QHamRotator(conffile):new QHamRotator()),
      polltimer(new QTimer), allowuiupdate(true)
{
    ui->setupUi(this);
    rotator_status = new QLabel(tr("Standby"));
    connection_status = new QLabel(tr("Disconnected"));
    ui->statusbar->addWidget(rotator_status);
    ui->statusbar->addPermanentWidget(connection_status);
    ui->az_compass->setNeedle(new QwtCompassMagnetNeedle(QwtCompassMagnetNeedle::TriangleStyle,ui->az_compass->palette().mid().color(),ui->az_compass->palette().dark().color()));
    QwtCompassScaleDraw* scale = new QwtCompassScaleDraw({{0,"N"},{22.5,"NNE"},{45,"NE"},{67.5,"ENE"},{90,"E"},{112.5,"ESE"},{135,"SE"},{157.5,"SSE"},{180,"S"},{202.5,"SSW"},{225,"SW"},{247.5,"WSW"},{270,"W"},{292.5,"WNW"},{315,"NW"},{337.5,"NNW"}});

    ui->az_compass->setScaleDraw(scale);
    ui->el_dial->setNeedle(new QwtDialSimpleNeedle(QwtDialSimpleNeedle::Ray));
    reloadUi();
    connect(polltimer, SIGNAL(timeout()), this, SLOT(poll_rotator()));
}

void MainWindow::reloadUi()
{
    double min_el = 0;
    double max_el = 180;
    double min_az = 0;
    double max_az = 360;
    try {
        if (rotator->isOpened()) target_pos = rotator->getPosition();
        polltimer->setInterval(rotator->getPollingInterval());
        min_el = QLocale().toDouble(rotator->getConf("min_el"));
        max_el = QLocale().toDouble(rotator->getConf("max_el"));
        min_az = QLocale().toDouble(rotator->getConf("min_az"));
        max_az = QLocale().toDouble(rotator->getConf("max_az"));
    }
    catch (const RotException& e) {
        QMessageBox::information(this,tr("Error"),tr(e.what()));
    }

    ui->el_dial->setMinScaleArc(min_el);
    ui->el_dial->setMaxScaleArc(max_el);
    ui->el_dial->setLowerBound(min_el);
    ui->el_dial->setUpperBound(max_el);
    ui->el_number->setMinimum(min_el);
    ui->el_number->setMaximum(max_el);
    ui->az_compass->setMinScaleArc(min_az);
    ui->az_compass->setMaxScaleArc((max_az>360)?360:max_az);
    ui->az_compass->setLowerBound(min_az);
    ui->az_compass->setUpperBound((max_az>360)?360:max_az);
    ui->az_number->setMinimum(min_az);
    ui->az_number->setMaximum(max_az);
}

MainWindow::~MainWindow()
{
    if (rotator->isOpened()) rotator->close();
    delete ui;
    delete rotator;
    delete polltimer;
    delete connection_status;
    delete rotator_status;
}

void MainWindow::on_actionExit_triggered()
{
    QApplication::quit();
}

void MainWindow::poll_rotator()
{
    TRotatorPosition pos;
    try {
        pos = rotator->getPosition();
    }
    catch (const RotException& e) {
        QMessageBox::information(this,tr("Error"),tr(e.what()));
    }
    if (pos != target_pos) {
        rotator_status->setText(tr("Moving to") + " (" + QString::number(target_pos.first) + "," + QString::number(target_pos.second) + ")");
    }
    else {
        rotator_status->setText(tr("Standby"));
    }
    if (allowuiupdate)
    {
        ui->az_compass->setValue((pos.first>360)?(pos.first-360):pos.first);
        ui->el_dial->setValue(pos.second);
    }
}

void MainWindow::on_az_compass_valueChanged(double value)
{
    ui->az_number->setValue(value);
}

void MainWindow::on_el_dial_valueChanged(double value)
{
    ui->el_number->setValue(value);
}

void MainWindow::on_az_compass_sliderPressed()
{
    allowuiupdate = false;
}

void MainWindow::on_el_dial_sliderPressed()
{
    allowuiupdate = false;
}

void MainWindow::on_az_compass_sliderReleased()
{
    allowuiupdate = true;
    try {
        if (rotator->isOpened())
        {
            target_pos.first = ui->az_compass->value();
            rotator->setPosition(target_pos);
        }
    }
    catch (const RotException& e) {
        QMessageBox::information(this,tr("Error"),tr(e.what()));
    }
}

void MainWindow::on_el_dial_sliderReleased()
{
    allowuiupdate = true;
    try {
        if (rotator->isOpened())
        {
            target_pos.second = ui->el_dial->value();
            rotator->setPosition(target_pos);
        }
    }
    catch (const RotException& e) {
        QMessageBox::information(this,tr("Error"),tr(e.what()));
    }
}

void MainWindow::on_actionConnect_to_rotator_triggered()
{
    try {
        if (!rotator->isOpened())
        {
            rotator->open();
            polltimer->start();
            target_pos = rotator->getPosition();
            QString path(rotator->getConf("rot_pathname"));
            if (!path.isEmpty()) path = " " + tr("on") + " " + path;
            connection_status->setText(tr("Connected to") + " " + QString(rotator->caps->mfg_name) + " " + QString(rotator->caps->model_name) + path );
        }
    }
    catch (const RotException& e) {
        polltimer->stop();
        rotator->close();
        QMessageBox::information(this,tr("Error"),tr(e.what()));
    }
}

void MainWindow::on_actionDisconnect_triggered()
{
    try {
        if (rotator->isOpened())
        {
            polltimer->stop();
            rotator->close();
            connection_status->setText(tr("Disconnected"));
        }
    }
    catch (const RotException& e) {
        QMessageBox::information(this,tr("Error"),tr(e.what()));
    }
}

void MainWindow::on_actionSettings_triggered()
{
     SettingsDialog *settings = new SettingsDialog(configfile,this);
     settings->setModal(true);
     if (settings->exec() == QDialog::DialogCode::Accepted)
     {
         try {
            delete rotator;
            rotator = new QHamRotator(configfile);
         }
         catch (const RotException& e) {
             QMessageBox::information(this,tr("Error"),tr(e.what()));
         }
         reloadUi();
     }
     delete settings;
}

void MainWindow::on_go_button_clicked()
{
    ui->az_compass->setValue(ui->az_number->value());
    ui->el_dial->setValue(ui->el_number->value());
    target_pos.first = ui->az_number->value(); target_pos.second = ui->el_number->value();
    try {
        if (rotator->isOpened())
        {
            rotator->setPosition(target_pos);
        }
    }
    catch (const RotException& e) {
        QMessageBox::information(this,tr("Error"),tr(e.what()));
    }
}
