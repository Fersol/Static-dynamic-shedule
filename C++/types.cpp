#include "types.h"



Web::Web()
{
        cw = 0;
        n = 0;
        q = 0;
}

int Web::finsetneq(SInt& set, int a)
{
     for (SInt::iterator it = set.begin(); it != set.end(); it++){
        if (*it != a) return *it;
     }
     return 0;
}

void Web::lift(int l_u, int u)
{
        int height = n;
        // проход по соседям, минимальная вершина среди тех, куда возможно проталкивание
        for(Neighbors::iterator it_layer = layers[l_u].vertexes[u].neighbors.begin(); it_layer != layers[l_u].vertexes[u].neighbors.end(); it_layer++){
            map<int, NeighborInfo > layer_neighbors = it_layer->second;
            // Номер слоя соседа
            int l_v = it_layer->first;
            for (map<int, NeighborInfo >::iterator it = layer_neighbors.begin(); it != layer_neighbors.end(); it++){
            // int change = 0;
            // if (vertexVector[it->first].type == INTERVAL){
            //     change = test(u,it->first)*cw;
            // }
                // Номер вершины соседа
                int v = it->first;
                NeighborInfo vertex = it->second;
                //if (layers[l].vertexes[u].cap[it->first] > layers[l].vertexes[u].flow[it->first]){
                if (vertex.cap > vertex.flow){
                    // Кажется это лишее, теперь не будет источника и стока
                    // // skip if this is way to destination
                    // if (it->first == 1) {
                    //     height = layers[l].vertexes[u].h;
                    // }
                    //else{
                        height = min(height, layers[l_v].vertexes[v].h);
                    //}
                }
            }
            layers[l_u].vertexes[u].h = height + 1;
        }
}

void Web::window(int l_u, int u)
{       
    // ограничивает пропускную способность в
        if (layers[l_u].vertexes[u].type == INTERVAL){
            layers[l_u].vertexes[u].capacity -= layers[l_u].vertexes[u].cTime;
            if (layers[l_u].vertexes[u].flow > layers[l_u].vertexes[u].capacity){
             int value = layers[l_u].vertexes[u].capacity - layers[l_u].vertexes[u].flow;
             layers[l_u].vertexes[u].flow += value;
             layers[l_u].vertexes[u].exf -= value;

             if (layers[l_u].vertexes[u].exf > 0) P[layers[l_u].vertexes[u].part].insert(u);
            }
            layers[l_u].vertexes[u].chWdw++;
        }
}

void Web::clwindow(int l_u, int u)
{
        if (layers[l_u].vertexes[u].type == INTERVAL){
            layers[l_u].vertexes[u].capacity += layers[l_u].vertexes[u].cTime;
            if (layers[l_u].vertexes[u].exf >= layers[l_u].vertexes[u].cTime){
                int value = layers[l_u].vertexes[u].cTime;
                layers[l_u].vertexes[u].flow += value;
                layers[l_u].vertexes[u].exf -= value;
            }
            if (layers[l_u].vertexes[u].exf == 0){
                P[layers[l_u].vertexes[u].part].erase(u);
            }
            layers[l_u].vertexes[u].chWdw--;
        }
}

