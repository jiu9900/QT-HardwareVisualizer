#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QPainter>
#include <QMouseEvent>
#include <QDebug>
#include <QLayout>
#include <QGraphicsView>
#include <QPen>
#include <QBrush>
#include <QtMath>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    scene = new QGraphicsScene(this);
    QGraphicsView *view = new QGraphicsView(scene, ui->visualizationWidget);
    view->setGeometry(0, 0, ui->visualizationWidget->width(), ui->visualizationWidget->height());
    view->setRenderHint(QPainter::Antialiasing);

    infoManager = new InfoManager(this);
        infoManager->setTextEdit(ui->infoWidget);

    drawComponents();

}

MainWindow::~MainWindow()
{
    delete ui;
}



void MainWindow::drawComponents()
{
    scene->clear();

    // 放大硬件框尺寸
    const int moduleWidth = 150;
    const int moduleHeight = 80;

    // 进一步增大节点圆圈尺寸 (50x50)
    const int nodeSize = 50;

    // 设定节点位置（Bus 总线节点）并垂直居中
    const int busY = 400; // 总线垂直位置
    QList<QPointF> busNodes = {
        {250, busY - nodeSize/2}, // Y位置调整为节点中心在busY
        {450, busY - nodeSize/2},
        {650, busY - nodeSize/2},
        {850, busY - nodeSize/2},
        {1050, busY - nodeSize/2}
    };

    // 画出放大后的 Bus 总线节点
    for (int i = 0; i < busNodes.size(); ++i) {
        QPointF pos = busNodes[i];
        QGraphicsEllipseItem *node = new QGraphicsEllipseItem(pos.x(), pos.y(), nodeSize, nodeSize);
        node->setBrush(QBrush(Qt::lightGray));
        node->setFlag(QGraphicsItem::ItemIsSelectable);
        scene->addItem(node);
        scene->addText(QString("Node%1").arg(i))->setPos(pos.x() - 10, pos.y() + nodeSize + 5);

        connect(scene, &QGraphicsScene::selectionChanged, this, [=]() {
            if (node->isSelected()) {
                infoManager->showInfo("BusNode");
            }
        });
    }

    // 在节点下方绘制总线（水平线）
    QPointF firstNodeCenter = busNodes.first() + QPointF(nodeSize/2, nodeSize/2);
    QPointF lastNodeCenter = busNodes.last() + QPointF(nodeSize/2, nodeSize/2);
    scene->addLine(firstNodeCenter.x(), busY + 25,
                   lastNodeCenter.x(), busY + 25,
                   QPen(Qt::black, 3));
    // 创建文本对象
    QGraphicsTextItem *busText = scene->addText("Bus");
    busText->setPos((firstNodeCenter.x() + lastNodeCenter.x())/2 + 50, busY + 30);

    // 设置字体样式
    QFont font = busText->font();
    font.setPointSize(12);    // 设置字号
    font.setBold(true);       // 加粗
    busText->setFont(font);

    // 设置文字颜色
    busText->setDefaultTextColor(Qt::darkBlue);

    // 添加所有需要的端口
    QMap<int, QList<int>> nodePorts;
    nodePorts[0] = {0, 1};
    nodePorts[1] = {2, 3};
    nodePorts[2] = {4};
    nodePorts[3] = {5, 6};
    nodePorts[4] = {7, 8};

    // 存储端口位置
    QMap<QPair<int, int>, QPointF> portPositions;

    // 为每个节点绘制指定的端口（考虑放大后的节点）
    for (int nodeId = 0; nodeId < busNodes.size(); ++nodeId) {
        QPointF nodeCenter = busNodes[nodeId] + QPointF(nodeSize/2, nodeSize/2); // 中心点计算
        QList<int> ports = nodePorts[nodeId];

        for (int i = 0; i < ports.size(); ++i) {
            int portId = ports[i];
            // 计算端口位置（分布在节点周围）
            qreal angle = 2 * M_PI * i / ports.size();
            qreal offsetX = 30 * qCos(angle);
            qreal offsetY = 30 * qSin(angle);
            QPointF portPos = nodeCenter + QPointF(offsetX, offsetY);

            // 绘制端口
            scene->addEllipse(portPos.x()-4, portPos.y()-4, 8, 8, QPen(Qt::red), QBrush(Qt::red));
            scene->addText(QString("P%1").arg(portId))->setPos(portPos + QPointF(5, -10));

            // 存储端口位置
            portPositions.insert(qMakePair(nodeId, portId), portPos);
        }
    }

    // L3和MemoryNode
    const int l3Y = 600;
    QList<QPointF> l3Positions = {{200, l3Y}, {400, l3Y}, {800, l3Y}, {1000, l3Y}};
    QList<int> l3Connections = {1, 3, 6, 8};
    QList<int> l3Nodes = {0, 1, 3, 4};

    // 存储L3分组框位置
    QList<QRectF> l3GroupRects;

    for (int i = 0; i < 4; ++i) {
        QPointF pos = l3Positions[i];
        drawModule(pos.x(), pos.y(), QString("L3Cache%1\nport %2").arg(i).arg(l3Connections[i]), QColor(255, 200, 0));

        // 连接点改为模块上边中心
        QPointF moduleTopCenter = pos + QPointF(moduleWidth/2, 0);
        QPointF portPos = portPositions.value(qMakePair(l3Nodes[i], l3Connections[i]));
        if (!portPos.isNull()) {
            QPainterPath path;
            path.moveTo(moduleTopCenter);
            path.lineTo(moduleTopCenter.x(), moduleTopCenter.y() - 50); // 垂直向上
            path.lineTo(portPos.x(), moduleTopCenter.y() - 50);         // 水平移动
            path.lineTo(portPos);                                       // 垂直连接到端口

            scene->addPath(path, QPen(Qt::darkGray, 2));
            QPointF labelPos((moduleTopCenter.x() + portPos.x())/2, moduleTopCenter.y() - 50);
            scene->addText(QString("port %1").arg(l3Connections[i]))->setPos(labelPos);
        }

        // 收集L3位置用于分组框
        l3GroupRects.append(QRectF(pos.x(), pos.y(), moduleWidth, moduleHeight));
    }

    // 添加L3分组框
    if (!l3GroupRects.isEmpty()) {
        QRectF l3GroupRect = l3GroupRects.first();
        for (const QRectF& rect : l3GroupRects) {
            l3GroupRect = l3GroupRect.united(rect);
        }
        l3GroupRect.adjust(-20, -20, 20, 20);
        QGraphicsRectItem* groupBox = scene->addRect(l3GroupRect);

        groupBox->setFlag(QGraphicsItem::ItemIsSelectable);
        groupBox->setData(0, "L3Group");
        groupBox->setZValue(-1);  //确保在下层

        connect(scene, &QGraphicsScene::selectionChanged, this, [=]() {
            if (groupBox->isSelected()) {
                infoManager->showInfo("L3Group");
            }
        });

    }

    // 添加 cache_event_trace 标签
    QRectF traceLabelRect(l3GroupRect.bottomRight() + QPointF(1010, 720), QSizeF(160, 60));
    QGraphicsRectItem* traceLabel = scene->addRect(traceLabelRect, QPen(Qt::darkGreen), QBrush(Qt::yellow));
    scene->addText("cache_event\ntrace")->setPos(traceLabelRect.topLeft() + QPointF(2, 2));

    //连接线到Bus
    QPointF traceRightCenter = traceLabelRect.center() + QPointF(traceLabelRect.width() / 2, 0);
    QPointF busNodeRightCenter = busNodes.last() + QPointF(nodeSize/2, nodeSize/2);
    QPointF busRightPoint = busNodeRightCenter + QPointF(nodeSize / 2 - 25, 28) ;

    QPainterPath tracePath;
    tracePath.moveTo(traceRightCenter);
    tracePath.lineTo(traceRightCenter.x() + 30, traceRightCenter.y());  // 右移
    tracePath.lineTo(traceRightCenter.x() + 30, busRightPoint.y()) ;     // 下移（或上移）
    tracePath.lineTo(busRightPoint);                                    // 连到 Bus 节点右侧
    scene->addPath(tracePath, QPen(Qt::darkGray, 2));



    // 响应
    traceLabel->setFlag(QGraphicsItem::ItemIsSelectable);
    connect(scene, &QGraphicsScene::selectionChanged, this, [=]() {
        if (traceLabel->isSelected()) {
            infoManager->showInfo("L3Group");
        }
    });


    // MemoryNode0
    QPointF node2Center = busNodes[2] + QPointF(nodeSize/2, nodeSize/2);
    qreal memX = node2Center.x() - moduleWidth/2;
    qreal memY = 800;
    drawModule(memX, memY, "MemoryNode0\nport 4", Qt::magenta);

    // 添加 DMA 框在 MemoryNode0 下方
    qreal dmaX = memX;
    qreal dmaY = memY + moduleHeight + 20;
    drawModule(dmaX, dmaY, "DMA", Qt::gray);

    // 添加连接线 MemoryNode → DMA
    QPainterPath dmaPath;
    dmaPath.moveTo(memX + moduleWidth/2, memY + moduleHeight);
    dmaPath.lineTo(memX + moduleWidth/2, dmaY);
    scene->addPath(dmaPath, QPen(Qt::darkGray, 2));


    // 连接到 Node2 的 port4 - 从上边中心引出
    QPointF memTopCenter = QPointF(memX + moduleWidth/2, memY);
    QPointF memPortPos = portPositions.value(qMakePair(2, 4));
    if (!memPortPos.isNull()) {
        QPainterPath path;
        path.moveTo(memTopCenter);
        path.lineTo(memTopCenter.x(), memTopCenter.y() - 50); // 垂直向上
        path.lineTo(memPortPos.x(), memTopCenter.y() - 50);   // 水平移动
        path.lineTo(memPortPos);                              // 垂直连接到端口

        scene->addPath(path, QPen(Qt::darkGray, 2));
        QPointF labelPos((memTopCenter.x() + memPortPos.x())/2, memTopCenter.y() - 50);
        scene->addText("port 4")->setPos(labelPos);
    }

    // 绘制 CPU 和 L2 (位置不变)
    const int cpuY = 100;
    const int l2Y = 200;
    QList<QPointF> cpuPositions, l2Positions;
    for (int i = 0; i < 4; ++i) {
        qreal x = l3Positions[i].x();
        cpuPositions.append({x, cpuY});
        l2Positions.append({x, l2Y});
    }

    QList<int> l2Connections = {0, 2, 5, 7};
    QList<int> l2Nodes = {0, 1, 3, 4};

    // 存储CPU-L2分组框
    QList<QRectF> cpuGroupRects;

    for (int i = 0; i < 4; ++i) {
        drawModule(cpuPositions[i].x(), cpuPositions[i].y(), QString("CPU%1").arg(i), QColor(0, 255, 255));

        drawModule(l2Positions[i].x(), l2Positions[i].y(),
                  QString("L2Cache%1\nport %2").arg(i).arg(l2Connections[i]), QColor(0, 255, 0));

        QPointF cpuBottomCenter = cpuPositions[i] + QPointF(moduleWidth/2, moduleHeight);
        QPointF l2TopCenter = l2Positions[i] + QPointF(moduleWidth/2, 0);

        QPainterPath path;
        path.moveTo(cpuBottomCenter);
        path.lineTo(cpuBottomCenter.x(), cpuBottomCenter.y() + 30); // 垂直向下
        path.lineTo(l2TopCenter.x(), cpuBottomCenter.y() + 30);     // 水平移动
        path.lineTo(l2TopCenter);                                   // 垂直向上

        scene->addPath(path, QPen(Qt::darkGray, 2));

        // L2Cache 连接到总线节点 - 从L2底部中心引出
        QPointF l2BottomCenter = l2Positions[i] + QPointF(moduleWidth/2, moduleHeight);
        QPointF portPos = portPositions.value(qMakePair(l2Nodes[i], l2Connections[i]));
        if (!portPos.isNull()) {
            QPainterPath busPath;
            busPath.moveTo(l2BottomCenter);
            busPath.lineTo(l2BottomCenter.x(), l2BottomCenter.y() + 50); // 垂直向下
            busPath.lineTo(portPos.x(), l2BottomCenter.y() + 50);        // 水平移动
            busPath.lineTo(portPos);                                     // 垂直连接到端口

            scene->addPath(busPath, QPen(Qt::darkGray, 2));
            QPointF labelPos((l2BottomCenter.x() + portPos.x())/2, l2BottomCenter.y() + 50);
            scene->addText(QString("port %1").arg(l2Connections[i]))->setPos(labelPos);
        }

        // 创建CPU-L2分组框
        QRectF cpuRect(cpuPositions[i].x(), cpuPositions[i].y(), moduleWidth, moduleHeight);
        QRectF l2Rect(l2Positions[i].x(), l2Positions[i].y(), moduleWidth, moduleHeight);
        QRectF groupRect = cpuRect.united(l2Rect);
        groupRect.adjust(-20, -20, 20, 20);
        cpuGroupRects.append(groupRect);
    }

    // 绘制CPU-L2分组框
    for (int i = 0; i < cpuGroupRects.size(); ++i) {
        QGraphicsRectItem* groupBox = scene->addRect(cpuGroupRects[i]);
        groupBox->setPen(QPen(Qt::red, 2, Qt::DashLine));
        scene->addText(QString("CPU-L2 Group %1").arg(i))->setPos(cpuGroupRects[i].topLeft() + QPointF(10, -40));
    }
}

void MainWindow::drawModule(qreal x, qreal y, const QString &label, const QColor &color)
{
    const int moduleWidth = 150;
    const int moduleHeight = 80;

    auto rect = scene->addRect(x, y, moduleWidth, moduleHeight, QPen(Qt::black), QBrush(color));
    rect->setData(0, label); // 用于标识模块
    rect->setFlag(QGraphicsItem::ItemIsSelectable);

    // 添加文字
    QStringList lines = label.split("\\n");
    for (int i = 0; i < lines.size(); i++) {
        scene->addText(lines[i])->setPos(x + 5, y + 5 + i * 15);
    }

    // 监听点击自己时显示模块信息
    connect(scene, &QGraphicsScene::selectionChanged, this, [=]() {
        if (rect->isSelected()) {
            QString name = label.split('\n').first();  // 如 L3Cache1
            infoManager->showInfo(name);
        }
    });
}

