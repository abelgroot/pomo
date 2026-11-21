#pragma once

#include <QObject>
#include <QTimer>

// A pomodoro cycle: focus, short break, and a long break after every fourth
// focus session. The remaining time is derived from a deadline rather than
// counted in ticks, so a suspended laptop doesn't drift.
class Backend : public QObject {
    Q_OBJECT
    Q_PROPERTY(int phase READ phase NOTIFY phaseChanged)
    Q_PROPERTY(int remaining READ remaining NOTIFY tick)
    Q_PROPERTY(int total READ total NOTIFY phaseChanged)
    Q_PROPERTY(bool running READ running NOTIFY runningChanged)
    Q_PROPERTY(int completed READ completed NOTIFY completedChanged)
    Q_PROPERTY(int count READ completed NOTIFY completedChanged)
    Q_PROPERTY(QString notifyError READ notifyError NOTIFY notifyErrorChanged)

public:
    enum Phase { Focus, ShortBreak, LongBreak };
    Q_ENUM(Phase)

    explicit Backend(QObject *parent = nullptr);

    int phase() const { return m_phase; }
    int remaining() const { return m_remaining; }
    int total() const { return duration(m_phase); }
    bool running() const { return m_running; }
    int completed() const { return m_completed; }
    QString notifyError() const { return m_notifyError; }

    // Seconds per phase; tests shrink these.
    void setDurations(int focus, int shortBreak, int longBreak);

    Q_INVOKABLE void primaryAction();   // start or pause
    Q_INVOKABLE void reset();
    Q_INVOKABLE void skip();

signals:
    void phaseChanged();
    void tick();
    void runningChanged();
    void completedChanged();
    void notifyErrorChanged();

private:
    int duration(int phase) const;
    void update();
    void finishPhase();
    void enter(int phase);
    void notify(const QString &title, const QString &body);

    int m_phase = Focus;
    int m_remaining;
    bool m_running = false;
    int m_completed = 0;
    int m_durations[3] = {25 * 60, 5 * 60, 15 * 60};
    qint64 m_deadline = 0;
    QTimer m_timer;
    QString m_notifyError;
};
