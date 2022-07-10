#include "qhamrotator.h"
#include <QFile>
#include <QTextStream>
#include <QRegularExpressionValidator>
#define CHECK_ROT(cmd) { int _result = cmd; if (_result != RIG_OK) \
                            throw RotException (_result); }

QStringList c_params;

static int hash_params(const struct confparams *cparams, void *)
{
    c_params.append(QString(cparams->name));
    return 1;  /* !=0, we want them all ! */
}

void QHamRotator::list_params()
{
    CHECK_ROT( rot_token_foreach(theRot, hash_params, NULL) );
    cfg_params = c_params; c_params.clear();
    return;
}

QHamRotator::QHamRotator():QHamRotator(ROT_MODEL_DUMMY) {}

QHamRotator::QHamRotator(rot_model_t r_model)
{
    rot_model=r_model;
    polling_interval=1000;
    portopened = false;
    theRot = rot_init(rot_model);
    if (!theRot)
        throw RotException ("Rotator initialization error");

    caps = theRot->caps;
    theRot->state.obj = (rig_ptr_t)this;
    list_params();
}

QHamRotator::QHamRotator(QString filename)
{
    portopened = false;
    QMap<QString,QString> config;
    QRegularExpressionValidator validator(QRegularExpression("\\w+:.*"));
    QFile file(filename);
    if(!file.open(QIODevice::ReadOnly)) {
        throw RotException(file.errorString().toStdString().c_str(),RIG_EIO);
    }

    QTextStream in(&file);

    while (!in.atEnd())
    {
       QString s = in.readLine().simplified(); int pos=0;
       if (validator.validate(s,pos) == QValidator::Acceptable)
       {
           QStringList param = s.split(":");
           config[param[0].simplified()] = param[1].simplified();
       }
    }

    file.close();
    rot_model = (config.contains("rot_model"))?config["rot_model"].toInt():RIG_MODEL_DUMMY;
    theRot = rot_init(rot_model);
    if (!theRot)
        throw RotException ("Rotator initialization error");

    caps = theRot->caps;
    theRot->state.obj = (rig_ptr_t)this;
    list_params();
    LoadSettings(filename);
}

QHamRotator::QHamRotator(QHamRotator &r):QHamRotator(r.getModel())
{

    polling_interval = r.polling_interval;

    for(QStringList::const_iterator it=cfg_params.begin();it!=cfg_params.end();it++)
    {
        setConf(*it,r.getConf(*it));
    }
}

QHamRotator::~QHamRotator()
{
    theRot->state.obj = NULL;
    rot_cleanup(theRot);
    caps = NULL;
}

QHamRotator *QHamRotator::clone()
{
    return new QHamRotator(*this);
}

QStringList QHamRotator::get_cfg_params() const
{
    return cfg_params;
}

rot_model_t QHamRotator::getModel() const
{
    return rot_model;
}

int QHamRotator::getPollingInterval() const
{
    return polling_interval;
}

bool QHamRotator::isOpened() const
{
    return portopened;
}
void QHamRotator::setPollingInterval(const int &i)
{
    polling_interval = i;
}
bool QHamRotator::SaveSettings(QString filename)
{

    bool success = true;
    QFile file(filename);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        success = false;
        throw RotException(file.errorString().toStdString().c_str(),RIG_EIO);
    }
    QTextStream out(&file);
    out << "rot_model" << ": " << rot_model << endl;
    out << "polling_interval" << ": " << polling_interval << endl;
    for(QStringList::const_iterator it=cfg_params.begin();it!=cfg_params.end();it++)
    {
        if (!param_blacklist.contains(*it))
            out << (*it) << ": " << getConf(*it) << endl;
    }

    file.close();
    return success;
}

bool QHamRotator::LoadSettings(QString filename)
{
    bool success = true;
    QMap<QString,QString> config;
    QRegularExpressionValidator validator(QRegularExpression("\\w+:.*"));
    QFile file(filename);
    if(!file.open(QIODevice::ReadOnly)) {
        success = false;
        throw RotException(file.errorString().toStdString().c_str(),RIG_EIO);
    }

    QTextStream in(&file);

    while (!in.atEnd())
    {
       QString s = in.readLine().simplified(); int pos=0;
       if (validator.validate(s,pos) == QValidator::Acceptable)
       {
           QStringList param = s.split(":");
           config[param[0].simplified()] = param[1].simplified();
       }
    }

    file.close();
    rot_model = (config.contains("rot_model"))?config["rot_model"].toInt():RIG_MODEL_DUMMY;
    polling_interval = (config.contains("polling_interval"))?config["polling_interval"].toInt():1000;
    for(QStringList::const_iterator it=cfg_params.begin();it!=cfg_params.end();it++)
    {
        if (!param_blacklist.contains(*it) && config.contains(*it))
        {
            setConf(*it, config[(*it)]);
        }
    }
    return success;
}

QString QHamRotator::getConf(const QString &pname)
{
    char* val = new char[200];
    CHECK_ROT( rot_get_conf(theRot, rot_token_lookup(theRot, pname.toStdString().c_str()), val) );
    QString res = QString(val);
    delete val;
    return res;
}

void QHamRotator::setConf(const QString &pname, const QString &val)
{
    CHECK_ROT( rot_set_conf(theRot, rot_token_lookup(theRot, pname.toStdString().c_str()), val.toStdString().c_str()) );
    return;
}

void QHamRotator::open()
{
    CHECK_ROT( rot_open(theRot) );
    portopened = true;
}

void QHamRotator::close()
{
    CHECK_ROT( rot_close(theRot) );
    portopened = false;
}

void QHamRotator::setPosition(azimuth_t az, elevation_t el)
{
    CHECK_ROT( rot_set_position(theRot, az, el) );
}

void QHamRotator::getPosition(azimuth_t &az, elevation_t &el)
{
    CHECK_ROT( rot_get_position(theRot, &az, &el) );
}

void QHamRotator::setPosition(TRotatorPosition pos)
{
    setPosition(pos.first, pos.second);
}

TRotatorPosition QHamRotator::getPosition()
{
    azimuth_t az; elevation_t el;
    getPosition(az,el);
    return QPair<azimuth_t,elevation_t>(az,el);
}

void QHamRotator::stop()
{
    CHECK_ROT(rot_stop(theRot));
}

void QHamRotator::park()
{
    CHECK_ROT(rot_park(theRot));
}

void QHamRotator::reset (rot_reset_t reset)
{
    CHECK_ROT( rot_reset(theRot, reset) );
}

void QHamRotator::move (int direction, int speed)
{
    CHECK_ROT( rot_move(theRot, direction, speed) );
}
