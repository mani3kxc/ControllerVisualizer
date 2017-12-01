#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QScrollBar>
#include <QMessageBox>
#include <QFile>
#include <QDebug>


// funkcje
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    info = NULL;


    searchDevices();            // Szukamy urządzeń

    ReadSettings();

    initPlot();                 // Inicjalizacja wykresu

    connect(ui->action_refreshPorts,SIGNAL(triggered()),this,SLOT(refresh())); //Trigger - akcja po wybraniu opcji odświeżenia urządzeń
    connect(&port, SIGNAL(readyRead()), this, SLOT(readData())); //Trigger - czytamy dane gdy tylko będzie gotowość do odczytu

}

MainWindow::~MainWindow()
{
    if(port.isOpen())
        port.close();

    SaveSettings();

    delete ui;
}
/* Odczyt pól z pliku CSV i ustawienie ich w programie */
void MainWindow::ReadSettings()
{

    QStringList fieldNames;
    QStringList defaultValues;

    fieldNames << "form_KPup_" << "form_KPdown_" << "form_KIup_" << "form_KIdown_" << "form_CP_" << "form_SP_" << "form_Delay_" << "form_Acc_" << "form_MO_" << "form_Setpoint_";
    defaultValues << "1" << "2" << "3" << "4" << "5" << "6" << "7" << "8" << "9" << "10";
    int j=1;

     QFile file(QCoreApplication::applicationDirPath() +"/parameters.csv");
         if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
         {
             qDebug() << QCoreApplication::applicationDirPath() << file.errorString();
                     QMessageBox::warning(this, "Uwaga", " Nie znaleziono pliku CSV wypełniającego ustawień.\n Pozostaną one puste!");

                     while(j<=5)
                     {
                     for(int i = 0; i<fieldNames.length(); i++)
                     {

                         qDebug() << QString(fieldNames[i]) + QString::number(j) + "----> 0" + QString::number(i);

                         QLineEdit *field =  ui->centralWidget->findChild<QLineEdit *>(QString(fieldNames[i]) + QString::number(j));

                         field->setText( defaultValues[i] );

                     }
                     j++;
                     }

             return;
         }


        if(file.size()==0)
        {

            qDebug() << "SIZE ZERO";

            while(j<=5)
            {
            for(int i = 0; i<fieldNames.length(); i++)
            {

                qDebug() << QString(fieldNames[i]) + QString::number(j) + "----> 0" + QString::number(i);

                QLineEdit *field =  ui->centralWidget->findChild<QLineEdit *>(QString(fieldNames[i]) + QString::number(j));

                field->setText( defaultValues[i] );

            }
            j++;
            }

            return;

        }

           qDebug() << "SIZE ZERO2?";

         QTextStream in(&file);
         while (!in.atEnd()) {

             QString line = in.readLine();


             if(j==1)
                {
                 j++;
                 continue;
                }

             QStringList wordList;
             wordList = line.split(";");

             for(int i = 0; i< wordList.length(); i++)
             {

                 qDebug() << QString(fieldNames[i]) + QString::number(j-1) + "---->" + wordList[i];

                 QLineEdit *field =  ui->centralWidget->findChild<QLineEdit *>(QString(fieldNames[i]) + QString::number(j-1));

                 if(wordList[i]!="")
                    field->setText( wordList[i] );
                 else
                    field->setText("0");
             }
             j++;
             if(j==7)
                 break;
         }





         if(j<7)
         {
             while(j<=6)
             {
             for(int i = 0; i<fieldNames.length(); i++)
             {

                 qDebug() << QString(fieldNames[i]) + QString::number(j-1) + "----> 0" + QString::number(i);

                 QLineEdit *field =  ui->centralWidget->findChild<QLineEdit *>(QString(fieldNames[i]) + QString::number(j-1));

                 field->setText( defaultValues[i] );

             }
             j++;
             }
         }

         file.close();

         preset = 1;

}