void Web::push(int l_u, int u, int l_v, int v, bool is_first_epoch)
{
        int ni, pi;
        int value = std::min(layers[l_u].vertexes[u].neighbors[l_v][v].cap - layers[l_u].vertexes[u].neighbors[l_v][v].flow, layers[l_u].vertexes[u].exf);
        // cout << "DEST CAPACITY " << layers[l_v].vertexes[v].capacity << endl;
        // cout << "DEST FLOW " << layers[l_v].vertexes[v].flow << endl;
        // cout << "DEST exf " << layers[l_v].vertexes[v].exf << endl; 
        // cout << "VALUE Norm " << value << endl;
        // cout << test(l_u, u, l_v, v, value)*layers[l_u].vertexes[u].cTime << endl;
        // cout << "HMMM" << value << endl;
        // cout << layers[l_v].vertexes[v].capacity - layers[l_v].vertexes[v].flow -(test(l_u, u, l_v, v, value))*layers[l_u].vertexes[u].cTime << endl;
        if (layers[l_v].vertexes[v].type == INTERVAL && is_first_epoch) value = std::max(0,(std::min(value, layers[l_v].vertexes[v].capacity - layers[l_v].vertexes[v].flow -(test(l_u, u, l_v, v, value))*layers[l_u].vertexes[u].cTime) -layers[l_v].vertexes[v].exf));
        cout << "VALUE " << value << endl;
        if (value == 0){
            return;
        }
        cout << "FRROM " << l_u << " " << u << " TO " << l_v << " " << v << endl;
        layers[l_u].vertexes[u].neighbors[l_v][v].flow += value;
        layers[l_v].vertexes[v].neighbors[l_u][u].flow = -layers[l_u].vertexes[u].neighbors[l_v][v].flow;
        layers[l_u].vertexes[u].exf -= value;
        layers[l_v].vertexes[v].exf += value;


        //добавить раздел в вершину
        if ((layers[l_u].vertexes[u].type == JOB) && (layers[l_v].vertexes[v].type == INTERVAL))
        {
            ni = findnext(l_v, v);
            pi = findprev(l_v, v);
            checkpartadd(l_v, v, layers[l_u].vertexes[u].part, value, ni, pi);
            correctwindows(l_v, v, ni, pi);
        }


        //вершина-интервал в вершину-работу
        if ((layers[l_u].vertexes[u].type == INTERVAL) && (layers[l_v].vertexes[v].type == JOB))
        {
            ni = findnext(l_u, u);
            pi = findprev(l_u, u);
            checkpartdec(l_u, u, layers[l_v].vertexes[v].part, value);
            correctwindows(l_u, u, ni, pi);
        }

        if(v != 1 && layers[l_v].vertexes[v].exf > 0){

            P[layers[l_v].vertexes[v].part].insert(v); //добавили в список переполненных

        }
}

bool Web::discharge(int l_u, int u, bool is_first)
{       
    bool lifted = false;
    bool is_first_epoch = true;
    cout << "Start discharge : " << layers[l_u].vertexes[u].exf << endl;
    while (layers[l_u].vertexes[u].exf > 0) {
        // cout << "Start" << endl;

        // проталкивание в сток, это первое возомжное действие
        if (l_u > q) {
            cout << "PUSH to DEST" << endl;
            // это слой с интервалами
            if (layers[l_u].vertexes[u].capacity > layers[l_u].vertexes[u].flow){
                int value = layers[l_u].vertexes[u].exf;
                if (layers[l_u].vertexes[u].flow + value > layers[l_u].vertexes[u].capacity){
                    value = layers[l_u].vertexes[u].capacity - layers[l_u].vertexes[u].flow;
                } 
                layers[l_u].vertexes[u].exf -= value;
                layers[l_u].vertexes[u].flow += value;
            }
        } 

        // проход по соседям
        for(Neighbors::iterator it_layer = layers[l_u].vertexes[u].neighbors.begin(); it_layer != layers[l_u].vertexes[u].neighbors.end(); it_layer++){
            map<int, NeighborInfo > layer_neighbors = it_layer->second;
            int l_v = it_layer->first; // Номер слоя соседа
            // 0 - это дефолтный и не участвует в вычислениях
            if (l_v == 0) continue;
            for (map<int, NeighborInfo >::iterator it = layer_neighbors.begin(); it != layer_neighbors.end(); it++){
                int v = it->first; // Номер вершины соседа
                NeighborInfo vertex_info = it->second;
                
                // cout << "NEIGHBOR " << l_v << " " << v << endl; 
                // cout << layers[l_u].vertexes[u].h << " " << layers[l_v].vertexes[v].h << endl;
                // cout << layers[l_u].vertexes[u].neighbors[l_v][v].flow << " " << layers[l_u].vertexes[u].neighbors[l_v][v].cap << endl;

                //Если можно - протолкнуть
                if (layers[l_u].vertexes[u].neighbors[l_v][v].cap > layers[l_u].vertexes[u].neighbors[l_v][v].flow && layers[l_u].vertexes[u].h == layers[l_v].vertexes[v].h + 1)
                {   
                    push(l_u, u, l_v, v, is_first_epoch);
                    cout << "PUSH from " << l_u << " " << u << " to " << l_v << " " << v << endl;
                }
            }
        }

        if (l_u <= q) {
            // это слой с работами
            // если уже никуда не проталкивается// TODO n
            if (layers[l_u].vertexes[u].exf > 0 && layers[l_u].vertexes[u].h > hints_layer){
                if (hints != 0){
                    // надо что-то менять
                    //    cout << "We want hint" << endl;
                    hints--;
                    part_from_proc(layers[l_u].vertexes[u].part, QP[layers[l_u].vertexes[u].part]);
                    //  decide_proc(layers[l].vertexes[u].part);
                    //    cout << "We execute part drawback" << endl;
                    lift(l_u, u);
                    // cout << "LIFT  " << l_u << " " << u << endl;
                    lifted = true;
                    continue;
                }
                else {
                    // Убираем поток навсегда отсюда
                    int value = layers[l_u].vertexes[u].exf;
                    layers[l_u].vertexes[u].exf -= value;
                    layers[l_u].vertexes[u].flow += value;
                    // source_flow -= value;
                    cout << "PUSH to Source" << endl;
                }
            }
        }

        // первый раз только один проход по всем вершинам
        if (is_first){
            return lifted;
        }
        if (!is_first_epoch){
            lift(l_u, u);
            // cout << "LIFT  " << l_u << " " << u << endl;
            is_first_epoch = true;
            lifted = true;
        }
        else is_first_epoch = false;

    }      
    return lifted;
}

