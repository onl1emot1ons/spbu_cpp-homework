//
// Created by konst on 27.10.2026.
//

#ifndef CPP_HOMEWORK_MATRIX_H
#define CPP_HOMEWORK_MATRIX_H

#include <memory>

class Matrix {
public:
    virtual int get_rows() const = 0;
    virtual int get_cols() const = 0;
    virtual double get_element(int row, int column) const = 0;

    virtual std::unique_ptr<Matrix> mul(const Matrix& second) const = 0;

    virtual ~Matrix() = default;
};


#endif //CPP_HOMEWORK_MATRIX_H
