#pragma once
#include <QMainWindow>
#include <QMap>
#include <QTimer>
#include <QTabWidget>
#include <QTreeWidget>
#include <QLineEdit>
#include <QTextEdit>
#include <QCheckBox>
#include <QFrame>

class PackageWorker;

class AURManager : public QMainWindow
{
    Q_OBJECT
public:
    explicit AURManager(QWidget *parent = nullptr);
    ~AURManager() override;

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void searchPackages();
    void installPackage();
    void removePackage();
    void checkUpdates();
    void updateAll();

private:
    
    void setupUi();
    void setupSearchTab();
    void setupUpdatesTab();
    void setupAboutTab();
    void setupTerminal();

    
    QMap<QString,QString> installedPackages() const;
    QStringList detectAurHelper() const;
    QString cachedSudoPassword();
    void cacheSudoPassword(const QString &pwd);
    void runSudoCommand(const QStringList &cmd);
    void log(const QString &text);

    
    void startWorker(PackageWorker *w);
    void handleSudoRequest(const QStringList &cmd, const QVariantMap &extra);

    
    QTabWidget   *m_tabWidget{};
    QLineEdit    *m_searchEdit{};
    QTreeWidget  *m_pkgTree{};
    QTreeWidget  *m_updatesTree{};
    QCheckBox    *m_showTermCheck{};
    QFrame       *m_termFrame{};
    QTextEdit    *m_termOutput{};

    
    QMap<QString,QString> m_installed;
    QString               m_sudoPwd;
    qint64                m_sudoTs{0};
    const int             m_sudoTimeout = 300; 

    PackageWorker        *m_currentWorker{nullptr};
};
