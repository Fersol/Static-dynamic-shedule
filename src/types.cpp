#include "types.h"



Web::Web() {
    cw = 0;
    n = 0;
    q = 0;
}

int Web::finsetneq(SInt& set, int a) {
     for (SInt::iterator it = set.begin(); it != set.end(); it++){
        if (*it != a) return *it;
     }
     return 0;
}

void Web::lift(int l_u, int u) {
    cout << "start lift " << l_u << " " << u << " " << "\n";
    int height = -1;
    // Проход по соседям, минимальная вершина среди тех, куда возможно проталкивание
    for(Neighbors::iterator it_layer = layers[l_u].vertexes[u].neighbors.begin(); it_layer != layers[l_u].vertexes[u].neighbors.end(); it_layer++){
        map<int, NeighborInfo > layer_neighbors = it_layer->second;
        // Номер слоя соседа
        int l_v = it_layer->first;
        for (map<int, NeighborInfo >::iterator it = layer_neighbors.begin(); it != layer_neighbors.end(); it++){
            // Номер вершины соседа
            int v = it->first;
            NeighborInfo vertex = it->second;
            cout << l_u << " " << u << " to " << l_v << " " << v << "\n";
            cout << layers[l_u].vertexes[u].neighbors[l_v][v].cap << " " << layers[l_u].vertexes[u].neighbors[l_v][v].flow << "\n";
            if (layers[l_u].vertexes[u].neighbors[l_v][v].cap > layers[l_u].vertexes[u].neighbors[l_v][v].flow){
                cout <<  layers[l_v].vertexes[v].h << "\n";
                if (height == -1){
                    height = layers[l_v].vertexes[v].h;
                } else {
                    height = min(height, layers[l_v].vertexes[v].h);
                }
            }
        }
    }
    if (height == -1){
        height = hints_layer;
    }
    layers[l_u].vertexes[u].h = height + 1;
    cout << " Result is to " << layers[l_u].vertexes[u].h << "\n";
}

void Web::window(int l_u, int u) {   
    // Ограничивает пропускную способность в
    if (layers[l_u].vertexes[u].type == INTERVAL){
        layers[l_u].vertexes[u].capacity -= layers[l_u].vertexes[u].cTime;
        if (layers[l_u].vertexes[u].flow > layers[l_u].vertexes[u].capacity){
            long long value = layers[l_u].vertexes[u].capacity - layers[l_u].vertexes[u].flow;
            layers[l_u].vertexes[u].flow += value;
            layers[l_u].vertexes[u].exf -= value;

            if (layers[l_u].vertexes[u].exf > 0) P[layers[l_u].vertexes[u].part].insert(u);
        }
        layers[l_u].vertexes[u].chWdw++;
    }
}

void Web::clwindow(int l_u, int u) {
    if (layers[l_u].vertexes[u].type == INTERVAL){
        layers[l_u].vertexes[u].capacity += layers[l_u].vertexes[u].cTime;
        if (layers[l_u].vertexes[u].exf >= layers[l_u].vertexes[u].cTime){
            long long value = layers[l_u].vertexes[u].cTime;
            layers[l_u].vertexes[u].flow += value;
            layers[l_u].vertexes[u].exf -= value;
        }
        if (layers[l_u].vertexes[u].exf == 0){
            P[layers[l_u].vertexes[u].part].erase(u);
        }
        layers[l_u].vertexes[u].chWdw--;
    }
}

void Web::push(int l_u, int u, int l_v, int v, bool is_first_epoch) {
    int ni, pi;
    long long value = std::min(layers[l_u].vertexes[u].neighbors[l_v][v].cap - layers[l_u].vertexes[u].neighbors[l_v][v].flow, layers[l_u].vertexes[u].exf);
    if (layers[l_v].vertexes[v].type == INTERVAL && is_first_epoch) value = std::max((long long)0,(std::min(value, layers[l_v].vertexes[v].capacity - layers[l_v].vertexes[v].flow -(test(l_u, u, l_v, v, value))*layers[l_u].vertexes[u].cTime) -layers[l_v].vertexes[v].exf));
    if (value == 0){
        return;
    }
    
    layers[l_u].vertexes[u].neighbors[l_v][v].flow += value;
    layers[l_v].vertexes[v].neighbors[l_u][u].flow = -layers[l_u].vertexes[u].neighbors[l_v][v].flow;
    layers[l_u].vertexes[u].exf -= value;
    layers[l_v].vertexes[v].exf += value;

    // Добавить раздел в вершину
    if ((layers[l_u].vertexes[u].type == JOB) && (layers[l_v].vertexes[v].type == INTERVAL)) {
        ni = findnext(l_v, v);
        pi = findprev(l_v, v);
        checkpartadd(l_v, v, layers[l_u].vertexes[u].part, value, ni, pi);
        correctwindows(l_v, v, ni, pi);
    }

    // Вершина-интервал в вершину-работу
    if ((layers[l_u].vertexes[u].type == INTERVAL) && (layers[l_v].vertexes[v].type == JOB)) {
        ni = findnext(l_u, u);
        pi = findprev(l_u, u);
        checkpartdec(l_u, u, layers[l_v].vertexes[v].part, value);
        correctwindows(l_u, u, ni, pi);
    }

    if (layers[l_v].vertexes[v].exf > 0) {
        layers[l_v].extended.push_back(v); // добавили в список переполненных
    }
}

