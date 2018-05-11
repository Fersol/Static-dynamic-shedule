#include "types.h"



Web::Web()
{
        cw = 0;
        n = 0;
        q = 0;
        src = 0;
        dest = 1;
        V.resize(0);
}

int Web::finsetneq(SInt& set, int a)
{
     for (SInt::iterator it = set.begin(); it != set.end(); it++){
        if (*it != a) return *it;
     }
     return 0;
}

void Web::lift(int u)
{
        int height = n;
        // проход по соседям, минимальная вершина среди тех, куда возможно проталкивание
        for (QMap<int,int>::iterator it = V[u].cap.begin(); it != V[u].cap.end(); it++){
           // int change = 0;
           // if (V[it.key()].type == INTERVAL){
           //     change = test(u,it.key())*cw;
           // }
            if (V[u].cap[it.key()] > V[u].flow[it.key()]){
                // skip if this is way to destination
                if (it.key() == 1) {
                    height = V[u].h;
                }
                else{
                    height = std::min(height, V[it.key()].h);
                }
            }
        }
        V[u].h = height + 1;
}

void Web::window(int u)
{       
    // ограничивает пропускную способность в
        if (V[u].type == INTERVAL){
            V[u].cap[dest] -= cw;
            if (V[u].flow[dest] > V[u].cap[dest]){
             int value = V[u].cap[dest] - V[u].flow[dest];
             V[u].flow[dest] += value;
             V[dest].flow[u] = -V[u].flow[dest];
             V[u].exf -= value;
             V[dest].exf += value;

             if (V[u].exf > 0) P[V[u].part]+=u;
            }
            V[u].chwdw++;
        }
}

void Web::clwindow(int u)
{
        if (V[u].type == INTERVAL){
            V[u].cap[dest] += cw;
            if (V[u].exf >= cw){
                int value = cw;
                V[u].flow[dest] += value;
                V[dest].flow[u] = -V[u].flow[dest];
                V[u].exf -= value;
                V[dest].exf += value;
            }
            if (V[u].exf == 0){
                P[V[u].part]-=u;
            }
            V[u].chwdw--;
        }
}

void Web::push(int u, int v, bool is_first_epoch)
{
        int ni, pi;
        int value = std::min(V[u].cap[v] - V[u].flow[v], V[u].exf);
        if (V[v].type == INTERVAL && is_first_epoch) value = std::max(0,(std::min(value, V[v].cap[dest] - V[v].flow[dest] -(test(u,v, value))*cw) -V[v].exf));
        if (value == 0){
            return;
        }
        V[u].flow[v] += value;
        V[v].flow[u] = -V[u].flow[v];
        V[u].exf -= value;
        V[v].exf += value;


        //добавить раздел в вершину
        if ((V[u].type == JOB) && (V[v].type == INTERVAL))
        {
            // первый раз отправили и определили раздел
            if (nproc > 1 && QP[V[u].part] == -1){
                part_to_proc(V[u].part, V[v].proc);
            }
            ni = findnext(v);
            pi = findprev(v);
            checkpartadd(v, V[u].part, value, ni, pi);
            correctwindows(v, ni, pi);
        }


        //вершина-интервал в вершину-работу
        if ((V[u].type == INTERVAL) && (V[v].type == JOB))
        {
         //сказать, что сюда не надо больше посылать столько
         // записать в COR
            // main very main
         //V[v].cap[u] -= value;
         //COR[qMakePair(u ,v)] = value;

         ni = findnext(u);
         pi = findprev(u);
         checkpartdec(u, V[v].part, value);
         correctwindows(u, ni, pi);
        }

        if(v != 1 && V[v].exf > 0){

            P[V[v].part].insert(v); //добавили в список переполненных

        }
}

