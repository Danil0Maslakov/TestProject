#include <QObject>
#include <QRunnable>
#include <QFile>

        class FileProcessor : public QObject, public QRunnable
{
    Q_OBJECT
public:
    // collisionMode: 0 = overwrite, 1 = add counter
    FileProcessor(const QString &inputPath, const QString &outputDir, bool deleteInput, int collisionMode, const QByteArray &xor8);
    void run() override;

signals:
    void progress(qint64 processed, qint64 total, const QString &fileName);
    void finished(const QString &fileName, bool ok, const QString &message);

private:
    QString m_inputPath;
    QString m_outputDir;
    bool m_deleteInput;
    int m_collisionMode;
    QByteArray m_xor8;

    QString chooseOutputPath(const QString &inFile);
};
