#pragma once

#include "core/session.h"

#include <QDialog>

class QComboBox;
class QLineEdit;
class QSpinBox;
class QCheckBox;
class QTableWidget;
class QPlainTextEdit;

class SessionEditDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SessionEditDialog(QWidget *parent = nullptr);
    void setSession(const Session &session);
    Session session() const;

private:
    void syncProtocolUi();
    void addJumpRow(const JumpHop &hop = JumpHop());
    void addForwardRow(const PortForward &fwd = PortForward());
    QVector<JumpHop> hopsFromTable() const;
    QVector<PortForward> forwardsFromTable() const;

    Session m_original;
    QLineEdit *m_name = nullptr;
    QComboBox *m_protocol = nullptr;
    QComboBox *m_folder = nullptr;
    QLineEdit *m_host = nullptr;
    QSpinBox *m_port = nullptr;
    QLineEdit *m_user = nullptr;
    QComboBox *m_auth = nullptr;
    QLineEdit *m_password = nullptr;
    QLineEdit *m_keyPath = nullptr;
    QTableWidget *m_jumps = nullptr;
    QTableWidget *m_forwards = nullptr;
    QLineEdit *m_extra = nullptr;
    QLineEdit *m_startup = nullptr;
    QLineEdit *m_tags = nullptr;
    QComboBox *m_scheme = nullptr;
    QComboBox *m_sshCompat = nullptr;
    QCheckBox *m_x11 = nullptr;
    QCheckBox *m_log = nullptr;
    QCheckBox *m_favorite = nullptr;
    QCheckBox *m_savePassword = nullptr;
    QSpinBox *m_keepAlive = nullptr;
    QLineEdit *m_serial = nullptr;
    QSpinBox *m_baud = nullptr;
    QPlainTextEdit *m_notes = nullptr;
};
