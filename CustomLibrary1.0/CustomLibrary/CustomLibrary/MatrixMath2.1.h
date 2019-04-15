#ifndef MATRIXMATH21
#define MATRIXMATH21

#include <vector>
#include <ostream>

#include "RandomGenerator.h"
#include "Error.h"

template<class T>
class Matrix
{
private:

	T * m_matrix;
	int m_columns, m_rows;

	//Premade methods for modifyEach
	static void add(T &x, const T &y) { x += y; }
	static void subtract(T &x, const T &y) { x -= y; }
	static void multiply(T &x, const T &y) { x *= y; }
	static void become(T &x, const T &y) { x = y; }
	static void negative(T &x) { x = -x; }

	Matrix& modifyEach(void(*func)(T &, const T &), const T &num)
	{
		for (int i = 0; i < m_columns * m_rows; i++)
			func(m_matrix[i], num);
		return *this;
	}
	Matrix& modifyEach(void(*func)(T &, const T &), const T *mat)
	{
		for (int i = 0; i < m_columns * m_rows; i++)
			func(m_matrix[i], mat[i]);
		return *this;
	}

	//Methods for deep-copying pointer & vector
	Matrix& deepCopy(const T *from, T *to, const int &length)
	{
		for (int i = 0; i < length; i++)
			to[i] = from[i];
		return *this;
	}

public:

	//----------------------------------Constructor & Destructor----------------------------------

	Matrix(const int &columns, const int &rows, const T &num = 0)
		: m_columns{ columns }, m_rows{ rows }
	{
		if (columns <= 0 || rows <= 0)
			throw Error("Matrix Default Construcion -> Invalid parameters.");

		m_matrix = new T[m_columns * m_rows];
		for (int i = 0; i < m_columns * m_rows; i++)
			m_matrix[i] = num;
	}

	Matrix(const std::vector<T> &vect)
		: m_columns{ 1 }, m_rows{ (int)vect.size() }
	{
		m_matrix = new T[m_rows];
		for (int i = 0; i < m_rows; i++)
			m_matrix[i] = vect[i];
	}

	Matrix(Matrix &&mat)
		: m_rows{ mat.m_rows }, m_columns{ mat.m_columns }, m_matrix{ mat.m_matrix }
	{
		mat.m_matrix = nullptr;
	}

	Matrix(const Matrix &mat)
		: m_rows{ mat.m_rows }, m_columns{ mat.m_columns }
	{
		m_matrix = new T[m_rows * m_columns];
		for (int i = 0; i < m_rows * m_columns; i++)
			m_matrix[i] = mat.m_matrix[i];
	}

	~Matrix()
	{
		delete[] m_matrix;
	}

	//-------------------------------------Function Methods--------------------------------------

	T& accessLoc(const int &x, const int &y) { return m_matrix[x + m_columns * y]; }
	T& accessLoc(const int &xy) { return m_matrix[xy]; }

	Matrix matrixProd(const Matrix &mat2)
	{
		if (m_columns != mat2.m_rows)
			throw Error("Matrix Dotproduct -> Number of columns and rows of second matrix aren't the same.");

		Matrix temp(mat2.m_columns, m_rows);
		temp = 0;
		for (int i = 0; i < m_rows; i++)
			for (int c = 0; c < mat2.m_columns; c++)
				for (int f = 0; f < m_columns; f++)
					temp.m_matrix[i * temp.m_columns + c] += m_matrix[i * m_columns + f] * mat2.m_matrix[f * mat2.m_columns + c];
		return temp;
	}

	Matrix& randomizeAll(RandomGenerator<> &randNum, const T &min, const T &max)
	{
		for (int i = 0; i < m_columns * m_rows; i++)
			m_matrix[i] = randNum.randNumber<T>(min, max);
		return *this;
	}

	Matrix transpose()
	{
		Matrix<double> temp(m_rows, m_columns);
		for (int i = 0; i < m_columns; i++)
			for (int b = 0; b < m_rows; b++)
				temp.m_matrix[i * temp.m_columns + b] = m_matrix[b * m_columns + i];
		return temp;
	}

	Matrix& modifyEach(void(*func)(T &))
	{
		for (int i = 0; i < m_columns * m_rows; i++)
			func(m_matrix[i]);
		return *this;
	}

	const int size() const { return m_columns * m_rows; }

	//------------------------------------Operator Overloads-------------------------------------

	Matrix& operator=(const T &num) { modifyEach(become, num); return *this; }
	Matrix& operator=(const Matrix<T> &mat) 
	{ 
		m_columns = mat.m_columns;
		m_rows = mat.m_rows;

		deepCopy(mat.m_matrix, m_matrix, m_columns * m_rows); 
		return *this;
	}
	Matrix& operator=(const std::vector<T> &vec) 
	{ 
		m_columns = 1;
		m_rows = vec.size();

		deepCopy(&vec[0], m_matrix, m_rows);
		return *this; 
	}
	Matrix& operator=(Matrix<T> &&mat)
	{
		m_columns = std::move(mat.m_columns);
		m_rows = std::move(mat.m_rows);

		delete[] m_matrix;
		m_matrix = std::move(mat.m_matrix);
		mat.m_matrix = nullptr;
		return *this;
	}

	friend std::ostream& operator<<(std::ostream &out, const Matrix<T> &mat) 
	{ 
		out << mat.m_columns << " " << mat.m_rows << "\n";
		for (int i = 0; i < mat.m_columns * mat.m_rows; i++)
			out << mat.m_matrix[i] << " ";
		return out;
	}

