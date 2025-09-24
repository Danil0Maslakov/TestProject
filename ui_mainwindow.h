#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QTextEdit>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMainWindow>

namespace Ui {
class MainWindow {
public:
    QWidget *centralWidget;
    QLineEdit *lineEditInputMask;
    QPushButton *pushButtonBrowseInput;
    QLineEdit *lineEditOutput;
    QPushButton *pushButtonBrowseOutput;
    QLineEdit *lineEditXorValue;
    QCheckBox *checkBoxDeleteInput;
    QComboBox *comboBoxOnCollision;
    QRadioButton *radioButtonOnce;
    QRadioButton *radioButtonTimer;
    QSpinBox *spinBoxInterval;
    QPushButton *pushButtonStart;
    QTextEdit *textEditLog;
    QTableWidget *tableWidget;

    void setupUi(QMainWindow *MainWindow) {
        MainWindow->setWindowTitle("XOR File Processor");
        MainWindow->resize(800,600);

        // Центральный виджет
        centralWidget = new QWidget(MainWindow);
        MainWindow->setCentralWidget(centralWidget);

        QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

        // Input
        QHBoxLayout *inputLayout = new QHBoxLayout();
        inputLayout->addWidget(new QLabel("Input mask/file:"));
        lineEditInputMask = new QLineEdit(centralWidget);
        inputLayout->addWidget(lineEditInputMask);
        pushButtonBrowseInput = new QPushButton("...", centralWidget);
        inputLayout->addWidget(pushButtonBrowseInput);
        mainLayout->addLayout(inputLayout);

        // Output
        QHBoxLayout *outputLayout = new QHBoxLayout();
        outputLayout->addWidget(new QLabel("Output directory:"));
        lineEditOutput = new QLineEdit(centralWidget);
        outputLayout->addWidget(lineEditOutput);
        pushButtonBrowseOutput = new QPushButton("...", centralWidget);
        outputLayout->addWidget(pushButtonBrowseOutput);
        mainLayout->addLayout(outputLayout);

        // XOR value
        QHBoxLayout *xorLayout = new QHBoxLayout();
        xorLayout->addWidget(new QLabel("XOR key (8 bytes):"));
        lineEditXorValue = new QLineEdit(centralWidget);
        lineEditXorValue->setPlaceholderText("0x1122334455667788");
        xorLayout->addWidget(lineEditXorValue);
        mainLayout->addLayout(xorLayout);

        // Options
        checkBoxDeleteInput = new QCheckBox("Delete input after processing", centralWidget);
        mainLayout->addWidget(checkBoxDeleteInput);

        QHBoxLayout *collisionLayout = new QHBoxLayout();
        collisionLayout->addWidget(new QLabel("On collision:"));
        comboBoxOnCollision = new QComboBox(centralWidget);
        comboBoxOnCollision->addItems({"Overwrite", "Add counter"});
        collisionLayout->addWidget(comboBoxOnCollision);
        mainLayout->addLayout(collisionLayout);

        QHBoxLayout *modeLayout = new QHBoxLayout();
        radioButtonOnce = new QRadioButton("Once", centralWidget);
        radioButtonTimer = new QRadioButton("Timer", centralWidget);
        radioButtonOnce->setChecked(true);
        modeLayout->addWidget(radioButtonOnce);
        modeLayout->addWidget(radioButtonTimer);
        spinBoxInterval = new QSpinBox(centralWidget);
        spinBoxInterval->setMinimum(1);
        spinBoxInterval->setSuffix(" s");
        modeLayout->addWidget(spinBoxInterval);
        mainLayout->addLayout(modeLayout);

        // Start button
        pushButtonStart = new QPushButton("Start", centralWidget);
        mainLayout->addWidget(pushButtonStart);

        // Log
        mainLayout->addWidget(new QLabel("Log:"));
        textEditLog = new QTextEdit(centralWidget);
        textEditLog->setReadOnly(true);
        mainLayout->addWidget(textEditLog, 1); // тянется по вертикали

        // Progress table
        mainLayout->addWidget(new QLabel("Progress:"));
        tableWidget = new QTableWidget(centralWidget);
        tableWidget->setColumnCount(4);
        tableWidget->setHorizontalHeaderLabels({"File","Progress","Status","Message"});
        mainLayout->addWidget(tableWidget, 2);
    }
};
}

#endif // UI_MAINWINDOW_H
