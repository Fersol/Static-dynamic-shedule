#ifndef MAXFLOW_H
#define MAXFLOW_H

#include <QFile>
#include <QTextStream>
#include "types.h"

/**
 * Считывает задачи из файла и помещает их в список
 * @param filename - имя файла
 */
QList<Task*> ReadTasksFromFile(QString filename);

/**
 * Преобразует задачи в работы
 */
QList<Job*> TasksToJobs(QList<Task*> tasks);

/**
 * Считает интервал планирования для всех задач
 * НОК для всех периодов
 */
long long TimeToShedule(QList<Task*> tasks);

/**
 * Считывает работы из файла и помещает их в список
 * Порядок считывания: старт, финиш, время выполнения, раздел
 * @param filename - имя файла
 */
QList<Job*> ReadJobsFromFile(QString filename);

/**
 * Создает сеть для списка задач
 */
//Web* CreateWebFromTasks(QList<Task*> tasks);

/**
 * Создает сеть для списка работ
 */
Web CreateWebFromJobs(QList<Job*> jobs, int c, int nproc);

/**
 * Создает окна по сети, в которой уже построен максимальный поток
 */
QList< QList<Window*> > CreateWindows(Web* web);


/**
 * Запись окон в файл
 */
bool WriteWindowsToFile(QList< QList<Window*> > windows, QString filename, QString time, QString works, QString eff);

/**
 * @brief NOD
 * @param n1
 * @param n2
 * @return
 */
long long NOD(long long n1, long long n2);

/**
 * @brief NOK
 * @param n1
 * @param n2
 * @return
 */
long long NOK(long long n1, long long n2);
#endif // MAXFLOW_H