bool Web::discharge(int l_u, int u, bool is_first) {       
    bool lifted = false;
    bool is_first_epoch = true;
    while (layers[l_u].vertexes[u].exf > 0) {
        
        // Проталкивание в сток, это первое возомжное действие
        if (l_u > q && is_first_epoch) {      
            if (layers[l_u].vertexes[u].capacity > layers[l_u].vertexes[u].flow) { 
                long long value = layers[l_u].vertexes[u].exf;
                if (layers[l_u].vertexes[u].flow + value > layers[l_u].vertexes[u].capacity) {
                    value = layers[l_u].vertexes[u].capacity - layers[l_u].vertexes[u].flow;
                } 
                layers[l_u].vertexes[u].exf -= value;
                layers[l_u].vertexes[u].flow += value;
            }
        } 

        // Проход по соседям
        for(Neighbors::iterator it_layer = layers[l_u].vertexes[u].neighbors.begin(); it_layer != layers[l_u].vertexes[u].neighbors.end(); it_layer++){
            map<int, NeighborInfo > layer_neighbors = it_layer->second;
            int l_v = it_layer->first; // номер слоя соседа
            // 0 - это дефолтный и не участвует в вычислениях
            if (l_v == 0) continue;
            for (map<int, NeighborInfo >::iterator it = layer_neighbors.begin(); it != layer_neighbors.end(); it++){
                int v = it->first; // номер вершины соседа
                NeighborInfo vertex_info = it->second;
                // Если можно - протолкнуть
                if (layers[l_u].vertexes[u].neighbors[l_v][v].cap > layers[l_u].vertexes[u].neighbors[l_v][v].flow && layers[l_u].vertexes[u].h == layers[l_v].vertexes[v].h + 1) { 
                    cout << "TRY PUSH AGAIN " << layers[l_u].vertexes[u].exf << endl;  
                    push(l_u, u, l_v, v, is_first_epoch);
                    if (layers[l_u].vertexes[u].exf == 0){
                        return lifted;
                    }
                }
            }
        }

        if (l_u <= q) {
            cout << "TRY AGAIN" << endl;
            // Это слой с работами
            // Если уже никуда не проталкивается// TODO n
            if (layers[l_u].vertexes[u].exf > 0 && layers[l_u].vertexes[u].h > hints_layer) {
                if (hints != 0) {
                    cout << "Critical h:" << layers[l_u].vertexes[u].h << "/" << hints_layer << endl;
                    hints--;
                    part_from_proc(layers[l_u].vertexes[u].part, QP[layers[l_u].vertexes[u].part]);
                    lift(l_u, u);
                    cout << "THERE 2" << endl;
                    lifted = true;
                }
                else {
                    // Убираем поток навсегда отсюда
                    long long value = layers[l_u].vertexes[u].exf;
                    layers[l_u].vertexes[u].exf -= value;
                    layers[l_u].vertexes[u].flow += value;
                    cout << "PUSH to Source " << l_u << " " << u << " " << layers[l_u].vertexes[u].flow << endl;
                }
            }
            cout << "im here" << endl;
        }

        // Первый раз только один проход по всем вершинам
        if (is_first) {
            return lifted;
        }
        if (!is_first_epoch) {
            lift(l_u, u);
            cout << "THERE 1" << endl;
            is_first_epoch = false;
            lifted = true;
        }
        else is_first_epoch = false;
    }
    return lifted;
}

