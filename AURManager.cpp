#include "AURManager.h"
#include "PasswordDialog.h"
#include "PackageWorker.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>
#include <QProcess>
#include <QScrollBar>
#include <QTimer>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QLineEdit>
#include <QTreeWidget>
#include <QTextEdit>
#include <QCheckBox>
#include <QRegularExpression>
#include <QThread>

AURManager::AURManager(QWidget *parent)
    : QMainWindow(parent)
{
    m_installed = installedPackages();
    setupUi();
}

AURManager::~AURManager()
{
    if (m_currentWorker) m_currentWorker->stop();
}




void AURManager::setupUi()
{
    setWindowTitle("Oracle – AUR Helper Wrapper");
    resize(1100, 720);

    
    setStyleSheet(R"(
        QMainWindow, QWidget { background:#2e2e2e; color:white; }
        QTabWidget::pane { border:none; }
        QTabBar::tab { background:#3e3e3e; padding:8px 16px; margin-right:2px; }
        QTabBar::tab:selected { background:#0066cc; }
        QTreeWidget { background:#1e1e1e; alternate-background-color:#262626; border:none; }
        QTreeWidget::item:selected { background:#0066cc; }
        QPushButton { background:#0066cc; border:none; border-radius:4px; padding:8px 16px; }
        QPushButton:hover { background:#0077dd; }
        QLineEdit { background:#3e3e3e; border:1px solid #555; border-radius:4px; padding:8px; }
        QTextEdit { background:#1e1e1e; border:none; font-family:Consolas,Monospace; }
    )");

    auto *central = new QWidget;
    setCentralWidget(central);
    auto *mainLay = new QVBoxLayout(central);

    m_tabWidget = new QTabWidget;
    mainLay->addWidget(m_tabWidget);

    setupSearchTab();
    setupUpdatesTab();
    setupAboutTab();
    setupTerminal();
}

void AURManager::setupSearchTab()
{
    auto *w = new QWidget;
    auto *l = new QVBoxLayout(w);

    l->addWidget(new QLabel("<b>Package Search</b>"));

    auto *searchLay = new QHBoxLayout;
    m_searchEdit = new QLineEdit;
    m_searchEdit->setPlaceholderText("Search packages…");
    connect(m_searchEdit, &QLineEdit::returnPressed, this, &AURManager::searchPackages);
    searchLay->addWidget(m_searchEdit);

    auto *btn = new QPushButton("Search");
    connect(btn, &QPushButton::clicked, this, &AURManager::searchPackages);
    searchLay->addWidget(btn);
    l->addLayout(searchLay);

    m_pkgTree = new QTreeWidget;
    m_pkgTree->setHeaderLabels({"", "Name", "Version", "Source", "Description"});
    m_pkgTree->setColumnWidth(0, 30);
    m_pkgTree->setColumnWidth(1, 220);
    m_pkgTree->setColumnWidth(2, 120);
    m_pkgTree->setColumnWidth(3, 100);
    m_pkgTree->setAlternatingRowColors(true);
    l->addWidget(m_pkgTree);

    auto *btnLay = new QHBoxLayout;
    auto *installBtn = new QPushButton("Install");
    auto *removeBtn  = new QPushButton("Remove");
    connect(installBtn, &QPushButton::clicked, this, &AURManager::installPackage);
    connect(removeBtn,  &QPushButton::clicked, this, &AURManager::removePackage);
    btnLay->addWidget(installBtn);
    btnLay->addWidget(removeBtn);
    btnLay->addStretch();
    l->addLayout(btnLay);

    m_tabWidget->addTab(w, "Search");
}

void AURManager::setupUpdatesTab()
{
    auto *w = new QWidget;
    auto *l = new QVBoxLayout(w);

    l->addWidget(new QLabel("<b>System Updates</b>"));

    auto *btnLay = new QHBoxLayout;
    auto *checkBtn = new QPushButton("Check for Updates");
    auto *upAllBtn = new QPushButton("Update All");
    connect(checkBtn, &QPushButton::clicked, this, &AURManager::checkUpdates);
    connect(upAllBtn, &QPushButton::clicked, this, &AURManager::updateAll);
    btnLay->addWidget(checkBtn);
    btnLay->addWidget(upAllBtn);
    btnLay->addStretch();
    l->addLayout(btnLay);

    m_updatesTree = new QTreeWidget;
    m_updatesTree->setHeaderLabels({"Name","Current","New","Source"});
    m_updatesTree->setColumnWidth(0,220);
    m_updatesTree->setAlternatingRowColors(true);
    l->addWidget(m_updatesTree);

    m_tabWidget->addTab(w, "Updates");
}

void AURManager::setupAboutTab()
{
    auto *w = new QWidget;
    auto *l = new QVBoxLayout(w);
    l->setContentsMargins(20,20,20,20);
    l->setSpacing(20);

    auto *title = new QLabel("About Oracle");
    title->setFont(QFont("",16,QFont::Bold));
    l->addWidget(title);

    l->addWidget(new QLabel("Author: 0xGingi"));
    l->addWidget(new QLabel(
        "Oracle is a wrapper for AUR helpers and Pacman.\n"
        "It provides an easy way to search, install, and manage packages from both\n"
        "the official Arch repositories and the Arch User Repository (AUR).\n\n"));

    l->addStretch();
    m_tabWidget->addTab(w, "About");
}

void AURManager::setupTerminal()
{
    auto *toggleLay = new QHBoxLayout;
    m_showTermCheck = new QCheckBox("Show Terminal Output");
    connect(m_showTermCheck, &QCheckBox::checkStateChanged, this, [this](Qt::CheckState state){
        m_termFrame->setVisible(state == Qt::Checked);
    });
    toggleLay->addWidget(m_showTermCheck);
    toggleLay->addStretch();
    static_cast<QVBoxLayout*>(centralWidget()->layout())->addLayout(toggleLay);

    m_termFrame = new QFrame;
    m_termFrame->setVisible(false);
    auto *termLay = new QVBoxLayout(m_termFrame);
    m_termOutput = new QTextEdit;
    m_termOutput->setReadOnly(true);
    m_termOutput->setMinimumHeight(150);
    termLay->addWidget(m_termOutput);
    centralWidget()->layout()->addWidget(m_termFrame);
}




QMap<QString,QString> AURManager::installedPackages() const
{
    QProcess p;
    p.start("pacman", {"-Q"});
    p.waitForFinished();
    QMap<QString,QString> map;
    while (p.canReadLine()) {
        auto line = p.readLine().trimmed();
        auto parts = line.split(' ');
        if (parts.size() >= 2)
            map[parts[0]] = parts[1];
    }
    return map;
}

QStringList AURManager::detectAurHelper() const
{
    const QStringList candidates = {"yay","paru","pamac","aurman","pikaur"};
    for (const auto &name : candidates) {
        QProcess which;
        which.start("which", {name});
        which.waitForFinished();
        if (which.exitCode() == 0)
            return {name};
    }
    return {};
}

QString AURManager::cachedSudoPassword()
{
    if (m_sudoPwd.isEmpty())
        return {};
    if (QDateTime::currentSecsSinceEpoch() - m_sudoTs > m_sudoTimeout) {
        m_sudoPwd.clear();
        return {};
    }
    return m_sudoPwd;
}

void AURManager::cacheSudoPassword(const QString &pwd)
{
    m_sudoPwd = pwd;
    m_sudoTs  = QDateTime::currentSecsSinceEpoch();
}

void AURManager::log(const QString &text)
{
    m_termOutput->append(text);
    m_termOutput->verticalScrollBar()->setValue(m_termOutput->verticalScrollBar()->maximum());
}




void AURManager::runSudoCommand(const QStringList &cmd)
{
    QString pwd = cachedSudoPassword();

    while (true) {
        if (pwd.isEmpty()) {
            PasswordDialog dlg(this);
            if (dlg.exec() != QDialog::Accepted) {
                throw std::runtime_error("Authentication cancelled by user");
            }
            pwd = dlg.password();
            if (dlg.remember())
                cacheSudoPassword(pwd);
        }

        QProcess sudo;
        sudo.setProgram("sudo");
        sudo.setArguments(QStringList{"-S"} + cmd);
        sudo.start();
        sudo.waitForStarted();

        sudo.write((pwd + "\n").toUtf8());
        sudo.closeWriteChannel();
        sudo.waitForFinished(-1);

        if (sudo.exitCode() == 0) {
            if (!sudo.readAllStandardOutput().isEmpty())
                log(sudo.readAllStandardOutput());
            if (!sudo.readAllStandardError().isEmpty())
                log(sudo.readAllStandardError());
            return;
        }

        const QString err = sudo.readAllStandardError();
        if (err.contains("incorrect password", Qt::CaseInsensitive)) {
            pwd.clear();
            log("Incorrect password – try again");
            continue;
        }
        log(err);
        throw std::runtime_error(err.toStdString());
    }
}




void AURManager::startWorker(PackageWorker *w)
{
    auto *thread = new QThread;
    w->moveToThread(thread);
    connect(thread, &QThread::started, w, &PackageWorker::runTask);
    connect(w, &PackageWorker::finished, thread, &QThread::quit);
    connect(w, &PackageWorker::finished, w, &QObject::deleteLater);
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);
    connect(w, &PackageWorker::output, this, &AURManager::log);
    connect(w, &PackageWorker::error, this, &AURManager::log);
    connect(w, &PackageWorker::error, this, [this](const QString &msg){
        QMessageBox::critical(this, "Error", msg);
    });
    connect(w, &PackageWorker::finished, this, [this](){
        m_currentWorker = nullptr;
    });

    
    connect(w, &PackageWorker::output, this, [this](const QString &txt){
        if (txt.startsWith("[sudo]")) {
            
            
        }
    });

    
    QObject::connect(w, &PackageWorker::output, this, [this, w](const QString &txt){
        if (!txt.startsWith("[sudo]")) return;
        
        QStringList cmd = txt.mid(7).trimmed().split(' ', Qt::SkipEmptyParts);
        try {
            runSudoCommand(cmd);
            w->provideSudoResponse(QVariant()); 
        } catch (...) {
            w->provideSudoResponse(QVariant::fromValue(std::current_exception()));
        }
    });

    thread->start();
}




void AURManager::searchPackages()
{
    const QString query = m_searchEdit->text().trimmed();
    if (query.isEmpty()) return;

    m_pkgTree->clear();
    log("\nSearching for: " + query);

    auto *worker = new PackageWorker([this, query](PackageWorker *w){
        
        QProcess pacman;
        pacman.start("pacman", {"-Ss", query});
        pacman.waitForFinished(-1);
        QString out = pacman.readAllStandardOutput();
        QStringList lines = out.split('\n', Qt::SkipEmptyParts);
        for (int i = 0; i + 1 < lines.size(); i += 2) {
            const QString pkgLine = lines[i].trimmed();
            const QString desc    = lines[i+1].trimmed();

            QRegularExpression re(R"(^(\w+)/([^ ]+) (.+)$)");
            auto m = re.match(pkgLine);
            if (m.hasMatch()) {
                const QString repo = m.captured(1);
                const QString name = m.captured(2);
                const QString ver  = m.captured(3);
                w->packageFound({
                    {"status", m_installed.contains(name) ? "Check" : ""},
                    {"name", name},
                    {"version", ver},
                    {"source", repo},
                    {"description", desc}
                });
            }
        }

        
        auto helper = detectAurHelper();
        if (!helper.isEmpty()) {
            emit w->output("Searching AUR with " + helper[0] + "...");
            QProcess aur;
            aur.start(helper[0], QStringList{"-Ss", query});
            aur.waitForFinished(-1);
            QString aurOut = aur.readAllStandardOutput();
            QStringList aurLines = aurOut.split('\n');

            for (int i = 0; i < aurLines.size(); ) {
                QString line = aurLines[i].trimmed();
                if (line.isEmpty()) { ++i; continue; }
                
                QRegularExpression re(R"(^aur/([^ ]+) (.+)$)");
                auto m = re.match(line);
                QString name, ver, desc;
                if (m.hasMatch()) {
                    name = m.captured(1);
                    ver  = m.captured(2);
                    if (i+1 < aurLines.size()) desc = aurLines[i+1].trimmed();
                    i += 2;
                } else {
                    ++i; continue;
                }
                w->packageFound({
                    {"status", m_installed.contains(name) ? "Check" : ""},
                    {"name", name},
                    {"version", ver},
                    {"source", "AUR"},
                    {"description", desc}
                });
            }
        }
    }, this);

    startWorker(worker);
}




void AURManager::installPackage()
{
    auto items = m_pkgTree->selectedItems();
    if (items.isEmpty()) { QMessageBox::warning(this,"Warning","Select a package first"); return; }
    const QString name   = items[0]->text(1);
    const QString source = items[0]->text(3);

    auto *worker = new PackageWorker([this, name, source](PackageWorker *w){
        try {
            
            emit w->output("Updating package databases…");
            runSudoCommand({"pacman","-Sy"});

            
            QProcess check;
            check.start("pacman", {"-Si", name});
            check.waitForFinished(-1);
            bool official = (check.exitCode() == 0);

            if (official) {
                emit w->output(QString("Installing %1 from official repos…").arg(name));
                runSudoCommand({"pacman","-S","--noconfirm",name});
            } else {
                auto helper = detectAurHelper();
                if (helper.isEmpty())
                    throw std::runtime_error("No AUR helper found");
                emit w->output(QString("Installing %1 with %2…").arg(name,helper[0]));
                if (helper[0] == "pamac")
                    runSudoCommand(QStringList{helper[0],"install","--no-confirm",name});
                else
                    runSudoCommand(helper + QStringList{"-S","--noconfirm",name});
            }
            emit w->output(name + " installed successfully!");
        } catch (const std::exception &e) {
            emit w->error(QString::fromUtf8(e.what()));
        }
    }, this);

    connect(worker, &PackageWorker::finished, this, [this,name]{
        m_installed = installedPackages();
        QMessageBox::information(this,"Done", name + " has been installed");
    });

    startWorker(worker);
}

void AURManager::removePackage()
{
    auto items = m_pkgTree->selectedItems();
    if (items.isEmpty()) { QMessageBox::warning(this,"Warning","Select a package first"); return; }
    const QString name = items[0]->text(1);

    int reply = QMessageBox::question(this,"Remove","Remove " + name + " ?", QMessageBox::Yes|QMessageBox::No);
    if (reply != QMessageBox::Yes) return;

    auto *worker = new PackageWorker([this,name](PackageWorker *){
        runSudoCommand({"pacman","-R","--noconfirm",name});
    }, this);

    connect(worker, &PackageWorker::finished, this,[this]{
        m_installed = installedPackages();
        searchPackages(); 
    });

    startWorker(worker);
}




void AURManager::checkUpdates()
{
    m_updatesTree->clear();
    log("\nChecking for updates…");

    auto *worker = new PackageWorker([this](PackageWorker *w){
        auto helper = detectAurHelper();
        if (helper.isEmpty()) {
            emit w->output("No AUR helper detected");
            return;
        }

        
        try { runSudoCommand({"pacman","-Sy"}); }
        catch (...) { emit w->output("DB sync failed – continuing anyway"); }

        QStringList args;
        if (helper[0] == "yay")   args = {"-Qu","--devel","--needed"};
        else if (helper[0] == "pamac") args = {"checkupdates","-a"};
        else args = {"-Qu"};

        QProcess proc;
        proc.start(helper[0], args);
        proc.waitForFinished(-1);
        QString out = proc.readAllStandardOutput();

        for (const QString &line : out.split('\n')) {
            if (line.trimmed().isEmpty()) continue;
            
            QRegularExpression re(R"(^(\S+)\s+(\S+)\s+->\s+(\S+))");
            auto m = re.match(line);
            if (m.hasMatch()) {
                QString name = m.captured(1);
                w->packageFound({
                    {"name", name},
                    {"current_version", m.captured(2)},
                    {"new_version", m.captured(3)},
                    {"source", "AUR"}
                });
            }
        }
    }, this);

    startWorker(worker);
}

void AURManager::updateAll()
{
    if (m_updatesTree->topLevelItemCount() == 0) {
        QMessageBox::information(this,"Info","No updates available");
        return;
    }
    if (QMessageBox::question(this,"Update all","Install all updates ?", QMessageBox::Yes|QMessageBox::No) != QMessageBox::Yes)
        return;

    auto *worker = new PackageWorker([this](PackageWorker *w){
        
        runSudoCommand({"pacman","-Syu","--noconfirm"});

        
        auto helper = detectAurHelper();
        if (!helper.isEmpty()) {
            if (helper[0] == "pamac")
                runSudoCommand({helper[0],"upgrade","-a","--no-confirm"});
            else if (helper[0] == "yay" || helper[0] == "paru")
                runSudoCommand(helper + QStringList{"-Sua","--noconfirm"});
            else
                runSudoCommand(helper + QStringList{"-Su","--noconfirm"});
        }
        emit w->output("System is now up-to-date!");
    }, this);

    connect(worker, &PackageWorker::finished, this, &AURManager::checkUpdates);
    startWorker(worker);
}




void AURManager::closeEvent(QCloseEvent *e)
{
    if (m_currentWorker)
        m_currentWorker->stop();
    QMainWindow::closeEvent(e);
}
