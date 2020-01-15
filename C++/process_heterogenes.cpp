#include "process_heterogenes.h"


vector<Processor*> ReadSystemFromFile(string filename)
{
    vector<Processor*> processors;
    Processor* processor;
    string str;
    ifstream file(filename);
    // if (!file.exists()) return tasks;
    if (!file.is_open()){ // если файл не открыт
        std::cout << "Couldn't open system file!\n";
        return vector<Processor*>();
    } // сообщить об этом
    // if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) return tasks;
    // QTextStream stream(&file);

    bool isok = true;
    getline(file, str);
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
        processors.push_back(processor);
    }
    file.close();
    if (!isok) processors.clear();
    return processors;
}

void WriteWindowsToFile(list< list<Window*> > windows, string filename, string time, string works, string eff)
{
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
    file << "Processor, Partition, Window, Works\n";

    int proc = 0;
    for(list<list<Window*> >::iterator itproc = windows.begin(); itproc != windows.end(); itproc ++){
        // file<< "Processor:" << proc << "\n";
        // file<<"Windows:\n";
        for(list<Window*>::iterator it = (*itproc).begin(); it != (*itproc).end(); it++)
        {   
            file << proc;
            file << ", ";

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

list<TaskHeterogenes*> ReadTasksFromFile(string filename)
{
    list<TaskHeterogenes*> tasks;
    TaskHeterogenes* task;
    string str;
    ifstream file(filename);
    // if (!file.exists()) return tasks;
    if (!file.is_open()){ // если файл не открыт
        // std::cout << "Файл не может быть открыт!\n";
        cout << "Couldn't open tasks file\n";
        return list<TaskHeterogenes*>();
        
    } // сообщить об этом
    // if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) return tasks;
    // QTextStream stream(&file);

    bool isok = true;
    getline(file, str);
    while(getline(file, str)){
        task = new TaskHeterogenes;
        vector<string> strs = split(str, "\t");
        vector<string>::iterator it = strs.begin();
        it++;
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
        vector<string> funcVec = split(*it,";");
        set<string> funcSet(funcVec.begin(), funcVec.end());
        task->functionality = funcSet;
        tasks.push_back(task);
    }
    file.close();
    // cout<<"IsOk:" <<  isok << endl;
    if (!isok) tasks.clear();
    return tasks;
}

list<JobHeterogenes*> ReadJobsFromFile(string filename)
{
    list<JobHeterogenes*> jobs;
    JobHeterogenes* job;
    string str;
    bool isok = true;
    ifstream file(filename);
    if (!file.is_open()){ // если файл не открыт
        // std::cout << "Файл не может быть открыт!\n";
        return jobs;
    }


    getline(file, str);
    while(getline(file, str)){
        job = new JobHeterogenes;
        vector<string> strs = split(str, "\t");
        vector<string>::iterator it = strs.begin();    
        job->numTask = stoi(*it);
        it++;
        job->partition = stoi(*it);
        jobs.push_back(job);

        it++;
        job->complexity = stoi(*it);

        it++;
        if (it == strs.end() || !isok){
            isok = false;
            break;
        }
        job->start = stod(*it);
        it++;
        if (it == strs.end() || !isok){
            isok = false;
            break;
        }
        job->finish = stod(*it);
        if (it == strs.end() || !isok){
            isok = false;
            break;
        }
        it++;
        if (it == strs.end() || !isok){
            isok = false;
            break;
        }
        vector<string> funcVec = split(*it,";");
        set<string> funcSet(funcVec.begin(), funcVec.end());
        job->functionality = funcSet;

    }
    file.close();
    if (!isok) jobs.clear();
    return jobs;
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
        for (int i = 0; i < timelap/period; i++ )
        {
            job = new JobHeterogenes;
            job->start = period * i;
            job->finish = period * (i + 1);
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

Web CreateWebFromJobsAndSystem(list<JobHeterogenes*> jobs, vector<Processor*> processors, int cTime)
{
   //добавить вершины работы и источник по задачам
   Web web;
   
   cout << "Start creating web\n";
   web.processors = processors;
   web.mainLoop = 0;
   web.nproc = processors.size();
   
   set<int> parts;

   int maxpart = 0;
   map<int, int> valforinterval; // для разбиение на интервалы

   Vertex temp;

   int k = 2; //номера работ
   for (list<JobHeterogenes*>::iterator it = jobs.begin(); it != jobs.end(); it++)
   {        
           // Ищем интервал планирования
           if (web.mainLoop < (*it)->finish){
               web.mainLoop = (*it)->finish;
           }
           // Добавляем вершину
           Vertex temp;
           temp.duration = (*it)->complexity;
           temp.part = (*it)->partition;
           if (web.partitionComplexity.count(temp.part)){
               web.partitionComplexity[temp.part] += temp.duration;
           } else {
               web.partitionComplexity[temp.part] = temp.duration;
           }

           temp.stTime = (*it)->start;
           temp.finTime = (*it)->finish;
           temp.h = 1;
           temp.type = JOB;
           temp.numTask = (*it)->numTask;
           temp.options = (*it)->functionality;
          
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

           web.vPart[temp.part].insert(k);
           web.P[temp.part].insert(k);
           k++;

           parts.insert(temp.part);
           if (temp.part > maxpart) maxpart = temp.part;

           valforinterval[temp.stTime]= 0;
           valforinterval[temp.finTime]= 0;
   }
   
   cout << "End of works layer\n";
   // Интервалы - это слой

   //сделать вершины интервалов по valforinterval, продублировать столько раз, сколько процессоров есть
   k = maxpart + 1; // Первый 
   web.layer_int = maxpart + 1;
   for (map<int,int>::iterator it = valforinterval.begin(); next(it) != valforinterval.end(); it++){
       for (int i = web.layer_int; i < web.layer_int + web.nproc; i++){
           Vertex temp;
           temp.stTime = it->first;
           temp.finTime = next(it)->first;
           temp.type = INTERVAL;
           temp.h = 0;
           temp.proc = i;

           temp.duration = (temp.finTime - temp.stTime) * processors[i - web.layer_int]->performance;
           temp.cTime = cTime;
           temp.options = processors[i - web.layer_int]->functionality;
           web.layers[i].vertexes.push_back(temp);
       }
   }


   //заполнить cap для дуг от работ до интервалов и для них же сделать flow = 0
   for (int l_i=1; l_i < web.layer_int ; l_i++){
        for (int i=0; i < web.layers[l_i].vertexes.size(); i++){
            for (int l_j=web.layer_int; l_j < web.layer_int + web.nproc; l_j++){
                 for (int j=0; j < web.layers[l_j].vertexes.size(); j++){
                    bool isFunctionality = true;
                    set<string> result;
                    std::set_intersection(web.layers[l_i].vertexes[i].options.begin(),web.layers[l_i].vertexes[i].options.end(),
                        web.layers[l_j].vertexes[j].options.begin(),web.layers[l_j].vertexes[j].options.end(),
                        std::inserter(result, result.begin()));
                    if (result != web.layers[l_i].vertexes[i].options){
                        isFunctionality = false;
                    }
                    if (web.layers[l_i].vertexes[i].stTime <= web.layers[l_j].vertexes[j].stTime && web.layers[l_i].vertexes[i].finTime >= web.layers[l_j].vertexes[j].finTime && isFunctionality){
                        web.layers[l_i].vertexes[i].neighbors[l_j][j].cap = web.layers[l_j].vertexes[j].duration;
                        web.layers[l_j].vertexes[j].neighbors[l_i][i].cap = 0;

                        web.layers[l_i].vertexes[i].neighbors[l_j][j].cap = 0;
                        web.layers[l_j].vertexes[j].neighbors[l_i][i].flow = 0;
                    }
                }
            }
        }
    }



    //дополнительная информация
    web.hints = web.nproc*web.nproc - 1;
    int n = 0;
    for(map<int, Layer>::iterator it = web.layers.begin(); it != web.layers.end(); it++){
        n = n + it->second.vertexes.size();
    }
    web.n = n;
    web.numOfWork = jobs.size();
    web.q = maxpart;

    // добавляем вместительность процессоров
    for (int i=0;i < web.nproc; i++){
        web.processorLoad[i] = web.mainLoop * web.processors[i]->performance;
    }

    for (int i=web.layer_int; i < web.layer_int + web.nproc; i++){
        for (int j=0; j < web.layers[i].vertexes.size(); j++){
            web.layers[i].vertexes[j].partIn.resize(web.q + 1);
            for(int k = 0; k < web.q + 1; k++){
                web.layers[i].vertexes[j].partIn[k] = 0;
            }
        }
    }
 
    // добавить структуру для раздела-процессора
    for(int i = 1; i <= web.q; i++){
        web.QP[i] = -1;
    }
    //время на переключение
    web.cw = cTime;
    // Создать порядок разделов по сортировке
    vector<pair<int,int> >vec;
    for(auto x=web.partitionComplexity.begin(); x!=web.partitionComplexity.end(); x++) vec.push_back(*x);
        sort(vec.begin(), vec.end(), [](pair<int,int> elem1, pair<int,int> elem2) {return elem1.second > elem2.second;});
    for(auto x:vec)web.partitionOrder.push_back(x.first);

    return web;
}

list< list<Window*> > CreateWindows(Web* web)
{   list< list<Window*> > all_windows;
    int nproc = web->nproc;
    // Цикл по номерам слоев
    for(int iproc = web->layer_int; iproc < web->layer_int + web->nproc; iproc++){
        list<Window*> windows;
        Window* win = new Window;
        float curtime = 0;
        for (int it = 0; it < web->layers[iproc].vertexes.size(); it+=1){
            //добавить работы в окно
            //for(map<int, int>::const_iterator _it = web->verVec[it].flow.begin(); _it != web->verVec[it].flow.end(); _it++){
            //  if (_it->first == 0 || _it->first == 1 ) continue;
            //  if(web->verVec[_it->first].flow[it] > 0){
            //    if (win->works.contains(web->verVec[_it->first].numTask)){
            //      win->works[web->verVec[_it->first].numTask] += web->verVec[_it->first].flow[it];
            //    }
            //    else{
            //      win->works[web->verVec[_it->first].numTask] = web->verVec[_it->first].flow[it];
            //    }
            //  }
            //}

            //подготтовительные мероприятия для облегчения вычислений
            int chWdw = web->layers[iproc].vertexes[it].chWdw;
            if (web->layers[iproc].vertexes[it].isLWin) chWdw--;
            if (web->layers[iproc].vertexes[it].isRWin) chWdw--;
            curtime = web->layers[iproc].vertexes[it].stTime;

            //анализ самого начала
            if (it == web->numOfWork + 2 + iproc){
                win->start = 0;
                win->partition = web->layers[iproc].vertexes[it].firstPart;
            }

            //для пустых интервалов
            if (win->partition == 0 && web->layers[iproc].vertexes[it].firstPart != 0) win->partition = web->layers[iproc].vertexes[it].firstPart;

            //анализ левой части
            if (web->layers[iproc].vertexes[it].isLWin) {
                win->finish = curtime;
                windows.push_back(win);

                win = new Window;
                //открываем новое окно
                curtime+=float(web->cw)/web->processors[iproc]->performance;
                win->start = curtime;
                win->partition = web->layers[iproc].vertexes[it].firstPart;
            }

            //анализ средней части
            if (chWdw != 0){
                //закрыть то, что началось
                curtime+=float(web->layers[iproc].vertexes[it].partIn[web->layers[iproc].vertexes[it].firstPart])/web->processors[iproc]->performance;
                win->finish = curtime;

                // добавить работы
                for(Neighbors::iterator it_layer = web->layers[iproc].vertexes[it].neighbors.begin(); it_layer != web->layers[iproc].vertexes[it].neighbors.end(); it_layer++){
                    map<int, NeighborInfo > layer_neighbors = it_layer->second;
                    // Номер слоя соседа
                    int l_v = it_layer->first;
                    for (map<int, NeighborInfo >::iterator _it = layer_neighbors.begin(); _it != layer_neighbors.end(); _it++){
                        // Номер вершины соседа
                        int v = _it->first;
                        NeighborInfo vertex = _it->second;
                
                        if (web->layers[l_v].vertexes[v].part != win->partition) continue;
                        if(web->layers[l_v].vertexes[v].neighbors[iproc][it].flow > 0){
                            if (win->works.count(web->layers[l_v].vertexes[v].numTask)){
                            win->works[web->layers[l_v].vertexes[v].numTask] += float(web->layers[l_v].vertexes[v].neighbors[iproc][it].flow) /web->processors[web->layers[iproc].vertexes[it].proc]->performance;
                            }
                            else{
                            win->works[web->layers[l_v].vertexes[v].numTask] = float(web->layers[l_v].vertexes[v].neighbors[iproc][it].flow) / web->processors[web->layers[iproc].vertexes[it].proc]->performance;
                            }
                        }
                    }
                }

                windows.push_back(win);


                //открыть - закрыть вместе
                for(set<int>::iterator its = web->layers[iproc].vertexes[it].setpart.begin(); its != web->layers[iproc].vertexes[it].setpart.end(); its++){
                    if (*its == web->layers[iproc].vertexes[it].lastPart || *its == web->layers[iproc].vertexes[it].firstPart) continue;
                    win = new Window;
                    curtime+=float(web->cw)/web->processors[iproc]->performance;
                    win->start = curtime;
                    win->partition = *its;
                    curtime+=float(web->layers[iproc].vertexes[it].partIn[*its])/web->processors[iproc]->performance;
                    win->finish = curtime;

                    // добавить работы
                    for(Neighbors::iterator it_layer = web->layers[iproc].vertexes[it].neighbors.begin(); it_layer != web->layers[iproc].vertexes[it].neighbors.end(); it_layer++){
                        map<int, NeighborInfo > layer_neighbors = it_layer->second;
                        // Номер слоя соседа
                        int l_v = it_layer->first;
                        for (map<int, NeighborInfo >::iterator _it = layer_neighbors.begin(); _it != layer_neighbors.end(); _it++){
                            // Номер вершины соседа
                            int v = _it->first;
                            NeighborInfo vertex = _it->second;
                    
                            if (web->layers[l_v].vertexes[v].part != win->partition) continue;
                            if(web->layers[l_v].vertexes[v].neighbors[iproc][it].flow > 0){
                                if (win->works.count(web->layers[l_v].vertexes[v].numTask)){
                                win->works[web->layers[l_v].vertexes[v].numTask] += float(web->layers[l_v].vertexes[v].neighbors[iproc][it].flow) /web->processors[web->layers[iproc].vertexes[it].proc]->performance;
                                }
                                else{
                                win->works[web->layers[l_v].vertexes[v].numTask] = float(web->layers[l_v].vertexes[v].neighbors[iproc][it].flow) / web->processors[web->layers[iproc].vertexes[it].proc]->performance;
                                }
                            }
                        }
                    }
                    windows.push_back(win);
                }
                win = new Window;
                curtime+=float(web->cw)/web->processors[iproc]->performance;
                win->start = curtime;
                win->partition = web->layers[iproc].vertexes[it].lastPart;
            }


            curtime+=float(web->layers[iproc].vertexes[it].partIn[web->layers[iproc].vertexes[it].lastPart])/web->processors[iproc]->performance;
            // добавить работы
            for(Neighbors::iterator it_layer = web->layers[iproc].vertexes[it].neighbors.begin(); it_layer != web->layers[iproc].vertexes[it].neighbors.end(); it_layer++){
                map<int, NeighborInfo > layer_neighbors = it_layer->second;
                // Номер слоя соседа
                int l_v = it_layer->first;
                for (map<int, NeighborInfo >::iterator _it = layer_neighbors.begin(); _it != layer_neighbors.end(); _it++){
                    // Номер вершины соседа
                    int v = _it->first;
                    NeighborInfo vertex = _it->second;
            
                    if (web->layers[l_v].vertexes[v].part != win->partition) continue;
                    if(web->layers[l_v].vertexes[v].neighbors[iproc][it].flow > 0){
                        if (win->works.count(web->layers[l_v].vertexes[v].numTask)){
                        win->works[web->layers[l_v].vertexes[v].numTask] += float(web->layers[l_v].vertexes[v].neighbors[iproc][it].flow) /web->processors[web->layers[iproc].vertexes[it].proc]->performance;
                        }
                        else{
                        win->works[web->layers[l_v].vertexes[v].numTask] = float(web->layers[l_v].vertexes[v].neighbors[iproc][it].flow) / web->processors[web->layers[iproc].vertexes[it].proc]->performance;
                        }
                    }
                }
            }
            //открыть последенее

            //анализ правой части
            if (web->layers[iproc].vertexes[it].isRWin) {
                curtime = web->layers[iproc].vertexes[it].finTime - float(web->cw)/web->processors[iproc]->performance;
                win->finish = curtime;
                windows.push_back(win);

                win = new Window;
                //открываем новое окно
                curtime+=float(web->cw)/web->processors[iproc]->performance;
                win->start = curtime;
                win->partition = web->layers[iproc].vertexes[it+1].firstPart;
            }



            //анализ самого конца
            if (it == web->n - (nproc-iproc)){
                win->finish = web->layers[iproc].vertexes[it].finTime;
                windows.push_back(win);
            }

        }
        all_windows.push_back(windows);
     }
     return all_windows;
}


long long TimeToShedule(list<TaskHeterogenes*> tasks)
{
    long long timelap = 1;
    for (list<TaskHeterogenes*>::iterator it = tasks.begin(); it != tasks.end(); it++)
    {
        timelap = NOK(timelap, (*it)->period);
    }
    return timelap;
}