void MainWindow::SaveSettings()
{
    QFile file(QCoreApplication::applicationDirPath() +"/parameters.csv");
        if (!file.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text))
        {
            qDebug() << QCoreApplication::applicationDirPath() << file.errorString();
                    QMessageBox::warning(this, "Uwaga", "Błąd zapisu do pliku CSV. Ustawienia pól nie zostały zachowane");
            return;}

            QTextStream stream(&file);

         QStringList fieldNames;

         fieldNames << "form_KPup_" << "form_KPdown_" << "form_KIup_" << "form_KIdown_" << "form_CP_" << "form_SP_" << "form_Delay_" << "form_Acc_" << "form_MO_" << "form_Setpoint_";

         QString line;

         stream << "KPup;KPdown;KIup;KIdown;CP;SP;Delay;Acc;MO;Setpoint" << endl;


         for(int j=1; j<6; j++)
         {

             line = "";

             for(int i = 0; i< fieldNames.length(); i++)
             {

                 QLineEdit *field =  ui->centralWidget->findChild<QLineEdit *>(QString(fieldNames[i]) + QString::number(j));

                 line += field->text() + ";";

             }

           stream << line.left(line.length() - 1) << endl;

        }



file.close();



}

/* aktualizacja listy urządzen */
void MainWindow::searchDevices()
{
    // dodanie ich do listy
    available_port = QSerialPortInfo::availablePorts();

    int porty = available_port.size();
    QString message = QString::number(porty) + (porty == 1 ? " port ready to use" : " ports ready to use");

    // Informacja do paska statusu
    ui->statusBar->showMessage(message,3000);

    // Lista wyboru
    info = NULL;
    ui->port->clear();
    ui->port->addItem("Wybierz Urządzenie");
    for(int i=0;i<porty;i++)
    {
        ui->port->addItem(available_port.at(i).description());
    }
}


/* Wybór portu */
void MainWindow::on_port_currentIndexChanged(int index)
{



    if(port.isOpen()) port.close();
    QString txt = "Wybierz Urządzenie";
    if (index > 0){
        info = &available_port.at(index-1);
        txt = info->description();

        port.setPort(available_port.at(index-1));

        if(!port.open(QIODevice::ReadWrite))
            QMessageBox::warning(this,"Błąd urządzenia","Nie można otworzyć portu.");

        plotindex=0;
        plot_y_max = 0;
        port.setBaudRate(115200);
        port.setDataTerminalReady(true);




        QTime dieTime= QTime::currentTime().addSecs(2);
            while (QTime::currentTime() < dieTime)
                QCoreApplication::processEvents(QEventLoop::AllEvents, 100);

         read = 1; //Można czytać
         on_clearButton_clicked();
         on_updateButton_1_clicked();

    }
    else
        info = NULL;

    ui->statusBar->showMessage("Wybrano urządzenie: " + txt,2000);


}

/* Odświeżenie listy urządzeń na rządanie */
void MainWindow::refresh()
{
    searchDevices();
}


/* Po enterze na commandline - wysłanie polecenia */
void MainWindow::on_commandLine_returnPressed()
{
    if(info == NULL) {
        ui->statusBar->showMessage("No port selected",3000);
        return;
    }

    addTextToConsole(ui->commandLine->text(),true);
    ui->commandLine->clear();
}

/* To samo po kliknięciu - wysłanie polecenia */
void MainWindow::on_enterButton_clicked()
{
    on_commandLine_returnPressed();
}

/* Obsługa przycisku Start */
void MainWindow::on_startButton_clicked()
{

    send("start");

    //addTextToConsole("start", true); Opcja gdybysmy chcieli w konsoli informacje o wysylaniu

    status=1;

    ui->commandLine->clear();
}

/* Obsługa przycisku Stop */
void MainWindow::on_stopButton_clicked()
{
     send("stop");
     //addTextToConsole("stop", true); Opcja w przypadku gdybyśmy chcieli w konsoli info o wysylaniu
}

