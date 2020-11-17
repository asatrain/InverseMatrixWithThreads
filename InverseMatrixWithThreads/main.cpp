#include <iostream>
#include <string>
#include <iomanip>
#include <thread>

using namespace std;

/**
 * Метод получения подматрицы из матрицы
 * @param matrix входная матрица
 * @param i номер строки для удаления
 * @param j номер столбца для удаления
 * @param n размерность matrix
 */
int** getSubMatrix(int** matrix, int i, int j, int n) {
    // Выделение памяти для подматрицы
    int** subMatrix = new int* [n - 1];
    for (int k = 0; k < n - 1; ++k) {
        subMatrix[k] = new int[n - 1];
    }

    int subMatrixRow = 0, subMatrixCol = 0;
    // Проход по всем элементам матрицы
    for (int matrixRow = 0; matrixRow < n; matrixRow++) {
        for (int matrixCol = 0; matrixCol < n; matrixCol++) {
            // Копирование тех элементов, которые не лежат в строке i и столбце j matrix
            if (matrixRow != i && matrixCol != j) {
                subMatrix[subMatrixRow][subMatrixCol++] = matrix[matrixRow][matrixCol];

                // Переход на новую строку в подматрице
                if (subMatrixCol == n - 1) {
                    subMatrixCol = 0;
                    subMatrixRow++;
                }
            }
        }
    }

    return subMatrix;
}

/**
 * Метод нахождения определителя матрицы
 * @param matrix матрица
 * @param n размерность
 */
int determinant(int** matrix, int n) {
    // Базовый случай
    if (n == 1)
        return matrix[0][0];

    int D = 0;

    // Подматрица matrix
    int** subMatrix;

    // Знак минора при разложении по первой строке
    int sign = 1;

    // Проход по первой строке
    for (int j = 0; j < n; j++) {
        subMatrix = getSubMatrix(matrix, 0, j, n);
        D += sign * matrix[0][j]
             * determinant(subMatrix, n - 1);

        // Смена знака и высвобождение памяти
        sign = -sign;
        for (int rowInd = 0; rowInd < n - 1; ++rowInd) {
            delete[] subMatrix[rowInd];
        }
        delete[] subMatrix;
    }

    return D;
}

/**
 * Метод вывода целочисленной матрицы
 * @param matrix матрица
 * @param n размерность
 */
void display(int** matrix, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++)
            cout << matrix[i][j] << '\t';
        cout << endl;
    }
}

/**
 * Метод вывода вещественной матрицы
 * @param matrix матрица
 * @param n размерность
 */
void display(double** matrix, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++)
            cout << matrix[i][j] << '\t';
        cout << endl;
    }
}

// Максимальное по модулю значение элементов матрицы
const int maxMatrixElementAbsValue = 5;

/**
 * Метод генерации случайной целочисленной матрицы
 * @param n размерность
 */
int** generateRandomMatrix(int n) {
    int** matrix = new int* [n];
    for (int i = 0; i < n; ++i) {
        matrix[i] = new int[n];
        for (int j = 0; j < n; ++j) {
            int randNum = rand() % (maxMatrixElementAbsValue + 1);
            matrix[i][j] = rand() % 2 ? randNum : -randNum;
        }
    }
    return matrix;
}

// Потоки
thread* threads;


// Генерируемая матрица
int** matrix;

// Обратная матрица
double** inverseMatrix;

// Коэффициент перед транспонированной матрицей алгебраических дополнений
double multiplier;

/**
 * Метод параллельного заполнения обратной матрицы
 * @param threadsCount кол-во потоков
 * @param threadInd индекс потока, выполняющего заполнение
 * @param n размерность
 */
