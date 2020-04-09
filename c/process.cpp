#include "process.h"

bool WriteWindowsToFile(list< list<Window*> > windows, string filename, string time, string works, string eff)
{
    ofstream file(filename);
    //if (!file.exists())
    //{
        //QMessageBox::warning("Ошибка", "Файл для окон не существует");
    //    return false;
    //}
    // if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
    // {
    //     //QMessageBox::warning(this, "Ошибка", "Некорректный файл для окон");
    //     return false;
    // }
    file<<"Time: ";
    file<<time;
    file<<"\n";

    file<<"Works: ";
    file<<works;
    file<<"\n";

    file<<"Effectivness: ";
    file<<eff;
    file<<"\n";

    int proc = 0;
    for(list<list<Window*> >::iterator itproc = windows.begin(); itproc != windows.end(); itproc ++){
        file<< "Processor:" << proc << "\n";
        file<<"Windows:\n";
        for(list<Window*>::iterator it = (*itproc).begin(); it != (*itproc).end(); it++)
        {
            file<<(*it)->partition;
            file<<" [";
            file<<(*it)->start;
            file<<", ";
            file<<(*it)->finish;
            file<<"]  ---   ";
            for(map<int, int>::const_iterator _it = (*it)->works.begin(); _it != (*it)->works.end(); _it++){
              file << _it->first << ":" << _it->second << " ; ";
            }
            file<<"\n";
        }
        proc++;
         file<<"\n";
    }
    file.close();
}

list<Task*> ReadTasksFromFile(string filename)
{
    list<Task*> tasks;
    Task* task;
    string str;
    ifstream file(filename);
    // if (!file.exists()) return tasks;
    if (!file.is_open()){ // если файл не открыт
        // std::cout << "Файл не может быть открыт!\n";
        return list<Task*>();
    } // сообщить об этом
    // if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) return tasks;
    // QTextStream stream(&file);

    bool isok = true;
    getline(file, str);
    while(getline(file, str)){
        task = new Task;
        vector<string> strs = split(str, "\t");
        vector<string>::iterator it = strs.begin();
        it++;
        task->partition = stoi(*it);
        it++;
        if (it == strs.end() || !isok){
            isok = false;
            break;
        }
        task->duration = stoi(*it);
        it++;
        if (it == strs.end() || !isok){
            isok = false;
            break;
        }
        task->period = stoi(*it);
        if (it == strs.end() || !isok){
            isok = false;
            break;
        }
        tasks.push_back(task);
    }
    file.close();
    if (!isok) tasks.clear();
    return tasks;
}

list<Job*> ReadJobsFromFile(string filename)
{
    list<Job*> jobs;
    Job* job;
    string str;
    bool isok = true;
    ifstream file(filename);
    if (!file.is_open()){ // если файл не открыт
        // std::cout << "Файл не может быть открыт!\n";
        return jobs;
    }


    getline(file, str);
    while(getline(file, str)){
        job = new Job;
        vector<string> strs = split(str, "\t");
        vector<string>::iterator it = strs.begin();    
        job->numTask = stoi(*it);
        it++;
        job->partition = stoi(*it);
        jobs.push_back(job);

        it++;
        job->duration = stoi(*it);

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

    }
    file.close();
    if (!isok) jobs.clear();
    return jobs;
}

list<Job*> TasksToJobs(list<Task*> tasks)
{
    list<Job*> jobs;
    Job* job;
    int k = 0;
    long long timelap = TimeToShedule(tasks);
    for (list<Task*>::iterator it = tasks.begin(); it != tasks.end(); it++)
    {
        long long period = (*it)->period;
        for (int i = 0; i < timelap/period; i++ )
        {
            job = new Job;
            job->start = period * i;
            job->finish = period * (i + 1);
            job->duration = (*it)->duration;
            job->partition = (*it)->partition;
            job->numTask = k;
            jobs.push_back(job);
        }
        k++;
     }
     return jobs;
}

