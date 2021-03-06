#include "process_heterogenes.h"


vector<Processor*> ReadSystemFromFile(string filename) {
    vector<Processor*> processors;
    Processor* processor;
    string str;
    ifstream file(filename);

    if (!file.is_open()){
        std::cout << "Couldn't open system file!\n";
        return vector<Processor*>();
    } 

    bool isok = true;   // Идентификатор успешности считвания данных
    getline(file, str);

    // Далее идет парсинг параметров из csv файла
    while(getline(file, str)){
        processor = new Processor;
        vector<string> strs = split(str, "\t");
        vector<string>::iterator it = strs.begin();
        it++;
        processor->performance = stoi(*it);
        it++;
        if (it == strs.end() || !isok){
            isok = false;
            break;
        }
        vector<string> funcVec = split(*it,";");
        set<string> funcSet(funcVec.begin(), funcVec.end());
        processor->functionality = funcSet;
        it++;
        processor->cost = stoi(*it);
        processors.push_back(processor);
    }
    file.close();
    if (!isok) processors.clear();
    return processors;
}

void WriteWindowsToFile(list< list<Window*> > windows, string filename, string time, string works, string eff) {
    ofstream fileres(filename+".res");

    fileres<<"Time, Works, Performance\n";
    fileres<<time;
    fileres<<", ";

    fileres<<works;
    fileres<<", ";
 
    fileres<<eff;
    fileres<<"\n";
    fileres.close();

    ofstream file(filename+".sol");
    file << "Processor, Processor Type, Partition, Window, Works\n";

    int proc = 0;
    for(list<list<Window*> >::iterator itproc = windows.begin(); itproc != windows.end(); itproc ++){
        for(list<Window*>::iterator it = (*itproc).begin(); it != (*itproc).end(); it++)
        {   
            file << proc;
            file << ", ";

            file<<(*it)->ptype;
            file<<", ";

            file<<(*it)->partition;
            file<<", ";

            file<<(*it)->start;
            file<<"-";
            file<<(*it)->finish;
            file<<", ";
            for(map<int, float>::const_iterator _it = (*it)->works.begin(); _it != (*it)->works.end(); _it++){
              file << _it->first << "-" << _it->second << ";";
            }
            file<<"\n";
        }
        proc++;
    }
    file.close();
}

list<TaskHeterogenes*> ReadTasksFromFile(string filename) {
    list<TaskHeterogenes*> tasks;
    TaskHeterogenes* task;
    string str;
    ifstream file(filename);

    if (!file.is_open()){ 
        cout << "Couldn't open tasks file\n";
        return list<TaskHeterogenes*>();
        
    } 

    bool isok = true;
    getline(file, str);
    while(getline(file, str)){
        task = new TaskHeterogenes;
        vector<string> strs = split(str, "\t");
        vector<string>::iterator it = strs.begin();
        it++;
        cout << *it << "\n";
        task->partition = stoi(*it);
        it++;
        if (it == strs.end() || !isok){
            isok = false;
            break;
        }
        task->complexity = stoi(*it);
        it++;
        if (it == strs.end() || !isok){
            isok = false;
            break;
        }
        task->period = stoi(*it);
        it++;
        if (it == strs.end() || !isok){
            isok = false;
            break;
        }
        task->left = stoi(*it);
        it++;
        if (it == strs.end() || !isok){
            isok = false;
            break;
        }
        task->right = stoi(*it);
        it++;
        if (it == strs.end() || !isok){
            isok = false;
            break;
        }
        vector<string> funcVec = split(*it,";");
        set<string> funcSet(funcVec.begin(), funcVec.end());
        task->functionality = funcSet;
        tasks.push_back(task);
    }
    file.close();
    if (!isok) tasks.clear();
    return tasks;
}

list<JobHeterogenes*> TasksToJobs(list<TaskHeterogenes*> tasks)
{
    list<JobHeterogenes*> jobs;
    JobHeterogenes* job;
    int k = 0;
    long long timelap = TimeToShedule(tasks);
    for (list<TaskHeterogenes*>::iterator it = tasks.begin(); it != tasks.end(); it++)
    {
        long long period = (*it)->period;
        long long left = (*it)->left;
        long long right = (*it)->right;
        for (int i = 0; i < timelap/period; i++ )
        {
            job = new JobHeterogenes;
            job->start = period * i + left;
            job->finish = period * i + right;
            job->complexity = (*it)->complexity;
            job->partition = (*it)->partition;
            job->functionality = (*it)->functionality;
            job->numTask = k;
            jobs.push_back(job);
        }
        k++;
    }
    return jobs;
}