void Web::maxflow()
{
     //Инициализация

     //layers[l].vertexes[0].h = n; Это лишнее - общая переменная будет в сети
    
    // Опредляем порядок разделов изначальные предпочтения
    for(int i = 1; i <= q;i++){
        //decide_proc(partitionOrder[i],-1);
        //cout <<"Partition "<< partitionOrder[i] <<":"<<QP[partitionOrder[i]];
        // Наивный вариант нужно что умнее
        QP[i] = q + 1;
    }
     source_flow = 0;
     bool isfirsttime = true;
     bool isendwork = false;
     while (!isendwork)
     {  
        hints = nproc*nproc - 1;
        cout << "Number of hints:" << hints << endl;
        isendwork = true;
        //layers[l].vertexes[0].h = attemptCount;

        // Выставляем начальные потоки - пересмотреть
        cout << "Layer int: " << layer_int << endl;
        for(int q=1; q < layer_int; q++){
            for(int i = 0; i < layers[q].vertexes.size(); i++){
                layers[q].vertexes[i].exf = layers[q].vertexes[i].duration;
                cout << q << " " << i << " " << layers[q].vertexes[i].exf << endl;
                source_flow += layers[q].vertexes[i].exf;
                layers[q].vertexes[i].h = 1;
            }
        }

        if (isfirsttime) hard = source_flow;

        // Выставляем высоты для остальных вершин
        for(int q=layer_int; q < layer_int + nproc; q++){
            for(int i = 0; i < layers[q].vertexes.size(); i++){
                layers[q].vertexes[i].h = 0;
            }
        }

        //  for(map<int,int>::iterator it = layers[l].vertexes[0].cap.begin(); it != layers[l].vertexes[0].cap.end(); it++)
        //  {
        //     int value = layers[l].vertexes[0].cap[it->first] - layers[l].vertexes[0].flow[it->first] ;
        //     layers[l].vertexes[0].flow[it->first] += value;
        //     layers[l].vertexes[it->first].h = 1;
        //     layers[l].vertexes[it->first].flow[0] = - layers[l].vertexes[0].flow[it->first];
        //     layers[l].vertexes[it->first].exf += value;
        //     layers[l].vertexes[0].exf -= value;
        //     if (layers[l].vertexes[it->first].exf > 0) P[layers[l].vertexes[it->first].part].insert(it->first);
        //  }

        if (isfirsttime)
        {   
            // TODO - нужна переменная, которая за этим следит
            hard = source_flow;
            isfirsttime = false;
        }

        //цикл по внутренним вершинам, пока не останется переполненных вершин
        bool isend = false;
        while (!isend)
        {
            isend = true;
            cout << "New way" << endl;
            for (int i = 1; i < q + 1; i++)//по порядку разделов по порядку слоев 
            {   
                cout << "Partition now: " << i << " to " << QP[i] << endl;
                Layer& current_layer = layers[i];
                Layer& target_layer = layers[QP[i]];
                bool is_first = true;
                cout << "For is started " << current_layer.extended.size() << " " << target_layer.extended.size() << endl;
                while (!current_layer.extended.empty() || !target_layer.extended.empty())
                {   
                    // cout << "We are here" << endl;
                    if(is_first){
                        for(set<int>::iterator it1 = current_layer.extended.begin(); it1 != current_layer.extended.end(); it1++){
                            cout << "Discharge " << i << " " << *it1 << endl;
                            discharge(i, *it1, true);
                        }
                        is_first = false;
                    }

                    while(!target_layer.extended.empty()){
                        set<int>::iterator it0 = target_layer.extended.begin();
                        cout << "Discharge " << QP[i] << " " << *it0 << endl;
                        discharge(QP[i], *it0, false);
                        isend = false;
                        target_layer.extended.erase(*it0);
                    }

                    if(!current_layer.extended.empty()){
                        set<int>::iterator it = current_layer.extended.begin();
                        cout << "Discharge " << i << " " << *it << endl;
                        discharge(i, *it, false);
                        isend = false;
                        current_layer.extended.erase(*it);
                    }
                }
                cout << "For is ended " << current_layer.extended.size() << " " << target_layer.extended.size() << endl;
            }
            cout << "End for partitions " << endl;
         }
        cout << "What' new!!!" << endl;

        // Тут нужно будет удалить работу TODO
       }

}


