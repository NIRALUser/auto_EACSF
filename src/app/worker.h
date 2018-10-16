#ifndef WORKER_H
#define WORKER_H

#include <QObject>

class worker : public QObject
{
    Q_OBJECT
public:
    explicit worker(QObject *parent = nullptr);

signals:

public slots:
};

#endif // WORKER_H