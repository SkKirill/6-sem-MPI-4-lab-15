#include <iostream>
#include <vector>
#include <mpi.h>
#include <string>
#include <random>
#include <iomanip>

using namespace std;

const int rows = 6; // Константы чтобы задавать везде одинаковую матрицу
const int cols = 6;

// Процедура printArray - выводит матрицу в консоль
void printArray(int array[rows][cols], int rows)
{
	for (int i = 0; i < rows; i++)
	{
		for (int j = 0; j < cols; j++)
		{
			cout << setw(4) << array[i][j];
		}
		cout << endl;
	}
	cout << endl;
}


int main(int argc, char* argv[]) {
	int rank, size;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	/*инициализируем матрицу*/
	int array[rows][cols]{};

	/*заполняем матрицу случайными значениями*/
	srand(time(NULL) + rank);
	if (rank == 0) {
		for (int i = 0; i < rows; i++)
		{
			for (int j = 0; j < cols; j++)
			{
				array[i][j] = rand() % 100;
			}
		}
		printArray(array, rows);//вывод матрицы
	}

	int blocklengths[rows * 2 - 4]{}; // массив с длинами блоков
	int count = (rows * 2 - 4);
	int i = 0;
	blocklengths[i] = rows / 2;
	i++;
	for (; i < (count / 2) - 1; i++)
	{
		blocklengths[i] = 1;
	}
	i++;
	blocklengths[i] = rows / 2;
	i++;
	blocklengths[i] = rows / 2;
	for (; i < count - 1; i++)
	{
		blocklengths[i] = 1;
	}
	i++;
	blocklengths[count] = rows / 2;


	int displacements[rows * 2 - 4]{};
	count = (rows * 2 - 4);
    i = 0;
	displacements[i] = 0;
	i++;
	displacements[i] = cols;
	i++;
	for (; i < (count / 2) - 1; i++)
	{
		if (i % 2 == 0) {
			displacements[i] = displacements[i - 1] + cols / 2;
		}
		else {
			displacements[i] = displacements[i - 1] + (cols - cols / 2);
		}
	}
	i++;
	displacements[i] = displacements[i - 1] + (cols - cols / 2);
	i++;
	displacements[i] = displacements[i - 1] + cols + cols / 2;
	i++;
	displacements[i] = displacements[i - 1] + cols;
	for (; i < count; i++)
	{
		if (i % 2 == 0) {
			displacements[i] = displacements[i - 1] + cols / 2;
		}
		else {
			displacements[i] = displacements[i - 1] + (cols - cols / 2);
		}
	}

	MPI_Datatype MyDataType; // создаем свой тип

	MPI_Type_indexed(rows * 2 - 4, blocklengths, displacements, MPI_INT, &MyDataType); //функция-конструктор нашего типа
	MPI_Type_commit(&MyDataType);//регистрирует созданный производный тип.

	if (rank == 0)
	{
		MPI_Send(array, 1, MyDataType, 1, 0, MPI_COMM_WORLD); // первым процессом отправляем нашу матрицу преобразованную в наш тип
	}
	else if (rank == 1) // Процесс с рангом 1 принимает данные, отправленные процессом с рангом 0, используя пользовательский тип данных MyDataType
	{
		MPI_Recv(array, 1, MyDataType, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

		printArray(array, rows); // после чего выводим матрицу
	}

	MPI_Type_free(&MyDataType); //Освобождает ресурсы, связанные с пользовательским типом данных MyDataType.

	MPI_Finalize();
	return 0;
}

