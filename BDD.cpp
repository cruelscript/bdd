#pragma comment(lib, "bdd.lib")

#include <iostream>
#include <fstream>
#include <cmath>
#include "bdd.h"
#include <vector>

const size_t N = 9;
const size_t M = 4;
const size_t K = 24;
const size_t LOG_N = static_cast<size_t>(std::ceil(std::log2(N)));
const size_t VAR_NUM = N * M * LOG_N;
const size_t ROW_SIZE = static_cast<int>(std::sqrt(N));

// функция, используемая для вывода решений
void fun(char* varset, int size);

enum Neighbour
{
  LEFT,
  RIGHT
};

void constraint1(const bdd p[M][N][N], bdd& F, int property, int object, int value);
void constraint2(const bdd p[M][N][N], bdd& F, int property1, int value1, int property2, int value2);
void constraint3(const bdd p[M][N][N], bdd& F, int property1, int value1, int property2, int value2, Neighbour neighbour);
void constraint4(const bdd p[M][N][N], bdd& F, int property1, int value1, int property2, int value2);
void constraint5(const bdd p[M][N][N], bdd& F);
void constraint6(const bdd p[M][N][N], bdd& F);
void constraint7(const bdd p[M][N][N], bdd& F);

void init(bdd p[M][N][N]);


int main()
{
  bdd_init(50000000, 1000000);
  bdd_setcacheratio(1000000);
  bdd_setvarnum(VAR_NUM);

  bdd p[M][N][N];
  init(p);

  bdd F = bddtrue;

  constraint6(p, F);

  // ограничения типа 1.
  constraint1(p, F, 1, 5, 2); 
  constraint1(p, F, 3, 4, 8);

  // дополнительные ограничения типа 1
  constraint1(p, F, 2, 0, 6);
  constraint1(p, F, 0, 8, 6);
  constraint1(p, F, 3, 1, 6);
  constraint1(p, F, 1, 7, 1);

  // ограничения типа 2.
  constraint2(p, F, 1, 4, 2, 3); 
  constraint2(p, F, 3, 4, 0, 1); 
  constraint2(p, F, 0, 0, 2, 4); 
  constraint2(p, F, 1, 6, 0, 7);

  // дополнительные ограничения типа 2.
  constraint2(p, F, 0, 2, 1, 0);
  constraint2(p, F, 2, 2, 3, 2);
  constraint2(p, F, 0, 2, 3, 5);
  constraint2(p, F, 2, 1, 3, 4);

  // ограничения типа 3.
  constraint3(p, F, 0, 3, 0, 2, LEFT); 
  constraint3(p, F, 1, 7, 1, 8, RIGHT);
  constraint3(p, F, 3, 2, 3, 8, LEFT);
  constraint3(p, F, 1, 1, 0, 4, LEFT);
  constraint3(p, F, 3, 1, 2, 0, RIGHT);

  // ограничения типа 4.
  constraint4(p, F, 0, 5, 0, 8);
  constraint4(p, F, 2, 5, 1, 0);
  constraint4(p, F, 0, 3, 1, 3);
  constraint4(p, F, 3, 7, 2, 8);

  // Дополнительные ограничения типа 4.
  constraint4(p, F, 1, 8, 3, 0);

  constraint5(p, F);

  constraint7(p, F);

  // Вывод результатов
  std::cout << bdd_satcount(F) << " solution(s):\n";
  bdd_allsat(F, fun);

  bdd_done();

  return 0;
}

void init(bdd p[M][N][N])
{
  size_t I = 0;
  for (size_t i = 0; i < N; ++i)
  {
    for (size_t j = 0; j < N; ++j)
    {
      for (size_t k = 0; k < M; ++k)
      {
        p[k][i][j] = bddtrue;
        for (size_t t = 0; t < LOG_N; ++t)
        {
          p[k][i][j] &= (j >> t) & 1
            ? bdd_ithvar(I + LOG_N * k + t)
            : bdd_nithvar(I + LOG_N * k + t);
        }
      }
    }
    I += LOG_N * M;
  }
}


std::vector<char> var(VAR_NUM);

void print()
{
  for (size_t i = 0; i < N; i++)
  {
    std::cout << i << ": ";
    for (size_t j = 0; j < M; j++)
    {
      int J = i * M * LOG_N + j * LOG_N;
      int num = 0;
      for (size_t k = 0; k < LOG_N; k++) num += static_cast<size_t>(var[J + k] << k);
      std::cout << num << ' ';
    }
    std::cout << "\n";
  }
  std::cout << "\n";
}


