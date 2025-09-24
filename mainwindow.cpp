#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QStandardPaths>
#include <QMessageBox>
#include <QDir>
#include <QFileInfo>
#include <QDateTime>
#include <QMutexLocker>
#include<QInputDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // default values
    ui->lineEditOutput->setText(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
    ui->spinBoxInterval->setValue(5);
    ui->checkBoxDeleteInput->setChecked(false);
    ui->comboBoxOnCollision->addItems({"Overwrite","Add counter"});

    connect(ui->pushButtonBrowseInput, &QPushButton::clicked, this, &MainWindow::on_browseInputMask);
    connect(ui->pushButtonBrowseOutput, &QPushButton::clicked, this, &MainWindow::on_browseOutputPath);
    connect(ui->pushButtonStart, &QPushButton::clicked, this, &MainWindow::on_startStop);

    connect(&m_pollTimer, &QTimer::timeout, this, &MainWindow::pollTimerTimeout);

    // thread pool
    m_threadPool.setMaxThreadCount(QThread::idealThreadCount());

    // UI: progress table
    ui->tableWidget->setColumnCount(4);
    ui->tableWidget->setHorizontalHeaderLabels({"File","Progress","Status","Message"});

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_browseInputMask()
{
    QString mask = QInputDialog::getText(this, "Input mask or file", "Enter mask (e.g. *.bin or C:/path/*.bin) or single file:");
    if (!mask.isEmpty()) ui->lineEditInputMask->setText(mask);
}

void MainWindow::on_browseOutputPath()
{
    QString dir = QFileDialog::getExistingDirectory(this, "Select output directory", ui->lineEditOutput->text());
    if (!dir.isEmpty()) ui->lineEditOutput->setText(dir);
}

void MainWindow::on_startStop()
{
    if (!m_running) {
        m_running = true;
        ui->pushButtonStart->setText("Stop");
        ui->textEditLog->append("Starting...");
        if (ui->radioButtonOnce->isChecked()) {
            startProcessingOnce();
            m_running = false;
            ui->pushButtonStart->setText("Start");
        } else {
            schedulePolling();
        }
    } else {
        m_running = false;
        m_pollTimer.stop();
        ui->pushButtonStart->setText("Start");
        ui->textEditLog->append("Stopped by user");
    }
}

void MainWindow::schedulePolling()
{
    int seconds = ui->spinBoxInterval->value();
    m_pollTimer.start(seconds * 1000);
    ui->textEditLog->append(QString("Polling every %1 s").arg(seconds));
    // trigger immediately
    pollTimerTimeout();
}

void MainWindow::pollTimerTimeout()
{
    if (!m_running) return;
    QString mask = ui->lineEditInputMask->text();
    if (mask.isEmpty()) return;
    qDebug()<<mask;
    // simple mask handling: if contains wildcard, use QDir::entryList
    QFileInfo fi(mask);

    QString path = fi.path();
    qDebug()<<path;
    QString pattern = fi.fileName();
    if (path.isEmpty()) path = QDir::currentPath();
    QDir dir(path);
    qDebug()<<dir;
    QStringList found;
    if (pattern.contains('*') || pattern.contains('?')) {
        found = dir.entryList(QStringList(pattern), QDir::Files);
        for (QString f : found) processFile(dir.filePath(f));
    } else {
        // treat as full filename
        QFileInfo single(mask);
        qDebug()<<"single info: "<<single.absoluteFilePath();
        qDebug()<<"single exists: "<<single.exists();
        QString path = single.absoluteFilePath();
        qDebug() << path << path.size();
        qDebug() << "Current path:" << QDir::currentPath();

        if (single.exists()) processFile(single.absoluteFilePath());
    }
}

void MainWindow::startProcessingOnce()
{
    QString mask = ui->lineEditInputMask->text();
    qDebug() << "Mask string:" << mask;

    if (mask.isEmpty()) {
        QMessageBox::warning(this, "Error", "Input mask or file is empty");
        return;
    }
    // reuse polling logic but only once
    m_running = true;
    pollTimerTimeout();
}

void MainWindow::processFile(const QString &path)
{
    qDebug()<<"ProcessFile";
    // create worker
    auto xorVal = parseXorValue(ui->lineEditXorValue->text());
    if (xorVal.size() != 8) {
        ui->textEditLog->append("XOR value must be 8 bytes (provide as hex like 0x1122334455667788)");
        return;
    }

    FileProcessor *worker = new FileProcessor(path,
                                              ui->lineEditOutput->text(),
                                              ui->checkBoxDeleteInput->isChecked(),
                                              ui->comboBoxOnCollision->currentIndex(),
                                              xorVal);
    worker->setAutoDelete(true);

    connect(worker, &FileProcessor::progress, this, &MainWindow::on_processProgress);
    connect(worker, &FileProcessor::finished, this, &MainWindow::on_processFinished);

    m_threadPool.start(worker);

    ui->textEditLog->append(QString("Scheduled: %1").arg(path));
}

void MainWindow::on_processProgress(qint64 processed, qint64 total, const QString &fileName)
{
    // update table or single progress bar
    Q_UNUSED(processed)
    Q_UNUSED(total)
    Q_UNUSED(fileName)
    // For brevity: append to log
    ui->textEditLog->append(QString("Progress %1/%2 for %3").arg(processed).arg(total).arg(fileName));
}

void MainWindow::on_processFinished(const QString &fileName, bool ok, const QString &message)
{
    qDebug()<<"PrFinished";
    ui->textEditLog->append(QString("Finished %1 : %2 (%3)").arg(fileName).arg(ok?"OK":"FAIL").arg(message));
}

QByteArray MainWindow::parseXorValue(const QString &text)
{
    QString t = text.trimmed();
    if (t.startsWith("0x") || t.startsWith("0X")) t = t.mid(2);
    QByteArray out;
    bool ok = true;
    for (int i=0;i+1<t.size() && out.size()<8;i+=2) {
        bool conv;
        quint8 b = t.mid(i,2).toUInt(&conv,16);
        if (!conv) { ok=false; break; }
        out.append((char)b);
    }
    if (out.size() == 8 && ok) return out;
    // try interpreting as text
    if (t.size() >=8) return t.left(8).toUtf8();
    // pad zeros
    while (out.size()<8) out.append('\0');
    return out;
}
