#include "HudRenderer.h"
#include <QFont>
#include <QColor>
#include <QRect>

void HudRenderer::render(QPainter& painter, int viewW, int viewH,
                          int score, int maxScore,
                          float elapsedSec, float battery,
                          int events, const QString& missionStatus) {
    painter.setRenderHint(QPainter::Antialiasing);

    // 半透明背景条（左上角）
    painter.fillRect(0, 0, 220, 130, QColor(20, 20, 30, 180));

    QFont font("Monospace", 10);
    font.setBold(true);
    painter.setFont(font);

    // 任务状态颜色
    QColor statusColor = Qt::white;
    if (missionStatus == "Running")   statusColor = QColor(0x00, 0xff, 0x80);
    if (missionStatus == "Completed") statusColor = QColor(0x00, 0xff, 0x00);
    if (missionStatus == "Failed")    statusColor = QColor(0xff, 0x40, 0x40);

    int y = 18;
    painter.setPen(statusColor);
    painter.drawText(8, y, QString("STATUS: %1").arg(missionStatus));

    y += 20;
    painter.setPen(QColor(0xff, 0xcc, 0x00));
    painter.drawText(8, y, QString("SCORE:  %1 / %2").arg(score).arg(maxScore));

    y += 20;
    painter.setPen(QColor(0x80, 0xd0, 0xff));
    int mins = (int)(elapsedSec / 60);
    int secs = (int)(elapsedSec) % 60;
    painter.drawText(8, y, QString("TIME:   %1:%2")
                    .arg(mins, 2, 10, QChar('0'))
                    .arg(secs, 2, 10, QChar('0')));

    y += 20;
    QColor batColor = (battery > 30) ? QColor(0x40, 0xff, 0x80) : QColor(0xff, 0x60, 0x20);
    painter.setPen(batColor);
    painter.drawText(8, y, QString("BAT:    %1%").arg((int)battery));

    y += 20;
    painter.setPen(QColor(0xd0, 0xd0, 0xd0));
    painter.drawText(8, y, QString("EVENTS: %1").arg(events));
}
