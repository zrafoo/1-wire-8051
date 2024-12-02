#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "stdio.h"
#include <QQmlContext>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    populatePorts();

    connect(&serial, SIGNAL(readyRead()), this, SLOT(handleReadyRead()));
    connect(ui->comboBox_format, &QComboBox::currentTextChanged, this, &MainWindow::onFormatChanged);
    chartData = new ChartData(this);

    // Загружаем QML окно
    engine = new QQmlApplicationEngine(this);
    engine->rootContext()->setContextProperty("chartData", chartData);
    engine->load(QUrl::fromLocalFile("C:/Users/nisergeev/Documents/ьвшвшрош/terminal 2/Chart.qml"));

    if (engine->rootObjects().isEmpty())
        qDebug() << "Не удалось загрузить QML";
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::populatePorts()
{
    ui->portname->clear();

    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        ui->portname->addItem(info.portName());
    }
}

void MainWindow::on_pushButton_connect_clicked()
{
    serial.setPortName(ui->portname->currentText());
    qDebug() << ui->portname->currentText();
    serial.setBaudRate(ui->baudrate->currentText().toUInt());
    qDebug() << serial.baudRate();

    if (ui->stopbits->currentText() == "One Stop Bit")
        serial.setStopBits(QSerialPort::OneStop);

    qDebug() << serial.stopBits();
    if (ui->parity->currentText() == "None")
        serial.setParity(QSerialPort::NoParity);
    qDebug() << serial.parity();
    if (ui->databits->currentText() == "Eight")
        serial.setDataBits(QSerialPort::Data8);
    qDebug() << serial.dataBits();
    serial.open(QSerialPort::ReadWrite);
}

void MainWindow::on_pushButton_disconnect_clicked()
{
    if (serial.isOpen())
        serial.close();
}

void MainWindow::on_send_clicked()
{
    QString text = ui->lineEdit_input->text();
    QString format = ui->comboBox_format->currentText();
    QByteArray bytesToSend = prepareDataToSend(text, format);
    serial.write(bytesToSend);
    input.append(bytesToSend);
    if (!serial.waitForBytesWritten()){
        qDebug() << bytesToSend;
    }
    QString formattedData = convertData(input, format);
    ui->textEdit_input->setPlainText(formattedData);
}

void MainWindow::on_clear_input_clicked()
{
    ui->textEdit_input->clear();
    input.clear();
}

void MainWindow::on_clear_output_clicked()
{
    ui->textEdit_output->clear();
    output.clear();
}


void MainWindow::handleReadyRead()
{
    QByteArray data = serial.readAll();
    buffer.append(data);

    // Выводим входящие данные для отладки
    //qDebug() << "Incoming data:" << data.toHex(' ');

    while (buffer.size() >= 4) {
        // Ищем стартовый байт
        int startIndex = buffer.indexOf(static_cast<char>(0xFF));
        if (startIndex == -1) {
            // Стартовый байт не найден, очищаем буфер
            buffer.clear();
            return;
        }

        // Убеждаемся, что у нас достаточно данных после стартового байта
        if (buffer.size() - startIndex < 4) {
            // Ждем прихода оставшихся байт
            return;
        }

        // Извлекаем байты пакета
        QByteArray packet = buffer.mid(startIndex, 4);
        buffer.remove(0, startIndex + 4);

        unsigned char start_byte = static_cast<unsigned char>(packet.at(0));
        unsigned char data_byte1 = static_cast<unsigned char>(packet.at(1));
        unsigned char data_byte2 = static_cast<unsigned char>(packet.at(2));
        unsigned char received_checksum = static_cast<unsigned char>(packet.at(3));

        // Вычисляем контрольную сумму
        unsigned char computed_checksum = (start_byte + data_byte1 + data_byte2) % 256;

        if (computed_checksum == received_checksum) {
            // Контрольная сумма совпадает, обрабатываем данные
            int16_t rawTemperature = (data_byte2 << 8) | data_byte1;
            double temperature = rawTemperature * 0.0625;

                        // Округляем напряжение до 3 знаков после запятой
            temperature = (temperature * 100.0) / 100.0;

                    // Добавляем точку на график
                    chartData->appendData(temperature);

                    // Отображаем температуру в текстовом выводе
                    ui->textEdit_output->append(QString::number(temperature, 'f', 2) + " °C");
        } else {
            // Контрольная сумма не совпадает, пакет поврежден
            qDebug() << "Checksum mismatch. Packet discarded.";
        }
    }
}
void MainWindow::onFormatChanged(const QString &format) {
    QString formattedData = convertData(output, format);
    QString formattedInput = convertData(input, format);
    ui->textEdit_output->setPlainText(formattedData);
    ui->textEdit_input->setPlainText(formattedInput);
}


QString MainWindow::convertData(const QByteArray &data, const QString &format) {
    if (format == "ASCII") {
        return QString::fromLatin1(data);
    } else if (format == "HEX") {
        return data.toHex(' ').toUpper();
    } else if (format == "BIN") {
        QString binOutput;
        for (char byte : data) {
            binOutput.append(QString("%1 ").arg(static_cast<unsigned char>(byte), 8, 2, QLatin1Char('0')));
        }
        return binOutput.trimmed();
    }
    return QString::fromLatin1(data);
}

QByteArray MainWindow::prepareDataToSend(const QString &data, const QString &format) {
    if (format == "ASCII") {
        // Отправляем данные как есть в формате ASCII
        return data.toLatin1();
    } else if (format == "HEX") {
        // Преобразуем строку HEX в байты
        QByteArray bytes;
        QStringList hexList = data.split(' ', Qt::SkipEmptyParts);
        for (const QString &hexStr : hexList) {
            bool ok;
            uint byte = hexStr.toUInt(&ok, 16);
            if (ok) {
                bytes.append(static_cast<char>(byte));
            } else {
                // Обработка ошибки при неверном формате
                qDebug() << "Invalid HEX value:" << hexStr;
            }
        }
        return bytes;
    } else if (format == "BIN") {
        // Преобразуем бинарную строку в байты
        QByteArray bytes;
        QStringList binList = data.split(' ', Qt::SkipEmptyParts);
        for (const QString &binStr : binList) {
            bool ok;
            uint byte = binStr.toUInt(&ok, 2);
            if (ok) {
                bytes.append(static_cast<char>(byte));
            } else {
                qDebug() << "Invalid BIN value:" << binStr;
            }
        }
        return bytes;
    }
    // По умолчанию отправляем как ASCII
    return data.toLatin1();
}
