#pragma once
#include <QObject>
#include <QMutex>
#include <QWaitCondition>
#include <QVariantMap>
#include <QVariant>
#include <functional>

class PackageWorker : public QObject
{
    Q_OBJECT
public:
    using Task = std::function<void(PackageWorker*)>;

    explicit PackageWorker(Task task, QObject *parent = nullptr);
    ~PackageWorker() override;

    void stop();

    
    void requestSudo(const QStringList &cmd, const QVariantMap &extra = {});

    
    void provideSudoResponse(const QVariant &response);

signals:
    void output(const QString &text);
    void packageFound(const QVariantMap &info);
    void error(const QString &msg);
    void finished();

public slots:
    void runTask();

private:
    Task m_task;
    bool m_stop{false};
    QMutex m_mutex;
    QWaitCondition m_cond;

    
    QStringList m_pendingCmd;
    QVariantMap m_pendingExtra;
    QVariant    m_sudoResponse;
    bool        m_waitingForSudo{false};
};
