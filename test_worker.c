#include <stdio.h>

extern int num;
extern int ncc;

int write_my_res(int i, int j, double res[][num]);

int main(){
  double res[num][num];
  int i=0;
  int j=0;
  write_my_res(i, j, res);
  return 0;
}
