#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsEllipseItem>
#include <QGraphicsRectItem>
#include <QGraphicsTextItem>
#include <QGraphicsLineItem>
#include <QMap>
#include "info.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QGraphicsScene *scene;
    InfoManager *infoManager;

    void drawComponents();
    void drawModule(qreal x, qreal y, const QString &label, const QColor &color);
    void drawConnection(QPointF start, QPointF end, const QString &label = "");
    QPointF drawPort(QPointF nodeCenter, QPointF offset, int portId);
    QGraphicsView *view;
    QRectF l3GroupRect;

};

#endif // MAINWINDOW_H