void build(char* varset, unsigned n, unsigned I)
{
  if (I == n - 1)
  {
    if (varset[I] >= 0)
    {
      var[I] = varset[I];
      print();
      return;
    }

    var[I] = 0;
    print();
    var[I] = 1;
    print();
    return;
  }

  if (varset[I] >= 0)
  {
    var[I] = varset[I];
    build(varset, n, I + 1);
    return;
  }

  var[I] = 0;
  build(varset, n, I + 1);
  var[I] = 1;
  build(varset, n, I + 1);
}


void fun(char* varset, int size)
{
  build(varset, size, 0);
}


void constraint1(const bdd p[M][N][N], bdd& F, int property, int object, int value)
{
  F &= p[property][object][value];
  std::cout << "CS 1 done\n";
}

void constraint2(const bdd p[M][N][N], bdd& F, int property1, int value1, int property2, int value2)
{
  for (size_t i = 0; i < N; ++i)
  {
    F &= !(p[property1][i][value1] ^ p[property2][i][value2]);
  }
  std::cout << "CS 2 done\n";
}

void constraint3(const bdd p[M][N][N], bdd& F, int property1, int value1, int property2, int value2, Neighbour neighbour)
{
  switch (neighbour)
  {
  case LEFT:
    for (size_t i = 0; i < N; ++i)
    {
      if (i % ROW_SIZE != 0 && i > ROW_SIZE)
      {
        F &= !(p[property1][i][value1] ^ p[property2][i - 1 - ROW_SIZE][value2]);
      }
      else if (i % ROW_SIZE == 0 || i <= ROW_SIZE)
      {
        F &= !p[property1][i][value1];
      }
      else if ((i + 1) % ROW_SIZE == 0 || i >= N - ROW_SIZE)
      {
        F &= !p[property2][i][value2];
      }
    }
    break;

  case RIGHT:
    for (size_t i = 0; i < N; ++i)
    {
      if ((i + 1) % ROW_SIZE != 0 && i >= ROW_SIZE)
      {
        F &= !(p[property1][i][value1] ^ p[property2][i + 1 - ROW_SIZE][value2]);
      }
      else if ((i + 1) % ROW_SIZE == 0 || i < ROW_SIZE)
      {
        F &= !p[property1][i][value1];
      }
      else if (i % ROW_SIZE == 0 || i <= N - ROW_SIZE)
      {
        F &= !p[property2][i][value2];
      }
    }
  }
  std::cout << "CS 3 done\n";
}


void constraint4(const bdd p[M][N][N], bdd& F, int property1, int value1, int property2, int value2)
{
  bdd temp1 = bddtrue;
  constraint3(p, temp1, property1, value1, property2, value2, LEFT);

  bdd temp2 = bddtrue;
  constraint3(p, temp2, property1, value1, property2, value2, RIGHT);

  F &= temp1 | temp2;

  std::cout << "CS 4 done\n";;
}


void constraint5(const bdd p[M][N][N], bdd& F)
{
  for (size_t i = 0; i < N - 1; i++)
  {
    for (size_t j = i + 1; j < N; j++)
    {
      for (size_t k = 0; k < N; k++)
      {
        for (size_t m = 0; m < M; m++)
        {
          F &= (p[m][i][k] >> !p[m][j][k]);
        }
      }
    }
  }
  std::cout << "CS 5 done\n";
}


void constraint6(const bdd p[M][N][N], bdd& F)
{
  for (size_t i = 0; i < N; ++i)
  {
    bdd temp = bddtrue;
    for (size_t m = 0; m < M; ++m)
    {
      bdd current = bddfalse;
      for (size_t j = 0; j < N; j++)
      {
        current |= p[m][i][j]; 
      }
      temp &= current;
    }
    F &= temp;
  }
  std::cout << "CS 6 done\n";
}


void constraint7(const bdd p[M][N][N], bdd& F)
{
  for (size_t i = 0; i < N - ROW_SIZE; ++i) // цикл по возможным соседям
  {
    bdd sum = bddfalse;

    // перебираем все возможные суммы значений свойств
    for (size_t j1 = 0; j1 < N; j1++)
    {
      for (size_t j2 = 0; j2 < N; j2++)
      {
        for (size_t j3 = 0; j3 < N; j3++)
        {
          for (size_t j4 = 0; j4 < N; j4++)
          {
            if (j1 + j2 + j3 + j4 <= K)
            {
              sum |= p[0][i][j1] & p[1][i][j2] & p[2][i][j3] & p[3][i][j4];
            }
          }
        }
      }
    }
    F &= sum;
  }
  std::cout << "CS 7 done\n";
}