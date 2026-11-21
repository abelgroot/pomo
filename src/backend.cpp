#include "backend.h"

#include <QDateTime>
#include <QProcess>
#include <QStandardPaths>

Backend::Backend(QObject *parent) : QObject(parent) {
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
