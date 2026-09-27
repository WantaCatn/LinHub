#pragma once

#include <QByteArray>
#include <QObject>
#include <QString>
#include <QStringList>

class QSocketNotifier;

class PtyProcess : public QObject
{
    Q_OBJECT
public:
    explicit PtyProcess(QObject *parent = nullptr);
    ~PtyProcess() override;

    bool start(const QString &program,
               const QStringList &arguments,
               const QString &workingDir = QString());
    void write(const QByteArray &data);
    void setWinsize(int cols, int rows);
    void terminate(bool notify = true);
    bool isRunning() const;
    int pid() const { return m_pid; }
    QString lastError() const { return m_lastError; }

signals:
    void readyRead(const QByteArray &data);
    void finished(int exitCode);

private:
    void onMasterReadable();
    void closeMaster();
    bool spawn(const QString &program, const QStringList &arguments,
               const QString &workingDir);

    int m_masterFd = -1;
    int m_pid = -1;
    int m_cols = 80;
    int m_rows = 24;
    QSocketNotifier *m_notifier = nullptr;
    QString m_lastError;
};
