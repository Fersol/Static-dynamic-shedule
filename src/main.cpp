#include "process_heterogenes.h"
#include <ctime>

int main(int argc, char** argv) {
    cout << "Start scheduling\n";
    int c = stoi(argv[1]);                  // 1 аргумент - это сложность переключения окна
    string  filenametasks = argv[2];        // 2 аргумент - файл с программами для расписания
    string  filenameprocessors = argv[3];   // 3 аргумент - файл с описанием системы
    string  typeoftask = argv[4];           // 4 аргумент - тип задачи (синтез или расписание)

    string filenamewindows = split(filenametasks, ".")[0];

    list<TaskHeterogenes*> tasks = ReadTasksFromFile(filenametasks);
    cout << "Tasks have been read from file\n";

    vector<Processor*> processors = ReadSystemFromFile(filenameprocessors);
    cout << "System has been read from file\n";

    if (tasks.size() == 0) {
        cout << "Error";
        return 1;
    }

    list<JobHeterogenes*> jobs = TasksToJobs(tasks);
    cout << "Tasks have been transformed to jobs\n";
    unsigned int startIime = clock();

    Web web = CreateWebFromJobsAndSystem(jobs, processors, c);
    cout << "The network has been created\n";
    if (web.find_alloc(typeoftask)) {
        // Запуск алгоритма построения потока в сети
        web.maxflow();
        cout << "The maxflow of the network has been found\n";
        list< list<Window*> > windows = CreateWindows(&web);
        cout << "The windows have been found\n";
        unsigned int endIime = clock();
        WriteWindowsToFile(windows, filenamewindows, to_string(float(endIime-startIime)/1000), to_string(web.sheduledjobs()), to_string(web.Effectivness()));
        cout << "Solution has been writen to the file\n"; 
        cout << "Exit\n";
        return 0;
    } else {
        cout << "NO TYPE OF TASK like " << typeoftask;
        return 1;
    }
}