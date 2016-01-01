#!/bin/bash
# #SBATCH --ntasks=4
# #SBATCH -t 00:10:00

#. ~/.bash_profile
#ls /usr/bin/modulecmd

#module load intel

#export I_MPI_PROCESS_MANAGER=mpd
#mpirun  mycc
#yhrun  -p ceshi -n 4 mycc
srun  -p ceshi -n 20 -t 00:10:00 mycc
rm res*