bool Web::discharge(int u, bool is_first)
{
        bool lifted = false;
        bool is_first_epoch = true;
        QMap<int,int>::iterator cur = V[u].cap.begin();
        //lift(u);
       // if (is_first){
            while (V[u].exf > 0) {

                //Поднять вершину если все пути просмотены и начать с начала
                if (cur == V[u].cap.end()) {
                    // первый раз только один проход по всем вершинам
                    if (is_first){
                        return lifted;
                    }
                    if (!is_first_epoch)
                    {
                        lift(u);
                        is_first_epoch = true;
                        lifted = true;
                    }
                    else is_first_epoch = false;
                    cur = V[u].cap.begin();

                    continue;
                }

                int v = cur.key();

                // изменение пропускной способности при проталкивании
                //int change = 0;
                //if (V[v].type == INTERVAL){
                //    change = test(u,v)*cw;
                //}

                // проталкивание в сток
                if (v == dest && V[u].cap[v] > V[u].flow[v]){
                   push(u, v, is_first_epoch);
                }


                //Если можно - протолкнуть
                if (V[u].cap[v] > V[u].flow[v] && V[u].h == V[v].h + 1)
                {
                   if (v == 0){
                       if (hints != 0 && V[u].type == JOB){
                           // надо что-то менять
                           hints--;
                           part_from_proc(V[u].part, QP[V[u].part]);
                           lift(u);
                           lifted = true;
                           cur = V[u].cap.begin();
                           continue;
                       }
                       else{
                           push(u, v, is_first_epoch);
                       }

                   }
                   else{
                       // сначала проверяем поток дальше
                   //if (!is_first_epoch || V[v].type == INTERVAL && V[u].cap[v] - test(u,v)*cw > V[v].flow[dest] + V[u].flow[v] + V[v].exf )
                    push(u, v, is_first_epoch);
                   }
                }

                cur++;

            }
        // }
        /*
        else{
            cur = V[u].cap.end()-1;
            while (V[u].exf > 0) {

                //Поднять вершину если все пути просмотены и начать с начала
                if (cur == V[u].cap.begin()-1) {
                    // первый раз только один проход по всем вершинам
                    if (is_first){
                        return lifted;
                    }
                    if (!is_first_epoch)
                    {
                        lift(u);
                        is_first_epoch = true;
                        lifted = true;
                    }
                    else is_first_epoch = false;
                    cur = V[u].cap.end()-1;

                    continue;
                }

                int v = cur.key();

                // изменение пропускной способности при проталкивании
                //int change = 0;
                //if (V[v].type == INTERVAL){
                //    change = test(u,v)*cw;
                //}
                //Если можно - протолкнуть
                if (V[u].cap[v] > V[u].flow[v] && V[u].h == V[v].h + 1)
                {
                   if (v == 0){
                       if (hints != 0 && V[u].type == JOB){
                           // надо что-то менять
                           hints--;
                           part_from_proc(V[u].part, QP[V[u].part]);
                           lift(u);
                           lifted = true;
                           cur = V[u].cap.begin();
                           continue;
                       }
                       else{
                           push(u, v, is_first_epoch);
                       }

                   }
                   else{
                       // сначала проверяем поток дальше
                   if (!is_first_epoch || V[v].type == INTERVAL && V[u].cap[v] - test(u,v)*cw > V[v].flow[dest] + V[u].flow[v] + V[v].exf )
                    push(u, v, is_first_epoch);
                   }
                }

                cur--;

            }
         }*/
        //P[V[u].part] -= u;
        return lifted;
}

