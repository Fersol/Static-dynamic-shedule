#include "types.h"



Web::Web()
{
        cw = 0;
        n = 0;
        q = 0;
        src = 0;
        dest = 1;
        verVec.resize(0);
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
        for (map<int,int>::iterator it = verVec[u].cap.begin(); it != verVec[u].cap.end(); it++){
           // int change = 0;
           // if (vertexVector[it->first].type == INTERVAL){
           //     change = test(u,it->first)*cw;
           // }
            if (verVec[u].cap[it->first] > verVec[u].flow[it->first]){
                // skip if this is way to destination
                if (it->first == 1) {
                    height = verVec[u].h;
                }
                else{
                    height = min(height, verVec[it->first].h);
                }
            }
        }
        verVec[u].h = height + 1;
}

void Web::window(int u)
{       
    // ограничивает пропускную способность в
        if (verVec[u].type == INTERVAL){
            verVec[u].cap[dest] -= verVec[u].cTime;
            if (verVec[u].flow[dest] > verVec[u].cap[dest]){
             int value = verVec[u].cap[dest] - verVec[u].flow[dest];
             verVec[u].flow[dest] += value;
             verVec[dest].flow[u] = -verVec[u].flow[dest];
             verVec[u].exf -= value;
             verVec[dest].exf += value;

             if (verVec[u].exf > 0) P[verVec[u].part].insert(u);
            }
            verVec[u].chWdw++;
        }
}

void Web::clwindow(int u)
{
        if (verVec[u].type == INTERVAL){
            verVec[u].cap[dest] += verVec[u].cTime;
            if (verVec[u].exf >= verVec[u].cTime){
                int value = verVec[u].cTime;
                verVec[u].flow[dest] += value;
                verVec[dest].flow[u] = -verVec[u].flow[dest];
                verVec[u].exf -= value;
                verVec[dest].exf += value;
            }
            if (verVec[u].exf == 0){
                P[verVec[u].part].erase(u);
            }
            verVec[u].chWdw--;
        }
}

void Web::push(int u, int v, bool is_first_epoch)
{
        int ni, pi;
        int value = std::min(verVec[u].cap[v] - verVec[u].flow[v], verVec[u].exf);
        if (verVec[v].type == INTERVAL && is_first_epoch) value = std::max(0,(std::min(value, verVec[v].cap[dest] - verVec[v].flow[dest] -(test(u,v, value))*verVec[u].cTime) -verVec[v].exf));
        if (value == 0){
            return;
        }
        verVec[u].flow[v] += value;
        verVec[v].flow[u] = -verVec[u].flow[v];
        verVec[u].exf -= value;
        verVec[v].exf += value;


        //добавить раздел в вершину
        if ((verVec[u].type == JOB) && (verVec[v].type == INTERVAL))
        {
            // первый раз отправили и определили раздел
            // if (nproc > 1 && QP[verVec[u].part] == -1){
            //     part_to_proc(verVec[u].part, verVec[v].proc);
            // }
            ni = findnext(v);
            pi = findprev(v);
            checkpartadd(v, verVec[u].part, value, ni, pi);
            correctwindows(v, ni, pi);
        }


        //вершина-интервал в вершину-работу
        if ((verVec[u].type == INTERVAL) && (verVec[v].type == JOB))
        {
         //сказать, что сюда не надо больше посылать столько
         // записать в COR
            // main very main
         //verVec[v].cap[u] -= value;
         //COR[qMakePair(u ,v)] = value;

         ni = findnext(u);
         pi = findprev(u);
         checkpartdec(u, verVec[v].part, value);
         correctwindows(u, ni, pi);
        }

        if(v != 1 && verVec[v].exf > 0){

            P[verVec[v].part].insert(v); //добавили в список переполненных

        }
}

