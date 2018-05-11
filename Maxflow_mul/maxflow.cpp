#include "maxflow.h"
#include "types.h"
#include "QMessageBox"

bool WriteWindowsToFile(QList< QList<Window*> > windows, QString filename, QString time, QString works, QString eff)
{
    QFile file(filename);
    //if (!file.exists())
    //{
        //QMessageBox::warning("Ошибка", "Файл для окон не существует");
    //    return false;
    //}
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        //QMessageBox::warning(this, "Ошибка", "Некорректный файл для окон");
        return false;
    }
    QTextStream stream(&file);
        stream<<"Time: ";
        stream<<time;
        stream<<"\n";

        stream<<"Works: ";
        stream<<works;
        stream<<"\n";

        stream<<"Effectivness: ";
        stream<<eff;
        stream<<"\n";

    int proc = 0;
    for(QList<QList<Window*> >::iterator itproc = windows.begin(); itproc != windows.end(); itproc ++){
        stream<< "Processor:" << proc << "\n";
        stream<<"Windows:\n";
        for(QList<Window*>::iterator it = (*itproc).begin(); it != (*itproc).end(); it++)
        {
            stream<<(*it)->partition;
            stream<<" [";
            stream<<(*it)->start;
            stream<<", ";
            stream<<(*it)->finish;
            stream<<"]  ---   ";
            for(QMap<int, int>::const_iterator _it = (*it)->works.begin(); _it != (*it)->works.end(); _it++){
              stream << _it.key() << ":" << _it.value() << " ; ";
            }
            stream<<"\n";
        }
        proc++;
         stream<<"\n";
    }
    file.close();
}

QList<Task*> ReadTasksFromFile(QString filename)
{
    QList<Task*> tasks;
    Task* task;
    QFile file(filename);
    if (!file.exists()) return tasks;

    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) return tasks;
    QTextStream stream(&file);

    bool isok = true;

    stream.readLine();
    while(!stream.atEnd()){
        QString str = stream.readLine();
        task = new Task;
        QStringList strs = str.split("\t", QString::SkipEmptyParts);
        QStringList::iterator it = strs.begin();
        it++;
        task->partition = int((*it).toDouble(&isok));
        it++;
        if (it == strs.end() || !isok){
            isok = false;
            break;
        }
        task->duration = int((*it).toDouble(&isok));
        it++;
        if (it == strs.end() || !isok){
            isok = false;
            break;
        }
        task->period = int((*it).toDouble(&isok));
        if (it == strs.end() || !isok){
            isok = false;
            break;
        }
        tasks.append(task);
    }
    file.close();
    if (!isok) tasks.clear();
    return tasks;
}

QList<Job*> ReadJobsFromFile(QString filename)
{
    QList<Job*> jobs;
    Job* job;
    bool isok = true;
    QFile file(filename);
    if (!file.exists()) return jobs;

    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) return jobs;
    QTextStream stream(&file);

    stream.readLine();
    while(!stream.atEnd()){
        QString str = stream.readLine();
        job = new Job;
        QStringList strs = str.split("\t", QString::SkipEmptyParts);
        QStringList::iterator it = strs.begin();    
        job->numtask = (*it).toDouble(&isok);
        it++;
        job->partition = (*it).toDouble(&isok);
        jobs.append(job);

        it++;
        job->duration = (*it).toDouble(&isok);

        it++;
        if (it == strs.end() || !isok){
            isok = false;
            break;
        }
        job->start = (*it).toDouble(&isok);
        it++;
        if (it == strs.end() || !isok){
            isok = false;
            break;
        }
        job->finish = (*it).toDouble(&isok);
        if (it == strs.end() || !isok){
            isok = false;
            break;
        }

    }
    file.close();
    if (!isok) jobs.clear();
    return jobs;
}

QList<Job*> TasksToJobs(QList<Task*> tasks)
{
    QList<Job*> jobs;
    Job* job;
    int k = 0;
    long long timelap = TimeToShedule(tasks);
    for (QList<Task*>::iterator it = tasks.begin(); it != tasks.end(); it++)
    {
        long long period = (*it)->period;
        for (int i = 0; i < timelap/period; i++ )
        {
            job = new Job;
            job->start = period * i;
            job->finish = period * (i + 1);
            job->duration = (*it)->duration;
            job->partition = (*it)->partition;
            job->numtask = k;
            jobs.append(job);
        }
        k++;
     }
     return jobs;
}

