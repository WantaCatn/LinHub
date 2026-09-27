#include "pty/ptyprocess.h"

#include <QSocketNotifier>
#include <QtGlobal>
#include <QByteArray>
#include <QVector>
#include <vector>

#ifdef Q_OS_UNIX
#include <cerrno>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>
#ifdef __linux__
#include <pty.h>
#include <utmp.h>
#else
#include <util.h>
#endif
#endif

PtyProcess::PtyProcess(QObject *parent)
    : QObject(parent)
{
}

PtyProcess::~PtyProcess()
{
    terminate(false);
}

bool PtyProcess::start(const QString &program,
                       const QStringList &arguments,
                       const QString &workingDir)
{
    if (isRunning())
        terminate(false);
    return spawn(program, arguments, workingDir);
}

void PtyProcess::write(const QByteArray &data)
{
#ifdef Q_OS_UNIX
    if (m_masterFd < 0 || data.isEmpty())
        return;
    const char *p = data.constData();
    int left = data.size();
    while (left > 0) {
        const ssize_t n = ::write(m_masterFd, p, static_cast<size_t>(left));
        if (n < 0) {
            if (errno == EINTR)
                continue;
            break;
        }
        p += n;
        left -= static_cast<int>(n);
    }
#else
    Q_UNUSED(data)
#endif
}

void PtyProcess::setWinsize(int cols, int rows)
{
    m_cols = qMax(2, cols);
    m_rows = qMax(1, rows);
#ifdef Q_OS_UNIX
    if (m_masterFd < 0)
        return;
    struct winsize ws {};
    ws.ws_col = static_cast<unsigned short>(m_cols);
    ws.ws_row = static_cast<unsigned short>(m_rows);
    ioctl(m_masterFd, TIOCSWINSZ, &ws);
#endif
}

void PtyProcess::terminate(bool notify)
{
#ifdef Q_OS_UNIX
    const bool was = m_pid > 0 || m_masterFd >= 0;
    if (m_pid > 0) {
        ::kill(m_pid, SIGHUP);
        ::kill(m_pid, SIGTERM);
        int status = 0;
        waitpid(m_pid, &status, WNOHANG);
        m_pid = -1;
    }
    closeMaster();
    if (notify && was)
        emit finished(0);
#else
    Q_UNUSED(notify)
#endif
}

bool PtyProcess::isRunning() const
{
#ifdef Q_OS_UNIX
    if (m_pid <= 0)
        return false;
    if (::kill(m_pid, 0) == 0)
        return true;
    return errno == EPERM;
#else
    return false;
#endif
}

void PtyProcess::onMasterReadable()
{
#ifdef Q_OS_UNIX
    char buf[4096];
    const ssize_t n = ::read(m_masterFd, buf, sizeof(buf));
    if (n > 0) {
        emit readyRead(QByteArray(buf, static_cast<int>(n)));
        return;
    }
    if (n == 0 || (n < 0 && errno != EAGAIN && errno != EINTR)) {
        int status = 0;
        int code = 0;
        if (m_pid > 0) {
            waitpid(m_pid, &status, WNOHANG);
            if (WIFEXITED(status))
                code = WEXITSTATUS(status);
        }
        closeMaster();
        m_pid = -1;
        emit finished(code);
    }
#endif
}

void PtyProcess::closeMaster()
{
#ifdef Q_OS_UNIX
    if (m_notifier) {
        m_notifier->setEnabled(false);
        m_notifier->deleteLater();
        m_notifier = nullptr;
    }
    if (m_masterFd >= 0) {
        ::close(m_masterFd);
        m_masterFd = -1;
    }
#endif
}

bool PtyProcess::spawn(const QString &program, const QStringList &arguments, const QString &workingDir)
{
#ifndef Q_OS_UNIX
    Q_UNUSED(program)
    Q_UNUSED(arguments)
    Q_UNUSED(workingDir)
    m_lastError = QStringLiteral("PTY 仅在 Linux/麒麟系统上可用");
    return false;
#else
    struct winsize ws {};
    ws.ws_col = static_cast<unsigned short>(m_cols);
    ws.ws_row = static_cast<unsigned short>(m_rows);

    int slaveFd = -1;
    const int ret = openpty(&m_masterFd, &slaveFd, nullptr, nullptr, &ws);
    if (ret != 0) {
        m_lastError = QStringLiteral("openpty 失败: %1").arg(QString::fromLocal8Bit(strerror(errno)));
        return false;
    }

    fcntl(m_masterFd, F_SETFL, O_NONBLOCK);

    const pid_t pid = fork();
    if (pid < 0) {
        m_lastError = QStringLiteral("fork 失败");
        ::close(m_masterFd);
        ::close(slaveFd);
        m_masterFd = -1;
        return false;
    }

    if (pid == 0) {
        ::close(m_masterFd);
        setsid();
        ioctl(slaveFd, TIOCSCTTY, nullptr);
        dup2(slaveFd, STDIN_FILENO);
        dup2(slaveFd, STDOUT_FILENO);
        dup2(slaveFd, STDERR_FILENO);
        if (slaveFd > STDERR_FILENO)
            ::close(slaveFd);

        if (!workingDir.isEmpty())
            chdir(workingDir.toLocal8Bit().constData());

        unsetenv("SSH_ASKPASS_REQUIRE");
        setenv("TERM", "xterm-256color", 1);
        setenv("COLORTERM", "truecolor", 1);
        setenv("CLICOLOR", "1", 1);
        setenv("FORCE_COLOR", "1", 0);
        setenv("CLICOLOR_FORCE", "0", 0);
        if (!getenv("LS_COLORS") || QByteArray(getenv("LS_COLORS")).isEmpty()) {
            setenv("LS_COLORS",
                   "rs=0:di=01;34:ln=01;36:mh=00:pi=40;33:so=01;35:do=01;35:"
                   "bd=40;33;01:cd=40;33;01:or=40;31;01:ex=01;32:"
                   "*.tar=01;31:*.tgz=01;31:*.zip=01;31:*.gz=01;31:*.bz2=01;31:"
                   "*.xz=01;31:*.7z=01;31:*.rar=01;31:"
                   "*.jpg=01;35:*.jpeg=01;35:*.png=01;35:*.gif=01;35:*.bmp=01;35:*.svg=01;35:"
                   "*.mp3=00;36:*.mp4=00;36:*.mkv=00;36:*.avi=00;36:"
                   "*.pdf=01;33:*.doc=01;33:*.txt=00;33:"
                   "*.c=00;32:*.h=00;32:*.cpp=00;32:*.py=00;32:*.sh=01;32",
                   1);
        }
        if (!getenv("LANG") || QByteArray(getenv("LANG")).isEmpty())
            setenv("LANG", "zh_CN.UTF-8", 1);

        QStringList argv = arguments;
        argv.prepend(program);
        QVector<QByteArray> storage;
        storage.reserve(argv.size());
        std::vector<char *> cargv;
        cargv.reserve(static_cast<size_t>(argv.size()) + 1);
        for (const QString &a : argv) {
            storage.push_back(a.toLocal8Bit());
            cargv.push_back(storage.back().data());
        }
        cargv.push_back(nullptr);
        execvp(cargv[0], cargv.data());
        _exit(127);
    }

    ::close(slaveFd);
    m_pid = pid;
    m_notifier = new QSocketNotifier(m_masterFd, QSocketNotifier::Read, this);
    connect(m_notifier, &QSocketNotifier::activated, this, [this] { onMasterReadable(); });
    return true;
#endif
}
