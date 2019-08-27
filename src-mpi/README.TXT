This is CoMD version 1.1
========================

To contact the developers of CoMD send email to exmatex-comd@llnl.gov.

CoMD is a reference implementation of typical classical molecular
dynamics algorithms and workloads.  It is created and maintained by
The Exascale Co-Design Center for Materials in Extreme Environments
(ExMatEx).  http://codesign.lanl.gov/projects/exmatex.  The
code is intended to serve as a vehicle for co-design by allowing
others to extend and/or reimplement it as needed to test performance of 
new architectures, programming models, etc.

The current version of CoMD is available from:

https://github.com/exmatex/CoMD


Obtaining Documentation
=======================

CoMD documentation is produced by doxygen (www.doxygen.org).  
In order to generate the call graphs, you will also need graphviz 
installed. Alternatively, you can set 

HAVE_DOT = NO 

in the file 'Doxyfile' in the source directory. 

To build the documentation, navigate to the source directory 
(src-mpi) and run the command:

$ doxygen

To read the documenation, point a browser at 'html/index.html'.  
If you are unable to run doxygen on your local system you can download a
pre-built html directory from

XXXXXXXXXXXXXXXXXXXXXXX

Or, point your browser at

XXXXXXXXXXXXXXXXXXXXXX


Building CoMD
=============

CoMD is written with portability in mind and should compile using
practically any compiler that implements the C99 standard.  You will
need to create a Makefile by copying the sample provided with the
distribution (Makefile.vanilla).

$ cp Makefile.vanilla Makefile

and use the make command to build the code

$ make

The sample Makefile will compile the code on many platforms.  See
comments in Makefile.vanilla for information about specifying the name
of the C compiler and/or additional compiler switches that might be
necessary for your platform.


What's New
==========

For information about what has changed in this version of CoMD, please
consult the html documentation.


ZMQ Add-on
==========

This version of CoMD was modified to generate a ZeroMQ data stream
with the main results (basically, atom's id&position) obtained for
each timestep (or a set of timesteps).

This ZMQ support was only added to the MPI version. The compilation
flag `DO_ZMQ=ON/OFF` enables/disables this option in the Makefile.

When compiled with ZMQ support, some extra parameters are needed to set
the recipient's hostname(s)/port(s) (`--help` to list them). Examples of use:

- Serial execution with 256000 atoms (4 x 40 x 40 x 40) using ZMQ with
  default port (9000):

  ```bin/CoMD-serial --hostDir ~/tmp/ --nx 40 --ny 40 --nz 40```

  With `--hostDir` we indicate the path where a file for each port used
  sets the destination hostname (eg. ) for the ZMQ connection. In this example
  this file is needed: ${HOME}/tmp/9000

  ```
     cat ~/tmp/9000

     dahu-7.grenoble.grid5000.fr
  ```

- 4 MPI processes using 4 different ZMQ ports:

  ```mpirun -np 4 bin/CoMD-mpi --hostDir ~/tmp/ --nx 40 --ny 40 --nz 40 --xproc 2 --yproc 2 --zproc 1```

  4 files in ${HOME}/tmp are needed:
    ${HOME}/tmp/9000
    ${HOME}/tmp/9001
    ${HOME}/tmp/9002
    ${HOME}/tmp/9003

Credits for this add-on: Omar A. Mures, Henrique C. Zanúz, Bruno Raffin
and Emilio J. Padrón
