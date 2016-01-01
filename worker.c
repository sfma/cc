#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int length=40000;
int num=10000;
int num1=2000;
//int datablocklen=2*num1*length+2;
int datablocklen=160000002;
int write_my_res(int i, int j, double *res);

int NoMoreTask(double *array_task){
  int status=1;
  for(int i=0;i<datablocklen;i++){
    if(array_task[i]!=0){
      status=0;
      break;
    }
  }
  return status;
}

int kernel_func(double *array_task ){
  double *myres=(double *)malloc(num1*num1*sizeof(double));
  int begin_i=(int)(array_task[datablocklen-2]);
  int begin_j=(int)(array_task[datablocklen-1]);

  int current_i=begin_i;
  int current_j=begin_j;

  double *arr_a=(double *)malloc(length*sizeof(double));
  double *arr_b=(double *)malloc(length*sizeof(double));
  for(int x=0;x<num1;x++){
    //Copy the first matrix
    for(int k=0;k<length;k++){
      arr_a[k]=array_task[x*length+k];
    } 
    //mean
    double mean_a=0;
    for(int p=0;p<length;p++){
        mean_a+=arr_a[p];
    }
    mean_a/=length;
    //variance
    double variance_a=0;
    for(int p=0;p<length;p++){
      variance_a+=(arr_a[p]-mean_a)*(arr_a[p]-mean_a);
    }
    double std_a=sqrt(variance_a);

    for(int y=0;y<num1;y++){
      //Copy the second matrix
      for(int k=0;k<length;k++){
        arr_b[k]=array_task[y*length+k];
      }
      //Calculate the mean
      double mean_b=0;
          for(int p=0;p<length;p++){
        mean_b+=arr_b[p];
      }
      mean_b/=length;
      //Calculate the variance and standardDeviation
      double variance_b=0;
      for(int p=0;p<length;p++){
        variance_b+=(arr_b[p]-mean_b)*(arr_b[p]-mean_b);
      }
      double std_b=sqrt(variance_b);

      //Correlation coefficient
      double cc=0;
      for(int p=0;p<length;p++){
        cc+=(arr_a[p]-mean_a)*(arr_b[p]-mean_b);
      }
      cc/=std_a;
      cc/=std_b;
      int mi=(int)(begin_i/num1/length);
      int mj=(int)(begin_j/num1/length);
      //printf("i:%d, j: %d, cc: %f\n", mi, mj, cc);
      myres[x*num1+y]=cc;
      current_j+=length;
    }
    current_i+=length;
  }
  
  free(arr_a);
  free(arr_b);
  write_my_res((int)(begin_i/num1/length), (int)(begin_j/num1/length), myres);
  free(myres);
  return 0;
}

int write_my_res(int i, int j, double *res){
  char filename[20]="res";
  int size=20;
  char str_i[10], str_j[10];
  snprintf(str_i, 10, "%d", i);
  snprintf(str_j, 10, "%d", j);
  strcat(filename, "_");
  strcat(filename, str_i);
  strcat(filename, "_");
  strcat(filename, str_j);
  //printf("filename: %s\n", filename);
  FILE *f;
  f=fopen(filename, "wb+");
  //write the data to the file
  //If needed, index i and j should be written before the cc data.
  //fwrite(res, num1*num1*sizeof(double), 1, f);
  fwrite(res, num1*num1*sizeof(double), 1, f);
  fclose(f);
  return 0;
}


int serial_driver(){
  double arr[num*length];
  double res[num][num];
  //Initialization of res array
  for(int i=0;i<num;i++){
    for(int j=0;j<num;j++){
      res[i][j]=0;
    }
  }
  //Assignment of array
  for(int i=0;i<(num*length);i++){
    arr[i]=i;
  }
  int ntile=(int)(num/num1);
  double array_task[datablocklen];
  for(int i=0;i<ntile;i++){
    for(int j=0;j<ntile;j++){
      array_task[datablocklen-2]=i*num1*length;
      array_task[datablocklen-1]=j*num1*length;
      
      for(int k=0;k<num1*length;k++){
        array_task[k]=arr[i*num1*length+k];
        array_task[num1*length+k]=arr[j*num1*length+k];
      }
      
      //kernel_func(array_task);
    }
  }
  return 0;
}

