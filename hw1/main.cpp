#include <iostream>
#include <functional>
#include <vector>
#include <iterator>
using namespace std;


template <
    typename It,
    typename Compare = less<typename iterator_traits<It>::value_type>
>


void mySort(It first, It last, Compare comp = Compare{}) {
    auto size = last - first;

    if (size <= 1) return;

    It middle = first + size / 2;

    mySort(first, middle, comp);
    mySort(middle, last, comp);

    using value = typename iterator_traits<It>::value_type;
    vector<value> buffer;
    buffer.reserve(size);

    It left = first;
    It right = middle;

    while (left != middle && right != last) {
        if (comp(*left, *right)) {
            buffer.push_back(*left);
            ++left;
        }
        else {
            buffer.push_back(*right);
            ++right;
        }
    }

    while (left != middle) {
        buffer.push_back(*left);
        ++left;
    }
    while (right != last) {
        buffer.push_back(*right);
        ++right;
    }

    It destination = first;
    for (auto& element : buffer) {
        *destination = element;
        ++destination;
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

    vector<int> new_v = v;
    mySort(new_v.begin(), new_v.end());
    for (auto& element : new_v) {
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