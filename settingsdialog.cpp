#include "settingsdialog.h"
#include "ui_settingsdialog.h"
#include <QFile>
#include <QMessageBox>
#include <QTextStream>
SettingsDialog::SettingsDialog(const QString &conffile, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog), configfile(conffile)
{
    ui->setupUi(this);
    loadmodellist();
    QMapIterator<int,QString> it(modellist);
    bool oldState = ui->rotator_model->blockSignals(true);
    while (it.hasNext()) {
        it.next();
        ui->rotator_model->addItem("["+QString::number(it.key())+"] "+it.value(),it.key());
    }
    ui->rotator_model->blockSignals(oldState);
    oldState = ui->serial_speed->blockSignals(true);
    ui->serial_speed->addItems({"300", "1200", "2400", "4800", "9600", "19200", "38400", "57600", "115200"});
    ui->serial_speed->setCurrentIndex(4);
    ui->serial_speed->blockSignals(oldState);
    oldState = ui->parity->blockSignals(true);
    ui->parity->addItems({"None", "Odd", "Even", "Mark", "Space"});
    ui->parity->blockSignals(oldState);
    oldState = ui->handshake->blockSignals(true);
    ui->handshake->addItems({"None", "XONXOFF", "Hardware"});
    ui->handshake->blockSignals(oldState);
    try {
        rotator=QFile::exists(configfile)?new QHamRotator(configfile):new QHamRotator();
    }
    catch (const RotException& e) {
        QMessageBox::information(this,tr("Error"),tr(e.what()));
    }
    this->reloadUI();
}

void SettingsDialog::reloadUI()
{
    try {
        bool oldState = ui->rotator_model->blockSignals(true);
        ui->rotator_model->setCurrentIndex(ui->rotator_model->findData(rotator->getModel()));
        ui->rotator_model->blockSignals(oldState);

        oldState = ui->serial_settings->blockSignals(true);
        ui->serial_settings->setEnabled(rotator->caps->port_type==RIG_PORT_SERIAL);
        ui->serial_settings->blockSignals(oldState);

        oldState = ui->poll_interval->blockSignals(true);
        ui->poll_interval->setValue(rotator->getPollingInterval());
        ui->poll_interval->blockSignals(oldState);

        oldState = ui->port_path->blockSignals(true);
        ui->port_path->setText(rotator->getConf("rot_pathname"));
        ui->port_path->blockSignals(oldState);

        oldState = ui->write_delay->blockSignals(true);
        ui->write_delay->setValue(rotator->getConf("write_delay").toInt());
        ui->write_delay->blockSignals(oldState);

        oldState = ui->post_write_delay->blockSignals(true);
        ui->post_write_delay->setValue(rotator->getConf("post_write_delay").toInt());
        ui->post_write_delay->blockSignals(oldState);

        oldState = ui->timeout->blockSignals(true);
        ui->timeout->setValue(rotator->getConf("timeout").toInt());
        ui->timeout->blockSignals(oldState);

        oldState = ui->retry->blockSignals(true);
        ui->retry->setValue(rotator->getConf("retry").toInt());
        ui->retry->blockSignals(oldState);

        oldState = ui->min_az->blockSignals(true);
        ui->min_az->setValue(QLocale().toDouble(rotator->getConf("min_az")));
        ui->min_az->blockSignals(oldState);

        oldState = ui->max_az->blockSignals(true);
        ui->max_az->setValue(QLocale().toDouble(rotator->getConf("max_az")));
        ui->max_az->blockSignals(oldState);

        oldState = ui->min_el->blockSignals(true);
        ui->min_el->setValue(QLocale().toDouble(rotator->getConf("min_el")));
        ui->min_el->blockSignals(oldState);

        oldState = ui->max_el->blockSignals(true);
        ui->max_el->setValue(QLocale().toDouble(rotator->getConf("max_el")));
        ui->max_el->blockSignals(oldState);

        if (rotator->caps->port_type==RIG_PORT_SERIAL)
        {
            oldState = ui->serial_speed->blockSignals(true);
            ui->serial_speed->setCurrentIndex(ui->serial_speed->findText(rotator->getConf("serial_speed")));
            ui->serial_speed->blockSignals(oldState);

            oldState = ui->data_bits->blockSignals(true);
            ui->data_bits->setValue(rotator->getConf("data_bits").toInt());
            ui->data_bits->blockSignals(oldState);

            oldState = ui->stop_bits->blockSignals(true);
            ui->stop_bits->setValue(rotator->getConf("stop_bits").toInt());
            ui->stop_bits->blockSignals(oldState);

            oldState = ui->parity->blockSignals(true);
            ui->parity->setCurrentIndex(ui->parity->findText(rotator->getConf("serial_parity")));
            ui->parity->blockSignals(oldState);

            oldState = ui->handshake->blockSignals(true);
            ui->handshake->setCurrentIndex(ui->handshake->findText(rotator->getConf("serial_handshake")));
            ui->handshake->blockSignals(oldState);
        }
    }
    catch (const RotException& e) {
        QMessageBox::information(this,tr("Error"),tr(e.what()));
    }
}

TModelList mlist;

static int hash_model_list(const struct rot_caps *caps, void *)
{
    mlist.insert(caps->rot_model,QString(caps->mfg_name) + " " + QString(caps->model_name));
    return 1;  /* !=0, we want them all ! */
}

