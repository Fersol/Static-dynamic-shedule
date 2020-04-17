#ifndef TYPES_H
#define TYPES_H

#include <vector>
#include <set>
#include <list>
#include <map>
#include <string>
#include <utility>
#include <algorithm>
#include <iostream>
#include <iterator>

using namespace std;

typedef vector<int> VInt;
typedef set<int> SInt;
typedef vector<VInt> VVInt;
typedef list<int> LInt;
typedef vector<LInt> VLInt;
typedef LInt::iterator LIter;
typedef vector<LIter> VLIter;

const int attemptCount = 5;

enum VType{ DEFAULT, JOB, INTERVAL, SOURCE, DEST };

struct TaskHeterogenes
{
    long long period;          // период задачи
    long long left;            // левая граница
    long long right;           // правая граница
    long long partition;       // раздел задачи
    long long complexity;      // сложность выполнения задачи
    set<string> functionality; // требуемая функциональность
};

struct JobHeterogenes
{   
    int numTask;               // номер задачи
    long long start;           // время начала директивного интервала
    long long finish;          // время завершения директивного интервала
    int partition;             // раздел работы
    long complexity;           // сложность выполнения работы
    set<string> functionality; // требуемая функциональность
};

struct Processor
{
    int performance;           // производительность процессора
    int cost;                  // стоимость процессора
    set<string> functionality; // обеспечиваемая функциональность
};

struct Window
{
    float start;
    float finish;
    long long partition;
    long long ptype;
    map<int, float> works;
};

struct NeighborInfo
{
    int flow = 0;
    int cap = 0;
};

typedef map<int, map<int, NeighborInfo> > Neighbors;

struct Vertex
{
 public:
  int l = 0;        // слой вершины 
  int me = 0;       // номер вершины в слое 
  int exf = 0;			// избыточный поток через вершину
  int part = 0;			// разделы к которым принадлежит вершина для работ, 0 для интервалов
  int h = 0;				// высота вершины
  int proc = -1;    // номер процессора для вершины интервала
  int cTime;        // сложность переключения
  long long stTime = 0;
  long long finTime = 0;
  long long duration = 0;
  set<string> options;
  Neighbors neighbors;    // здесь определены соседи
  int capacity;           // cколько вершина может принять потока
  int flow;               // сколько вершина приняла потока
  int numTask;            // номер задания
  int chWdw = 0;		  		// количество переключений в интервале = количество разделов - 1;
  VType type = DEFAULT;		// тип вершины в зависимости от слоя - работа, интервал, сток, исток
  vector<int> partIn;		  // размер вектора равен количеству разделов + 1,
  // Хранит кол-во поток каждого раздела в вершину
  // Только для вершин интервалов
  set<int> setpart;       // множество разделов
  bool isRWin = false;    // есть ли переключение на правом стыке интервала
  bool isLWin = false;    // есть ли переключение на левом стыке интервала
  int firstPart = 0;      // 0 - значит нет
  int lastPart = 0;       // 0 - значит нет
};

class Layer
{ 
 public:
  vector<Vertex> vertexes;
  vector<int> extended;      // переполненные вершины
  int ptype = -1;            // тип процессора слоя, -1 - слои с разделами.
  int complexity;            // сложность для раздела
  int load;                  // доступная нагрузка для процессора
  set<string> functionality; // возможности процессора и разделов

};

class Web
{
public:
    int hints;               // число попыток на перестройку сети, до скидывания работы. -> lift
    int hints_layer;         // число попыток
    int cw; 			           // время на переключение окна
    int n;				           // количество вершин
    int nproc;               // число процессоров
    int num_of_works;        // количество вершин-работ
    int q;				           // количество разделов
    int source_flow;         // размер потока из источника
    int dest_flow;           // Размер потока в сток
    int layer_int;           // первый слой с интервалами
    int free_layer;          // номер свободного слоя
    int hard;                // сложность расписания
    map<int,set<int> > P;    // переполненные вершины по разделам, 0 - вершины интервалы
    map<int, Layer> layers;  // cлои c вершинами
    map< int, int> QP;       // Соответствие раздела слою (который соответствует процессору).
    vector<Processor*> processors;
    int mainLoop;            // НОК работ - интервал планирования
    vector<pair<int,int> > partitionOrder;
    vector<pair<int,int> > processorOrder;
    map< int, vector<int> > tab;   // Таблица для поиска оптимальных процессоров
    map<int, int>  processorLoad;
    map<int, set<string> > partitionFunctionality;
    vector< pair<int, set<int> > > best_system;  // Для записи лучше системы - тип процессора и разделы на нем

    /**
     * Конструктор по умолчанию
     */
    Web();

    /**
     * Поиск элемента в множестве, не равный данному
     */
    int finsetneq(set<int>& set, int a);

