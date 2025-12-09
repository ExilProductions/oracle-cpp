#include "PasswordDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QFont>

PasswordDialog::PasswordDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Authentication Required");
    setFixedSize(400, 180);

    auto *layout = new QVBoxLayout(this);
    layout->setSpacing(10);
    layout->setContentsMargins(20,20,20,20);

    
    auto *header = new QHBoxLayout;
    auto *icon   = new QLabel(QStringLiteral(u"Key"));
    icon->setFont(QFont("", 16));
    header->addWidget(icon);
    auto *title = new QLabel("Authentication Required");
    title->setFont(QFont("", 12, QFont::Bold));
    header->addWidget(title);
    header->addStretch();
    layout->addLayout(header);

    layout->addWidget(new QLabel("Enter sudo password:"));

    m_passwordEdit = new QLineEdit;
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setMinimumHeight(30);
    layout->addWidget(m_passwordEdit);

    m_rememberCheck = new QCheckBox("Remember password for 5 minutes");
    layout->addWidget(m_rememberCheck);

    
    auto *btns = new QHBoxLayout;
    btns->addStretch();
    auto *cancel = new QPushButton("Cancel");
    auto *ok     = new QPushButton("OK");
    ok->setDefault(true);
    connect(cancel, &QPushButton::clicked, this, &QDialog::reject);
    connect(ok,     &QPushButton::clicked, this, &QDialog::accept);
    btns->addWidget(cancel);
    btns->addWidget(ok);
    layout->addLayout(btns);

    m_passwordEdit->setFocus();
}

QString PasswordDialog::password() const { return m_passwordEdit->text(); }
bool    PasswordDialog::remember() const { return m_rememberCheck->isChecked(); }
