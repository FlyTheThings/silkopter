#pragma once

#include <QObject>

class OSQMLProxy : public QObject
{
    Q_OBJECT
public:
    Q_PROPERTY(float brightness READ getBrightness WRITE setBrightness NOTIFY brightnessChanged)

    explicit OSQMLProxy(QObject *parent = 0);

    float getBrightness() const;
    void setBrightness(float v);

    Q_INVOKABLE void poweroffSystem();

signals:
    void brightnessChanged(float);

public slots:

private:
    float m_brightness = 1.f;
};

