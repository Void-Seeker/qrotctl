#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QMap>
#include "qhamrotator.h"

typedef QMap<int,QString> TModelList;
namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(const QString &conffile, QWidget *parent = nullptr);
    ~SettingsDialog();

private slots:

    void on_rotator_model_currentIndexChanged(int index);

    void on_port_path_editingFinished();

    void on_poll_interval_valueChanged(int arg1);


    void on_write_delay_valueChanged(const QString &arg1);

    void on_post_write_delay_valueChanged(const QString &arg1);

    void on_timeout_valueChanged(const QString &arg1);

    void on_retry_valueChanged(const QString &arg1);

    void on_min_az_valueChanged(const QString &arg1);

    void on_max_az_valueChanged(const QString &arg1);

    void on_min_el_valueChanged(const QString &arg1);

    void on_max_el_valueChanged(const QString &arg1);

    void on_serial_speed_currentIndexChanged(int index);

    void on_data_bits_valueChanged(const QString &arg1);

    void on_stop_bits_valueChanged(const QString &arg1);

    void on_parity_currentIndexChanged(int index);

    void on_handshake_currentIndexChanged(int index);

    void on_buttonBox_accepted();

private:
    Ui::SettingsDialog *ui;
    QString configfile;
    TModelList modellist;
    void loadmodellist();
    QHamRotator *rotator;
    QHamRotator *realrot;
    void reloadUI();
};

#endif // SETTINGSDIALOG_H