void Web::deletework(int l_u, int u)
{   
    // проход по соседям
    for(Neighbors::iterator it_layer = layers[l_u].vertexes[u].neighbors.begin(); it_layer != layers[l_u].vertexes[u].neighbors.end(); it_layer++){
        map<int, NeighborInfo > layer_neighbors = it_layer->second;
        int l_v = it_layer->first; // Номер слоя соседа
        for (map<int, NeighborInfo >::iterator it = layer_neighbors.begin(); it != layer_neighbors.end(); it++){
            int v = it->first; // Номер вершины соседа
            NeighborInfo vertex_info = it->second;

            if (layers[l_v].vertexes[v].type == INTERVAL){
                int value = layers[l_u].vertexes[u].neighbors[l_v][v].flow;

                // layers[l].vertexes[1].exf -= value;
                // layers[l].vertexes[1].flow[v] += value;
                // layers[l].vertexes[v].flow[1] -= value;
                layers[l_u].vertexes[u].neighbors[l_v][v].flow -= value;
                layers[l_v].vertexes[v].neighbors[l_u][u].flow += value;

                int ni = findnext(l_v, v);
                int pi = findprev(l_v, v);
                checkpartdec(l_v, v, layers[l_u].vertexes[u].part, value);
                correctwindows(l_v, v, ni, pi);

                //int value1 = layers[l].vertexes[0].flow[u];
                // Переделать
                // layers[l].vertexes[0].cap[u] = 0;
                // layers[l].vertexes[0].flow[u] = 0;
                // layers[l].vertexes[u].flow[0] = 0;
                // layers[l].vertexes[0].exf += value;
                
            }
        }
    }
    // Убрать в начале работу
    source_flow -= layers[l_u].vertexes[u].capacity;
    layers[l_u].vertexes[u].flow = layers[l_u].vertexes[u].capacity;
    layers[l_u].vertexes[u].exf = 0;

    num_of_works -= 1;
}

double Web::Effectivness()
{   
    return (double)source_flow /hard;
}

int Web::test(int l_u, int u, int l_v, int v, int value)
{   
    cout << "TESTING" << endl;
    if (l_u <= q && l_v > q){
        checkpartadd(l_v, v, layers[l_u].vertexes[u].part, value, findnext(l_v, v), findprev(l_v, v));
        cout << "TESTING1" << endl;
        correctwindows(l_v, v, findnext(l_v, v),findprev(l_v, v));
        cout << "TESTING2" << endl;
        int win1 = layers[l_v].vertexes[v].chWdw;
        checkpartdec(l_v, v, layers[l_u].vertexes[u].part, value);
        correctwindows(l_v, v, findnext(l_v, v),findprev(l_v, v));
        int win2 = layers[l_v].vertexes[v].chWdw;
        return win1-win2;
    }
    else return 0;

}