/* Obsługa przycisku Clear */
void MainWindow::on_clearButton_clicked()
{

    plot_X.clear(); // Oś X

    plot_SP.clear(); // Setpoint
    plot_FB.clear(); // Feedback
    plot_ERR.clear(); // Error
    plot_I.clear(); // Integral
    plot_OUT.clear(); // Output

    plot_y_min=0;
    plot_y_max=0;
    set_y_max=0;

    status=0;

    plotindex=0;

    ui->customPlot->clearPlottables();
    initPlot();
    ui->customPlot->replot();

    ui->output->clear();
    ui->commandLine->clear();
}

/* Obsługa przycisku Refresh */
void MainWindow::on_refreshButton_clicked()
{
    /*QString sp, kPd, kPu, kId, kIu, acc, cp, se, i, max, fb, delay, msg;

    sp = "sp" + ui->form_SP->text() + ";"; //Setpoint
    kPd = "kPd" + ui->form_kPdown->text() + ";";
    kPu = "kPu" + ui->form_kPup->text() + ";";
    kId = "kId" + ui->form_kIdown->text() + ";";
    kIu = "kIu" + ui->form_kIup->text() + ";";
    acc = "acc" + ui->form_AC->text() + ";";
    cp = "cp" + ui->form_CP->text() + ";";
    se = "se" + ui->form_SP2->text() + ";"; //Błąd! //Sensor Period
    max = "max" + ui->form_MO->text() + ";";
    delay = "delay" + ui->form_Delay->text() + ";";

    set_y_max = ui->form_MO->text().toInt();

    send(sp);
    send(kPd);
    send(kPu);
    send(kId);
    send(kIu);
    send(acc);
    send(cp);
    send(se);
    send(max);
    send(delay);

    //send(sp+kPd+kPu+kId+kIu+acc+cp+se+max+delay+"start");


    status=1;

    ui->commandLine->clear();*/
}

void MainWindow::on_updateButton_1_clicked()
{
    QString sp, kPd, kPu, kId, kIu, acc, cp, se, i, max, fb, delay, msg;

    sp = "sp" + ui->form_Setpoint_1->text() + ";"; //Setpoint
    kPd = "kPd" + ui->form_KPdown_1->text() + ";";
    kPu = "kPu" + ui->form_KPup_1->text() + ";";
    kId = "kId" + ui->form_KIdown_1->text() + ";";
    kIu = "kIu" + ui->form_KIup_1->text() + ";";
    acc = "acc" + ui->form_Acc_1->text() + ";";
    cp = "cp" + ui->form_CP_1->text() + ";";
    se = "se" + ui->form_SP_1->text() + ";"; //Błąd! //Sensor Period
    max = "max" + ui->form_MO_1->text() + ";";
    delay = "delay" + ui->form_Delay_1->text() + ";";

    set_y_max = ui->form_MO_1->text().toInt();

    send(sp);
    send(kPd);
    send(kPu);
    send(kId);
    send(kIu);
    send(acc);
    send(cp);
    send(se);
    send(max);
    send(delay);

    status=1;

    ui->commandLine->clear();

    resetButtonsColors();
    ui->updateButton_1->setStyleSheet("* { background-color: rgb(0,195,000) }");

    preset = 1;

}

