#include <iostream>
#include <functional> // для сравнение в компараторе less
#include <vector>
#include <iterator> // для iterator trails
using namespace std;


template <  // компаратор
    typename It, // создаём тип для итераторов (указателей на элемент сортируемого типа)
    typename Compare = less<typename iterator_traits<It>::value_type> //сравниваем два значения, которые передаются через итератор
> //iterator_traits достаёт инфу о типе итератора It, а value_type достаёт тип элемента (int, float, ...), typename говорит, что это тип элемента с которым будет работать less


void mySort(It first, It last, Compare comp = Compare{}) {
    auto size = last - first;  //size автоматически получает кол-во позиций между итераторами, целочисленное

    if (size <= 1) return; // если 1 элемент - возвращаемся на предыдущий уровень рекурсии, так как 1 элем уже отсрчен

    It middle = first + size / 2; //указатель на середину - так как указатели целочисленные => деление нацело

    mySort(first, middle, comp);
    mySort(middle, last, comp); //две рекурсии - сортируем половины

    using value = typename iterator_traits<It>::value_type; //с помощью typename мы получаем тип данных, которые передаются через It, и называем этот тип value
    vector<value> buffer; // таким образом создаётся промежуточный вектор куда мы складываем уже отсортированные подмассивы
    buffer.reserve(size); // заранее выделяем в нём место минимум для size элементов, чтобы быстрее работал push_back

    It left = first; // передаём в лефт итератор на первый элемент - левая граница первого "подмассива"
    It right = middle; // в правый - левая граница правого "подмассива"

    while (left != middle && right != last) { //начинаем путь по двум массивам
        if (comp(*left, *right)) { //элем левого меньше элем правого?
            buffer.push_back(*left); //да = добавляем элем из левого
            ++left; //переходим на след элемент левой части
        }
        else { //иначе (правый меньше левого)
            buffer.push_back(*right); //добавляем элем из правого
            ++right; //добавляем к итератору 1
        }
    }
    // ВАЖНО УЧИТЫВАТЬ !!!!! здесь подмассивы - не массивы по сути, а лишь указатели на элементы, которые мы сортируем
    // - в конкретной итерации рекурсии у нас свой first, last И middle, которые указывают на нужные элементы массива и ограничивают чтобы мы не трогали лишние элементы
    while (left != middle) { //если остались элементы в левом - они точно больше правого, так как он дошёл до конца, а левый отсорчен
        buffer.push_back(*left);
        ++left; //аналогично как в части алгоритма для сравнения элементов
    }
    while (right != last) { //аналогично для правого подмассива
        buffer.push_back(*right);
        ++right;
    }

    It destination = first; //финальная стадия - будем перезаписывать исходный массив. в destination кладём итератор на первый элемент разделённого и уже отсорченного в буфере массива
    for (auto& element : buffer) { //для каждого элемента в буфере
        *destination = element; //в элемент за ссылкой destination кладём element
        ++destination; // пеерходим к след элементу
    }
}


struct point {
    double x;
    double y;
};


bool compare_points(const point& a, const point& b) {
    return (a.x * a.x + a.y * a.y) < (b.x * b.x + b.y * b.y);
}


int main() {

    cout << "hello world" << endl;


    cout << "Vector of numbers:" << '\n';
    vector<int> v = {3, 5, 1, 3, 7, 0, -1, 4};

    for (auto& element : v) {
        cout << element << " ";
    }
    cout << '\n';

    vector<int> sorted_v = v;
    mySort(sorted_v.begin(), sorted_v.end());
    for (auto& element : sorted_v) {
        cout << element << " ";
    }
    cout << '\n';


    cout << "Vector of points:" << '\n';
    vector<point> points = {{3, 4},{1, 0},{0, 0},{2, 2}};
    for (auto& element : points) {
        cout << "(" << element.x << ", " << element.y << ") ";
    }
    cout << '\n';

    vector<point> sorted_points = points;
    mySort(sorted_points.begin(), sorted_points.end(), compare_points);
    for (auto& element : sorted_points) {
        cout << "(" << element.x << ", " << element.y << ") ";
    }


    return 0;
}