bool Web::maxflow(bool check, set<int> parts) {
     //Инициализация

     //layers[l].vertexes[0].h = n; Это лишнее - общая переменная будет в сети
    // Определяем сложности разделов и процессоров
     bool full_location = true; // для определения все ли разместили
     if (parts.size() == 0) {
         // Добавляем все
         for(int i=1; i<=q; i++) {
             parts.insert(i);
         }
     }
     source_flow = 0;
     bool isfirsttime = true;
     bool isendwork = false;
     while (!isendwork) {  
        hints = nproc*nproc - 1;
        cout << "Number of hints:" << hints << endl;
        isendwork = true;
        //layers[l].vertexes[0].h = attemptCount;

        // Выставляем начальные потоки 
        cout << "Layer int: " << layer_int << endl;
        for(int q=1; q < layer_int; q++) {
            for(int i = 0; i < layers[q].vertexes.size(); i++) {
                if (isfirsttime) {
                    layers[q].vertexes[i].exf = layers[q].vertexes[i].capacity;
                } else {
                    layers[q].vertexes[i].exf = layers[q].vertexes[i].flow;
                }      
                layers[q].vertexes[i].flow = 0;
                if (layers[q].vertexes[i].exf > 0) { 
                    layers[q].extended.push_back(i);
                }
                cout << q << " " << i << " " << layers[q].vertexes[i].exf << endl;
                if (isfirsttime) {
                    source_flow += layers[q].vertexes[i].exf;
                }
                // Высоты для вершин-программ изначально равны 1
                layers[q].vertexes[i].h = 1;
            }
        }

        // Выставляем высоты для остальных вершин
        for(int q=layer_int; q < layer_int + nproc; q++) {
            for(int i = 0; i < layers[q].vertexes.size(); i++) {
                layers[q].vertexes[i].h = 0;
            }
        }

        if (isfirsttime) {   
            hard = source_flow;
            isfirsttime = false;
        }

        // Цикл по внутренним вершинам, пока не останется переполненных вершин
        bool isend = false;
        while (!isend) {
            isend = true;
            for(auto i: parts) {   
                bool is_first = true;
                while (!layers[i].extended.empty() || !layers[QP[i]].extended.empty()) {   
                    while(!layers[QP[i]].extended.empty()) {
                        auto it0 = layers[QP[i]].extended[0];
                        discharge(QP[i], it0, false);
                        isend = false;
                        if(!layers[QP[i]].extended.empty()) {
                            layers[QP[i]].extended.erase(layers[QP[i]].extended.begin());
                        }
                    }
                    if(!layers[i].extended.empty()) {
                        auto it = layers[i].extended[0];
                        discharge(i, it, false);
                        isend = false;
                        if(!layers[i].extended.empty()){
                            layers[i].extended.erase(layers[i].extended.begin());
                        }
                    }
                }
            }
         }

        // Тут будет удалена
        int max_not_dist_flow = 0;
        int l_del = -1;
        int v_del = -1;

        // Проходимся по слоям с программами и заполняем недостачу по каждому заданию
        map< int, int > surplus;
        for(int l=1; l <= q; l++){
            for(int v=0; v < layers[l].vertexes.size(); v++) {
                auto flow = layers[l].vertexes[v].flow;
                if (flow>0) {
                    auto task = layers[l].vertexes[v].numTask;
                    if(surplus.find(task)== surplus.end()) {
                        surplus[task] = flow;
                    } else {
                        surplus[task] += flow;
                    }
                }
            }
        }

        // Удаляем программу плохую
        if (surplus.size() != 0){
            // Если нужно выяснить только возможность, то тут все плохо
            if (check) {
                return false;
            }
            full_location = false;
            isendwork = false;
            auto max = max_element(surplus.begin(), surplus.end(), [](const auto& l, const auto& r) { return l.second < r.second; });
            auto task_to_del = max->first;
            for(int l=1; l <= q; l++){
                for(int v=0; v < layers[l].vertexes.size(); v++) {
                    if (layers[l].vertexes[v].numTask == task_to_del) {
                        deletework(l, v);
                    }     
                }
            }
        }
    }
    return full_location;
}


void Web::deletework(int l_u, int u) {   
    // Проход по соседям
    for(Neighbors::iterator it_layer = layers[l_u].vertexes[u].neighbors.begin(); it_layer != layers[l_u].vertexes[u].neighbors.end(); it_layer++) {
        map<int, NeighborInfo > layer_neighbors = it_layer->second;
        int l_v = it_layer->first; // номер слоя соседа
        if (l_v == 0) continue;
        for (map<int, NeighborInfo >::iterator it = layer_neighbors.begin(); it != layer_neighbors.end(); it++){
            int v = it->first; // номер вершины соседа
            NeighborInfo vertex_info = it->second;

            if (layers[l_v].vertexes[v].type == INTERVAL) {
                long long value = layers[l_u].vertexes[u].neighbors[l_v][v].flow;

                layers[l_u].vertexes[u].neighbors[l_v][v].flow -= value;
                layers[l_v].vertexes[v].neighbors[l_u][u].flow += value;

                long long end_value = max((long long)0, value - layers[l_v].vertexes[v].exf);
                long long new_value = value - end_value;
                layers[l_v].vertexes[v].flow -= end_value;
                layers[l_v].vertexes[v].exf -= new_value;
                
                int ni = findnext(l_v, v);
                int pi = findprev(l_v, v);
                checkpartdec(l_v, v, layers[l_u].vertexes[u].part, value);
                correctwindows(l_v, v, ni, pi);
            }
        }
    }
    // Нужно освобождать нагрузку
    layers[QP[l_u]].load += layers[l_u].vertexes[u].capacity;
    layers[l_u].complexity -= layers[l_u].vertexes[u].capacity;

    // Убрать в начале работу
    source_flow -= layers[l_u].vertexes[u].capacity;
    layers[l_u].vertexes[u].flow = 0;
    layers[l_u].vertexes[u].capacity = 0;
    layers[l_u].vertexes[u].exf = 0;

    num_of_works -= 1;
}

double Web::Effectivness() {   
    return (double)source_flow /hard;
}

