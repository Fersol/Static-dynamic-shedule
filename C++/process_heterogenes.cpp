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

   web.processors = processors;
   web.mainLoop = 0;
   web.nproc = processors.size();
   
   set<int> parts;

   int maxpart = 0;
   map<int, int> valforinterval; // для разбиение на интервалы

   //добавить исток и сток
   Vertex temp;
   temp.type = SOURCE;
   temp.proc = -2;
   web.verVec.push_back(temp);
   temp.type = DEST;
   temp.proc = -2;
   web.verVec.push_back(temp);

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
           
           web.verVec.push_back(temp);

           web.vPart[temp.part].insert(k);
           web.P[temp.part].insert(k);
           k++;

           parts.insert(temp.part);
           if (temp.part > maxpart) maxpart = temp.part;

           valforinterval[temp.stTime]= 0;
           valforinterval[temp.finTime]= 0;
   }

   int beginofintver = web.verVec.size();  //номер вершины, с которой начнутся вершины интервалы

   //сделать вершины интервалов по valforinterval, продублировать столько раз, сколько процессоров есть
   k = 0;
   for (map<int,int>::iterator it = valforinterval.begin(); next(it) != valforinterval.end(); it++){
       for (int i = 0; i < web.nproc; i++){
           Vertex temp;
           temp.stTime = it->first;
           temp.finTime = next(it)->first;
           temp.type = INTERVAL;
           temp.h = 0;
           temp.proc = i;

           temp.nextItr = beginofintver + k + web.nproc;
           temp.prevItr = beginofintver + k - web.nproc;
           temp.duration = (temp.finTime - temp.stTime) * processors[i]->performance;
           temp.cTime = cTime;
           temp.options = processors[i]->functionality;
           web.verVec.push_back(temp);
           k++;
       }
   }
   //указать правильные соседние интревалы для концевых интервалов
   for(int i = 0; i < web.nproc; i++){
       web.verVec[beginofintver+i].prevItr = 0;
       web.verVec[web.verVec.size() - (i+1)].nextItr = 0;
   }
   //заполнить cap для дуг от работ до интервалов и для них же сделать flow = 0
   for (int i = 2; i < beginofintver; i++){
       // здесь нормально, не нужно учитывать процессоры, сами учтутся по времени
       for(int j = beginofintver; j < web.verVec.size(); j++){
           bool isFunctionality = true;
           set<string> result;
           std::set_intersection(web.verVec[i].options.begin(),web.verVec[i].options.end(),
            web.verVec[j].options.begin(),web.verVec[j].options.end(),
            std::inserter(result, result.begin()));
           if (result != web.verVec[i].options){
               isFunctionality = false;
           }
           
           if (web.verVec[i].stTime <= web.verVec[j].stTime && web.verVec[i].finTime >= web.verVec[j].finTime && isFunctionality){
               web.verVec[i].cap[j] = web.verVec[j].duration;
               web.verVec[j].cap[i] = 0;

               web.verVec[i].flow[j] = 0;
               web.verVec[j].flow[i] = 0;
           }
       }
       //соединение с источником вершин-работ
       web.verVec[0].cap[i] = web.verVec[i].duration;
       web.verVec[i].cap[0] = 0;
       web.verVec[i].flow[0] = 0;
       web.verVec[0].flow[i] = 0;
   }

   //соединение со стоком вершин-интервалов
   for (int i = beginofintver; i < web.verVec.size(); i++){
       web.verVec[i].cap[1] = web.verVec[i].duration;
       web.verVec[1].cap[i] = 0;
       web.verVec[i].flow[1] = 0;
       web.verVec[1].flow[i] = 0;
   }

  //дополнительная информация
  web.src = 0;
  web.dest = 1;
  web.hints = web.nproc*web.nproc - 1;
  web.n = web.verVec.size();
  web.numOfWork = jobs.size();
  web.q = maxpart;
  // добавляем вместительность процессоров
  for (int i=0;i < web.nproc; i++){
       web.processorLoad[i] = web.mainLoop * web.processors[i]->performance;
   }
  //добавить структуру для разделов в вершины-интервалов
  for(int i = beginofintver; i < web.verVec.size(); i++){
      web.verVec[i].partIn.resize(web.q + 1);
      for(int j = 0; j < web.q + 1; j++){
          web.verVec[i].partIn[j] = 0;
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
    for(int iproc = 0; iproc < web->nproc; iproc++){
        list<Window*> windows;
        Window* win = new Window;
        float curtime = 0;
        for (int it = web->numOfWork + 2 + iproc; it < web->n; it+=nproc){
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
            int chWdw = web->verVec[it].chWdw;
            if (web->verVec[it].isLWin) chWdw--;
            if (web->verVec[it].isRWin) chWdw--;
            //web->verVec[it].setpart-=web->verVec[it].firstPart;
            //web->verVec[it].setpart-=web->verVec[it].lastPart;
            curtime = web->verVec[it].stTime;

            //анализ самого начала
            if (it == web->numOfWork + 2 + iproc){
                win->start = 0;
                win->partition = web->verVec[it].firstPart;
            }

            //для пустых интервалов
            if (win->partition == 0 && web->verVec[it].firstPart != 0) win->partition = web->verVec[it].firstPart;

            //анализ левой части
            if (web->verVec[it].isLWin) {
                win->finish = curtime;
                windows.push_back(win);

                win = new Window;
                //открываем новое окно
                curtime+=float(web->cw)/web->processors[iproc]->performance;
                win->start = curtime;
                win->partition = web->verVec[it].firstPart;
            }

            //анализ средней части
            if (chWdw != 0){
                //закрыть то, что началось
                curtime+=float(web->verVec[it].partIn[web->verVec[it].firstPart])/web->processors[iproc]->performance;
                win->finish = curtime;
                // добавить работы
                for(map<int, int>::const_iterator _it = web->verVec[it].flow.begin(); _it != web->verVec[it].flow.end(); _it++){
                  if (web->verVec[_it->first].part != win->partition || _it->first == 0 || _it->first == 1 ) continue;
                  if(web->verVec[_it->first].flow[it] > 0){
                    if (win->works.count(web->verVec[_it->first].numTask)){
                      win->works[web->verVec[_it->first].numTask] += float(web->verVec[_it->first].flow[it]) /web->processors[web->verVec[it].proc]->performance;
                    }
                    else{
                      win->works[web->verVec[_it->first].numTask] = float(web->verVec[_it->first].flow[it]) / web->processors[web->verVec[it].proc]->performance;
                    }
                  }
                }
                windows.push_back(win);


                //открыть - закрыть вместе
                for(set<int>::iterator its = web->verVec[it].setpart.begin(); its != web->verVec[it].setpart.end(); its++){
                    if (*its == web->verVec[it].lastPart || *its == web->verVec[it].firstPart) continue;
                    win = new Window;
                    curtime+=float(web->cw)/web->processors[iproc]->performance;
                    win->start = curtime;
                    win->partition = *its;
                    curtime+=float(web->verVec[it].partIn[*its])/web->processors[iproc]->performance;
                    win->finish = curtime;
                    // добавить работы
                    for(map<int, int>::const_iterator _it = web->verVec[it].flow.begin(); _it != web->verVec[it].flow.end(); _it++){
                      if (web->verVec[_it->first].part != win->partition || _it->first == 0 || _it->first == 1 ) continue;
                      if(web->verVec[_it->first].flow[it] > 0){
                        if (win->works.count(web->verVec[_it->first].numTask)){
                          win->works[web->verVec[_it->first].numTask] += float(web->verVec[_it->first].flow[it])/web->processors[web->verVec[it].proc]->performance;
                        }
                        else{
                          win->works[web->verVec[_it->first].numTask] = float(web->verVec[_it->first].flow[it])/web->processors[web->verVec[it].proc]->performance;
                        }
                      }
                    }
                    windows.push_back(win);


                }
                win = new Window;
                curtime+=float(web->cw)/web->processors[iproc]->performance;
                win->start = curtime;
                win->partition = web->verVec[it].lastPart;
            }


            curtime+=float(web->verVec[it].partIn[web->verVec[it].lastPart])/web->processors[iproc]->performance;
            // добавить работы
            for(map<int, int>::const_iterator _it = web->verVec[it].flow.begin(); _it != web->verVec[it].flow.end(); _it++){
              if (web->verVec[_it->first].part != win->partition || _it->first == 0 || _it->first == 1 ) continue;
              if(web->verVec[_it->first].flow[it] > 0){
                if (win->works.count(web->verVec[_it->first].numTask)){
                  win->works[web->verVec[_it->first].numTask] += float(web->verVec[_it->first].flow[it])/web->processors[web->verVec[it].proc]->performance;
                }
                else{
                  win->works[web->verVec[_it->first].numTask] = float(web->verVec[_it->first].flow[it])/web->processors[web->verVec[it].proc]->performance;
                }
              }
            }
            //открыть последенее

            //анализ правой части
            if (web->verVec[it].isRWin) {
                curtime = web->verVec[it].finTime - float(web->cw)/web->processors[iproc]->performance;
                win->finish = curtime;
                windows.push_back(win);

                win = new Window;
                //открываем новое окно
                curtime+=float(web->cw)/web->processors[iproc]->performance;
                win->start = curtime;
                win->partition = web->verVec[it+nproc].firstPart;
            }



            //анализ самого конца
            if (it == web->n - (nproc-iproc)){
                win->finish = web->verVec[it].finTime;
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