void Web::checkpartadd(int l_v, int v, int part, int value, int ni, int pi)
{   
    cout << "IN CHECH ADD " << endl;
    int numpart1 = layers[l_v].vertexes[v].setpart.size();
    layers[l_v].vertexes[v].partIn[part]+=value;
    if (layers[l_v].vertexes[v].partIn[part] != 0) layers[l_v].vertexes[v].setpart.insert(part);
    int numpart2 = layers[l_v].vertexes[v].setpart.size();

    if (numpart1 != numpart2)
    {//добавился раздел

       // назначить первый и последний раздел в интервале
       if (numpart2 == 1)
       {//появился первый раздел
        layers[l_v].vertexes[v].firstPart = part;
        layers[l_v].vertexes[v].lastPart = part;
       }
       else if (numpart2 == 2)
       {//появился второй раздел
        if (ni != -1 && layers[l_v].vertexes[ni].firstPart != 0 && part == layers[l_v].vertexes[ni].firstPart) layers[l_v].vertexes[v].lastPart = part;
        else layers[l_v].vertexes[v].firstPart = part;
       }
       else if (numpart2 > 2)
       {
        if (ni != -1 && layers[l_v].vertexes[ni].firstPart != 0 && part == layers[l_v].vertexes[ni].firstPart) layers[l_v].vertexes[v].lastPart = part;
        else if (pi != -1 && layers[l_v].vertexes[pi].firstPart != 0 && part == layers[l_v].vertexes[pi].lastPart) layers[l_v].vertexes[v].firstPart = part;
       }
    }
}

void Web::checkpartdec(int l_v, int v, int part, int value)
{   
    // cout << "checkpartdec" << v <<endl;
    int numpart1 = layers[l_v].vertexes[v].setpart.size();
    layers[l_v].vertexes[v].partIn[part]-=value;
    if (layers[l_v].vertexes[v].partIn[part] == 0) layers[l_v].vertexes[v].setpart.erase(part);//убрали совсем
    int numpart2 = layers[l_v].vertexes[v].setpart.size();
    // cout << "checkpartdec 1" << endl;  
    if (numpart1 != numpart2)
    {
       if (numpart2 == 0)
       {//все разделы убрались
        layers[l_v].vertexes[v].firstPart = 0;
        layers[l_v].vertexes[v].lastPart = 0;
       }
       else if (numpart2 == 1)
       {//остался один раздел
        int part1 = layers[l_v].vertexes[v].lastPart;//оставшийся раздел
        if (part1 == part) part1 = layers[l_v].vertexes[v].firstPart;
        layers[l_v].vertexes[v].firstPart = part1;
        layers[l_v].vertexes[v].lastPart = part1;
       }
       else if (numpart2 > 1)
       {
         
        if (part == layers[l_v].vertexes[v].lastPart) layers[l_v].vertexes[v].lastPart = finsetneq(layers[l_v].vertexes[v].setpart, layers[l_v].vertexes[v].firstPart);
        else if (part == layers[l_v].vertexes[v].firstPart) layers[l_v].vertexes[v].firstPart = finsetneq(layers[l_v].vertexes[v].setpart, layers[l_v].vertexes[v].lastPart);
       }
    }
}

