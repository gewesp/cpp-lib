//
// Copyright 2015 KISS Technologies GmbH, Switzerland
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Component: MATH
//

//
// Wrapper typedefs and small functions for Eigen.
//
// This is used to isolate cpp-lib from the matrix library used.
// If you need additional features from Eigen, add wrapper functions
// here.
//

#ifndef CPP_LIB_MATRIX_WRAPPER_H
#define CPP_LIB_MATRIX_WRAPPER_H

#include <cmath>
#include <vector>

#include "cpp-lib/assert.h"

#include "Eigen/Dense"

namespace cpl {

namespace matrix {

typedef Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic> matrix_t;
typedef Eigen::Matrix<double, Eigen::Dynamic, 1             > vector_t;

typedef Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> matrix_f_t;
typedef Eigen::Matrix<float, Eigen::Dynamic, 1             > vector_f_t;

template<int N, int M> using matrix_fixed_t = Eigen::Matrix<double, N, M>;
template<int N>        using vector_fixed_t = Eigen::Matrix<double, N, 1>;

typedef Eigen::Matrix<double, 3, 3> matrix_3_t;
typedef Eigen::Matrix<double, 3, 1> vector_3_t;

typedef Eigen::Matrix<double, 2, 2> matrix_2_t;
typedef Eigen::Matrix<double, 2, 1> vector_2_t;

typedef Eigen::Matrix<double, 1, 1> matrix_1_t;
typedef Eigen::Matrix<double, 1, 1> vector_1_t;

template<typename T, int N, int M>
long n_rows   (Eigen::Matrix<T, N, M> const& A) { return A.rows(); }
template<typename T, int N, int M>
long n_columns(Eigen::Matrix<T, N, M> const& A) { return A.cols(); }

template<typename T, int N, int M>
void fill(Eigen::Matrix<T, N, M>& A, T const& x) 
{ A.fill(x); }

template<typename T, int N, int M>
bool isnan_any(Eigen::Matrix<T, N, M> const& A) {
  for (long i = 0; i < A.rows(); ++i) {
  for (long j = 0; j < A.cols(); ++j) {
    if (std::isnan(A(i, j))) {
      return true;
    }
  }}
  return false;
}

template<typename T, int N, int M>
bool isinf_any(Eigen::Matrix<T, N, M> const& A) {
  for (long i = 0; i < A.rows(); ++i) {
  for (long j = 0; j < A.cols(); ++j) {
    if (std::isinf(A(i, j))) {
      return true;
    }
  }}
  return false;
}

template<int N>
Eigen::Matrix<double, N, N> zero() {
  return Eigen::Matrix<double, N, N>::Zero();
}

template<int N>
Eigen::Matrix<double, N, N> identity() {
  return Eigen::Matrix<double, N, N>::Identity();
}

template<int N>
double determinant(matrix_fixed_t<N, N> const& A) {
  return A.determinant();
}

inline vector_1_t column_vector
(double const& x1) {
  vector_1_t ret;
  ret(0) = x1;
  return ret;
}

inline vector_2_t column_vector
(double const& x1, double const& x2) { 
  vector_2_t ret;
  ret(0) = x1;
  ret(1) = x2;
  return ret;
}

inline vector_3_t column_vector
(double const& x1, double const& x2, double const& x3) {
  vector_3_t ret;
  ret(0) = x1;
  ret(1) = x2;
  ret(2) = x3;
  return ret;
}

template<typename M>
auto transpose(M const& m) -> decltype(m.transpose())
{ return m.transpose(); }

template<typename V1, typename V2>
double inner_product(V1 const& v1, V2 const& v2)
{ return v1.dot(v2); }

template<typename V1>
double norm_2(V1 const& v)
{ return v.norm(); }

// Returns a Matrix copy of v in row-major form.
// Eigen::Map<> apparently only supports column-major layout.
inline matrix_t to_matrix(
    std::vector<double> const& v, long const rows, long const cols) {
  cpl::util::verify(rows >= 1, 
      "vector to matrix conversion: need at least one row");
  cpl::util::verify(cols >= 1,
      "vector to matrix conversion: need at least one column");
  cpl::util::verify(static_cast<long>(v.size()) == rows * cols,
      "vector to matrix conversion: size mismatch");

  // We trust the temporary Eigen::Map doesn't change v's data.
  return Eigen::Map<matrix_t>(
      const_cast<std::vector<double>&>(v).data(), rows, cols).transpose();
}

#if 0
template<typename V1>
double norm_inf(V1 const& v)
{ return v.lpNorm<Eigen::Infinity>(1); }
#endif

} // namespace matrix_wrapper

} // namespace cpl

namespace Eigen {

// TODO: Get rid of this one---it's somewhat dangerous due to surprising
// operator precedence.  Always use with parentheses.
template<typename T, int M, int N>
double operator|(Matrix<T, M, N> const& v1, Matrix<T, M, N> const& v2)
{ return v1.dot(v2); }

//
// Less-than operator based on lexicographic comparison starting at X(1,1) and
// proceeding rows first.
//

template<typename T, int M, int N>
bool operator<(Eigen::Matrix<T, M, N> const& A,
               Eigen::Matrix<T, M, N> const& B) {

  assert(A.rows() == B.rows());
  assert(A.cols() == B.cols());

  for( long j = 0 ; j < A.cols(); ++j ) {
  for( long i = 0 ; i < A.rows(); ++i ) {
    if( A( i , j ) < B( i , j ) ) { return true  ; }
    if( A( i , j ) > B( i , j ) ) { return false ; }
  }
  }

  return false;

}

} // namespace Eigen


#endif // CPP_LIB_MATRIX_WRAPPER_H