long long Web::test(int l_u, int u, int l_v, int v, long long value) {   
    if (l_u <= q && l_v > q){
        checkpartadd(l_v, v, layers[l_u].vertexes[u].part, value, findnext(l_v, v), findprev(l_v, v));
        correctwindows(l_v, v, findnext(l_v, v),findprev(l_v, v));
        int win1 = layers[l_v].vertexes[v].chWdw;
        checkpartdec(l_v, v, layers[l_u].vertexes[u].part, value);
        correctwindows(l_v, v, findnext(l_v, v),findprev(l_v, v));
        int win2 = layers[l_v].vertexes[v].chWdw;
        return win1-win2;
    }
    else return 0;

}

void Web::checkpartadd(int l_v, int v, int part, long long value, int ni, int pi) {   
    int numpart1 = layers[l_v].vertexes[v].setpart.size();
    layers[l_v].vertexes[v].partIn[part]+=value;
    if (layers[l_v].vertexes[v].partIn[part] != 0) layers[l_v].vertexes[v].setpart.insert(part);
    int numpart2 = layers[l_v].vertexes[v].setpart.size();
    
    if (numpart1 != numpart2) {
        // Добавился раздел

        // Назначить первый и последний раздел в интервале
        if (numpart2 == 1) {
            // Появился первый раздел
            layers[l_v].vertexes[v].firstPart = part;
            layers[l_v].vertexes[v].lastPart = part;
        }
        else if (numpart2 == 2) {
            // Появился второй раздел
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

void Web::checkpartdec(int l_v, int v, int part, long long value) {   
    int numpart1 = layers[l_v].vertexes[v].setpart.size();
    layers[l_v].vertexes[v].partIn[part]-=value;
    if (layers[l_v].vertexes[v].partIn[part] == 0) layers[l_v].vertexes[v].setpart.erase(part);//убрали совсем
    int numpart2 = layers[l_v].vertexes[v].setpart.size(); 
    if (numpart1 != numpart2) {
        if (numpart2 == 0) {
            // Все разделы убрались
            layers[l_v].vertexes[v].firstPart = 0;
            layers[l_v].vertexes[v].lastPart = 0;
        } else if (numpart2 == 1) {
            //  Остался один раздел
            int part1 = layers[l_v].vertexes[v].lastPart;  // оставшийся раздел
            if (part1 == part) part1 = layers[l_v].vertexes[v].firstPart;
            layers[l_v].vertexes[v].firstPart = part1;
            layers[l_v].vertexes[v].lastPart = part1;
        } else if (numpart2 > 1) {
            if (part == layers[l_v].vertexes[v].lastPart) layers[l_v].vertexes[v].lastPart = finsetneq(layers[l_v].vertexes[v].setpart, layers[l_v].vertexes[v].firstPart);
            else if (part == layers[l_v].vertexes[v].firstPart) layers[l_v].vertexes[v].firstPart = finsetneq(layers[l_v].vertexes[v].setpart, layers[l_v].vertexes[v].lastPart);
        }
    }
}

void Web::correctwindows(int l_v, int v, int ni, int pi) {   
    int numofpart = layers[l_v].vertexes[v].setpart.size();  // число разделов в интервале
    int chwdwinter = layers[l_v].vertexes[v].chWdw;          // число внутренних переключений
    if (layers[l_v].vertexes[v].isLWin) chwdwinter--;
    if (layers[l_v].vertexes[v].isRWin) chwdwinter--;

    if (numofpart != 0) {
        // Корректировка числа внутренних переключений
        while (chwdwinter != numofpart -1) {
            if (chwdwinter > numofpart -1) {
                clwindow(l_v, v);
                chwdwinter--;
            } else {
                window(l_v, v);
                chwdwinter++;
            }
        }

        // Корректировка правого переключения
        if (ni != -1) {
            // Условие открытия
            if (layers[l_v].vertexes[ni].firstPart != layers[l_v].vertexes[v].lastPart && !layers[l_v].vertexes[ni].isLWin && !layers[l_v].vertexes[v].isRWin) {
                if( layers[l_v].vertexes[ni].capacity - layers[l_v].vertexes[ni].flow >= layers[l_v].vertexes[v].cTime && layers[l_v].vertexes[v].exf + layers[l_v].vertexes[v].flow > layers[l_v].vertexes[v].capacity - layers[l_v].vertexes[v].cTime) {
                    window(l_v, ni);
                    layers[l_v].vertexes[ni].isLWin = true;
                } else {
                    window(l_v, v);
                    layers[l_v].vertexes[v].isRWin = true;
                }
            }

            // Условие закрытия
            if (layers[l_v].vertexes[ni].firstPart == layers[l_v].vertexes[v].lastPart) {
                if (layers[l_v].vertexes[ni].isLWin) {
                    clwindow(l_v, ni);
                    layers[l_v].vertexes[ni].isLWin = false;
                }
                if (layers[l_v].vertexes[v].isRWin) {
                    clwindow(l_v, v);
                    layers[l_v].vertexes[v].isRWin = false;
                }
            }
        } else {
            if (layers[l_v].vertexes[v].isRWin) {
                clwindow(l_v, v);
                layers[l_v].vertexes[v].isRWin = false;
            }
        }

        // Корректировка левого переключения
        if (pi != -1) {
            // Условие открытия
            if (layers[l_v].vertexes[pi].lastPart != layers[l_v].vertexes[v].firstPart && !layers[l_v].vertexes[pi].isRWin && !layers[l_v].vertexes[v].isLWin) {
                if( layers[l_v].vertexes[pi].capacity - layers[l_v].vertexes[pi].flow >= layers[l_v].vertexes[v].cTime && layers[l_v].vertexes[v].exf + layers[l_v].vertexes[v].flow > layers[l_v].vertexes[v].capacity - layers[l_v].vertexes[v].cTime) {
                    window(l_v, pi);
                    layers[l_v].vertexes[pi].isRWin = true;
                } else {
                    window(l_v, v);
                    layers[l_v].vertexes[v].isLWin = true;
                }
            }
            // Условие закрытия
            if (layers[l_v].vertexes[pi].lastPart == layers[l_v].vertexes[v].firstPart) {
                    if (layers[l_v].vertexes[pi].isRWin) {
                        clwindow(l_v, pi);
                        layers[l_v].vertexes[pi].isRWin = false;
                    }
                    if (layers[l_v].vertexes[v].isLWin) {
                        clwindow(l_v, v);
                        layers[l_v].vertexes[v].isLWin = false;
                    }
            }
        } else {
            if (layers[l_v].vertexes[v].isLWin) {
                clwindow(l_v, v);
                layers[l_v].vertexes[v].isLWin = false;
            }
        }
    } else {
        // Корректировка внутренних
        while (chwdwinter != 0) {
            clwindow(l_v, v);
            chwdwinter--;
        }

        // Корректировка краевых
        if (ni == -1 || pi == -1) {
            if (layers[l_v].vertexes[v].isLWin) {
                clwindow(l_v, v);
                layers[l_v].vertexes[v].isLWin = false;
            }
            if (layers[l_v].vertexes[v].isRWin) {
                clwindow(l_v, v);
                layers[l_v].vertexes[v].isRWin = false;
            }
            if (ni != -1 && layers[l_v].vertexes[ni].isLWin) {
                clwindow(l_v, ni);
                layers[l_v].vertexes[ni].isLWin = false;
            }
            if (pi != -1 && layers[l_v].vertexes[pi].isRWin) {
                clwindow(l_v, pi);
                layers[l_v].vertexes[pi].isRWin = false;
            }
        } else {
            // Убрать переключения в интервале
            if (layers[l_v].vertexes[v].isLWin) {
                clwindow(l_v, v);
                layers[l_v].vertexes[v].isLWin = false;
            }
            if (layers[l_v].vertexes[v].isRWin) {
                clwindow(l_v, v);
                layers[l_v].vertexes[v].isRWin = false;
            }

            // Обработать переключение между крайними
            // Условие открытия
            if (layers[l_v].vertexes[ni].firstPart != layers[l_v].vertexes[pi].lastPart && !layers[l_v].vertexes[ni].isLWin && !layers[l_v].vertexes[pi].isRWin) {
               window(l_v, pi);
               layers[l_v].vertexes[pi].isRWin = true;
            }

            // Условие закрытия
            if (layers[l_v].vertexes[ni].firstPart == layers[l_v].vertexes[pi].lastPart) {
                if (layers[l_v].vertexes[ni].isLWin) {
                    clwindow(l_v, ni);
                    layers[l_v].vertexes[ni].isLWin = false;
                }
                if (layers[l_v].vertexes[pi].isRWin) {
                    clwindow(l_v, pi);
                    layers[l_v].vertexes[pi].isRWin = false;
                }
            }
        }
    }
}

int Web::findnext(int l_v, int v) {
    int ni = v + 1;
    while (ni != layers[l_v].vertexes.size() && layers[l_v].vertexes[ni].firstPart == 0) {
        ni = ni + 1;
    }
    if (ni == layers[l_v].vertexes.size()) ni = -1;
    return ni;
}

int Web::findprev(int l_v, int v) {
    int pi = v - 1;
    while (pi != -1 && layers[l_v].vertexes[pi].lastPart == 0) {
        pi = pi - 1;
    }
    return pi;
}

int Web::sheduledjobs() {   
    return num_of_works;
}


void Web::part_from_proc(int l_q, int l_p) {
    layers[l_p].load += layers[l_q].complexity;
    // Убрать поток
    for(int u=0; u < layers[l_q].vertexes.size(); u++) {
        for (map<int, NeighborInfo >::iterator it = layers[l_q].vertexes[u].neighbors[l_p].begin(); it != layers[l_q].vertexes[u].neighbors[l_p].end(); it++) {
            // Номер вершины соседа
            int v = it->first;
            long long value = layers[l_q].vertexes[u].neighbors[l_p][v].flow;
            layers[l_q].vertexes[u].neighbors[l_p][v].flow -= value;
            layers[l_p].vertexes[v].neighbors[l_q][u].flow += value;
            
            // Нужно от самого конца начинать убирать
            // Считаем, что что-то может утечь exf, а потом уже только flow
            // Дальше число, на которое flow должно измениться
            long long end_value = max((long long)0, value - layers[l_p].vertexes[v].exf);
            long long new_value = value - end_value;
            layers[l_p].vertexes[v].flow -= end_value;
            layers[l_p].vertexes[v].exf -= new_value;
                
            int ni = findnext(l_p, v);
            int pi = findprev(l_p, v);
            checkpartdec(l_p, v, layers[l_q].vertexes[u].part, value);
            correctwindows(l_p, v, ni, pi);

            layers[l_q].vertexes[u].exf += value;
            layers[l_p].vertexes[v].h = 1;
            layers[l_q].vertexes[u].h = 1;

            if (layers[l_q].vertexes[v].exf > 0) layers[l_q].extended.push_back(v);
        }
    }

    // Назначить новый процессор
    decide_proc(l_q);
}


void Web::part_to_proc(int part, int idx_proc_layer) {
    QP[part] = idx_proc_layer;

    // Запретить другие пути
    // Идем по вершинам слоя с работами
    for(int i=0; i < layers[part].vertexes.size(); i++) {
        // Проход по соседям, минимальная вершина среди тех, куда возможно проталкивание
        for(Neighbors::iterator it_layer = layers[part].vertexes[i].neighbors.begin(); it_layer != layers[part].vertexes[i].neighbors.end(); it_layer++) {
            map<int, NeighborInfo > layer_neighbors = it_layer->second;
            // Номер слоя соседа
            int l_v = it_layer->first;

            // Если не наш процессор, то делаем до него пропускные способности в 0
            if (l_v != idx_proc_layer) {
                for (map<int, NeighborInfo >::iterator it = layer_neighbors.begin(); it != layer_neighbors.end(); it++)  {
                    // Номер вершины соседа
                    int v = it->first;
                    layers[part].vertexes[i].neighbors[l_v][v].cap = 0;
                }
            } else {
                for (map<int, NeighborInfo >::iterator it = layer_neighbors.begin(); it != layer_neighbors.end(); it++) {
                    // Номер вершины соседа
                    int v = it->first;
                    //  Восстанавливаем пропускные способности
                    layers[part].vertexes[i].neighbors[l_v][v].cap = layers[l_v].vertexes[v].capacity;
                }
            }
        }

    }
    cout << "Part " << part << " to Proc " << idx_proc_layer << endl;
}

void Web::decide_proc(int part) {
    // Предыдущее место размещения
    int prohibit_layer = QP[part];
    int idx_proc_layer = 0;
    int freeSpace = 0;
    int complexity = layers[part].complexity;

    // Идем по слоям-процессорам и выбираем подходящий слой
    for(int i=layer_int; i <free_layer; i++) {
        bool isFunctionality = true;
        int iproc = layers[i].ptype;
        set<string> result;
        std::set_intersection(processors[iproc]->functionality.begin(),processors[iproc]->functionality.end(),
                              partitionFunctionality[part].begin(), partitionFunctionality[part].end(),
                              std::inserter(result, result.begin()));
        if (result != partitionFunctionality[part]){
            isFunctionality = false;
        }
        if (layers[i].load > freeSpace && i != prohibit_layer && isFunctionality) {
            idx_proc_layer = i;
            freeSpace = layers[i].load; 
        }
    }


    layers[idx_proc_layer].load -= complexity;
    if (idx_proc_layer!=0){
        part_to_proc(part, idx_proc_layer);
    } else { 
        cout << "UNABLE TO PLACE PART " << part << endl;
        QP[part] = idx_proc_layer;

        // Удаляем все отсюда
        for(int i=0; i < layers[part].vertexes.size(); i++){
            deletework(part, i);
        }

        // Удаляем extended
        layers[part].extended.resize(0);
    }
}

int Web::add_proc_layer(int iproc) {
    // Добавляем по дефолтному слою
    int l_j = free_layer;
    free_layer++;
    auto options = processors[iproc]->functionality;
    int proc_performance = processors[iproc]->performance;

    // Копирование слоя
    layers[l_j] = layers[0];
    layers[l_j].load = proc_performance * mainLoop;
    layers[l_j].ptype = iproc;
    layers[l_j].functionality = options;
    cout << "SIZE " << layers[l_j].vertexes.size() << endl;

    // Изменяем параметры
    for (int l_i=1; l_i < layer_int ; l_i++){
        cout << l_i << " " << layer_int<< endl;
        bool isFunctionality = true;
        set<string> result;
        cout << "Hello" << layers[l_i].vertexes[0].options.size() << endl;
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
                    layers[l_i].vertexes[i].neighbors[l_j][j].cap = layers[l_i].vertexes[i].neighbors[0][j].cap * proc_performance;
                    layers[l_j].vertexes[j].capacity = layers[l_i].vertexes[i].neighbors[l_j][j].cap;
                    cout << layers[l_i].vertexes[i].neighbors[l_j][j].cap << endl;
                }
            }
        }
    }

    // Заполняем инфу о разделах в проце
    for (int j=0; j < layers[l_j].vertexes.size(); j++) {
        layers[l_j].vertexes[j].partIn.resize(q + 1);
        //  Учитываем, что на переключение задано время
        layers[l_j].vertexes[j].cTime = layers[l_j].vertexes[j].cTime; //* proc_performance;
        for(int k = 0; k < q + 1; k++) {
            layers[l_j].vertexes[j].partIn[k] = 0;
        }
    }
    cout << "END OF ADDING PROCESSOR\n";
    return l_j;
}

