#ifndef PROCESS_HETEROGENES_H
#define PROCESS_HETEROGENES_H

#include <algorithm>
#include <fstream>
#include "types.h"
#include <sstream>
#include <iostream>
#include <iterator>

using namespace std;

/**
 * Считывает конфигурацию системы из файла и помещает их в список
 * @param filename - имя файла
 */
vector<Processor*> ReadSystemFromFile(string filename);

/**
 * Считывает задачи из файла и помещает их в список
 * @param filename - имя файла
 */
list<TaskHeterogenes*> ReadTasksFromFile(string filename);

/**
 * Преобразует задачи в работы
 * @param tasks - список задач
 */
list<JobHeterogenes*> TasksToJobs(list<TaskHeterogenes*> tasks);

/**
 * Считает интервал планирования для всех задач
 * НОК для всех периодов
 * @param tasks - список задач
 */
long long TimeToShedule(list<TaskHeterogenes*> tasks);

/**
 * Считывает работы из файла и помещает их в список
 * Порядок считывания: старт, финиш, время выполнения, раздел
 * @param filename - имя файла
 */
list<JobHeterogenes*> ReadJobsFromFile(string filename);

/**
 * Создает сеть для списка работ
 * @param jobs - список программ
 * @param processors - список процессоров
 * @param cTime - сложность переключения раздела
 */
Web CreateWebFromJobsAndSystem(list<JobHeterogenes*> jobs, vector<Processor*> processors, int cTime);

/**
 * Создает окна по сети, в которой уже построен максимальный поток
 * @param web - сеть с построенным потоком
 */
list< list<Window*> > CreateWindows(Web* web);


/**
 * Запись окон в файл
 * @param windows - список окон
 * @param filename - имя файла для записи результата
 * @param time - время на переключение
 * @param works - список программ
 */
void WriteWindowsToFile(list< list<Window*> > windows, string filename, string time, string works, string eff);


#endif // PROCESS_HETEROGENES_H
