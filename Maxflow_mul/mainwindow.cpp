#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QMessageBox"
#include "QPair"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    bool ok;
    QString filenamejobs = ui->lineEdit->text();
    QString filenamewindows = *(filenamejobs.split(".", QString::SkipEmptyParts).begin()) + ".res.txt";
    int c = (ui->textC->text()).toInt(&ok, 10);
    int nproc = (ui->lineEdit_2->text()).toInt(&ok, 10);
    QList<Job*> jobs = ReadJobsFromFile(filenamejobs);
    if (jobs.size() == 0)
    {
        QMessageBox::warning(this, "Ошибка", "Некорректный файл");
        return;
    }
    QTime time;
    time.restart();
    Web web = CreateWebFromJobs(jobs, c, nproc);
    ui->label_6->setText(QString("%1").arg((double)time.elapsed()/1000));
    web.maxflow();
    QList< QList<Window*> > windows = CreateWindows(&web);
    ui->label_7->setText(QString("%1").arg((double)time.elapsed()/1000));
    time.restart();
    ui->label_8->setText(QString("%1 из %2").arg(web.sheduledjobs()).arg( web.numofwork));
    ui->label_10->setText(QString("%1").arg(web.Effectivness()));

     WriteWindowsToFile(windows, filenamewindows, ui->label_7->text(), ui->label_8->text(), ui->label_10->text() );
     //{
     //    QMessageBox::warning(this, "Ошибка", "Некорректный файл окон");
     //}
     ok = true;
}

void MainWindow::on_pushButton_2_clicked()
{
    bool ok;
    int c = (ui->textC->text()).toInt(&ok, 10);
    int nproc = (ui->lineEdit_2->text()).toInt(&ok, 10);
    QStringList files = (ui->lineEdit_3->text()).split(",", QString::SkipEmptyParts);
    for(QStringList::Iterator it = files.begin(); it != files.end(); ++it)
    {
        QString filenametasks = *it;
        QString filenamewindows = *(filenametasks.split(".", QString::SkipEmptyParts).begin()) + ".res.txt";

        QList<Task*> tasks = ReadTasksFromFile(filenametasks);
        if (tasks.size() == 0)
        {
            QMessageBox::warning(this, "Ошибка", "Некорректный файл");
            return;
        }
        QList<Job*> jobs = TasksToJobs(tasks);
        QTime time;
        time.restart();
        Web web = CreateWebFromJobs(jobs, c, nproc);
        ui->label_6->setText(QString("%1").arg((double)time.elapsed()/1000));
        web.maxflow();
        QList< QList<Window*> > windows = CreateWindows(&web);
        ui->label_7->setText(QString("%1").arg((double)time.elapsed()/1000));
        time.restart();
        ui->label_8->setText(QString("%1 из %2").arg(web.sheduledjobs()).arg(web.numofwork));
        ui->label_10->setText(QString("%1").arg(web.Effectivness()));

        WriteWindowsToFile(windows, filenamewindows, ui->label_7->text(), ui->label_8->text(), ui->label_10->text());
        ok = true;
    }
}

void MainWindow::on_pushButton_3_clicked()
{
    bool ok;
    QString directory = ui->lineEdit_4->text();
    QFile file(directory+"/TEST.txt");
    if (!file.exists()) {
        QMessageBox::warning(this, "Ошибка", "Нет файла TEST.txt в ");
        return;
    }
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;
    QTextStream stream(&file);
    int nproc = (ui->lineEdit_2->text()).toInt(&ok, 10);
    while(!stream.atEnd()){
        QString str = stream.readLine();
        int c = (*(str.split(" ").begin()+1)).toInt(&ok, 10);

        QString filenametasks = *(str.split(" ").begin());
        QString filenamewindows = *(filenametasks.split(".", QString::SkipEmptyParts).begin()) + ".res.txt";

        QList<Task*> tasks = ReadTasksFromFile(filenametasks);
        if (tasks.size() == 0)
        {
            QMessageBox::warning(this, "Ошибка", "Некорректный файл");
            return;
        }
        QList<Job*> jobs = TasksToJobs(tasks);
        QTime time;
        time.restart();
        Web web = CreateWebFromJobs(jobs, c, nproc);
        ui->label_6->setText(QString("%1").arg((double)time.elapsed()/1000));
        web.maxflow();
        QList< QList<Window*> > windows = CreateWindows(&web);
        ui->label_7->setText(QString("%1").arg((double)time.elapsed()/1000));
        time.restart();
        ui->label_8->setText(QString("%1 из %2").arg(web.sheduledjobs()).arg(web.numofwork));
        ui->label_10->setText(QString("%1").arg(web.Effectivness()));

        WriteWindowsToFile(windows, filenamewindows, ui->label_7->text(), ui->label_8->text(), ui->label_10->text());
        ok = true;
    }
}



void MainWindow::on_pushButton_4_clicked()
{
    bool ok;
    QFile fileTest(ui->lineEdit_4->text());
    if (!fileTest.exists()) {
        QMessageBox::warning(this, "Ошибка", "Нет такого файла ");
        return;
    }
    if(!fileTest.open(QIODevice::ReadOnly | QIODevice::Text)) return;
    QTextStream streamTest(&fileTest);
    while(!streamTest.atEnd()){
        QString str = streamTest.readLine();
        QString directory = str;
        QFile file(directory+"/TEST.txt");
        if (!file.exists()) {
            QMessageBox::warning(this, "Ошибка", "Нет файла TEST.txt в ");
            return;
        }
        if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;
        QTextStream stream(&file);
        int nproc = (ui->lineEdit_2->text()).toInt(&ok, 10);
        while(!stream.atEnd()){
            QString str = stream.readLine();
            int c = (*(str.split(" ").begin()+1)).toInt(&ok, 10);

            QString filenameworks = *(str.split(" ").begin());
            QString filenamewindows = *(filenameworks.split(".", QString::SkipEmptyParts).begin()) + ".res.txt";


            QList<Job*> jobs = ReadJobsFromFile(filenameworks);
            if (jobs.size() == 0)
            {
                QMessageBox::warning(this, "Ошибка", "Некорректный файл работ");
                return;
            }

            QTime time;
            time.restart();
            Web web = CreateWebFromJobs(jobs, c, nproc);
            ui->label_6->setText(QString("%1").arg((double)time.elapsed()/1000));
            web.maxflow();
            QList< QList<Window*> > windows = CreateWindows(&web);
            ui->label_7->setText(QString("%1").arg((double)time.elapsed()/1000));
            time.restart();
            ui->label_8->setText(QString("%1 из %2").arg(web.sheduledjobs()).arg(web.numofwork));
            ui->label_10->setText(QString("%1").arg(web.Effectivness()));

            WriteWindowsToFile(windows, filenamewindows, ui->label_7->text(), ui->label_8->text(), ui->label_10->text());

            ok = true;
        }
    }
}
