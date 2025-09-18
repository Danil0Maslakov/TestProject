#include <QMainWindow>
#include <QTimer>
#include <QThreadPool>
#include "fileprocessor.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_browseInputMask();
    void on_browseOutputPath();
    void on_startStop();
    void on_processProgress(qint64 processed, qint64 total, const QString &fileName);
    void on_processFinished(const QString &fileName, bool ok, const QString &message);
    void pollTimerTimeout();

private:
    Ui::MainWindow *ui;
    QTimer m_pollTimer;
    QThreadPool m_threadPool;
    bool m_running = false;

    void startProcessingOnce();
    void schedulePolling();
    void processFile(const QString &path);

    // helper
    QByteArray parseXorValue(const QString &text);
};
