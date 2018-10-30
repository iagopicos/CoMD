CoMD
====

Classical molecular dynamics proxy application.

This is CoMD version 1.1
------------------------

CoMD is a reference implementation of typical classical molecular
dynamics algorithms and workloads.  It is created and maintained by
ExMatEx: Exascale Co-Design Center for Materials in Extreme Environments
(<a href="http://exmatex.org">exmatex.org</a>).  The
code is intended to serve as a vehicle for co-design by allowing
others to extend and/or reimplement it as needed to test performance of 
new architectures, programming models, etc.

To view the generated Doxygen documentation for CoMD, please visit
<a href="http://exmatex.github.io/CoMD/doxygen-mpi/index.html">exmatex.github.io/CoMD/doxygen-mpi/index.html</a>.

To contact the developers of CoMD send email to exmatex-comd@llnl.gov.

ZMQ Add-on
----------

This version of CoMD was modified to generate a ZeroMQ data stream
with the main results (basically, atom's id&position) obtained for
each timestep (or a set of timesteps).

This ZMQ support was only added to the MPI version. The compilation
flag `DO_ZMQ=ON/OFF` enables/disables this option in the Makefile.

When compiled with ZMQ support, some extra parameters are needed to set
the hostname(s)/port(s) (`--help` to list them). Examples of use:

- Serial execution with 256000 atoms (4 x 40 x 40 x 40) using ZMQ with
  default port (9000):

  ```bin/CoMD-serial --hostDir ~/tmp/ --nx 40 --ny 40 --nz 40```

  With `--hostDir` we indicate the path where a file for each port used
  sets the hostname (eg. localhost) for the ZMQ connection. In this example
  this file is needed: ${HOME}/tmp/9000

- 4 MPI processes using 4 different ZMQ ports:

  ```mpirun -np 4 bin/CoMD-mpi --hostDir ~/tmp/ --nx 40 --ny 40 --nz 40 --xproc 2 --yproc 2 --zproc 1```

  4 files in ${HOME}/tmp are needed:
    ${HOME}/tmp/9000
    ${HOME}/tmp/9001
    ${HOME}/tmp/9002
    ${HOME}/tmp/9003

Credits for this add-on: Omar A. Mures, Henrique C. Zanúz, Bruno Raffin
and Emilio J. Padrón