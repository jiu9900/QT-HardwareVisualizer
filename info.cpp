// info.cpp
#include "info.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QCoreApplication>
#include <QDir>


InfoManager::InfoManager(QObject *parent)
    : QObject(parent)
{
    loadPerfData();
}

void InfoManager::setTextEdit(QTextEdit *textEdit)
{
    this->infoWidget = textEdit;
}

void InfoManager::loadPerfData()
{
    QString projectDir = QCoreApplication::applicationDirPath();

    // 向上回溯两级到项目根目录
    projectDir = QDir::cleanPath(projectDir + "/../..");

    // 构建正确的文件路径
    QString filePath = projectDir + "/HardwareVisualizer/perfdata.txt";
    qDebug() << "尝试路径:" << filePath;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "无法打开性能数据文件:" << filePath;
        qWarning() << "错误信息:" << file.errorString();
        return;
    }

    QString currentModule;
    QTextStream in(&file);

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();

        // 跳过空行和注释
        if (line.isEmpty() || line.startsWith("#")) continue;

        // 模块标题
        if (line.startsWith("[") && line.endsWith("]")) {
            currentModule = line.mid(1, line.length() - 2);
            continue;
        }

        // 键值对
        int eqIndex = line.indexOf('=');
        if (eqIndex != -1 && !currentModule.isEmpty()) {
            QString key = line.left(eqIndex).trimmed();
            QString value = line.mid(eqIndex + 1).trimmed();
            perfData[currentModule][key] = value;
        }
    }

    file.close();
}

void InfoManager::showInfo(const QString &moduleName)
{
    if (!infoWidget) return;

    QString info;

    // 检查是否有性能数据
    if (perfData.contains(moduleName)) {
        info = QString("===== %1 性能数据 =====\n\n").arg(moduleName);

        // 添加基本描述
        if (moduleName.startsWith("CPU")) {
            info += "中央处理器单元性能数据\n\n";
        } else if (moduleName.startsWith("L2Cache")) {
            info += "二级缓存性能数据\n\n";
        } else if (moduleName.startsWith("L3Cache")) {
            info += "三级缓存性能数据\n\n";
        } else if (moduleName == "MemoryNode0") {
            info += "主内存性能数据\n\n";
        } else if (moduleName == "DMA") {
            info += "DMA控制器性能数据\n\n";
        } else if (moduleName == "cache_event_trace") {
            info += "缓存事件跟踪数据\n\n";
        } else if (moduleName == "Bus") {
            info += "总线系统性能数据\n\n";
        } else if (moduleName == "L3Group") {
            info += "L3缓存组综合数据\n\n";
        }

        // 添加性能指标
        const QMap<QString, QString> &metrics = perfData[moduleName];
        for (auto it = metrics.constBegin(); it != metrics.constEnd(); ++it) {
            info += QString("%1: %2\n").arg(it.key(), it.value());
        }
    } else {
        // 没有性能数据时使用原来的静态信息
        if (moduleName == "CPU0") {
            info = "这是 CPU0 的详细信息：主频 2.5GHz，4 核，任务调度中枢。";
        } else if (moduleName == "CPU1") {
            info = "这是 CPU1 的详细信息：主频 2.4GHz，负责图形运算。";
        } else if (moduleName == "CPU2") {
            info = "这是 CPU2 的详细信息：主频 2.6GHz，处理后台服务。";
        } else if (moduleName == "CPU3") {
            info = "这是 CPU3 的详细信息：主频 2.3GHz，负责 IO 管理。";
        } else if (moduleName == "L2Cache0") {
            info = "这是 L2Cache0：连接 CPU0，容量 512KB，延迟低。";
        } else if (moduleName == "L2Cache1") {
            info = "这是 L2Cache1：连接 CPU1，容量 512KB，用于图形缓冲。";
        } else if (moduleName == "L2Cache2") {
            info = "这是 L2Cache2：连接 CPU2，容量 512KB，数据预取增强。";
        } else if (moduleName == "L2Cache3") {
            info = "这是 L2Cache3：连接 CPU3，容量 512KB，快速访问 IO 相关缓存。";
        } else if (moduleName == "L3Cache0") {
            info = "这是 L3Cache0：连接 L2Cache0，容量 2MB。";
        } else if (moduleName == "L3Cache1") {
            info = "这是 L3Cache1：连接 L2Cache1，容量 2MB。";
        } else if (moduleName == "L3Cache2") {
            info = "这是 L3Cache2：连接 L2Cache2，容量 2MB。";
        } else if (moduleName == "L3Cache3") {
            info = "这是 L3Cache3：连接 L2Cache3，容量 2MB。";
        } else if (moduleName == "MemoryNode0") {
            info = "这是内存模块 MemoryNode0：容量 8GB，DDR4。";
        } else if (moduleName == "L3Group") {
            info = "这是 L3 缓存组的综合信息：4 路 L3 缓存，共享型设计，总容量 8MB。";
        } else if (moduleName == "BusNode") {
            info = "这是 Bus 总线节点：用于核心互联，每个 Node 支持多个端口通信。";
        } else {
            info = "未知模块：" + moduleName;
        }
    }

    infoWidget->setPlainText(info);
}
