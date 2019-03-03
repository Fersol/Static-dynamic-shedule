#ifndef PROCESS_H
#define PROCESS_H

#include <fstream>
#include "types.h"
#include <sstream>
#include <iostream>

using namespace std;
/**
 * Считывает задачи из файла и помещает их в список
 * @param filename - имя файла
 */
list<Task*> ReadTasksFromFile(string filename);

/**
 * Преобразует задачи в работы
 */
list<Job*> TasksToJobs(list<Task*> tasks);

/**
 * Считает интервал планирования для всех задач
 * НОК для всех периодов
 */
long long TimeToShedule(list<Task*> tasks);

/**
 * Считывает работы из файла и помещает их в список
 * Порядок считывания: старт, финиш, время выполнения, раздел
 * @param filename - имя файла
 */
list<Job*> ReadJobsFromFile(string filename);

/**
 * Создает сеть для списка задач
 */
//Web* CreateWebFromTasks(list<Task*> tasks);

/**
 * Создает сеть для списка работ
 */
Web CreateWebFromJobs(list<Job*> jobs, int c, int nproc);

/**
 * Создает окна по сети, в которой уже построен максимальный поток
 */
list< list<Window*> > CreateWindows(Web* web);


/**
 * Запись окон в файл
 */
bool WriteWindowsToFile(list< list<Window*> > windows, string filename, string time, string works, string eff);

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


vector<string> split(string strToSplit, string delimeter);
#endif // MAXFLOW_H