void fillInverseMatrix(int threadsCount, int threadInd, int n) {
    // Количество строк для заполнения каждым потоком, кроме последнего
    int linesOptimalPortion = n / threadsCount;

    // В случае, когда потоков более чем достаточно, игнорируются лишние потоки, а остальные потоки
    // заполняют по одной строке
    if (linesOptimalPortion == 0) {
        linesOptimalPortion = 1;
        if (threadInd >= n)
            return;
    }

    // В условии цикла учитывается возможная потребность в заполнении сверхнормативного кол-ва строк последним потоком
    for (int i = linesOptimalPortion * threadInd;
         i < linesOptimalPortion * (threadInd + 1) || (threadInd + 1 == threadsCount && i < n); ++i) {
        int sign = i % 2 ? -1 : 1;
        for (int j = 0; j < n; ++j) {
            int** subMatrix = getSubMatrix(matrix, i, j, n);
            inverseMatrix[j][i] = sign * determinant(subMatrix, n - 1) * multiplier;

            // Смена знака и высвобождение памяти
            sign = -sign;
            for (int rowInd = 0; rowInd < n - 1; ++rowInd) {
                delete[] subMatrix[rowInd];
            }
            delete[] subMatrix;
        }
    }
}

/**
 * Метод высвобождения памяти после успешного завершения программы
 * @param n размерность созданных матриц
 */
void cleanMemory(int n) {
    for (int rowInd = 0; rowInd < n; ++rowInd) {
        delete[] matrix[rowInd];
    }
    delete[] matrix;
    for (int rowInd = 0; rowInd < n; ++rowInd) {
        delete[] inverseMatrix[rowInd];
    }
    delete[] inverseMatrix;
    delete[] threads;
}

const int maxN = 10;
const int maxThreadsCount = 1000;

/**
 * @param argv argv[1] = n - размерность матрицы, argv[2] = threadCount - кол-во потоков
 */
int main(int argc, char* argv[]) {
    // Добавление псевдослучайности и устанавливдение формата вывода вещественных чисел
    srand(time(nullptr));
    cout << fixed << setprecision(3);

    // Проверка корректности аргументов командной строки
    if (argc != 3) {
        cout << "Incorrect amount of input arguments." << endl;
        return 1;
    }
    int n;
    int threadsCount;
    bool inputCorrectness = true;
    try {
        n = stoi(argv[1]);
        threadsCount = stoi(argv[2]);
    } catch (...) {
        inputCorrectness = false;
    }
    if (!(n >= 1 && n <= maxN && threadsCount >= 1 && threadsCount <= maxThreadsCount && inputCorrectness)) {
        cout << "Incorrect input arguments." << endl;
        return 1;
    }

    // Генерация матрицы
    matrix = generateRandomMatrix(n);
    cout << "Generated matrix:" << endl;
    display(matrix, n);

    // Проверка обратимости матрицы
    int matrixDet = determinant(matrix, n);
    if (matrixDet == 0) {
        cout << "Determinant of the generated matrix = 0, there is no inverse matrix." << endl;
        for (int rowInd = 0; rowInd < n; ++rowInd) {
            delete[] matrix[rowInd];
        }
        delete[] matrix;
        return 1;
    }

    // Выделение памяти для обратной матрицы
    inverseMatrix = new double* [n];
    for (int i = 0; i < n; ++i) {
        inverseMatrix[i] = new double[n];
    }

    // Подсчет коэффициента перед транспонированной матрицей алгебраических дополнений и
    // подсчет обратной матрицы с помощью потоков (с обработкой случая n = 1)
    multiplier = 1 / (double) matrixDet;
    if (n > 1) {
        threads = new thread[threadsCount];
        for (int i = 0; i < threadsCount; ++i) {
            threads[i] = thread(fillInverseMatrix, threadsCount, i, n);
        }
        for (int i = 0; i < threadsCount; ++i) {
            threads[i].join();
        }
    } else {
        inverseMatrix[0][0] = multiplier;
    }

    // Вывод обратной матрицы и высвобожение памяти
    cout << "Inverse matrix:" << endl;
    display(inverseMatrix, n);
    cleanMemory(n);

    return 0;
}