void Web::print() {
    // Выведем последний слой сети
    cout << "WEB IMAGE" << endl;
    for(int i=0; i < free_layer; i++){
        for(int j=0; j < layers[i].vertexes.size(); j++) {
            cout << i << " " << j << " flow " << layers[i].vertexes[j].flow << " exf " << layers[i].vertexes[j].exf << endl;
            // Проход по соседям, минимальная вершина среди тех, куда возможно проталкивание
            for(Neighbors::iterator it_layer = layers[i].vertexes[j].neighbors.begin(); it_layer != layers[i].vertexes[j].neighbors.end(); it_layer++) {
                map<int, NeighborInfo > layer_neighbors = it_layer->second;
                // Номер слоя соседа
                int l_v = it_layer->first;
                for (map<int, NeighborInfo >::iterator it = layer_neighbors.begin(); it != layer_neighbors.end(); it++) {
                    // Номер вершины соседа
                    int v = it->first;
                    cout << i << " " << j << " to " << l_v << " " << v << " " << layers[i].vertexes[j].neighbors[l_v][v].flow << " / " <<layers[i].vertexes[j].neighbors[l_v][v].cap << endl;
                }
            }
        }
    }
}

void Web::sort_partition() {
    // Создать порядок разделов по сортировке и записать во внутреннюю структуру
    vector<pair<int,int> >vec;
    for(int l=1 ; l < q+1; l++) {
        vec.push_back(pair<int, int>(l, layers[l].complexity));
    }
    sort(vec.begin(), vec.end(), [](pair<int,int> elem1, pair<int,int> elem2) {return elem1.second > elem2.second;});
    partitionOrder = vec;
}