void Web::maxflow()
{
     //Инициализация
     V[0].h = n;

     bool isfirsttime = true;
     bool isendwork = false;
     while (!isendwork)
     {
         hints = nproc*nproc - 1;
         isendwork = true;
         V[0].h = ATTEMPT_COUNT;
         for(QMap<int,int>::iterator it = V[0].cap.begin(); it != V[0].cap.end(); it++)
         {
            int value = V[0].cap[it.key()] - V[0].flow[it.key()] ;
            V[0].flow[it.key()] += value;
            V[it.key()].h = 1;
            V[it.key()].flow[0] = - V[0].flow[it.key()];
            V[it.key()].exf += value;
            V[0].exf -= value;
            if (V[it.key()].exf > 0) P[V[it.key()].part]+=it.key();
         }

         if (isfirsttime)
         {
             hard = -V[0].exf;
             isfirsttime = false;
         }
         //цикл по внутренним вершинам, пока не останется переполненных вершин
         bool isend = false;
         while (!isend)
         {
            isend = true;
            for (int i = 1; i < q + 1; i++)//по порядку разделов
            {
                bool is_first = true;
                while (!P[i].isEmpty() || !P[0].isEmpty())
                {
                    if(is_first){
                        for(QSet<int>::iterator it1 = P[i].begin(); it1 != P[i].end(); it1++){
                            discharge(*it1, true);
                        }
                        is_first = false;
                    }

                    while(!P[0].isEmpty()){
                        QSet<int>::iterator it0 = P[0].begin();
                        discharge(*it0, false);
                        isend = false;
                        P[0]-=*it0;
                    }

                    if(!P[i].isEmpty()){
                        QSet<int>::iterator it = P[i].begin();
                        discharge(*it, false);
                        isend = false;
                        P[i]-=*it;
                    }
                }
            }
         }
        // if (!istryedSecond){
         //   isendwork = false;
        //    istryedSecond = true;
        //    back();
        //    continue;
        // }
         //проверка на размещение работ целиком
         // удаляем ту, которой больше всего не хватило
         int max_not_dist_flow = 0;
         int it_to_del = -1;
         for(int it = 2; it < numofwork +2; it ++){
             if (V[0].cap[it] - V[0].flow[it] > max_not_dist_flow)//работа размещена не полностью
             {   
                 max_not_dist_flow = V[0].cap[it] - V[0].flow[it];
                 it_to_del = it;
                 //isendwork = false;
                 //deletework(it_to_del);
             }
         }
         if (it_to_del != -1){
             isendwork = false;
             deletework(it_to_del);
             back();
             //noflow();
         }
       }


}


void Web::deletework(int u)
{
        for(QMap<int,int>::iterator it = V[u].cap.begin(); it != V[u].cap.end(); it++)
        {
            int v = it.key();
            if (V[v].type == INTERVAL){
                int value = V[u].flow[v];

                V[1].exf -= value;
                V[1].flow[v] += value;
                V[v].flow[1] -= value;
                V[u].flow[v] -= value;
                V[v].flow[u] += value;

                int ni = findnext(v);
                int pi = findprev(v);
                checkpartdec(v, V[u].part, value);
                correctwindows(v, ni, pi);

                //int value1 = V[0].flow[u];
                V[0].cap[u] = 0;
                V[0].flow[u] = 0;
                V[u].flow[0] = 0;
                V[0].exf += value;
            }
        }



}

double Web::Effectivness()
{
    return -(double)V[0].exf/hard;
}

int Web::test(int u, int v, int value)
{
    if (V[u].type == JOB && V[v].type == INTERVAL){
        checkpartadd(v, V[u].part, value, findnext(v), findprev(v));
        correctwindows(v, findnext(v),findprev(v));
        int win1 = V[v].chwdw;
        checkpartdec(v, V[u].part, value);
        correctwindows(v, findnext(v),findprev(v));
        int win2 = V[v].chwdw;
        return win1-win2;
    }
    else return 0;

}

void Web::checkpartadd(int v, int part, int value, int ni, int pi)
{
    int numpart1 = V[v].setpart.size();
    V[v].partin[part]+=value;
    if (V[v].partin[part] != 0) V[v].setpart.insert(part);
    int numpart2 = V[v].setpart.size();

    if (numpart1 != numpart2)
    {//добавился раздел

       // назначить первый и последний раздел в интервале
       if (numpart2 == 1)
       {//появился первый раздел
        V[v].firstpart = part;
        V[v].lastpart = part;
       }
       else if (numpart2 == 2)
       {//появился второй раздел
        if (ni != 0 && V[ni].firstpart != 0 && part == V[ni].firstpart) V[v].lastpart = part;
        else V[v].firstpart = part;
       }
       else if (numpart2 > 2)
       {
        if (ni != 0 && V[ni].firstpart != 0 && part == V[ni].firstpart) V[v].lastpart = part;
        else if (pi != 0 && V[pi].firstpart != 0 && part == V[pi].lastpart) V[v].firstpart = part;
       }
    }
}