Web CreateWebFromJobs(list<Job*> jobs, int c_time, int nproc)
{
   //добавить вершины работы и источник по задачам
   Web web;
   set<int> parts;
   int maxpart = 0;
   map<int, int> valforinterval; // для разбиение на интервалы

   //добавить исток и сток
   Vertex temp;
   temp.type = SOURCE;
   web.verVec.push_back(temp);
   temp.type = DEST;
   web.verVec.push_back(temp);

   int k = 2; //номера работ
   for (list<Job*>::iterator it = jobs.begin(); it != jobs.end(); it++)
   {
           Vertex temp;
           temp.duration = (*it)->duration;
           temp.part = (*it)->partition;
           temp.stTime = (*it)->start;
           temp.finTime = (*it)->finish;
           temp.h = 1;
           temp.type = JOB;
           temp.numTask = (*it)->numTask;
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
       for (int i = 0; i < nproc; i++){
           Vertex temp;
           temp.stTime = it->first;
           temp.finTime = next(it)->first;
           temp.type = INTERVAL;
           temp.h = 0;
           temp.proc = i;

           temp.nextItr = beginofintver + k + nproc;
           temp.prevItr = beginofintver + k - nproc;
           temp.duration = temp.finTime - temp.stTime;
           web.verVec.push_back(temp);
           k++;
       }
   }
   //указать правильные соседние интревалы для концевых интервалов
   for(int i = 0; i < nproc; i++){
       web.verVec[beginofintver+i].prevItr = 0;
       web.verVec[web.verVec.size() - (i+1)].nextItr = 0;
   }
   //заполнить cap для дуг от работ до интервалов и для них же сделать flow = 0
   for (int i = 2; i < beginofintver; i++){
       // здесь нормально, не нужно учитывать процессоры, сами учтутся по времени
       for(int j = beginofintver; j < web.verVec.size(); j++){
           if (web.verVec[i].stTime <= web.verVec[j].stTime && web.verVec[i].finTime >= web.verVec[j].finTime){
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
  web.hints = nproc*nproc - 1;
  web.n = web.verVec.size();
  web.numOfWork = jobs.size();
  web.q = maxpart;
  web.nproc = nproc;
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
  web.cw = c_time;
  return web;
}

list< list<Window*> > CreateWindows(Web* web)
{   list< list<Window*> > all_windows;
    int nproc = web->nproc;
    for(int iproc = 0; iproc < web->nproc; iproc++){
        list<Window*> windows;
        Window* win = new Window;
        int curtime = 0;
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
                curtime+=web->cw;
                win->start = curtime;
                win->partition = web->verVec[it].firstPart;
            }

            //анализ средней части
            if (chWdw != 0){
                //закрыть то, что началось
                curtime+=web->verVec[it].partIn[web->verVec[it].firstPart];
                win->finish = curtime;
                // добавить работы
                for(map<int, int>::const_iterator _it = web->verVec[it].flow.begin(); _it != web->verVec[it].flow.end(); _it++){
                  if (web->verVec[_it->first].part != win->partition || _it->first == 0 || _it->first == 1 ) continue;
                  if(web->verVec[_it->first].flow[it] > 0){
                    if (win->works.count(web->verVec[_it->first].numTask)){
                      win->works[web->verVec[_it->first].numTask] += web->verVec[_it->first].flow[it];
                    }
                    else{
                      win->works[web->verVec[_it->first].numTask] = web->verVec[_it->first].flow[it];
                    }
                  }
                }
                windows.push_back(win);


                //открыть - закрыть вместе
                for(set<int>::iterator its = web->verVec[it].setpart.begin(); its != web->verVec[it].setpart.end(); its++){
                    if (*its == web->verVec[it].lastPart || *its == web->verVec[it].firstPart) continue;
                    win = new Window;
                    curtime+=web->cw;
                    win->start = curtime;
                    win->partition = *its;
                    curtime+=web->verVec[it].partIn[*its];
                    win->finish = curtime;
                    // добавить работы
                    for(map<int, int>::const_iterator _it = web->verVec[it].flow.begin(); _it != web->verVec[it].flow.end(); _it++){
                      if (web->verVec[_it->first].part != win->partition || _it->first == 0 || _it->first == 1 ) continue;
                      if(web->verVec[_it->first].flow[it] > 0){
                        if (win->works.count(web->verVec[_it->first].numTask)){
                          win->works[web->verVec[_it->first].numTask] += web->verVec[_it->first].flow[it];
                        }
                        else{
                          win->works[web->verVec[_it->first].numTask] = web->verVec[_it->first].flow[it];
                        }
                      }
                    }
                    windows.push_back(win);


                }
                win = new Window;
                curtime+=web->cw;
                win->start = curtime;
                win->partition = web->verVec[it].lastPart;
            }


            curtime+=web->verVec[it].partIn[web->verVec[it].lastPart];
            // добавить работы
            for(map<int, int>::const_iterator _it = web->verVec[it].flow.begin(); _it != web->verVec[it].flow.end(); _it++){
              if (web->verVec[_it->first].part != win->partition || _it->first == 0 || _it->first == 1 ) continue;
              if(web->verVec[_it->first].flow[it] > 0){
                if (win->works.count(web->verVec[_it->first].numTask)){
                  win->works[web->verVec[_it->first].numTask] += web->verVec[_it->first].flow[it];
                }
                else{
                  win->works[web->verVec[_it->first].numTask] = web->verVec[_it->first].flow[it];
                }
              }
            }
            //открыть последенее

            //анализ правой части
            if (web->verVec[it].isRWin) {
                curtime = web->verVec[it].finTime - web->cw;
                win->finish = curtime;
                windows.push_back(win);

                win = new Window;
                //открываем новое окно
                curtime+=web->cw;
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

long long TimeToShedule(list<Task*> tasks)
{
    long long timelap = 1;
    for (list<Task*>::iterator it = tasks.begin(); it != tasks.end(); it++)
    {
        timelap = NOK(timelap, (*it)->period);
    }
    return timelap;
}