void Web::sort_processors() {
    // Создать порядок разделов по сортировке и записать во внутреннюю струкктуру
    vector<pair<int,int> >vec;
    int i = 0;
    for(auto processor : processors) {
        vec.push_back(pair<int, int>(i++, processor->cost));
    }
    sort(vec.begin(), vec.end(), [](pair<int,int> elem1, pair<int,int> elem2) {return elem1.second < elem2.second;});
    processorOrder = vec;
}

void Web::create_cost_tab() {
    // Создать порядок разделов по сортировке и записать во внутреннюю структуру
    int i = 0;
    for(int i = 0; i < partitionOrder.size(); i++){
        auto partition = partitionOrder[i].first;
        auto l_p = layers[partition];
        tab[partition].resize(0);
        for(int i = 0; i < processorOrder.size(); i++){
            auto processor = processors[processorOrder[i].first];
            set<string> result;
            std::set_intersection(processor->functionality.begin(), processor->functionality.end(),
                                  l_p.functionality.begin(), l_p.functionality.end(),
                                  std::inserter(result, result.begin()));
            cout << layers[partition].complexity << "/" <<  processorLoad[processorOrder[i].first] << "\n";
            if (result == l_p.functionality && layers[partition].complexity < processorLoad[processorOrder[i].first]){
                tab[partition].push_back(processorOrder[i].first);
            }
        }
    }
}