Web CreateWebFromJobsAndSystem(list<JobHeterogenes*> jobs, vector<Processor*> processors, int cTime) {

   Web web;
   cout << "Start creating web\n";
   web.processors = processors;
   web.mainLoop = 0;
   web.nproc = processors.size();
   web.source_flow = 0;  
   set<int> parts;
   int maxpart = 0;
   map<int, int> valforinterval; // для разбиение на интервалы
   Vertex temp;

   web.num_of_works = jobs.size();
   for (list<JobHeterogenes*>::iterator it = jobs.begin(); it != jobs.end(); it++)
   {        
            // Ищем интервал планирования
            if (web.mainLoop < (*it)->finish){
                web.mainLoop = (*it)->finish;
            }
            // Добавляем вершину
            Vertex temp;
            temp.capacity = (*it)->complexity;
            temp.duration = (*it)->complexity;
            temp.part = (*it)->partition;     
            web.layers[temp.part].complexity += temp.duration;
            temp.stTime = (*it)->start;
            temp.finTime = (*it)->finish;
            temp.cTime = cTime;
            temp.h = 1;
            temp.type = JOB;
            temp.flow = 0;
            temp.numTask = (*it)->numTask;
            temp.options = (*it)->functionality;
        
            web.layers[temp.part].functionality.insert(temp.options.begin(), temp.options.end());

            // Добавляем знания о необходимомй функциональности всего раздела
            if (web.partitionFunctionality.count(temp.part)){
               set<string> result;
               std::set_union(web.partitionFunctionality[temp.part].begin(),web.partitionFunctionality[temp.part].end(),
               temp.options.begin(), temp.options.end(),
               std::inserter(result, result.begin()));
               web.partitionFunctionality[temp.part] = result;
           } else {
               web.partitionFunctionality[temp.part] = temp.options;
           }

           web.layers[temp.part].vertexes.push_back(temp);
           int size = web.layers[temp.part].vertexes.size();
           parts.insert(temp.part);
           if (temp.part > maxpart) maxpart = temp.part;
           valforinterval[temp.stTime]= 0;
           valforinterval[temp.finTime]= 0;
   }
   
   // Интервалы - это 0-ой слой, для сохранения структуры
   web.layer_int = maxpart + 1;
   for (map<int,int>::iterator it = valforinterval.begin(); next(it) != valforinterval.end(); it++){
        Vertex temp;
        temp.stTime = it->first;
        temp.finTime = next(it)->first;
        temp.type = INTERVAL;
        temp.h = 0;
        temp.duration = (temp.finTime - temp.stTime);
        temp.capacity = temp.duration;
        temp.flow = 0;
        temp.cTime = cTime;
        web.layers[0].vertexes.push_back(temp);
   }

   //заполнить cap для дуг от работ до интервалов и для них же сделать flow = 0 для default layer
   for (int l_i=1; l_i < web.layer_int ; l_i++){
        for (int i=0; i < web.layers[l_i].vertexes.size(); i++){
            for (int j=0; j < web.layers[0].vertexes.size(); j++){
                if (web.layers[l_i].vertexes[i].stTime <= web.layers[0].vertexes[j].stTime && web.layers[l_i].vertexes[i].finTime >= web.layers[0].vertexes[j].finTime){
                    cout << "ADD DEFAULT " << l_i << " " << i << " " << 0 << " " << j << endl;
                    web.layers[l_i].vertexes[i].neighbors[0][j].cap = web.layers[0].vertexes[j].duration;
                    web.layers[0].vertexes[j].neighbors[l_i][i].cap = 0;
                }
            }
        }
    }

    // установление начального слоя для процессоров
    web.free_layer = web.layer_int;
   
    // установить число разделов 
    web.q = maxpart;

    // добавить структуру для раздела-процессора неопределенную
    for(int i = 1; i <= web.q; i++){
        web.QP[i] = -1;
    }

    // время на переключение
    web.cw = cTime;

    return web;
}