void Web::checkpartdec(int v, int part, int value)
{
    int numpart1 = V[v].setpart.size();
    V[v].partin[part]-=value;
    if (V[v].partin[part] == 0) V[v].setpart-=(part);//убрали совсем
    int numpart2 = V[v].setpart.size();

    if (numpart1 != numpart2)
    {
       if (numpart2 == 0)
       {//все разделы убрались
        V[v].firstpart = 0;
        V[v].lastpart = 0;
       }
       else if (numpart2 == 1)
       {//остался один раздел
        int part1 = V[v].lastpart;//оставшийся раздел
        if (part1 == part) part1 = V[v].firstpart;
        V[v].firstpart = part1;
        V[v].lastpart = part1;
       }
       else if (numpart2 > 1)
       {
        if (part == V[v].lastpart) V[v].lastpart = finsetneq(V[v].setpart, V[v].firstpart);
        else if (part == V[v].firstpart) V[v].firstpart = finsetneq(V[v].setpart, V[v].lastpart);
       }
    }
}

void Web::correctwindows(int v, int ni, int pi)
{
    int numofpart = V[v].setpart.size();//число разделов в интервале
    int chwdwinter = V[v].chwdw; //число внутренних переключений
    if (V[v].islwin) chwdwinter--;
    if (V[v].isrwin) chwdwinter--;

    if (numofpart != 0)
    {
        //корректировка числа внутренних переключений
        while (chwdwinter != numofpart -1)
        {
            if (chwdwinter > numofpart -1)
            {
                clwindow(v);
                chwdwinter--;
            }
            else
            {
                window(v);
                chwdwinter++;
            }
        }

        //корректировка правого переключения
        if (ni != 0)
            {
         //условие открытия
         if (V[ni].firstpart != V[v].lastpart && !V[ni].islwin && !V[v].isrwin)
         {
            if( V[ni].cap[dest] - V[ni].flow[dest] >= cw && V[v].exf + V[v].flow[dest] > V[v].cap[dest] - cw){
                window(ni);
                V[ni].islwin = true;
            }
            else {
                window(v);
                V[v].isrwin = true;
            }
         }
         //услвие закрытия
         if (V[ni].firstpart == V[v].lastpart)
         {
            if (V[ni].islwin)
            {
             clwindow(ni);
             V[ni].islwin = false;
            }
            if (V[v].isrwin)
            {
             clwindow(v);
             V[v].isrwin = false;
            }
         }
        }
        else
        {
            if (V[v].isrwin)
            {
             clwindow(v);
             V[v].isrwin = false;
            }
        }


        //корректировка левого переключения
        if (pi != 0)
        {
         //условие открытия
         if (V[pi].lastpart != V[v].firstpart && !V[pi].isrwin && !V[v].islwin)
         {
             if( V[pi].cap[dest] - V[pi].flow[dest] >= cw && V[v].exf + V[v].flow[dest] > V[v].cap[dest] - cw){
             //if( V[pi].flow[dest] - V[pi].cap[dest] >= cw){
                 window(pi);
                 V[pi].isrwin = true;
             }
             else{
                window(v);
                V[v].islwin = true;
             }
         }
         //условие закрытия
         if (V[pi].lastpart == V[v].firstpart)
         {
            if (V[pi].isrwin)
            {
             clwindow(pi);
             V[pi].isrwin = false;
            }
            if (V[v].islwin)
            {
             clwindow(v);
             V[v].islwin = false;
            }
         }
        }
        else
        {
            if (V[v].islwin)
            {
             clwindow(v);
             V[v].islwin = false;
            }
        }
    }
    else
    {
        //корректировка внутренних
        while (chwdwinter != 0)
        {
            clwindow(v);
            chwdwinter--;
        }

        //корректировка краевых
        if (ni == 0 || pi ==0)
        {
            if (V[v].islwin)
            {
                clwindow(v);
                V[v].islwin = false;
            }
            if (V[v].isrwin)
            {
                clwindow(v);
                V[v].isrwin = false;
            }
            if (ni != 0 && V[ni].islwin)
            {
                clwindow(ni);
                V[ni].islwin = false;
            }
            if (pi != 0 && V[pi].isrwin)
            {
                clwindow(pi);
                V[pi].isrwin = false;
            }
        }
        else
        {
            //убрать переключения в интервале
            if (V[v].islwin)
            {
                clwindow(v);
                V[v].islwin = false;
            }
            if (V[v].isrwin)
            {
                clwindow(v);
                V[v].isrwin = false;
            }

            //обработать переключение между крайними
            //условие открытия
            if (V[ni].firstpart != V[pi].lastpart && !V[ni].islwin && !V[pi].isrwin)
            {
               window(pi);
               V[pi].isrwin = true;
            }
            //услвие закрытия
            if (V[ni].firstpart == V[pi].lastpart)
            {
               if (V[ni].islwin)
               {
                clwindow(ni);
                V[ni].islwin = false;
               }
               if (V[pi].isrwin)
               {
                clwindow(pi);
                V[pi].isrwin = false;
               }
            }
        }

    }

}

