#include "backend.h"

#include <QDateTime>
#include <QProcess>
#include <QStandardPaths>

Backend::Backend(const QString &settingsFile, QObject *parent)
    : QObject(parent),
      m_settings(settingsFile.isEmpty() ? QSettings("pomo", "pomo").fileName() : settingsFile,
                 QSettings::IniFormat) {
    static const char *keys[] = {"focus", "shortBreak", "longBreak"};
    for (int p = 0; p < 3; ++p) {
        const int minutes = m_settings.value(keys[p], m_durations[p] / 60).toInt();
        if (minutes >= 1 && minutes <= 180)
            m_durations[p] = minutes * 60;
    }
    m_remaining = m_durations[Focus];
    m_timer.setInterval(250);
    connect(&m_timer, &QTimer::timeout, this, &Backend::update);
}

int Backend::duration(int phase) const { return m_durations[phase]; }

void Backend::setDurations(int focus, int shortBreak, int longBreak) {
    m_durations[Focus] = focus;
    m_durations[ShortBreak] = shortBreak;
    m_durations[LongBreak] = longBreak;
    m_remaining = duration(m_phase);
    emit phaseChanged();
    emit tick();
}

// A running or half-used timer keeps its time; only an untouched one picks up
// the new length, so editing settings never throws away progress.
void Backend::setMinutes(int phase, int minutes) {
    minutes = qBound(1, minutes, 180);
    if (m_durations[phase] == minutes * 60)
        return;
    const bool untouched = !m_running && m_remaining == m_durations[phase];
    m_durations[phase] = minutes * 60;
    static const char *keys[] = {"focus", "shortBreak", "longBreak"};
    m_settings.setValue(keys[phase], minutes);
    m_settings.sync();
    emit durationsChanged();
    if (phase == m_phase) {
        if (untouched) {
            m_remaining = m_durations[phase];
            emit tick();
        }
        emit phaseChanged();
    }
}

void Backend::primaryAction() {
    m_running = !m_running;
    if (m_running) {
        m_deadline = QDateTime::currentMSecsSinceEpoch() + qint64(m_remaining) * 1000;
        m_timer.start();
    } else {
        m_timer.stop();
    }
    emit runningChanged();
}

void Backend::reset() {
    m_running = false;
    m_timer.stop();
    m_remaining = duration(m_phase);
    emit runningChanged();
    emit tick();
}

void Backend::skip() {
    // Skipping a focus session doesn't count as finishing it.
    enter(m_phase == Focus ? ShortBreak : Focus);
}

void Backend::update() {
    const qint64 ms = m_deadline - QDateTime::currentMSecsSinceEpoch();
    const int seconds = qMax<qint64>(0, (ms + 999) / 1000);
    if (seconds != m_remaining) {
        m_remaining = seconds;
        emit tick();
    }
    if (ms <= 0)
        finishPhase();
}

void Backend::finishPhase() {
    if (m_phase == Focus) {
        ++m_completed;
        emit completedChanged();
        const bool longBreak = m_completed % 4 == 0;
        notify(tr("Focus session done"), longBreak ? tr("Time for a long break.") : tr("Time for a short break."));
        enter(longBreak ? LongBreak : ShortBreak);
    } else {
        notify(tr("Break over"), tr("Ready to focus?"));
        enter(Focus);
    }
}

// The next phase waits for the user to press Space, so a break never starts
// counting while they are away from the keyboard.
void Backend::enter(int phase) {
    m_phase = phase;
    m_running = false;
    m_timer.stop();
    m_remaining = duration(phase);
    emit runningChanged();
    emit phaseChanged();
    emit tick();
}

void Backend::notify(const QString &title, const QString &body) {
    const QString tool = QStandardPaths::findExecutable("notify-send");
    QString error;
    if (tool.isEmpty()) {
        error = tr("notify-send is not installed, so desktop notifications are off.");
    } else {
        QProcess::startDetached(tool, {"-a", "pomo", title, body});
    }
    if (error != m_notifyError) {
        m_notifyError = error;
        emit notifyErrorChanged();
    }
}