void MainWindow::on_updateButton_2_clicked()
{
    QString sp, kPd, kPu, kId, kIu, acc, cp, se, i, max, fb, delay, msg;

    sp = "sp" + ui->form_Setpoint_2->text() + ";"; //Setpoint
    kPd = "kPd" + ui->form_KPdown_2->text() + ";";
    kPu = "kPu" + ui->form_KPup_2->text() + ";";
    kId = "kId" + ui->form_KIdown_2->text() + ";";
    kIu = "kIu" + ui->form_KIup_2->text() + ";";
    acc = "acc" + ui->form_Acc_2->text() + ";";
    cp = "cp" + ui->form_CP_2->text() + ";";
    se = "se" + ui->form_SP_2->text() + ";"; //Błąd! //Sensor Period
    max = "max" + ui->form_MO_2->text() + ";";
    delay = "delay" + ui->form_Delay_2->text() + ";";

    set_y_max = ui->form_MO_2->text().toInt();

    send(sp);
    send(kPd);
    send(kPu);
    send(kId);
    send(kIu);
    send(acc);
    send(cp);
    send(se);
    send(max);
    send(delay);

    status=1;

    ui->commandLine->clear();

    resetButtonsColors();
    ui->updateButton_2->setStyleSheet("* { background-color: rgb(0,195,000) }");

    preset = 2;
}

void MainWindow::on_updateButton_3_clicked()
{
    QString sp, kPd, kPu, kId, kIu, acc, cp, se, i, max, fb, delay, msg;

    sp = "sp" + ui->form_Setpoint_3->text() + ";"; //Setpoint
    kPd = "kPd" + ui->form_KPdown_3->text() + ";";
    kPu = "kPu" + ui->form_KPup_3->text() + ";";
    kId = "kId" + ui->form_KIdown_3->text() + ";";
    kIu = "kIu" + ui->form_KIup_3->text() + ";";
    acc = "acc" + ui->form_Acc_3->text() + ";";
    cp = "cp" + ui->form_CP_3->text() + ";";
    se = "se" + ui->form_SP_3->text() + ";"; //Błąd! //Sensor Period
    max = "max" + ui->form_MO_3->text() + ";";
    delay = "delay" + ui->form_Delay_3->text() + ";";

    set_y_max = ui->form_MO_3->text().toInt();

    send(sp);
    send(kPd);
    send(kPu);
    send(kId);
    send(kIu);
    send(acc);
    send(cp);
    send(se);
    send(max);
    send(delay);

    status=1;

    ui->commandLine->clear();

    resetButtonsColors();
    ui->updateButton_3->setStyleSheet("* { background-color: rgb(0,195,000) }");

    preset = 3;
}

void MainWindow::on_updateButton_4_clicked()
{
    QString sp, kPd, kPu, kId, kIu, acc, cp, se, i, max, fb, delay, msg;

    sp = "sp" + ui->form_Setpoint_4->text() + ";"; //Setpoint
    kPd = "kPd" + ui->form_KPdown_4->text() + ";";
    kPu = "kPu" + ui->form_KPup_4->text() + ";";
    kId = "kId" + ui->form_KIdown_4->text() + ";";
    kIu = "kIu" + ui->form_KIup_4->text() + ";";
    acc = "acc" + ui->form_Acc_4->text() + ";";
    cp = "cp" + ui->form_CP_4->text() + ";";
    se = "se" + ui->form_SP_4->text() + ";"; //Błąd! //Sensor Period
    max = "max" + ui->form_MO_4->text() + ";";
    delay = "delay" + ui->form_Delay_4->text() + ";";

    set_y_max = ui->form_MO_4->text().toInt();

    send(sp);
    send(kPd);
    send(kPu);
    send(kId);
    send(kIu);
    send(acc);
    send(cp);
    send(se);
    send(max);
    send(delay);

    status=1;

    ui->commandLine->clear();

    resetButtonsColors();
    ui->updateButton_4->setStyleSheet("* { background-color: rgb(0,195,000) }");

    preset = 4;
}

