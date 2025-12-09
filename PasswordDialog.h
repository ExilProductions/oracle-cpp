#pragma once
#include <QDialog>

class QLineEdit;
class QCheckBox;

class PasswordDialog : public QDialog
{
    Q_OBJECT
public:
    explicit PasswordDialog(QWidget *parent = nullptr);

    QString password() const;
    bool remember() const;

private:
    QLineEdit   *m_passwordEdit;
    QCheckBox   *m_rememberCheck;
};