void Web::correctwindows(int l_v, int v, int ni, int pi)
{
    int numofpart = layers[l_v].vertexes[v].setpart.size();//число разделов в интервале
    int chwdwinter = layers[l_v].vertexes[v].chWdw; //число внутренних переключений
    if (layers[l_v].vertexes[v].isLWin) chwdwinter--;
    if (layers[l_v].vertexes[v].isRWin) chwdwinter--;

    if (numofpart != 0)
    {
        //корректировка числа внутренних переключений
        while (chwdwinter != numofpart -1)
        {
            if (chwdwinter > numofpart -1)
            {
                clwindow(l_v, v);
                chwdwinter--;
            }
            else
            {
                window(l_v, v);
                chwdwinter++;
            }
        }

        //корректировка правого переключения
        if (ni != -1)
            {
         //условие открытия
         if (layers[l_v].vertexes[ni].firstPart != layers[l_v].vertexes[v].lastPart && !layers[l_v].vertexes[ni].isLWin && !layers[l_v].vertexes[v].isRWin)
         {
            if( layers[l_v].vertexes[ni].capacity - layers[l_v].vertexes[ni].flow >= layers[l_v].vertexes[v].cTime && layers[l_v].vertexes[v].exf + layers[l_v].vertexes[v].flow > layers[l_v].vertexes[v].capacity - layers[l_v].vertexes[v].cTime){
                window(l_v, ni);
                layers[l_v].vertexes[ni].isLWin = true;
            }
            else {
                window(l_v, v);
                layers[l_v].vertexes[v].isRWin = true;
            }
         }
         //услвие закрытия
         if (layers[l_v].vertexes[ni].firstPart == layers[l_v].vertexes[v].lastPart)
         {
            if (layers[l_v].vertexes[ni].isLWin)
            {
             clwindow(l_v, ni);
             layers[l_v].vertexes[ni].isLWin = false;
            }
            if (layers[l_v].vertexes[v].isRWin)
            {
             clwindow(l_v, v);
             layers[l_v].vertexes[v].isRWin = false;
            }
         }
        }
        else
        {
            if (layers[l_v].vertexes[v].isRWin)
            {
             clwindow(l_v, v);
             layers[l_v].vertexes[v].isRWin = false;
            }
        }


        //корректировка левого переключения
        if (pi != -1)
        {
         //условие открытия
         if (layers[l_v].vertexes[pi].lastPart != layers[l_v].vertexes[v].firstPart && !layers[l_v].vertexes[pi].isRWin && !layers[l_v].vertexes[v].isLWin)
         {
             if( layers[l_v].vertexes[pi].capacity - layers[l_v].vertexes[pi].flow >= layers[l_v].vertexes[v].cTime && layers[l_v].vertexes[v].exf + layers[l_v].vertexes[v].flow > layers[l_v].vertexes[v].capacity - layers[l_v].vertexes[v].cTime){
             //if( layers[l_v].vertexes[pi].flow[dest] - layers[l_v].vertexes[pi].cap[dest] >= cw){
                 window(l_v, pi);
                 layers[l_v].vertexes[pi].isRWin = true;
             }
             else{
                window(l_v, v);
                layers[l_v].vertexes[v].isLWin = true;
             }
         }
         //условие закрытия
         if (layers[l_v].vertexes[pi].lastPart == layers[l_v].vertexes[v].firstPart)
         {
            if (layers[l_v].vertexes[pi].isRWin)
            {
             clwindow(l_v, pi);
             layers[l_v].vertexes[pi].isRWin = false;
            }
            if (layers[l_v].vertexes[v].isLWin)
            {
             clwindow(l_v, v);
             layers[l_v].vertexes[v].isLWin = false;
            }
         }
        }
        else
        {
            if (layers[l_v].vertexes[v].isLWin)
            {
             clwindow(l_v, v);
             layers[l_v].vertexes[v].isLWin = false;
            }
        }
    }
    else
    {
        //корректировка внутренних
        while (chwdwinter != 0)
        {
            clwindow(l_v, v);
            chwdwinter--;
        }

        //корректировка краевых
        if (ni == -1 || pi == -1)
        {
            if (layers[l_v].vertexes[v].isLWin)
            {
                clwindow(l_v, v);
                layers[l_v].vertexes[v].isLWin = false;
            }
            if (layers[l_v].vertexes[v].isRWin)
            {
                clwindow(l_v, v);
                layers[l_v].vertexes[v].isRWin = false;
            }
            if (ni != -1 && layers[l_v].vertexes[ni].isLWin)
            {
                clwindow(l_v, ni);
                layers[l_v].vertexes[ni].isLWin = false;
            }
            if (pi != -1 && layers[l_v].vertexes[pi].isRWin)
            {
                clwindow(l_v, pi);
                layers[l_v].vertexes[pi].isRWin = false;
            }
        }
        else
        {
            //убрать переключения в интервале
            if (layers[l_v].vertexes[v].isLWin)
            {
                clwindow(l_v, v);
                layers[l_v].vertexes[v].isLWin = false;
            }
            if (layers[l_v].vertexes[v].isRWin)
            {
                clwindow(l_v, v);
                layers[l_v].vertexes[v].isRWin = false;
            }

            //обработать переключение между крайними
            //условие открытия
            if (layers[l_v].vertexes[ni].firstPart != layers[l_v].vertexes[pi].lastPart && !layers[l_v].vertexes[ni].isLWin && !layers[l_v].vertexes[pi].isRWin)
            {
               window(l_v, pi);
               layers[l_v].vertexes[pi].isRWin = true;
            }
            //услвие закрытия
            if (layers[l_v].vertexes[ni].firstPart == layers[l_v].vertexes[pi].lastPart)
            {
               if (layers[l_v].vertexes[ni].isLWin)
               {
                clwindow(l_v, ni);
                layers[l_v].vertexes[ni].isLWin = false;
               }
               if (layers[l_v].vertexes[pi].isRWin)
               {
                clwindow(l_v, pi);
                layers[l_v].vertexes[pi].isRWin = false;
               }
            }
        }

    }

}

