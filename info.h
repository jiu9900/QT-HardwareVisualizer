// info.h
#ifndef INFO_H
#define INFO_H

#include <QObject>
#include <QTextEdit>
#include <QMap>

class InfoManager : public QObject
{
    Q_OBJECT
public:
    explicit InfoManager(QObject *parent = nullptr);
    void setTextEdit(QTextEdit *textEdit);
    void showInfo(const QString &moduleName);

private:
    QTextEdit *infoWidget;
    QMap<QString, QMap<QString, QString>> perfData;
    void loadPerfData();
};

#endif // INFO_H
