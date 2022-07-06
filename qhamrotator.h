#ifndef QHAMROTATOR_H
#define QHAMROTATOR_H

#include <QString>
#include <QStringList>
#include <hamlib/rotator.h>
#include <QException>
typedef QPair<azimuth_t,elevation_t> TRotatorPosition;
class QHamRotator
{
public:
    QHamRotator();
    QHamRotator(rot_model_t r_model);
    QHamRotator(QString filename);
    QHamRotator(QHamRotator &r);
    virtual ~QHamRotator();
    virtual QHamRotator *clone();
    const struct rot_caps *caps;
    QStringList get_cfg_params() const;
    rot_model_t getModel() const;
    int getPollingInterval() const;
    void setPollingInterval(const int &i);
    bool isOpened() const;
    bool LoadSettings(QString filename);
    bool SaveSettings(QString filename);
    QString getConf(const QString &pname);
    void setConf(const QString &pname, const QString &val);
    void open();
    void close();
    void setPosition(azimuth_t az, elevation_t el);
    void getPosition(azimuth_t& az, elevation_t& el);
    void setPosition(TRotatorPosition pos);
    TRotatorPosition getPosition();
    void stop();
    void park();
    void reset(rot_reset_t reset);
    void move(int direction, int speed);
private:
    void list_params();
    QStringList cfg_params;
    rot_model_t rot_model;
    int polling_interval;
    bool portopened;
    ROT *theRot;
};

class RotException: public QException
{
public:
    const char* message;
    int errorno;

    RotException(const char *msg, int err) noexcept
        : message(msg), errorno(err)
    {}

    RotException(int err) noexcept
        : message(rigerror(err)), errorno(err)
    {}

    RotException(const char *msg) noexcept
        : message(msg), errorno(-RIG_EINTERNAL)
    {}

    RotException(const RotException &re) noexcept
        : message(re.message), errorno(re.errorno)
    {}

    virtual ~RotException() noexcept override
    {}

    virtual void raise() const override {throw *this;}

    virtual RotException *clone() const override {return new RotException(*this);}

    virtual const char* what() const noexcept override
    {
        return message;
    }

    virtual const char *classname() const
    {
        return "RotException";
    }
};


#endif // QHAMROTATOR_H