list< list<Window*> > CreateWindows(Web* web) {
    list< list<Window*> > all_windows;
    cout << "Creation of WINDOWS" << endl; 
    int nproc = web->nproc;

    // Цикл по номерам слоев
    for(int l_p = web->q+1; l_p < web->free_layer; l_p++){
        list<Window*> windows;
        Window* win = new Window;
        float curtime = 0;
        for (int it = 0; it < web->layers[l_p].vertexes.size(); it++){

            // подготтовительные мероприятия для облегчения вычислений
            int chWdw = web->layers[l_p].vertexes[it].chWdw;
            if (web->layers[l_p].vertexes[it].isLWin) chWdw--;
            if (web->layers[l_p].vertexes[it].isRWin) chWdw--;
            curtime = web->layers[l_p].vertexes[it].stTime;

            // анализ самого начала
            if (it == 0){
                win->start = 0;
                win->partition = web->layers[l_p].vertexes[it].firstPart;
                win->ptype = web->layers[l_p].ptype;
            }

            // для пустых интервалов
            if (win->partition == 0 && web->layers[l_p].vertexes[it].firstPart != 0) win->partition = web->layers[l_p].vertexes[it].firstPart;

            // анализ левой части
            if (web->layers[l_p].vertexes[it].isLWin) {
                win->finish = curtime;
                windows.push_back(win);
                win = new Window;
                //открываем новое окно
                curtime+=float(web->cw);//web->processors[web->layers[l_p].ptype]->performance;
                win->start = curtime;
                win->partition = web->layers[l_p].vertexes[it].firstPart;
                win->ptype = web->layers[l_p].ptype;
            }

            // анализ средней части
            if (chWdw != 0){
                // закрыть то, что началось
                curtime+=float(web->layers[l_p].vertexes[it].partIn[web->layers[l_p].vertexes[it].firstPart])/web->processors[web->layers[l_p].ptype]->performance;
                win->finish = curtime;

                // добавить работы
                for(Neighbors::iterator it_layer = web->layers[l_p].vertexes[it].neighbors.begin(); it_layer != web->layers[l_p].vertexes[it].neighbors.end(); it_layer++){
                    map<int, NeighborInfo > layer_neighbors = it_layer->second;
                    // Номер слоя соседа
                    int l_v = it_layer->first;
                    for (map<int, NeighborInfo >::iterator _it = layer_neighbors.begin(); _it != layer_neighbors.end(); _it++){
                        // Номер вершины соседа
                        int v = _it->first;
                        NeighborInfo vertex = _it->second;   
                        if (web->layers[l_v].vertexes[v].part != win->partition) continue;
                        if(web->layers[l_v].vertexes[v].neighbors[l_p][it].flow > 0){
                            if (win->works.count(web->layers[l_v].vertexes[v].numTask)){
                            win->works[web->layers[l_v].vertexes[v].numTask] += float(web->layers[l_v].vertexes[v].neighbors[l_p][it].flow) /web->processors[web->layers[l_p].ptype]->performance;
                            }
                            else{
                            win->works[web->layers[l_v].vertexes[v].numTask] = float(web->layers[l_v].vertexes[v].neighbors[l_p][it].flow) / web->processors[web->layers[l_p].ptype]->performance;
                            }
                        }
                    }
                }
                
                windows.push_back(win);

                // открыть - закрыть вместе
                for(set<int>::iterator its = web->layers[l_p].vertexes[it].setpart.begin(); its != web->layers[l_p].vertexes[it].setpart.end(); its++){
                    if (*its == web->layers[l_p].vertexes[it].lastPart || *its == web->layers[l_p].vertexes[it].firstPart) continue;
                    win = new Window;
                    curtime+=float(web->cw);//web->processors[web->layers[l_p].ptype]->performance;
                    win->start = curtime;
                    win->partition = *its;
                    win->ptype = web->layers[l_p].ptype;
                    curtime+=float(web->layers[l_p].vertexes[it].partIn[*its])/web->processors[web->layers[l_p].ptype]->performance;
                    win->finish = curtime;

                    // добавить работы
                    for(Neighbors::iterator it_layer = web->layers[l_p].vertexes[it].neighbors.begin(); it_layer != web->layers[l_p].vertexes[it].neighbors.end(); it_layer++){
                        map<int, NeighborInfo > layer_neighbors = it_layer->second;
                        // Номер слоя соседа
                        int l_v = it_layer->first;
                        for (map<int, NeighborInfo >::iterator _it = layer_neighbors.begin(); _it != layer_neighbors.end(); _it++){
                            // Номер вершины соседа
                            int v = _it->first;
                            NeighborInfo vertex = _it->second;
                    
                            if (web->layers[l_v].vertexes[v].part != win->partition) continue;
                            if(web->layers[l_v].vertexes[v].neighbors[l_p][it].flow > 0){
                                if (win->works.count(web->layers[l_v].vertexes[v].numTask)){
                                win->works[web->layers[l_v].vertexes[v].numTask] += float(web->layers[l_v].vertexes[v].neighbors[l_p][it].flow) /web->processors[web->layers[l_p].ptype]->performance;
                                }
                                else{
                                win->works[web->layers[l_v].vertexes[v].numTask] = float(web->layers[l_v].vertexes[v].neighbors[l_p][it].flow) / web->processors[web->layers[l_p].ptype]->performance;
                                }
                            }
                        }
                    }
                    windows.push_back(win);
                }
                win = new Window;
                curtime+=float(web->cw);//web->processors[web->layers[l_p].ptype]->performance;
                win->start = curtime;
                win->partition = web->layers[l_p].vertexes[it].lastPart;
                win->ptype = web->layers[l_p].ptype;
            }


            curtime+=float(web->layers[l_p].vertexes[it].partIn[web->layers[l_p].vertexes[it].lastPart])/web->processors[web->layers[l_p].ptype]->performance;

            // добавить работы
            for(Neighbors::iterator it_layer = web->layers[l_p].vertexes[it].neighbors.begin(); it_layer != web->layers[l_p].vertexes[it].neighbors.end(); it_layer++){
                map<int, NeighborInfo > layer_neighbors = it_layer->second;
                // Номер слоя соседа
                int l_v = it_layer->first;
                for (map<int, NeighborInfo >::iterator _it = layer_neighbors.begin(); _it != layer_neighbors.end(); _it++){
                    // Номер вершины соседа
                    int v = _it->first;
                    NeighborInfo vertex = _it->second;
            
                    if (web->layers[l_v].vertexes[v].part != win->partition) continue;
                    if(web->layers[l_v].vertexes[v].neighbors[l_p][it].flow > 0){
                        if (win->works.count(web->layers[l_v].vertexes[v].numTask)){
                        win->works[web->layers[l_v].vertexes[v].numTask] += float(web->layers[l_v].vertexes[v].neighbors[l_p][it].flow) /web->processors[web->layers[l_p].ptype]->performance;
                        }
                        else{
                        win->works[web->layers[l_v].vertexes[v].numTask] = float(web->layers[l_v].vertexes[v].neighbors[l_p][it].flow) / web->processors[web->layers[l_p].ptype]->performance;
                        }
                    }
                }
            }
            //открыть последенее

            //анализ правой части
            if (web->layers[l_p].vertexes[it].isRWin) {
                curtime = web->layers[l_p].vertexes[it].finTime - float(web->cw)/web->processors[web->layers[l_p].ptype]->performance;
                win->finish = curtime;
                windows.push_back(win);
                win = new Window;
                //открываем новое окно
                curtime+=float(web->cw);//web->processors[web->layers[l_p].ptype]->performance;
                win->start = curtime;
                win->partition = web->layers[l_p].vertexes[it+1].firstPart;
                win->ptype = web->layers[l_p].ptype;
            }


            //анализ самого конца
            if (it == web->layers[l_p].vertexes.size()-1){
                win->finish = web->layers[l_p].vertexes[it].finTime;
                windows.push_back(win);
            }

        }
        cout << windows.size() << endl;
        all_windows.push_back(windows);
     }
     cout << all_windows.size() << endl;
     return all_windows;
}


long long TimeToShedule(list<TaskHeterogenes*> tasks) {
    long long timelap = 1;
    for (list<TaskHeterogenes*>::iterator it = tasks.begin(); it != tasks.end(); it++)
    {
        timelap = NOK(timelap, (*it)->period);
    }
    return timelap;
}