Web CreateWebFromJobs(QList<Job*> jobs, int c_time, int nproc)
{
   //добавить вершины работы и источник по задачам
   Web web;
   QSet<int> parts;
   int maxpart = 0;
   QMap<int, int> valforinterval; // для разбиение на интервалы

   //добавить исток и сток
   Vertex temp;
   temp.type = SOURCE;
   web.V.append(temp);
   temp.type = DEST;
   web.V.append(temp);

   int k = 2; //номера работ
   for (QList<Job*>::iterator it = jobs.begin(); it != jobs.end(); it++)
   {
           Vertex temp;
           temp.duration = (*it)->duration;
           temp.part = (*it)->partition;
           temp.sttime = (*it)->start;
           temp.fintime = (*it)->finish;
           temp.h = 1;
           temp.type = JOB;
           temp.numtask = (*it)->numtask;
           web.V.append(temp);

           web.VPart[temp.part]+=k;
           web.P[temp.part]+=k;
           k++;

           parts.insert(temp.part);
           if (temp.part > maxpart) maxpart = temp.part;

           valforinterval[temp.sttime]= 0;
           valforinterval[temp.fintime]= 0;
   }

   int beginofintver = web.V.size();  //номер вершины, с которой начнутся вершины интервалы

   //сделать вершины интервалов по valforinterval, продублировать столько раз, сколько процессоров есть
   k = 0;
   for (QMap<int,int>::iterator it = valforinterval.begin(); it+1 != valforinterval.end(); it++){
       for (int i = 0; i < nproc; i++){
           Vertex temp;
           temp.sttime = it.key();
           temp.fintime = (it + 1).key();
           temp.type = INTERVAL;
           temp.h = 0;
           temp.proc = i;

           temp.nextitr = beginofintver + k + nproc;
           temp.previtr = beginofintver + k - nproc;
           temp.duration = temp.fintime - temp.sttime;
           web.V.append(temp);
           k++;
       }
   }
   //указать правильные соседние интревалы для концевых интервалов
   for(int i = 0; i < nproc; i++){
       web.V[beginofintver+i].previtr = 0;
       web.V[web.V.size() - (i+1)].nextitr = 0;
   }
   //заполнить cap для дуг от работ до интервалов и для них же сделать flow = 0
   for (int i = 2; i < beginofintver; i++){
       // здесь нормально, не нужно учитывать процессоры, сами учтутся по времени
       for(int j = beginofintver; j < web.V.size(); j++){
           if (web.V[i].sttime <= web.V[j].sttime && web.V[i].fintime >= web.V[j].fintime){
               web.V[i].cap[j] = web.V[j].duration;
               web.V[j].cap[i] = 0;

               web.V[i].flow[j] = 0;
               web.V[j].flow[i] = 0;
           }
       }
       //соединение с источником вершин-работ
       web.V[0].cap[i] = web.V[i].duration;
       web.V[i].cap[0] = 0;
       web.V[i].flow[0] = 0;
       web.V[0].flow[i] = 0;
   }

   //соединение со стоком вершин-интервалов
   for (int i = beginofintver; i < web.V.size(); i++){
       web.V[i].cap[1] = web.V[i].duration;
       web.V[1].cap[i] = 0;
       web.V[i].flow[1] = 0;
       web.V[1].flow[i] = 0;
   }

  //дополнительная информация
  web.src = 0;
  web.dest = 1;
  web.hints = nproc*nproc - 1;
  web.n = web.V.size();
  web.numofwork = jobs.size();
  web.q = maxpart;
  web.nproc = nproc;
  //добавить структуру для разделов в вершины-интервалов
  for(int i = beginofintver; i < web.V.size(); i++){
      web.V[i].partin.resize(web.q + 1);
      for(int j = 0; j < web.q + 1; j++){
          web.V[i].partin[j] = 0;
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

QList< QList<Window*> > CreateWindows(Web* web)
{   QList< QList<Window*> > all_windows;
    int nproc = web->nproc;
    for(int iproc = 0; iproc < web->nproc; iproc++){
        QList<Window*> windows;
        Window* win = new Window;
        int curtime = 0;
        for (int it = web->numofwork + 2 + iproc; it < web->n; it+=nproc){
            //добавить работы в окно
            //for(QMap<int, int>::const_iterator _it = web->V[it].flow.begin(); _it != web->V[it].flow.end(); _it++){
            //  if (_it.key() == 0 || _it.key() == 1 ) continue;
            //  if(web->V[_it.key()].flow[it] > 0){
            //    if (win->works.contains(web->V[_it.key()].numtask)){
            //      win->works[web->V[_it.key()].numtask] += web->V[_it.key()].flow[it];
            //    }
            //    else{
            //      win->works[web->V[_it.key()].numtask] = web->V[_it.key()].flow[it];
            //    }
            //  }
            //}

            //подготтовительные мероприятия для облегчения вычислений
            int chwdw = web->V[it].chwdw;
            if (web->V[it].islwin) chwdw--;
            if (web->V[it].isrwin) chwdw--;
            //web->V[it].setpart-=web->V[it].firstpart;
            //web->V[it].setpart-=web->V[it].lastpart;
            curtime = web->V[it].sttime;

            //анализ самого начала
            if (it == web->numofwork + 2 + iproc){
                win->start = 0;
                win->partition = web->V[it].firstpart;
            }

            //для пустых интервалов
            if (win->partition == 0 && web->V[it].firstpart != 0) win->partition = web->V[it].firstpart;

            //анализ левой части
            if (web->V[it].islwin) {
                win->finish = curtime;
                windows.append(win);

                win = new Window;
                //открываем новое окно
                curtime+=web->cw;
                win->start = curtime;
                win->partition = web->V[it].firstpart;
            }

            //анализ средней части
            if (chwdw != 0){
                //закрыть то, что началось
                curtime+=web->V[it].partin[web->V[it].firstpart];
                win->finish = curtime;
                // добавить работы
                for(QMap<int, int>::const_iterator _it = web->V[it].flow.begin(); _it != web->V[it].flow.end(); _it++){
                  if (web->V[_it.key()].part != win->partition || _it.key() == 0 || _it.key() == 1 ) continue;
                  if(web->V[_it.key()].flow[it] > 0){
                    if (win->works.contains(web->V[_it.key()].numtask)){
                      win->works[web->V[_it.key()].numtask] += web->V[_it.key()].flow[it];
                    }
                    else{
                      win->works[web->V[_it.key()].numtask] = web->V[_it.key()].flow[it];
                    }
                  }
                }
                windows.append(win);


                //открыть - закрыть вместе
                for(QSet<int>::iterator its = web->V[it].setpart.begin(); its != web->V[it].setpart.end(); its++){
                    if (*its == web->V[it].lastpart || *its == web->V[it].firstpart) continue;
                    win = new Window;
                    curtime+=web->cw;
                    win->start = curtime;
                    win->partition = *its;
                    curtime+=web->V[it].partin[*its];
                    win->finish = curtime;
                    // добавить работы
                    for(QMap<int, int>::const_iterator _it = web->V[it].flow.begin(); _it != web->V[it].flow.end(); _it++){
                      if (web->V[_it.key()].part != win->partition || _it.key() == 0 || _it.key() == 1 ) continue;
                      if(web->V[_it.key()].flow[it] > 0){
                        if (win->works.contains(web->V[_it.key()].numtask)){
                          win->works[web->V[_it.key()].numtask] += web->V[_it.key()].flow[it];
                        }
                        else{
                          win->works[web->V[_it.key()].numtask] = web->V[_it.key()].flow[it];
                        }
                      }
                    }
                    windows.append(win);


                }
                win = new Window;
                curtime+=web->cw;
                win->start = curtime;
                win->partition = web->V[it].lastpart;
            }


            curtime+=web->V[it].partin[web->V[it].lastpart];
            // добавить работы
            for(QMap<int, int>::const_iterator _it = web->V[it].flow.begin(); _it != web->V[it].flow.end(); _it++){
              if (web->V[_it.key()].part != win->partition || _it.key() == 0 || _it.key() == 1 ) continue;
              if(web->V[_it.key()].flow[it] > 0){
                if (win->works.contains(web->V[_it.key()].numtask)){
                  win->works[web->V[_it.key()].numtask] += web->V[_it.key()].flow[it];
                }
                else{
                  win->works[web->V[_it.key()].numtask] = web->V[_it.key()].flow[it];
                }
              }
            }
            //открыть последенее

            //анализ правой части
            if (web->V[it].isrwin) {
                curtime = web->V[it].fintime - web->cw;
                win->finish = curtime;
                windows.append(win);

                win = new Window;
                //открываем новое окно
                curtime+=web->cw;
                win->start = curtime;
                win->partition = web->V[it+nproc].firstpart;
            }



            //анализ самого конца
            if (it == web->n - (nproc-iproc)){
                win->finish = web->V[it].fintime;
                windows.append(win);
            }

        }
        all_windows.append(windows);
     }
     return all_windows;
}

long long TimeToShedule(QList<Task*> tasks)
{
    long long timelap = 1;
    for (QList<Task*>::iterator it = tasks.begin(); it != tasks.end(); it++)
    {
        timelap = NOK(timelap, (*it)->period);
    }
    return timelap;
}

long long NOK(long long a, long long b)
{
  return a*b / NOD(a, b);
}

long long NOD(long long a, long long b)
{

  while (a != 0 and b != 0)
  {
    if (a > b)
    {
              a %= b;
    }
    else
    {
              b %= a;
    }
  }

  return a + b;
}