bool Web::discharge(int u, bool is_first)
{       
        bool lifted = false;
        bool is_first_epoch = true;
        map<int,int>::iterator cur = verVec[u].cap.begin();
        //lift(u);
       // if (is_first){
            while (verVec[u].exf > 0) {
                //Поднять вершину если все пути просмотены и начать с начала
                if (cur == verVec[u].cap.end()) {
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
                    cur = verVec[u].cap.begin();

                    continue;
                }

                int v = cur->first;

                // изменение пропускной способности при проталкивании
                //int change = 0;
                //if (verVec[v].type == INTERVAL){
                //    change = test(u,v)*cw;
                //}

                // проталкивание в сток
                if (v == dest && verVec[u].cap[v] > verVec[u].flow[v]){
                   push(u, v, is_first_epoch);
                }


                //Если можно - протолкнуть
                if (verVec[u].cap[v] > verVec[u].flow[v] && verVec[u].h == verVec[v].h + 1)
                {
                   if (v == 0){
                       if (hints != 0 && verVec[u].type == JOB){
                           // надо что-то менять
                        //    cout << "We want hint" << endl;
                           hints--;
                           part_from_proc(verVec[u].part, QP[verVec[u].part]);
                         //  decide_proc(verVec[u].part);
                        //    cout << "We execute part drawback" << endl;
                           lift(u);
                           lifted = true;
                           cur = verVec[u].cap.begin();
                           continue;
                       }
                       else{
                           push(u, v, is_first_epoch);
                       }

                   }
                   else{
                       // сначала проверяем поток дальше
                   //if (!is_first_epoch || verVec[v].type == INTERVAL && verVec[u].cap[v] - test(u,v)*cw > verVec[v].flow[dest] + verVec[u].flow[v] + verVec[v].exf )
                    push(u, v, is_first_epoch);
                   }
                }

                cur++;

            }
        // }
        /*
        else{
            cur = verVec[u].cap.end()-1;
            while (verVec[u].exf > 0) {

                //Поднять вершину если все пути просмотены и начать с начала
                if (cur == verVec[u].cap.begin()-1) {
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
                    cur = verVec[u].cap.end()-1;

                    continue;
                }

                int v = cur.key();

                // изменение пропускной способности при проталкивании
                //int change = 0;
                //if (verVec[v].type == INTERVAL){
                //    change = test(u,v)*cw;
                //}
                //Если можно - протолкнуть
                if (verVec[u].cap[v] > verVec[u].flow[v] && verVec[u].h == verVec[v].h + 1)
                {
                   if (v == 0){
                       if (hints != 0 && verVec[u].type == JOB){
                           // надо что-то менять
                           hints--;
                           part_from_proc(verVec[u].part, QP[verVec[u].part]);
                           lift(u);
                           lifted = true;
                           cur = verVec[u].cap.begin();
                           continue;
                       }
                       else{
                           push(u, v, is_first_epoch);
                       }

                   }
                   else{
                       // сначала проверяем поток дальше
                   if (!is_first_epoch || verVec[v].type == INTERVAL && verVec[u].cap[v] - test(u,v)*cw > verVec[v].flow[dest] + verVec[u].flow[v] + verVec[v].exf )
                    push(u, v, is_first_epoch);
                   }
                }

                cur--;

            }
         }*/
        //P[verVec[u].part] -= u;
        return lifted;
}

