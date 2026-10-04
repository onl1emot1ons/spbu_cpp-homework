#include <iostream>
#include <functional>
using namespace std;


template <
    typename It,
    typename Compare = less<typename iterator_traits<It>::value_type>
>


void mySort(It first, It last, Compare comp = Compare{}) {

}


int main() {

    cout << "hello world" << endl;

    return 0;
}