#include "PackageWorker.h"
#include <QDebug>

PackageWorker::PackageWorker(Task task, QObject *parent)
    : QObject(parent), m_task(std::move(task))
{
}

PackageWorker::~PackageWorker()
{
    stop();
}

void PackageWorker::stop()
{
    QMutexLocker lk(&m_mutex);
    m_stop = true;
    m_cond.wakeAll();
}

void PackageWorker::requestSudo(const QStringList &cmd, const QVariantMap &extra)
{
    QMutexLocker lk(&m_mutex);
    m_pendingCmd   = cmd;
    m_pendingExtra = extra;
    m_waitingForSudo = true;
    emit output(QStringLiteral("\n[sudo] %1").arg(cmd.join(' ')));
}

void PackageWorker::provideSudoResponse(const QVariant &response)
{
    QMutexLocker lk(&m_mutex);
    m_sudoResponse   = response;
    m_waitingForSudo = false;
    m_cond.wakeAll();
}

void PackageWorker::runTask()
{
    try {
        if (m_task)
            m_task(this);
    } catch (const std::exception &e) {
        emit error(QString::fromUtf8(e.what()));
    } catch (...) {
        emit error("Unknown exception in worker");
    }
    emit finished();
}