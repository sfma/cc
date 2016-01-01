#include <string.h>
#include <mpi.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

extern int length;
extern int num;
extern int num1;
extern int datablocklen;
int NoMoreTask(double *array_task);
int kernel_func(double *array_task );

int main(){
  char name[20];
  int name_len;
  char r='R'; // A request signal
  
  int myrank;
  int mysize; //# of processes
  MPI_Status status;
  //MPI_Request request_send;
  MPI_Request request_recv;
  MPI_Init(NULL, NULL);
  MPI_Comm_rank(MPI_COMM_WORLD,&myrank);
  MPI_Comm_size(MPI_COMM_WORLD,&mysize);
  MPI_Get_processor_name(name,  &name_len);
  if(myrank==0){
    //printf("Number of processors: %d.\n", mysize); 
    //printf("Processor name: %s.\n", name);
    double *arr=(double *)malloc(num*length*sizeof(double));
    //Assignment of array
    for(int i=0;i<(num*length);i++){
      arr[i]=i;
    }
    double *res=(double *)malloc(num*num*sizeof(double));
    for(int i=0;i<(num*num);i++){
        res[i]=0;
    }
    clock_t begin, end;
    double time_spent;
    begin=clock();

    int ntile=(int)(num/num1);
    for(int i=0;i<ntile;i++){
      for(int j=0;j<ntile;j++){
      //for(int j=i;j<ntile;j++){
        double *arr_to_send=(double *)malloc(datablocklen*sizeof(double));
        MPI_Recv(&r,1,MPI_CHAR,MPI_ANY_SOURCE,1,MPI_COMM_WORLD,&status);
        arr_to_send[datablocklen-2]=i*num1*length;
        arr_to_send[datablocklen-1]=j*num1*length;
        for(int k=0;k<num1*length;k++){
          arr_to_send[k]=arr[i*num1*length+k];
          arr_to_send[num1*length+k]=arr[j*num1*length+k];
        }
        //tag=0, send data.
        MPI_Send(arr_to_send,datablocklen,MPI_DOUBLE,status.MPI_SOURCE,0,MPI_COMM_WORLD );
        free(arr_to_send);
      }
    }
    free(arr);
    //send to all processes the no more task message.
    double *arr_to_send=(double *)malloc(datablocklen*sizeof(double));
    for(int i=0;i<datablocklen;i++){
      arr_to_send[i]=0;
    }
    clock_t begin1, end1;
    double time_spent1;
    begin1=clock();
    for(int i=1;i<mysize;i++){
      //tag=1, send no more task message.
      MPI_Send(arr_to_send,datablocklen,MPI_DOUBLE,i,0,MPI_COMM_WORLD);
    }
    end1=clock();
    time_spent1=(double)(end1-begin1)/CLOCKS_PER_SEC;
    printf("time_spent1: %f.\n", time_spent1);
    free(arr_to_send);
    //Collect the seperate results into a single file.
    clock_t rw_begin, rw_end;
    double time_spent_for_rw;
    rw_begin=clock();
    double *myres=(double *)malloc(num1*num1*sizeof(double));
    for(int i=0; i<ntile; i++){
      for(int j=0; j<ntile; j++){
        char filename[20]="res";
        int size=20;
        char str_i[10], str_j[10];
        snprintf(str_i, 10, "%d", i);
        snprintf(str_j, 10, "%d", j);
        strcat(filename, "_");
        strcat(filename, str_i);
        strcat(filename, "_");
        strcat(filename, str_j);
        FILE *f_part;
        f_part=fopen(filename, "rb");
        fread(myres, num1*num1*sizeof(double),1, f_part);
        for(int p=i*num1;p<(i*num1+num1);p++){
          for(int q=j*num1;q<(j*num1+num1);q++){
            res[p*num+q]=myres[(p-i*num1)*num1+(q-j*num1)];
          }
        }
        fclose(f_part);
      }
    }
    free(myres);
    /*
    for(int i=0;i<num;i++){
      for(int j=0;j<num;j++){
        //printf("%f\t", res[i][j]);
      }
      //printf("\n");
    }
    */
    FILE *f;
    f=fopen("final_res", "wb");
    //fwrite(res, num*num*sizeof(double), 1, f);
    fwrite(res, num*num*sizeof(double), 1, f);
    fclose(f);

    //time to free res
    free(res);

    rw_end=clock();
    time_spent_for_rw=(double)(rw_end-rw_begin)/CLOCKS_PER_SEC;
    printf("time spent for reading and writing: %f", time_spent_for_rw);
    end=clock();
    time_spent=(double)(end-begin)/CLOCKS_PER_SEC;
    printf("time spent: %f.\n",time_spent);
  }
  else{
    //printf("Process No.%d.\n",myrank);
    clock_t begin, end;
    double time_spent=0;
    begin=clock();
    clock_t computing_begin, computing_end;
    double time_spent_for_computing=0;
    clock_t trm_begin, trm_end;
    double time_spent_for_trm=0;
    int work_count=0;
    double *array_task=(double *)malloc(datablocklen*sizeof(double));
    array_task[0]=1;
    while(!NoMoreTask(array_task)){
      trm_begin=clock();
      MPI_Send(&r,1,MPI_CHAR,0,1,MPI_COMM_WORLD);//send a request signal
      MPI_Recv(array_task,datablocklen,MPI_DOUBLE,0,0,MPI_COMM_WORLD,&status);
      trm_end=clock();
      time_spent_for_trm+=(double)(trm_end-trm_begin)/CLOCKS_PER_SEC;
      if(!NoMoreTask(array_task)){
        computing_begin=clock();
        kernel_func(array_task);
        work_count+=1;
        computing_end=clock();
        time_spent_for_computing+=(double)(computing_end-computing_begin)/CLOCKS_PER_SEC;
        /*
        It's better to call the write function inside the kernel func instead of get the result from kernel_fun and pass it as the parameter to the write function here, at this place. So now, omit this part.
        int i,j;
        i=(int)(array_task[datablocklen-2]/num1/length);
        j=(int)(array_task[datablocklen-2]/num1/length);
        //write their computation results seperately
        write_my_res(i,j);*/
      }
    }
    //printf("Task finished.\n");
    end=clock();
    time_spent=(double)(end-begin)/CLOCKS_PER_SEC;
    printf("Processor %d spend %fs totally on computing, and %fs on transmission. This worker has completed %d missions. Its life span is %fs.\n", myrank, time_spent_for_computing, time_spent_for_trm, work_count, time_spent);
    free(array_task);
  }
  MPI_Finalize();
  return 0;
}
