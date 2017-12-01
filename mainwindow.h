#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QSerialPortInfo>
#include "qcustomplot.h"


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_port_currentIndexChanged(int index); //Slot wywoływany przy zmianie aktywnego urządzenia
    void refresh();                              //Odświeżenie urządzeń

    void on_commandLine_returnPressed();


    /* Obsługa przycisków */
    void on_enterButton_clicked();
    void on_startButton_clicked();
    void on_stopButton_clicked();
    void on_refreshButton_clicked();
    void on_clearButton_clicked();


    /* Odczyt i parsowanie danych */
    void readData();



private:
    Ui::MainWindow *ui;

    /* poszukuje urządzeń */
    void searchDevices();

    /* Tekst na konsole, przy okazji wysłanie danych do arduino jeśli jest taka potrzeba */
    void addTextToConsole(QString text, bool sender=false);

    /* Tylko wysyłanie danych */
    void send(QString msg);

    /* Inicjalizacjai odświeżenie widgeta odpowiedzialnego za wykresy */
    void initPlot();
    void updatePlot();




    QList <QSerialPortInfo> available_port;     // lista dostępnych portów
    const QSerialPortInfo *info;                // wybrany port

    QSerialPort port;                           // otwarty port



    int plotindex;                              // Index na potrzeby plot X
    int oldplotindex;                           // Potrzebny przy wyswietlaniu numeru wiersza

    int read;                                   // Pozwolenie na odczyt (użyte przy chwilowym wyłączeniu odczytu na czas przesłania komendy)

    int status;                                 // status - run 1 lub stop 0


    /* Vektory na potrzeby rysowania */

    QVector<double> plot_X;                     // Oś X

    QVector<double> plot_SP;                    // Setpoint
    QVector<double> plot_FB;                    // Feedback
    QVector<double> plot_ERR;                   // Error
    QVector<double> plot_I;                     // Integral
    QVector<double> plot_OUT;                   // Output

    /* Pomoc przy skalowaniu wykresu */
    double plot_y_min;
    double plot_y_max;
    double set_y_max;


};



#endif // MAINWINDOW_H