    /**
     * Подъем вершины
     */
    void lift(int l, int u);

    /**
     * Создать перключение окна в интервале (только для вершин интервалов)
     */
    void window(int l, int u);
    
    /**
     * Отменить перключение окна в интервале (только для вершин интервалов)
     */
    void clwindow(int l, int u);

    /**
     * Проталкивание потока с учетом раздела
     */
    void push(int l1, int u, int l2, int v, bool isfull);

    /**
     * Разгрузка вершины (протолкнуть по максимуму) true - если подъем был
     * Подается на вход номер вершины
     */
    bool discharge(int l, int u, bool is_first);

    /**
     * Обычный алгоритм поиска максимального потока медотодом поднять и в начало
    */
    bool maxflow(bool check=false, set<int> parts=set<int>());

    /**
     * Обычный алгоритм поиска максимального потока методом поднять и в начало и построением конфигурации
    */
    void conf_maxflow();

    /**
     * Найти тип наиболее подходящего процессора для раздела
    */
    void find_best_proc(int part);

    /**
     * Удаляет работу из сети
     */
    void deletework(int l, int u);

    /**
     * Возвращает число окон, которые будут переключены при проталкивании потока из u в v
     * @param u - вершина работа
     * @param v - вершина интервал
     * @return
     */
    int test(int l1, int u, int l2, int v, int value);

    /**
     * Считает эффективность построенных окон по времнной сложности размещенных задач
     */
    double Effectivness();

    /**
     * Учитывает раздел для вершины при добавлении потока
     * @param u - номер вершины
     * @param part - номер раздела, который добавляется
     * @param value - размер добавляемого потока
     * @param ni - следующий непустой интервал или 0, если его нет
     * @param pi - предыдущий непустой интревал или 0, если его нет
     */
    void checkpartadd(int l, int u, int part, int value, int ni, int pi);

    /**
     * Учитывает раздел для вершины при отведении потока
     * @param u - номер вершины
     * @param part - номер раздела, который добавляется
     * @param value - размер добавляемого потока
     */
    void checkpartdec(int l, int v, int part, int value);

    /**
     * Корректирует окна для данной вершины
     * @param u - номер вершины
     * @param ni - следующий непустой интервал или 0, если его нет
     * @param pi - предыдущий непустой интревал или 0, если его нет
     */
    void correctwindows(int l, int v, int ni, int pi);

    /**
     * Ищет следующий непустой интервал
     * @param v - номер вершины-интервала
     * @return номер следующего интервала, 0 - если его нет
     */
    int findnext(int l, int v);

    /**
     * Ищет предыдущий непустой интервал
     * @param v - номер вершины-интервала
     * @return номер следующего интервала, 0 - если его нет
     */
    int findprev(int l, int v);

    /**
     * Вернуть проводимости между работами и интервалами
     */
    void back();

    /**
     * Убирает весь поток из сети
     */
    void noflow();

    /**
     * Считает число размещенных задач
     */
    int sheduledjobs();

    /**
     * Снимаем раздел с ядра и разрешаем ему идти в другие ядра.
     */
    void part_from_proc(int q, int proc);

    /**
     * Решаем на какой процессор разместить раздел
     */
   void decide_proc(int part);
   
   /**
     * Размещение раздела на процессор
     */
   void part_to_proc(int part, int idx_proc_layer);

    /**
     * Создает слой с вершинами-интервала процессра определенного вида
     */
   int add_proc_layer(int iproc);

    /**
     * Печатает состояние сети
     */
   void print();

  /**
     * Сортирует разделы по порядку убывания сложности раздела
     */
   void sort_partition();

   /**
     * Сортирует процессор по порядку увеличения стоимости процессора
     */
    void sort_processors();

    /**
      * Сортирует процессор по порядку увеличения стоимости процессора
      */
    void create_cost_tab();

    /**
      * Печатает процессоры по порядку увеличения стоимости процессора
      */
    void print_tab();

    /**
      * Печатают систему
      */
    void print_system();

    /**
      * Находит наилучшую схему для синтеза
      */
    void find_best_config();

    /**
      * Сортирует процессор по порядку увеличения стоимости процессора
      */
    bool find_alloc(string typeoftask);

    /**
      * Сортирует процессор по порядку увеличения стоимости процессора
      */
    int compute_cost(int proc, set<int> parts); 

};



/**
 * @brief NOD
 * @param n1
 * @param n2
 * @return
 */
long long NOD(long long n1, long long n2);

/**
 * @brief NOK
 * @param n1
 * @param n2
 * @return
 */
long long NOK(long long n1, long long n2);

vector<string> split(string strToSplit, string delimeter);

#endif // TYPES_H
