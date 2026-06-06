#pragma once
#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QSlider>
#include <QTextEdit>
#include <QGroupBox>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <memory>
#include "RenderWidget.h"
#include "gaussian/GaussianCloud.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override = default;

    void loadFromCommandLine(const QString &path);

private slots:
    void onOpenGaussian();
    void onReload();
    void onResetView();
    void onFitToCloud();
    void onRenderModeChanged(int idx);
    void onShowGridToggled(bool v);
    void onShowAxisToggled(bool v);
    void onShowBBoxToggled(bool v);
    void onPointSizeChanged(double v);
    void onSplatScaleChanged(double v);
    void onAlphaScaleChanged(double v);
    void onTakeScreenshot();
    void onFpsUpdated(float fps, float frameMs);

private:
    void buildUi();
    void loadGaussian(const QString &path);
    void updateStatsPanel();
    void appendLog(const QString &msg);
    QString estimateVramMB() const;

    RenderWidget *m_renderWidget = nullptr;

    // 左侧面板控件
    QPushButton *m_btnOpen = nullptr;
    QPushButton *m_btnReload = nullptr;
    QPushButton *m_btnResetView = nullptr;
    QPushButton *m_btnFitToCloud = nullptr;
    QPushButton *m_btnScreenshot = nullptr;
    QComboBox *m_cmbRenderMode = nullptr;
    QCheckBox *m_chkGrid = nullptr;
    QCheckBox *m_chkAxis = nullptr;
    QCheckBox *m_chkBBox = nullptr;
    QDoubleSpinBox *m_spnPointSize = nullptr;
    QDoubleSpinBox *m_spnSplatScale = nullptr;
    QDoubleSpinBox *m_spnAlphaScale = nullptr;

    // 统计面板控件
    QLabel *m_lblFilePath = nullptr;
    QLabel *m_lblFormat = nullptr;
    QLabel *m_lblCount = nullptr;
    QLabel *m_lblHasOpacity = nullptr;
    QLabel *m_lblHasScale = nullptr;
    QLabel *m_lblHasRotation = nullptr;
    QLabel *m_lblHasSH = nullptr;
    QLabel *m_lblBBoxMin = nullptr;
    QLabel *m_lblBBoxMax = nullptr;
    QLabel *m_lblBBoxSize = nullptr;
    QLabel *m_lblLoadTime = nullptr;
    QLabel *m_lblFps = nullptr;
    QLabel *m_lblFrameMs = nullptr;
    QLabel *m_lblRenderMode = nullptr;
    QLabel *m_lblPointSize = nullptr;
    QLabel *m_lblVram = nullptr;
    QLabel *m_lblRelation = nullptr;
    QLabel *m_lblFutureAlign = nullptr;

    // 日志
    QTextEdit *m_logEdit = nullptr;

    std::shared_ptr<GaussianCloud> m_cloud;
    QString m_currentFile;
};