int Web::findnext(int l_v, int v)
{
    int ni = v + 1;
    while (ni != layers[l_v].vertexes.size() && layers[l_v].vertexes[ni].firstPart == 0)
    {
        ni = ni + 1;
    }
    if (ni == layers[l_v].vertexes.size()) ni = -1;
    return ni;
}

int Web::findprev(int l_v, int v)
{
    int pi = v - 1;
    while (pi != -1 && layers[l_v].vertexes[pi].lastPart == 0)
    {
        pi = pi - 1;
    }
    return pi;
}

int Web::sheduledjobs()
{   
    return num_of_works;
}





// void Web::part_to_proc(int q, int proc){
//     // Раздел прикпрепляется к процессору, на других он выполнятся не может - пропускные способности в остальные равны 0
//     QP[q] = proc;
//     for(set<int>::iterator pit = (vPart[q]).begin(); pit != (vPart[q]).end(); pit++){
//         for (map<int,int>::iterator it = layers[l].vertexes[*pit].cap.begin(); it != layers[l].vertexes[*pit].cap.end(); it++){
//             if (layers[l].vertexes[it->first].proc != proc){
//                 layers[l].vertexes[*pit].cap[it->first] = 0;
//             }
//         }
//     }
// }

void Web::part_from_proc(int q, int proc){
    // Rewrite
    // //QP[q] = -1;
    // processorLoad[proc] += partitionComplexity[q];
    // // убрать поток
    // // cout << "we begin" << endl;
    // // открыть новые пути
    // for(set<int>::iterator pit = (vPart[q]).begin(); pit != (vPart[q]).end(); pit++){
    //     int v = *pit;
    //     // cout << "we begin:" << v <<endl;
    //     for (map<int,int>::iterator it = layers[l].vertexes[v].cap.begin(); it != layers[l].vertexes[v].cap.end(); it++){
    //         // cout << "we continue:" << it->first <<endl;
    //         int u = it->first;
    //         if (layers[l].vertexes[u].proc == proc){

    //             int value = layers[l].vertexes[v].flow[u];

    //             layers[l].vertexes[u].exf -= value;
    //             layers[l].vertexes[v].exf += value;
    //             layers[l].vertexes[v].h = 1;
    //             layers[l].vertexes[u].h = 1;
    //             if (v != 1 && layers[l].vertexes[v].exf > 0) P[layers[l].vertexes[v].part].insert(v);

    //             // layers[l].vertexes[1].flow[v] += value;
    //             // layers[l].vertexes[v].flow[1] -= value;
    //             layers[l].vertexes[v].flow[u] -= value;
    //             layers[l].vertexes[u].flow[v] += value;

    //             int ni = findnext(u);
    //             int pi = findprev(u);
    //             // cout << "we there 1:" << it->first <<endl;
    //             checkpartdec(u, layers[l].vertexes[v].part, value);
    //             // cout << "we there 2:" << it->first <<endl;
    //             correctwindows(u, ni, pi);
    //             // cout << "we there 3:" << it->first <<endl;
    //             layers[l].vertexes[v].cap[u] = 0;
    //         }
    //         else if (layers[l].vertexes[u].type == INTERVAL)
    //         {
    //             layers[l].vertexes[v].cap[u] = layers[l].vertexes[u].duration;
    //         }
    //     }
    // }

    // // Назначить новый процессор
    // decide_proc(q,proc);
    // cout << "New Proc" << q <<endl;
}