void MainWindow::on_updateButton_5_clicked()
{
    QString sp, kPd, kPu, kId, kIu, acc, cp, se, i, max, fb, delay, msg;

    sp = "sp" + ui->form_Setpoint_5->text() + ";"; //Setpoint
    kPd = "kPd" + ui->form_KPdown_5->text() + ";";
    kPu = "kPu" + ui->form_KPup_5->text() + ";";
    kId = "kId" + ui->form_KIdown_5->text() + ";";
    kIu = "kIu" + ui->form_KIup_5->text() + ";";
    acc = "acc" + ui->form_Acc_5->text() + ";";
    cp = "cp" + ui->form_CP_5->text() + ";";
    se = "se" + ui->form_SP_5->text() + ";"; //Błąd! //Sensor Period
    max = "max" + ui->form_MO_5->text() + ";";
    delay = "delay" + ui->form_Delay_5->text() + ";";

    set_y_max = ui->form_MO_5->text().toInt();

    send(sp);
    send(kPd);
    send(kPu);
    send(kId);
    send(kIu);
    send(acc);
    send(cp);
    send(se);
    send(max);
    send(delay);

    status=1;

    ui->commandLine->clear();

    resetButtonsColors();
    ui->updateButton_5->setStyleSheet("* { background-color: rgb(0,195,000) }");

    preset = 5;

}

void MainWindow::resetButtonsColors() {

    ui->updateButton_1->setStyleSheet("");
    ui->updateButton_2->setStyleSheet("");
    ui->updateButton_3->setStyleSheet("");
    ui->updateButton_4->setStyleSheet("");
    ui->updateButton_5->setStyleSheet("");

}


/* Dodanie tekstu do konsoli */
void MainWindow::addTextToConsole(QString msg,bool sender)
{
    if(msg.isEmpty()) return;

    // wysyłanie wiadomości do urządzenia
    if(sender) {
        send(msg);
        return;
        };


    // dodawanie tekstu do konsoli
    QString line = (sender ? ">>: " : "<<: ");
    if(oldplotindex == plotindex)
    {
        line += msg + "\n";
    }
    else
    {
     line += QString::number(plotindex) + ". " + msg + "\n";
    }
    ui->output->setPlainText(ui->output->toPlainText() + line);

    // auto scroll
    QScrollBar *scroll = ui->output->verticalScrollBar();
    scroll->setValue(scroll->maximum());

    oldplotindex = plotindex;

}

/* Wysłanie danych */
void MainWindow::send(QString msg)
{
    read = 0;

    if(port.isOpen()) {
        port.write(msg.toStdString().c_str());
        port.waitForBytesWritten(-1);

    }

    read = 1;

}


/* Odczyt danych*/
void MainWindow::readData()
{
    QByteArray receivedData;
    QString str;
    QString data[5];
    QStringList wordList;
    double sp = 0;
    double fb =0;
    double err = 0;
    double i = 0;
    double out = 0;


    while(port.canReadLine() && read==1)
    {
        receivedData = port.readLine();
        if(receivedData.length()>0)
        {
            str = receivedData;

            if(str.indexOf("Rec") == 1 )
            {
                addTextToConsole( str );

            }
            else if(str.indexOf("SP") >= 0)
            {
                wordList = str.split(';');

                //SP100.00;Fb10.00;Err90.00;I27.90;Out117.90;
                sp = wordList[0].remove(0, 2).toDouble();
                fb = wordList[1].remove(0, 2).toDouble();
                err = wordList[2].remove(0, 3).toDouble();
                i = wordList[3].remove(0, 1).toDouble();
                out = wordList[4].remove(0, 3).toDouble();


                plot_X.append( (double)plotindex );

                plot_SP.append(sp); // Setpoint
                plot_FB.append(fb); // Feedback
                plot_ERR.append(err); // Error
                plot_I.append(i); // Integral
                plot_OUT.append(out); // Output
                plotindex++;

                updatePlot();

                addTextToConsole( str );
            }
            else
            {
                addTextToConsole( str );
            }
        }

    }
}

