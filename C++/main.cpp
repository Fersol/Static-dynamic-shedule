#include "process.h"
#include <ctime>

int main(int argc, char** argv){
    cout << "Start\n";
    bool ok;
    int c = stoi(argv[1]);
    int nproc = stoi(argv[2]);
    string  filenametasks = argv[3];

    string filenamewindows = split(filenametasks, ".")[0] + ".res.txt";

    list<Task*> tasks = ReadTasksFromFile(filenametasks);
    cout << "Tasks have been read from file\n";
    if (tasks.size() == 0)
    {
        cout << "Error";
        return 1;
    }
    list<Job*> jobs = TasksToJobs(tasks);
    cout << "Tasks have been transformed to jobs\n";
    unsigned int startIime = clock();

    Web web = CreateWebFromJobs(jobs, c, nproc);
    cout << "The network has been created\n";
    web.maxflow();
    cout << "The maxflow of the network has been found\n";
    list< list<Window*> > windows = CreateWindows(&web);
    cout << "The windows have been found\n";
    unsigned int endIime = clock();

    WriteWindowsToFile(windows, filenamewindows, to_string(float(endIime-startIime)/1000) + "c", to_string(web.sheduledjobs()), to_string(web.Effectivness()));
    cout << "Solution has been writen to the file\n";

    cout << "Exit\n";
    return 0;
}