int Web::findnext(int v)
{
    int ni = V[v].nextitr;
    while (ni != 0 && V[ni].firstpart == 0)
    {
        ni = V[ni].nextitr;
    }
    return ni;
}

int Web::findprev(int v)
{
    int pi = V[v].previtr;
    while (pi != 0 && V[pi].lastpart == 0)
    {
        pi = V[pi].previtr;
    }
    return pi;
}

void Web::back()
{
    //for(int v = 1; v < numofwork + 2; v++)
    //{
    //    V[v].h = 0;
    //    for(QMap<int,int>::iterator it = V[v].cap.begin(); it != V[v].cap.end(); it++)
    //    {
    //        int u = it.key();
    //        if (V[u].type == INTERVAL && V[u].proc == QP[V[v].part])
    //        {
    //            V[v].cap[u] = V[u].duration;
    //        }
    //    }
    //}
    for(int v = 1; v < n; v++)
    {
        V[v].h = 0;
    }
}

void Web::noflow()
{
    for(int v = 0; v < n; v++)
    {
        V[v].h = 0;
        for(QMap<int,int>::iterator it = V[v].cap.begin(); it != V[v].cap.end(); it++)
        {
                V[v].flow[it.key()] = 0;
        }
        if (V[v].type == INTERVAL){
            V[v].firstpart = 0;
            V[v].lastpart = 0;
            QSet<int> set;
            V[v].setpart = set;
            for (int i = 0; i < q+1; i++){
                V[v].partin[i] = 0;
            }
            V[v].chwdw = 0;
            V[v].isrwin = false;
            V[v].islwin = false;
        }
    }
    V[0].exf = 0;
    V[1].exf = 0;
}

int Web::sheduledjobs()
{
    int k = 0;
    for(int it = 2; it < numofwork +2; it++)
    {
        if (V[0].cap[it] != 0) k++;
    }
    return k;
}





void Web::part_to_proc(int q, int proc){
    // Раздел прикпрепляется к процессору, на других он выполнятся не может - пропускные способности в остальные равны 0
    QP[q] = proc;
    for(QSet<int>::iterator pit = (VPart[q]).begin(); pit != (VPart[q]).end(); pit++){
        for (QMap<int,int>::iterator it = V[*pit].cap.begin(); it != V[*pit].cap.end(); it++){
            if (V[it.key()].proc != proc){
                V[*pit].cap[it.key()] = 0;
            }
        }
    }
}

void Web::part_from_proc(int q, int proc){
    QP[q] = -1;
    // убрать поток

    // открыть новые пути
    for(QSet<int>::iterator pit = (VPart[q]).begin(); pit != (VPart[q]).end(); pit++){
        for (QMap<int,int>::iterator it = V[*pit].cap.begin(); it != V[*pit].cap.end(); it++){
            int u = it.key();
            int v = *pit;
            if (V[u].proc == proc){

                int value = V[v].flow[u];

                V[1].exf -= value;
                V[v].exf += value;
                V[v].h = 1;
                V[u].h = 1;
                if (v != 1 && V[v].exf > 0) P[V[v].part] += v;

                V[1].flow[u] += value;
                V[u].flow[1] -= value;
                V[v].flow[u] -= value;
                V[u].flow[v] += value;

                int ni = findnext(u);
                int pi = findprev(u);
                checkpartdec(u, V[v].part, value);
                correctwindows(u, ni, pi);

                V[v].cap[u] = 0;
            }
            else if (V[u].type == INTERVAL)
            {
                V[*pit].cap[u] = V[u].duration;
            }
        }
    }
}