void Web::maxflow()
{
     //Инициализация
     verVec[0].h = n;

     for(int i =0; i < q;i++){
         decide_proc(partitionOrder[i],-1);
         cout <<"Partition "<< partitionOrder[i] <<":"<<QP[partitionOrder[i]];
     }


     bool isfirsttime = true;
     bool isendwork = false;
     while (!isendwork)
     {  
         hints = nproc*nproc - 1;
         cout << "Number of hints:" << hints << endl;
         isendwork = true;
         verVec[0].h = attemptCount;
         for(map<int,int>::iterator it = verVec[0].cap.begin(); it != verVec[0].cap.end(); it++)
         {
            int value = verVec[0].cap[it->first] - verVec[0].flow[it->first] ;
            verVec[0].flow[it->first] += value;
            verVec[it->first].h = 1;
            verVec[it->first].flow[0] = - verVec[0].flow[it->first];
            verVec[it->first].exf += value;
            verVec[0].exf -= value;
            if (verVec[it->first].exf > 0) P[verVec[it->first].part].insert(it->first);
         }

         if (isfirsttime)
         {
             hard = -verVec[0].exf;
             isfirsttime = false;
         }
         //цикл по внутренним вершинам, пока не останется переполненных вершин
         bool isend = false;
         while (!isend)
         {
            isend = true;
            cout << "New way" << endl;
            for (int i = 1; i < q + 1; i++)//по порядку разделов
            {   
                cout << "Partition now:" << i << endl;
                bool is_first = true;
                while (!P[i].empty() || !P[0].empty())
                {   
                    // cout << "We are here" << endl;
                    if(is_first){
                        for(set<int>::iterator it1 = P[i].begin(); it1 != P[i].end(); it1++){
                            discharge(*it1, true);
                        }
                        is_first = false;
                    }

                    while(!P[0].empty()){
                        set<int>::iterator it0 = P[0].begin();
                        discharge(*it0, false);
                        isend = false;
                        P[0].erase(*it0);
                    }

                    if(!P[i].empty()){
                        set<int>::iterator it = P[i].begin();
                        discharge(*it, false);
                        isend = false;
                        P[i].erase(*it);
                    }
                }
            }
            // cout << "For is ended" << endl;
         }
        //  cout << "We are stuck here!!!" << endl;
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
         for(int it = 2; it < numOfWork +2; it ++){
             if (verVec[0].cap[it] - verVec[0].flow[it] > max_not_dist_flow)//работа размещена не полностью
             {   
                 max_not_dist_flow = verVec[0].cap[it] - verVec[0].flow[it];
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
        for(map<int,int>::iterator it = verVec[u].cap.begin(); it != verVec[u].cap.end(); it++)
        {
            int v = it->first;
            if (verVec[v].type == INTERVAL){
                int value = verVec[u].flow[v];

                verVec[1].exf -= value;
                verVec[1].flow[v] += value;
                verVec[v].flow[1] -= value;
                verVec[u].flow[v] -= value;
                verVec[v].flow[u] += value;

                int ni = findnext(v);
                int pi = findprev(v);
                checkpartdec(v, verVec[u].part, value);
                correctwindows(v, ni, pi);

                //int value1 = verVec[0].flow[u];
                verVec[0].cap[u] = 0;
                verVec[0].flow[u] = 0;
                verVec[u].flow[0] = 0;
                verVec[0].exf += value;
            }
        }



}

double Web::Effectivness()
{
    return -(double)verVec[0].exf/hard;
}

int Web::test(int u, int v, int value)
{
    if (verVec[u].type == JOB && verVec[v].type == INTERVAL){
        checkpartadd(v, verVec[u].part, value, findnext(v), findprev(v));
        correctwindows(v, findnext(v),findprev(v));
        int win1 = verVec[v].chWdw;
        checkpartdec(v, verVec[u].part, value);
        correctwindows(v, findnext(v),findprev(v));
        int win2 = verVec[v].chWdw;
        return win1-win2;
    }
    else return 0;

}

void Web::checkpartadd(int v, int part, int value, int ni, int pi)
{
    int numpart1 = verVec[v].setpart.size();
    verVec[v].partIn[part]+=value;
    if (verVec[v].partIn[part] != 0) verVec[v].setpart.insert(part);
    int numpart2 = verVec[v].setpart.size();

    if (numpart1 != numpart2)
    {//добавился раздел

       // назначить первый и последний раздел в интервале
       if (numpart2 == 1)
       {//появился первый раздел
        verVec[v].firstPart = part;
        verVec[v].lastPart = part;
       }
       else if (numpart2 == 2)
       {//появился второй раздел
        if (ni != 0 && verVec[ni].firstPart != 0 && part == verVec[ni].firstPart) verVec[v].lastPart = part;
        else verVec[v].firstPart = part;
       }
       else if (numpart2 > 2)
       {
        if (ni != 0 && verVec[ni].firstPart != 0 && part == verVec[ni].firstPart) verVec[v].lastPart = part;
        else if (pi != 0 && verVec[pi].firstPart != 0 && part == verVec[pi].lastPart) verVec[v].firstPart = part;
       }
    }
}

void Web::checkpartdec(int v, int part, int value)
{   
    // cout << "checkpartdec" << v <<endl;
    int numpart1 = verVec[v].setpart.size();
    verVec[v].partIn[part]-=value;
    if (verVec[v].partIn[part] == 0) verVec[v].setpart.erase(part);//убрали совсем
    int numpart2 = verVec[v].setpart.size();
    // cout << "checkpartdec 1" << endl;  
    if (numpart1 != numpart2)
    {
       if (numpart2 == 0)
       {//все разделы убрались
        verVec[v].firstPart = 0;
        verVec[v].lastPart = 0;
       }
       else if (numpart2 == 1)
       {//остался один раздел
        int part1 = verVec[v].lastPart;//оставшийся раздел
        if (part1 == part) part1 = verVec[v].firstPart;
        verVec[v].firstPart = part1;
        verVec[v].lastPart = part1;
       }
       else if (numpart2 > 1)
       {
         
        if (part == verVec[v].lastPart) verVec[v].lastPart = finsetneq(verVec[v].setpart, verVec[v].firstPart);
        else if (part == verVec[v].firstPart) verVec[v].firstPart = finsetneq(verVec[v].setpart, verVec[v].lastPart);
       }
    }
}

void Web::correctwindows(int v, int ni, int pi)
{
    int numofpart = verVec[v].setpart.size();//число разделов в интервале
    int chwdwinter = verVec[v].chWdw; //число внутренних переключений
    if (verVec[v].isLWin) chwdwinter--;
    if (verVec[v].isRWin) chwdwinter--;

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
         if (verVec[ni].firstPart != verVec[v].lastPart && !verVec[ni].isLWin && !verVec[v].isRWin)
         {
            if( verVec[ni].cap[dest] - verVec[ni].flow[dest] >= verVec[v].cTime && verVec[v].exf + verVec[v].flow[dest] > verVec[v].cap[dest] - verVec[v].cTime){
                window(ni);
                verVec[ni].isLWin = true;
            }
            else {
                window(v);
                verVec[v].isRWin = true;
            }
         }
         //услвие закрытия
         if (verVec[ni].firstPart == verVec[v].lastPart)
         {
            if (verVec[ni].isLWin)
            {
             clwindow(ni);
             verVec[ni].isLWin = false;
            }
            if (verVec[v].isRWin)
            {
             clwindow(v);
             verVec[v].isRWin = false;
            }
         }
        }
        else
        {
            if (verVec[v].isRWin)
            {
             clwindow(v);
             verVec[v].isRWin = false;
            }
        }


        //корректировка левого переключения
        if (pi != 0)
        {
         //условие открытия
         if (verVec[pi].lastPart != verVec[v].firstPart && !verVec[pi].isRWin && !verVec[v].isLWin)
         {
             if( verVec[pi].cap[dest] - verVec[pi].flow[dest] >= verVec[v].cTime && verVec[v].exf + verVec[v].flow[dest] > verVec[v].cap[dest] - verVec[v].cTime){
             //if( verVec[pi].flow[dest] - verVec[pi].cap[dest] >= cw){
                 window(pi);
                 verVec[pi].isRWin = true;
             }
             else{
                window(v);
                verVec[v].isLWin = true;
             }
         }
         //условие закрытия
         if (verVec[pi].lastPart == verVec[v].firstPart)
         {
            if (verVec[pi].isRWin)
            {
             clwindow(pi);
             verVec[pi].isRWin = false;
            }
            if (verVec[v].isLWin)
            {
             clwindow(v);
             verVec[v].isLWin = false;
            }
         }
        }
        else
        {
            if (verVec[v].isLWin)
            {
             clwindow(v);
             verVec[v].isLWin = false;
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
            if (verVec[v].isLWin)
            {
                clwindow(v);
                verVec[v].isLWin = false;
            }
            if (verVec[v].isRWin)
            {
                clwindow(v);
                verVec[v].isRWin = false;
            }
            if (ni != 0 && verVec[ni].isLWin)
            {
                clwindow(ni);
                verVec[ni].isLWin = false;
            }
            if (pi != 0 && verVec[pi].isRWin)
            {
                clwindow(pi);
                verVec[pi].isRWin = false;
            }
        }
        else
        {
            //убрать переключения в интервале
            if (verVec[v].isLWin)
            {
                clwindow(v);
                verVec[v].isLWin = false;
            }
            if (verVec[v].isRWin)
            {
                clwindow(v);
                verVec[v].isRWin = false;
            }

            //обработать переключение между крайними
            //условие открытия
            if (verVec[ni].firstPart != verVec[pi].lastPart && !verVec[ni].isLWin && !verVec[pi].isRWin)
            {
               window(pi);
               verVec[pi].isRWin = true;
            }
            //услвие закрытия
            if (verVec[ni].firstPart == verVec[pi].lastPart)
            {
               if (verVec[ni].isLWin)
               {
                clwindow(ni);
                verVec[ni].isLWin = false;
               }
               if (verVec[pi].isRWin)
               {
                clwindow(pi);
                verVec[pi].isRWin = false;
               }
            }
        }

    }

}

int Web::findnext(int v)
{
    int ni = verVec[v].nextItr;
    while (ni != 0 && verVec[ni].firstPart == 0)
    {
        ni = verVec[ni].nextItr;
    }
    return ni;
}

int Web::findprev(int v)
{
    int pi = verVec[v].prevItr;
    while (pi != 0 && verVec[pi].lastPart == 0)
    {
        pi = verVec[pi].prevItr;
    }
    return pi;
}

void Web::back()
{
    //for(int v = 1; v < numOfWork + 2; v++)
    //{
    //    verVec[v].h = 0;
    //    for(map<int,int>::iterator it = verVec[v].cap.begin(); it != verVec[v].cap.end(); it++)
    //    {
    //        int u = it->first;
    //        if (verVec[u].type == INTERVAL && verVec[u].proc == QP[verVec[v].part])
    //        {
    //            verVec[v].cap[u] = verVec[u].duration;
    //        }
    //    }
    //}
    for(int v = 1; v < n; v++)
    {
        verVec[v].h = 0;
    }
}

void Web::noflow()
{
    for(int v = 0; v < n; v++)
    {
        verVec[v].h = 0;
        for(map<int,int>::iterator it = verVec[v].cap.begin(); it != verVec[v].cap.end(); it++)
        {
                verVec[v].flow[it->first] = 0;
        }
        if (verVec[v].type == INTERVAL){
            verVec[v].firstPart = 0;
            verVec[v].lastPart = 0;
            set<int> set;
            verVec[v].setpart = set;
            for (int i = 0; i < q+1; i++){
                verVec[v].partIn[i] = 0;
            }
            verVec[v].chWdw = 0;
            verVec[v].isRWin = false;
            verVec[v].isLWin = false;
        }
    }
    verVec[0].exf = 0;
    verVec[1].exf = 0;
}

int Web::sheduledjobs()
{
    int k = 0;
    for(int it = 2; it < numOfWork +2; it++)
    {
        if (verVec[0].cap[it] != 0) k++;
    }
    return k;
}





// void Web::part_to_proc(int q, int proc){
//     // Раздел прикпрепляется к процессору, на других он выполнятся не может - пропускные способности в остальные равны 0
//     QP[q] = proc;
//     for(set<int>::iterator pit = (vPart[q]).begin(); pit != (vPart[q]).end(); pit++){
//         for (map<int,int>::iterator it = verVec[*pit].cap.begin(); it != verVec[*pit].cap.end(); it++){
//             if (verVec[it->first].proc != proc){
//                 verVec[*pit].cap[it->first] = 0;
//             }
//         }
//     }
// }

void Web::part_from_proc(int q, int proc){
    //QP[q] = -1;
    processorLoad[proc] += partitionComplexity[q];
    // убрать поток
    // cout << "we begin" << endl;
    // открыть новые пути
    for(set<int>::iterator pit = (vPart[q]).begin(); pit != (vPart[q]).end(); pit++){
        int v = *pit;
        // cout << "we begin:" << v <<endl;
        for (map<int,int>::iterator it = verVec[v].cap.begin(); it != verVec[v].cap.end(); it++){
            // cout << "we continue:" << it->first <<endl;
            int u = it->first;
            if (verVec[u].proc == proc){

                int value = verVec[v].flow[u];

                verVec[u].exf -= value;
                verVec[v].exf += value;
                verVec[v].h = 1;
                verVec[u].h = 1;
                if (v != 1 && verVec[v].exf > 0) P[verVec[v].part].insert(v);

                // verVec[1].flow[v] += value;
                // verVec[v].flow[1] -= value;
                verVec[v].flow[u] -= value;
                verVec[u].flow[v] += value;

                int ni = findnext(u);
                int pi = findprev(u);
                // cout << "we there 1:" << it->first <<endl;
                checkpartdec(u, verVec[v].part, value);
                // cout << "we there 2:" << it->first <<endl;
                correctwindows(u, ni, pi);
                // cout << "we there 3:" << it->first <<endl;
                verVec[v].cap[u] = 0;
            }
            else if (verVec[u].type == INTERVAL)
            {
                verVec[v].cap[u] = verVec[u].duration;
            }
        }
    }

    // Назначить новый процессор
    decide_proc(q,proc);
    cout << "New Proc" << q <<endl;
}

void Web::decide_proc(int part, int prohibit_proc){
    int idxProc = 0;
    int freeSpace = 0;
    int complexity = partitionComplexity[part];

    for(int i=0; i < nproc;i++){
        bool isFunctionality = true;
        set<string> result;
        std::set_intersection(processors[i]->functionality.begin(),processors[i]->functionality.end(),
         partitionFunctionality[part].begin(), partitionFunctionality[part].end(),
        std::inserter(result, result.begin()));
        if (result != partitionFunctionality[part]){
            isFunctionality = false;
            //cout << "false part" << part << ": proc " << i << endl;
        }
        if (processorLoad[i] > freeSpace && i != prohibit_proc && isFunctionality) {
            idxProc = i;
            freeSpace = processorLoad[i]; 
        }
        //cout << "proc:" << idxProc << endl;
    }

    cout << "finel proc:" << idxProc;
    processorLoad[idxProc] -= complexity;
    QP[part] = idxProc;

    // запретить другие пути
    for(set<int>::iterator pit = (vPart[part]).begin(); pit != (vPart[part]).end(); pit++){
        for (map<int,int>::iterator it = verVec[*pit].cap.begin(); it != verVec[*pit].cap.end(); it++){
            if (verVec[it->first].proc != idxProc){
                verVec[*pit].cap[it->first] = 0;
            }
        }
    }
}

long long NOK(long long a, long long b)
{
  return a*b / NOD(a, b);
}

long long NOD(long long a, long long b)
{

  while (a != 0 && b != 0)
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


vector<string> split(string strToSplit, string delimeter)
{
     std::vector<std::string> splittedString;
     int startIndex = 0;
     int  endIndex = 0;
     while( (endIndex = strToSplit.find(delimeter, startIndex)) < strToSplit.size() )
    {
       string val = strToSplit.substr(startIndex, endIndex - startIndex);
       splittedString.push_back(val);
       startIndex = endIndex + delimeter.size();
     }
     if(startIndex < strToSplit.size())
     {
       string val = strToSplit.substr(startIndex);
       splittedString.push_back(val);
     }
     return splittedString;
}