	//Scalar

	Matrix operator+(const T &num) { Matrix<double> temp(m_columns, m_rows); deepCopy(m_matrix, temp.m_matrix, m_columns * m_rows); temp.modifyEach(add, num); return temp; }
	Matrix operator+(T &&num) { Matrix<double> temp(m_columns, m_rows); deepCopy(m_matrix, temp.m_matrix, m_columns * m_rows); temp.modifyEach(add, num); return temp; }
	Matrix& operator+=(const T &num) { modifyEach(add, num); return *this; }

	Matrix operator*(const T &num) { Matrix<double> temp(m_columns, m_rows); deepCopy(m_matrix, temp.m_matrix, m_columns * m_rows); temp.modifyEach(multiply, num); return temp; }
	Matrix operator*(T &&num) { Matrix<double> temp(m_columns, m_rows); deepCopy(m_matrix, temp.m_matrix, m_columns * m_rows); temp.modifyEach(multiply, num); return temp; }
	Matrix& operator*=(const T &num) { modifyEach(multiply, num); return *this; }

	Matrix operator-(const T &num) { Matrix<double> temp(m_columns, m_rows); deepCopy(m_matrix, temp.m_matrix, m_columns * m_rows); temp.modifyEach(subtract, num); return temp; }
	Matrix operator-(T &&num) { Matrix<double> temp(m_columns, m_rows); deepCopy(m_matrix, temp.m_matrix, m_columns * m_rows); temp.modifyEach(subtract, num); return temp; }
	Matrix& operator-=(const T &num) { modifyEach(subtract, num); return *this; }

	//Elementwise

	Matrix operator+(const Matrix<T> &mat)
	{
		if (m_columns != mat.m_columns || m_rows != mat.m_rows)
			throw Error("Matrix Plus Elementwise Operator -> columns or rows aren't the same with target matrix.");
		Matrix<double> temp(m_columns, m_rows);
		deepCopy(m_matrix, temp.m_matrix, m_columns * m_rows);
		temp.modifyEach(add, &mat.m_matrix[0]); return temp;
	}

	Matrix operator+(Matrix<T> &&mat)
	{
		if (m_columns != mat.m_columns || m_rows != mat.m_rows)
			throw Error("Matrix Move Plus Elementwise Operator -> columns or rows aren't the same with target matrix.");
		mat.modifyEach(add, &m_matrix[0]);
		return mat;
	}

	Matrix& operator+=(const Matrix<T> &mat)
	{
		if (m_columns != mat.m_columns || m_rows != mat.m_rows)
			throw Error("Matrix SinglePlus Elementwise Operator -> columns or rows aren't the same with target matrix.");
		modifyEach(add, &mat.m_matrix[0]);
		return *this;
	}

	//-----------//

	Matrix operator*(const Matrix<T> &mat)
	{
		if (m_columns != mat.m_columns || m_rows != mat.m_rows)
			throw Error("Matrix Multiply Elementwise Operator -> columns or rows aren't the same with target matrix.");
		Matrix<double> temp(m_columns, m_rows);
		deepCopy(m_matrix, temp.m_matrix, m_columns * m_rows);
		temp.modifyEach(multiply, &mat.m_matrix[0]);
		return temp;
	}

	Matrix operator*(Matrix<T> &&mat)
	{
		if (m_columns != mat.m_columns || m_rows != mat.m_rows)
			throw Error("Matrix Move Multiply Elementwise Operator -> columns or rows aren't the same with target matrix.");
		mat.modifyEach(multiply, &m_matrix[0]);
		return mat;
	}

	Matrix& operator*=(const Matrix<T> &mat)
	{
		if (m_columns != mat.m_columns || m_rows != mat.m_rows)
			throw Error("Matrix SingleMultiply Elementwise Operator -> columns or rows aren't the same with target matrix.");
		modifyEach(multiply, &mat.m_matrix[0]);
		return *this;
	}

	//-----------//

	Matrix operator-(const Matrix<T> &mat)
	{
		if (m_columns != mat.m_columns || m_rows != mat.m_rows)
			throw Error("Matrix Minus Elementwise Operator -> columns or rows aren't the same with target matrix.");
		Matrix<double> temp(m_columns, m_rows);
		deepCopy(m_matrix, temp.m_matrix, m_columns * m_rows);
		temp.modifyEach(subtract, &mat.m_matrix[0]);
		return temp;
	}

	Matrix operator-(Matrix<T> &&mat)
	{
		if (m_columns != mat.m_columns || m_rows != mat.m_rows)
			throw Error("Matrix Move Minus Elementwise Operator -> columns or rows aren't the same with target matrix.");
		mat.modifyEach(subtract, &m_matrix[0]);
		return mat;
	}

	Matrix& operator-=(const Matrix<T> &mat)
	{
		if (m_columns != mat.m_columns || m_rows != mat.m_rows)
			throw Error("Matrix SingleMinus Elementwise Operator -> columns or rows aren't the same with target matrix.");
		modifyEach(subtract, &mat.m_matrix[0]);
		return *this;
	}

	//Single

	Matrix operator-() { Matrix<double> temp(m_columns, m_rows); deepCopy(m_matrix, temp.m_matrix, m_columns * m_rows); temp.modifyEach(negative); return temp; }
};

#endif // !MATRIXMATH21
