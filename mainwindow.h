#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "qhamrotator.h"
#include <QTimer>
#include <QLabel>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(const QString &conffile = "settings.yaml", QWidget *parent = nullptr);
    ~MainWindow();

private slots:

    void on_actionExit_triggered();

    void poll_rotator();

    void on_az_compass_valueChanged(double value);

    void on_el_dial_valueChanged(double value);

    void on_az_compass_sliderPressed();

    void on_el_dial_sliderPressed();

    void on_az_compass_sliderReleased();

    void on_el_dial_sliderReleased();

    void on_actionConnect_to_rotator_triggered();

    void on_actionDisconnect_triggered();

    void on_actionSettings_triggered();

    void on_go_button_clicked();

private:
    Ui::MainWindow *ui;
    QLabel *rotator_status;
    QLabel *connection_status;
    const QString &configfile;
    QHamRotator *rotator;
    QTimer *polltimer;
    bool allowuiupdate;
    void reloadUi();
    TRotatorPosition target_pos;

};
#endif // MAINWINDOW_H