void SettingsDialog::loadmodellist()
{
    int status = rot_load_all_backends();
    if (status != RIG_OK)
    {
        QMessageBox::information(this, tr("Error"), tr(rigerror(status)));
        return;
    }
    status = rot_list_foreach(hash_model_list, NULL);
    if (status != RIG_OK)
    {
        QMessageBox::information(this, tr("Error"), tr(rigerror(status)));
        return;
    }
    modellist = mlist; mlist.clear();
    return;
}

SettingsDialog::~SettingsDialog()
{
    delete rotator;
    delete ui;
}

void SettingsDialog::on_rotator_model_currentIndexChanged(int index)
{
    delete rotator;
    try {
        rotator = new QHamRotator((rot_model_t) ui->rotator_model->itemData(index).toInt());
    }
    catch (const RotException& e) {
        QMessageBox::information(this,tr("Error"),tr(e.what()));
    }
    this->reloadUI();
}

void SettingsDialog::on_port_path_editingFinished()
{
    try {
        rotator->setConf("rot_pathname",ui->port_path->text());
    }
    catch (const RotException& e) {
        QMessageBox::information(this,tr("Error"),tr(e.what()));
    }
}

void SettingsDialog::on_poll_interval_valueChanged(int arg1)
{
    try {
        rotator->setPollingInterval(arg1);
    }
    catch (const RotException& e) {
        QMessageBox::information(this,tr("Error"),tr(e.what()));
    }
}


void SettingsDialog::on_write_delay_valueChanged(const QString &arg1)
{
    try {
        rotator->setConf("write_delay",arg1);
    }
    catch (const RotException& e) {
        QMessageBox::information(this,tr("Error"),tr(e.what()));
    }
}

void SettingsDialog::on_post_write_delay_valueChanged(const QString &arg1)
{
    try {
        rotator->setConf("post_write_delay",arg1);
    }
    catch (const RotException& e) {
        QMessageBox::information(this,tr("Error"),tr(e.what()));
    }
}

void SettingsDialog::on_timeout_valueChanged(const QString &arg1)
{
    try {
        rotator->setConf("timeout",arg1);
    }
    catch (const RotException& e) {
        QMessageBox::information(this,tr("Error"),tr(e.what()));
    }
}

void SettingsDialog::on_retry_valueChanged(const QString &arg1)
{
    try {
        rotator->setConf("retry",arg1);
    }
    catch (const RotException& e) {
        QMessageBox::information(this,tr("Error"),tr(e.what()));
    }
}

void SettingsDialog::on_min_az_valueChanged(const QString &arg1)
{
    try {
        rotator->setConf("min_az",arg1);
    }
    catch (const RotException& e) {
        QMessageBox::information(this,tr("Error"),tr(e.what()));
    }
}

void SettingsDialog::on_max_az_valueChanged(const QString &arg1)
{
    try {
        rotator->setConf("max_az",arg1);
    }
    catch (const RotException& e) {
        QMessageBox::information(this,tr("Error"),tr(e.what()));
    }
}

void SettingsDialog::on_min_el_valueChanged(const QString &arg1)
{
    try {rotator->setConf("min_el",arg1);
    }
    catch (const RotException& e) {
        QMessageBox::information(this,tr("Error"),tr(e.what()));
    }
}

void SettingsDialog::on_max_el_valueChanged(const QString &arg1)
{
    try {
        rotator->setConf("max_el",arg1);
    }
    catch (const RotException& e) {
        QMessageBox::information(this,tr("Error"),tr(e.what()));
    }
}

void SettingsDialog::on_serial_speed_currentIndexChanged(int index)
{
    try {
        if (rotator->caps->port_type==RIG_PORT_SERIAL)
        {
            rotator->setConf("serial_speed",ui->serial_speed->itemText(index));
        }
    }
    catch (const RotException& e) {
        QMessageBox::information(this,tr("Error"),tr(e.what()));
    }
}

void SettingsDialog::on_data_bits_valueChanged(const QString &arg1)
{
    try {
        if (rotator->caps->port_type==RIG_PORT_SERIAL)
        {
            rotator->setConf("data_bits",arg1);
        }
    }
    catch (const RotException& e) {
        QMessageBox::information(this,tr("Error"),tr(e.what()));
    }
}

void SettingsDialog::on_stop_bits_valueChanged(const QString &arg1)
{
    try {
        if (rotator->caps->port_type==RIG_PORT_SERIAL)
        {
            rotator->setConf("stop_bits",arg1.toStdString().c_str());
        }
    }
    catch (const RotException& e) {
        QMessageBox::information(this,tr("Error"),tr(e.what()));
    }
}

void SettingsDialog::on_parity_currentIndexChanged(int index)
{
    try {
        if (rotator->caps->port_type==RIG_PORT_SERIAL)
        {
            rotator->setConf("parity",ui->parity->itemText(index).toStdString().c_str());
        }
    }
    catch (const RotException& e) {
        QMessageBox::information(this,tr("Error"),tr(e.what()));
    }
}

void SettingsDialog::on_handshake_currentIndexChanged(int index)
{
    try {
        if (rotator->caps->port_type==RIG_PORT_SERIAL)
        {
            rotator->setConf("handshake",ui->handshake->itemText(index).toStdString().c_str());
        }
    }
    catch (const RotException& e) {
        QMessageBox::information(this,tr("Error"),tr(e.what()));
    }
}

void SettingsDialog::on_buttonBox_accepted()
{
    try {
        rotator->SaveSettings(configfile);
    }
    catch (const RotException& e) {
        QMessageBox::information(this,tr("Error"),tr(e.what()));
    }
}
