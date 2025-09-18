#include "fileprocessor.h"
#include <QFileInfo>
#include <QDir>
#include <QIODevice>

FileProcessor::FileProcessor(const QString &inputPath, const QString &outputDir, bool deleteInput, int collisionMode, const QByteArray &xor8)
    : m_inputPath(inputPath), m_outputDir(outputDir), m_deleteInput(deleteInput), m_collisionMode(collisionMode), m_xor8(xor8)
{
    setAutoDelete(false); // we will delete manually by QRunnable lifecycle in threadpool
}

QString FileProcessor::chooseOutputPath(const QString &inFile)
{
    QFileInfo fi(inFile);
    QString base = fi.fileName();
    QDir outDir(m_outputDir);
    if (!outDir.exists()) outDir.mkpath(".");
    QString outPath = outDir.filePath(base);
    if (QFile::exists(outPath)) {
        if (m_collisionMode == 0) return outPath; // overwrite
        // add counter
        int cnt = 1;
        QString name = fi.completeBaseName();
        QString ext = fi.suffix();
        while (QFile::exists(outDir.filePath(QString("%1_%2.%3").arg(name).arg(cnt).arg(ext)))) cnt++;
        return outDir.filePath(QString("%1_%2.%3").arg(name).arg(cnt).arg(ext));
    }
    return outPath;
}

void FileProcessor::run()
{
    QFile in(m_inputPath);
    if (!in.open(QIODevice::ReadOnly)) {
        emit finished(m_inputPath, false, "Cannot open input");
        return;
    }

    QString outPath = chooseOutputPath(m_inputPath);
    QFile out(outPath);
    if (!out.open(QIODevice::WriteOnly)) {
        emit finished(m_inputPath, false, "Cannot open output");
        in.close();
        return;
    }

    qint64 total = in.size();
    qint64 processed = 0;
    const qint64 chunkSize = 1024*1024; // 1 MB
    QByteArray buffer;
    buffer.resize(chunkSize);

    while (!in.atEnd()) {
        qint64 toRead = qMin(chunkSize, total - processed);
        qint64 got = in.read(buffer.data(), toRead);
        if (got <= 0) break;
        // apply XOR 8-byte key by repeating
        for (qint64 i=0;i<got;i++) {
            buffer[(int)i] = buffer[(int)i] ^ m_xor8.at(i % 8);
        }
        qint64 written = out.write(buffer.constData(), got);
        if (written != got) {
            emit finished(m_inputPath, false, "Write error");
            in.close(); out.close();
            return;
        }
        processed += got;
        emit progress(processed, total, m_inputPath);
    }

    out.flush();
    out.close();
    in.close();

    if (m_deleteInput) {
        QFile::remove(m_inputPath);
    }

    emit finished(m_inputPath, true, outPath);
}
