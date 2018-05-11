#ifndef TYPES_H
#define TYPES_H

#include <QVector>
#include <QSet>
#include <QList>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QPair>

typedef QVector<int> VInt;
typedef QSet<int> SInt;
typedef QVector<VInt> VVint;
typedef QList<int> LInt;
typedef QVector<LInt> VLInt;
typedef LInt::iterator LIter;
typedef QVector<LIter> VLIter;

const int ATTEMPT_COUNT = 10;

enum Vtype{ DEFAULT, JOB, INTERVAL, SOURCE, DEST };

struct Task
{
    long long period;     //период задачи
    long long partition;  //раздел задачи
    long long duration;   //длительность выполнения задачи
};



struct Job
{   
    int numtask;  // номер задачи
    long long start;      //время начала директивного интервала
    long long finish;     //время завершения директивного интервала
    int partition;  //раздел работы
    long duration;   //длительность выполнения работы
};

struct Window
{
    long long start;
    long long finish;
    long long partition;
    QMap<int, int> works;
};

struct Vertex
{
 public:
  int exf = 0;					//избыточный поток через вершину
  int part = 0;					//разделы к которым принадлежит вершина для работ, 0 для интервалов
  int h = 0;					//высота вершины
  int proc = -1;                     // номер процессора для вершины интервала
  long long sttime = 0;
  long long fintime = 0;
  long long duration = 0;
  //здесь определены соседи (в следующих 2-х строчках
  QMap<int, int> flow;
  QMap<int, int> cap;
  int numtask;     //номер задания
  int chwdw = 0;		  		//количество переключений в интервале = количество разделов - 1;
  Vtype type = DEFAULT;		//тип вершины в зависимости от слоя - работа, интервал, сток, исток
  QVector<int> partin;				//размер вектора равен количеству разделов + 1,
  //хранит кол-во поток каждого раздела в вершину
  //Только для вершин интервалов
  QSet<int> setpart;//множество разделов
  int nextitr=0;//следущая вершина интервал по времени, 0 - значит нет
  int previtr=0;//предыдущая вершина интервал по времени, 0 - значит нет
  bool isrwin = false;//есть ли переключение на правом стыке интервала
  bool islwin = false;//есть ли переключение на левом стыке интервала
  int firstpart=0;// 0 - значит нет
  int lastpart=0;// 0 - значит нет
};

class Web
{
public:
    int hints;           // число попыток на перестройку сети, до скидывания работы. -> lift
    int cw; 			 //время на переключение окна
    int n;				 //количество вершин
    int nproc;           // число процессоров
    int numofwork;       //количество вершин-работ
    int q;				 //количество разделов
    int src;   			 //номер вершины источника
    int dest;			 //номер вершины стока
    int hard;            //сложность расписания
   // QMap< QPair<int, int>, int > COR; // коррекции сети, которые были сделаны из работ в интервалы
    QMap< int,QSet<int> > P;    //переполненные вершины по разделам, 0 - вершины интервалы
    QMap< int, QSet<int> > VPart; // номера вершин по разделам
    QVector<Vertex> V;   //cами вершины
    QMap< int, int> QP; // Соответствие раздела процессору.

    /**
     * Конструктор по умолчанию
     */
    Web();

    /**
     * Поиск элемента в множестве, не равный данному
     */
    int finsetneq(QSet<int>& set, int a);

    /**
     * Подъем вершины
     */
    void lift(int u);

    /**
     * Создать перключение окна в интервале (только для вершин интервалов)
     */
    void window(int u);
    
    /**
     * Восстановить сеть от блоков по неудачам
     *
    void cureWeb();
    */
    
    /**
     * Отменить перключение окна в интервале (только для вершин интервалов)
     */
    void clwindow(int u);

    /**
     * Проталкивание потока с учетом раздела
     */
    void push(int u, int v, bool isfull);

    /**
     * Разгрузка вершины (протолкнуть по максимуму) true - если подъем был
     * Подается на вход номер вершины
     */
    bool discharge(int u, bool is_first);

    /**
     * Обычный алгоритм поиска максимального потока медотодом поднять и в начало
    */
    void maxflow();

    /**
     * Удаляет работу из сети
     */
    void deletework(int u);

    /**
     * Возвращает число окон, которые будут переключены при проталкивании потока из u в v
     * @param u - вершина работа
     * @param v - вершина интервал
     * @return
     */
    int test(int u, int v, int value);

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
    void checkpartadd(int u, int part, int value, int ni, int pi);

    /**
     * Учитывает раздел для вершины при отведении потока
     * @param u - номер вершины
     * @param part - номер раздела, который добавляется
     * @param value - размер добавляемого потока
     */
    void checkpartdec(int v, int part, int value);

    /**
     * Корректирует окна для данной вершины
     * @param u - номер вершины
     * @param ni - следующий непустой интервал или 0, если его нет
     * @param pi - предыдущий непустой интревал или 0, если его нет
     */
    void correctwindows(int v, int ni, int pi);

    /**
     * Ищет следующий непустой интервал
     * @param v - номер вершины-интервала
     * @return номер следующего интервала, 0 - если его нет
     */
    int findnext(int v);

    /**
     * Ищет предыдущий непустой интервал
     * @param v - номер вершины-интервала
     * @return номер следующего интервала, 0 - если его нет
     */
    int findprev(int v);

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
     * Запрещает размещать работы раздела на других ядрах
     */
    void part_to_proc(int q, int proc);

    /**
     * Снимаем раздел с ядра и разрешаем ему идти в другие ядра.
     */
    void part_from_proc(int q, int proc);
};


#endif // TYPES_H
