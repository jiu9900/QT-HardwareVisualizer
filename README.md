# Qt HardwareVisualizer

简单的基于 Qt Graphics Framework 构建的片上系统（SoC）拓扑图可视化工具，支持模块分层建模、交互响应与动态信息展示。

##  核心特性

- 硬件模块（CPU/L2/L3/Memory/DMA）图形建模
- 自定义 QGraphicsItem 派生类，支持分组、端口与富文本标注
- 模块连接使用动态箭头绘制（Bezier 支持）
- 鼠标悬停放大 + 动态阴影渲染
- 点击高亮 + 信息面板动态绑定
- 支持逻辑标签（如 trace 点）、Bus 复用显示与模块融合展示

##  使用说明

```bash
git clone <repo>
cd project
# 使用 Qt Creator 打开或 Qt 6 CMake 构建运行
```
## 效果展示
![效果图](D:\Advanced-Programming\QT\HardwareVisualizer\效果.png)

