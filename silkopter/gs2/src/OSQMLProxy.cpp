#include "OSQMLProxy.h"
#include <QSettings>
#include <QProcess>

OSQMLProxy::OSQMLProxy(QObject *parent)
    : QObject(parent)
{
    QSettings settings;

    {
        bool ok = false;
        float brightness = settings.value("screen/brightness").toFloat(&ok);
        m_brightness = 0.f; //to force a refresh
        setBrightness(ok ? brightness : 1.f);
    }
}

void OSQMLProxy::poweroffSystem()
{
#ifdef RASPBERRY_PI
    QProcess process;
    process.startDetached("shutdown -P now");
#endif
}

float OSQMLProxy::getBrightness() const
{
    return m_brightness;
}

void OSQMLProxy::setBrightness(float v)
{
    v = math::clamp(v, 0.1f, 1.f);
    if (m_brightness != v)
    {
        m_brightness = v;

#ifdef RASPBERRY_PI
        q::data::File_Sink fs(q::Path("/sys/class/backlight/rpi_backlight/brightness"));
        if (fs.is_open())
        {
            std::string val = q::util::format<std::string>("{}\n", static_cast<int>(m_brightness * 255.f));
            fs.write(reinterpret_cast<const uint8_t*>(val.data()), val.size());
        }
#endif

        emit brightnessChanged(m_brightness);
    }
}