void Web::find_best_config() {
    while (tab.size()!=0) {
        // Сейчас для каждого будем искать лучшее разбиение
        map<int, int> best_proc;
        map<int, set<int> > best_config;
        map<int, int> cost_function;
        for(auto item: tab){
            // Тут описана процедура для одного раздела
            int current_part = item.first;
            // Сейчас будет перебор по всем возможным процессорам
            map<int, set<int> > proc_config;
            map<int, int> cost_function_part;
            for(auto proc: item.second){
                set<int> parts;
                parts.insert(current_part);
                // А тут перебор по доступным разделам     
                for(auto part: tab){
                    // Если раздел подходит, тут нужно функцию написать TODO
                    bool accept = false;
                    for(int i=0; i < part.second.size(); i++) {
                        if (proc == part.second[i]) {
                            accept = true;
                            break;
                        }
                    }
                    bool is_space = false;
                    int space = 0;
                    space += layers[part.first].complexity;
                    for (auto it = parts.begin(); it != parts.end(); it++) {
                        space += layers[*it].complexity;
                        cout << *it << " ";
                    }
                    if (space < processorLoad[proc]) is_space = true;

                    // Теперь возможность размещения рассмотрим
                    if (accept && is_space) { 
                        auto web_copy = *this;

                        // Добавляем процессор и пытаемся найти поток
                        web_copy.add_proc_layer(proc);
                        parts.insert(part.first);
                        
                        auto good = web_copy.maxflow(true, parts);
                        if (!good) {
                            parts.erase(part.first);
                        }
                    }
                }
                proc_config[proc] = parts;
                cost_function_part[proc] = compute_cost(proc, parts);       
            }
            // Найти минимальный по костам процессор
            auto min = min_element(cost_function_part.begin(), cost_function_part.end(), 
                                   [](const auto& l, const auto& r) { return l.second < r.second; });
            auto part_proc = min->first;
            auto part_set = proc_config[part_proc];

            // Нашли лучшее для заданного раздела, теперь внесем инфу
            best_proc[current_part] = part_proc;
            best_config[current_part] = part_set;
            cost_function[current_part] = min->second;
        }

        auto min = min_element(cost_function.begin(), cost_function.end(), [](const auto& l, const auto& r) { return l.second < r.second; });
        cout << "Min " << min->second << "\n ";
        auto index_min = min->first;
        auto set = best_config[index_min];
        auto fit_proc = best_proc[index_min];

        // Записываем лучшее
        best_system.push_back(make_pair(fit_proc, set));

        // Удаляем из tab весь set
        for(auto item: set) {
            tab.erase(item);
        }
    }
}

