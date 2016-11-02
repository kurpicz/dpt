#!/bin/bash

###### PBS options for a parallel job using Infiniband ###########################

### Select the queue where to submit the job to
### Default: short
#PBS -q long

### Expected run time of the job.
### The job will be terminated if this time is exceeded.
### This is a parallel job, 
### - assuming you want to start 8 compute processes in parallel with infiniband interconnect
### - and consuming a maximum of 10 hours walltime 
### Default: 
#PBS -l walltime=10:00:00,nodes=8:ib

### Define the name of the job.
### Default: name of the PBS script
#PBS -N MyParallel_IB_Job

### Specify whether and on what event you want to use e-mail notification:
### [b]egin, [a]bort, [e]nd
### Default: no notification
#PBS -m bae

### e-mail address for job notification messages.
### If no full name is given (e.g. just the username), a .forward file must be
### configured in the user's home directory, otherwise the mail will be discarded.
### Default: username
### Attention: Be sure to specify an existing e-mail address
### ---------  instead of the template's address below !!!
#PBS -M someone@somewhere.edu

### File to redirect standard output of the job to.
### Make sure to use a unique file name.
### If you do not care about standard output, use "PBS -o /dev/null"
### Default: Name of jobfile plus ".o" plus number of PBS job
#PBS -o output.$PBS_JOBID.dat

### This option redirects stdout and stderr into the same output file
### (see PBS option -o).
#PBS -j oe

###### End of PBS options #######################################################


### The following command, if uncommented by deleting the hash sign in front of 'cat',
### saves the name of the compute nodes (to which the job is submitted by the batch system).
### This information may be useful when debugging.
### This information can also be retrieved while the job is being executed via "qstat -f jobid".
###
### Be sure to use a unique file name (!), though, to avoid concurrent write access
### which may happen when multiple jobs of yours are started simultaneously.
#cat $PBS_NODEFILE > $HOME/pbs-machine.$PBS_JOBID

### Go to the application's working directory
cd $HOME/workdir

### Start the application via the mpirun program,
### the number of processes must be identical to the one given above.
mpirun -np 8 ./parallelapp