void Web::decide_proc(int part, int prohibit_proc){
    // Rewrite
    // int idxProc = 0;
    // int freeSpace = 0;
    // int complexity = partitionComplexity[part];

    // for(int i=0; i < nproc;i++){
    //     bool isFunctionality = true;
    //     set<string> result;
    //     std::set_intersection(processors[i]->functionality.begin(),processors[i]->functionality.end(),
    //      partitionFunctionality[part].begin(), partitionFunctionality[part].end(),
    //     std::inserter(result, result.begin()));
    //     if (result != partitionFunctionality[part]){
    //         isFunctionality = false;
    //         //cout << "false part" << part << ": proc " << i << endl;
    //     }
    //     if (processorLoad[i] > freeSpace && i != prohibit_proc && isFunctionality) {
    //         idxProc = i;
    //         freeSpace = processorLoad[i]; 
    //     }
    //     //cout << "proc:" << idxProc << endl;
    // }

    // cout << "finel proc:" << idxProc;
    // processorLoad[idxProc] -= complexity;
    // QP[part] = idxProc;

    // // запретить другие пути
    // for(set<int>::iterator pit = (vPart[part]).begin(); pit != (vPart[part]).end(); pit++){
    //     for (map<int,int>::iterator it = layers[l].vertexes[*pit].cap.begin(); it != layers[l].vertexes[*pit].cap.end(); it++){
    //         if (layers[l].vertexes[it->first].proc != idxProc){
    //             layers[l].vertexes[*pit].cap[it->first] = 0;
    //         }
    //     }
    // }
}

void Web::add_proc_layer(int iproc){
    // Добавляем по дефолтному слою
    int l_j = free_layer;
    free_layer++;
    auto options = processors[iproc]->functionality;
    int proc_performance = processors[iproc]->performance;
    cout << "PERFORMANCE " << proc_performance << endl;
    // Копирование слоя
    layers[l_j] = layers[0];
    layers[l_j].ptype = iproc;
    cout << layers[l_j].vertexes.size() << endl;
    cout << layers[l_j].vertexes[0].neighbors[1][0].cap << endl;
    cout << layers[l_j].vertexes[0].neighbors[1][0].cap << endl;
    // Изменяем параметры
    for (int l_i=1; l_i < layer_int ; l_i++){
        bool isFunctionality = true;
        set<string> result;
        std::set_intersection(layers[l_i].vertexes[0].options.begin(), layers[l_i].vertexes[0].options.end(),
            options.begin(), options.end(),
            std::inserter(result, result.begin()));
        if (result != layers[l_i].vertexes[0].options){
            isFunctionality = false;
        }
        if (isFunctionality) {
            for(int i = 0; i < layers[l_i].vertexes.size(); i++){
                for (map<int, NeighborInfo >::iterator it = layers[l_i].vertexes[i].neighbors[0].begin(); it !=  layers[l_i].vertexes[i].neighbors[0].end(); it++){
                    int j = it->first;;
                    cout << "ADD " << l_i << " " << i << " " << l_j << " " << j << endl;
                    layers[l_i].vertexes[i].neighbors[l_j][j].cap = layers[l_i].vertexes[i].neighbors[0][j].cap * proc_performance;
                    layers[l_j].vertexes[j].capacity = layers[l_i].vertexes[i].neighbors[l_j][j].cap;
                    cout << layers[l_i].vertexes[i].neighbors[l_j][j].cap << endl;
                    // cout << web.layers[l_j].vertexes[j].capacity << endl;
                    // cout << web.layers[0].vertexes[j].capacity << endl;
                }
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