int Web::compute_cost(int proc, set<int> parts) {
    int result = processors[proc]->cost;
    for(auto item: parts) {
        // Стоимость раздела по размещенному разделу
        result -= processors[tab[item][0]]->cost;
    }
    return result;
}

void Web::print_tab() {
    cout << "TAB\n";
    for(auto item: tab) {
        cout << item.first << " : ";
        for (int i = 0; i < item.second.size(); i++) {
            cout << item.second[i] << ",";
        }
        cout << "\n";
    }
}

void Web::print_system() {
    cout << "SYSTEM\n";
    for(auto item: best_system) {
        cout << item.first << " : ";
        for (auto it = item.second.begin(); it != item.second.end(); it++) {
            cout << *it << ",";
        }
        cout << "\n";
    }
}



bool Web::find_alloc(string typeoftask) {
    if (typeoftask == "schedule") {
        // Добавляем процессоры по дефолтному слою
        for (int iproc=0; iproc < nproc; iproc++){
            add_proc_layer(iproc);
        }

        // Дополнительная информация
        hints = nproc*nproc - 1;
        hints_layer = 10;
        for(map<int, Layer>::iterator it = layers.begin(); it != layers.end(); it++) {
            n = n + it->second.vertexes.size();
        }
        // Добавляем вместительность процессоров
        for (int i=0;i < nproc; i++) {
            processorLoad[i] = mainLoop * processors[i]->performance;
        }
        // Определяем порядок разделов
        sort_partition();
        // Первоначальное распределение
        // Опредляем порядок разделов изначальные предпочтения
        for(int i = 0; i <= partitionOrder.size(); i++) {
            // Наивный вариант нужно что умнее
            decide_proc(partitionOrder[i].first);
        }
        return true;

    } else if (typeoftask == "synthesis") {
        // Добавляем вместительность процессоров
        for (int i=0;i < nproc; i++) {
            processorLoad[i] = mainLoop * processors[i]->performance;
        }
        sort_partition();
        sort_processors();
        for(int i = 0; i < partitionOrder.size(); i++) {
            cout << i << " " << partitionOrder[i].first << " " << partitionOrder[i].second << "\n";
        }
        cout << "Processors \n"; 
        for(int i = 0; i < processorOrder.size(); i++) {
            cout << i << " " << processorOrder[i].first << " " << processorOrder[i].second << "\n";
        }
        create_cost_tab();
        print_tab();
        find_best_config();
        cout<<"BEST CONFIG WAS FOUND\n";
        print_system();
        print_tab();
        // Добавим процессоры для полученной конфигурации
        int count = 0;
        for(auto item: best_system) {
            auto l = add_proc_layer(item.first);   
            count++;  
        }
        // Добавим привязку
        for(int i=0; i < best_system.size(); i++) {
            for(auto p: best_system[i].second) {
                part_to_proc(p, free_layer - count + i);
            }  
        }
        
        // Допонительная информация
        hints = nproc*nproc - 1;
        hints_layer = 10;
        return true;
    } else {
        return false;
    }
}

long long NOK(long long a, long long b) {
  return a*b / NOD(a, b);
}

long long NOD(long long a, long long b) {
  while (a != 0 && b != 0) {
    if (a > b) {
        a %= b;
    } else {
        b %= a;
    }
  }
  return a + b;
}


vector<string> split(string strToSplit, string delimeter) {
    std::vector<std::string> splittedString;
    int startIndex = 0;
    int  endIndex = 0;
    while( (endIndex = strToSplit.find(delimeter, startIndex)) < strToSplit.size() ) {
        string val = strToSplit.substr(startIndex, endIndex - startIndex);
        splittedString.push_back(val);
        startIndex = endIndex + delimeter.size();
    }
    if(startIndex < strToSplit.size()) {
        string val = strToSplit.substr(startIndex);
        splittedString.push_back(val);
    }
    return splittedString;
}