/* Inicjalizacja widgeta rysujacego wykres */
void MainWindow::initPlot()
{
    // Style rysowania:

    QPen setpointPen;
    setpointPen.setColor(QColor(Qt::darkGreen));
    setpointPen.setWidthF(2);

    QPen errorPen;
    errorPen.setColor(QColor(Qt::red));
    errorPen.setWidthF(2);

    QPen feedbackPen;
    feedbackPen.setColor(QColor(Qt::magenta));
    feedbackPen.setStyle(Qt::DotLine);
    feedbackPen.setWidthF(2);

    QPen integralPen;
    integralPen.setColor(QColor(Qt::blue));
    integralPen.setWidthF(2);

    QPen outputPen;
    outputPen.setColor(QColor(Qt::darkGray));
    outputPen.setWidthF(2);


    ui->customPlot->addGraph();
    ui->customPlot->graph(0)->setPen(setpointPen);
    ui->customPlot->addGraph();
    ui->customPlot->graph(1)->setPen(feedbackPen);
    ui->customPlot->addGraph();
    ui->customPlot->graph(2)->setPen(errorPen);
    ui->customPlot->addGraph();
    ui->customPlot->graph(3)->setPen(integralPen);
    ui->customPlot->addGraph();
    ui->customPlot->graph(4)->setPen(outputPen);


    ui->customPlot->xAxis2->setVisible(true);
    ui->customPlot->xAxis2->setTickLabels(false);
    ui->customPlot->yAxis2->setVisible(true);
    ui->customPlot->yAxis2->setTickLabels(false);

    // make left and bottom axes always transfer their ranges to right and top axes:
    connect(ui->customPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->customPlot->xAxis2, SLOT(setRange(QCPRange)));
    connect(ui->customPlot->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->customPlot->yAxis2, SLOT(setRange(QCPRange)));

    // Allow user to drag axis ranges with mouse, zoom with mouse wheel and select graphs by clicking:
    ui->customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);

    set_y_max = 300;
}

/* Odświeżenie wykresu */
void MainWindow::updatePlot()
{


    // limit dla wykresu
    if( plot_X.size() > 2000 ){
        plot_X.remove( 0 );
        plot_SP.remove( 0 );
        plot_FB.remove( 0 );
        plot_ERR.remove( 0 );
        plot_I.remove( 0 );
        plot_OUT.remove( 0 );
    }


    ui->customPlot->graph( 0 )->setData( plot_X , plot_SP );
    ui->customPlot->graph( 1 )->setData( plot_X , plot_FB );
    ui->customPlot->graph( 2 )->setData( plot_X , plot_ERR );
    ui->customPlot->graph( 3 )->setData( plot_X , plot_I );
    ui->customPlot->graph( 4 )->setData( plot_X , plot_OUT );


    QVector<double>::iterator xMax = std::max_element( plot_X.begin() , plot_X.end() );
    QVector<double>::iterator xMin = std::min_element( plot_X.begin() , plot_X.end() );

    QVector<double>::iterator yMax = std::max_element( plot_SP.begin() , plot_SP.end()) ;

    if(plot_y_max<*yMax)
        plot_y_max = *yMax;

    yMax = std::max_element( plot_FB.begin() , plot_FB.end()) ;

    if(plot_y_max<*yMax)
        plot_y_max = *yMax;

    yMax = std::max_element( plot_ERR.begin() , plot_ERR.end()) ;

    if(plot_y_max<*yMax)
        plot_y_max = *yMax;

    yMax = std::max_element( plot_I.begin() , plot_I.end()) ;

    if(plot_y_max<*yMax)
        plot_y_max = *yMax;

    yMax = std::max_element( plot_OUT.begin() , plot_OUT.end()) ;

    if(plot_y_max<*yMax)
        plot_y_max = *yMax;

    double yPlotMin = 0;
    double yPlotMax = plot_y_max;

    double xPlotMin = *xMin;
    double xPlotMax = *xMax;

    if(xPlotMax>300)
    {
        ui->customPlot->xAxis->setRange( xPlotMax-300, xPlotMax );
    }
    else
    {
        ui->customPlot->xAxis->setRange( xPlotMax-300 , xPlotMax);
    }

     ui->customPlot->yAxis->setRange( -10 , set_y_max*1.1);

     ui->customPlot->replot();

}
