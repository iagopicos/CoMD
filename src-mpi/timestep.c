/// \file
/// Leapfrog time integrator

#include "timestep.h"

#include "CoMDTypes.h"
#include "linkCells.h"
#include "parallel.h"
#include "performanceTimers.h"

#include <time.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef DO_ZMQ
#include "zmqstuff.h"
#endif


static void advanceVelocity(SimFlat* s, int nBoxes, real_t dt);
static void advancePosition(SimFlat* s, int nBoxes, real_t dt);

/// Advance the simulation time to t+dt using a leap frog method
/// (equivalent to velocity verlet).
///
/// Forces must be computed before calling the integrator the first time.
///
///  - Advance velocities half time step using forces
///  - Advance positions full time step using velocities
///  - Update link cells and exchange remote particles
///  - Compute forces
///  - Update velocities half time step using forces
///
/// This leaves positions, velocities, and forces at t+dt, with the
/// forces ready to perform the half step velocity update at the top of
/// the next call.
///
/// After nSteps the kinetic energy is computed for diagnostic output.
double timestep(SimFlat* s, int nSteps, real_t dt
#ifdef DO_ZMQ
                , int iStep, long* sentData, long *timeprev
#endif
                )
                {
   for (int ii=0; ii<nSteps; ++ii)
   {

      startTimer(velocityTimer);
      advanceVelocity(s, s->boxes->nLocalBoxes, 0.5*dt);
      stopTimer(velocityTimer);

      startTimer(positionTimer);
      advancePosition(s, s->boxes->nLocalBoxes, dt);
      stopTimer(positionTimer);

      startTimer(redistributeTimer);
      redistributeAtoms(s);
      stopTimer(redistributeTimer);

      startTimer(computeForceTimer);
      computeForce(s);
      stopTimer(computeForceTimer);

      startTimer(velocityTimer);
      advanceVelocity(s, s->boxes->nLocalBoxes, 0.5*dt);
      stopTimer(velocityTimer);
   }

#ifdef DO_ZMQ
   // ZeroMQ send ids, the current time in milis and the positions
   int ts = iStep + nSteps - 1;
   long time = get_current_time_in_ms();
   int period;

   zmq_send(s->sender, (void *)&(s->rank), sizeof(int), ZMQ_SNDMORE);
   zmq_send(s->sender, (void *)&ts, sizeof(int), ZMQ_SNDMORE);
   zmq_send(s->sender, (void *)&time, sizeof(long), ZMQ_SNDMORE);

   //Prepare messages to send: the positions and the IDs
   real3* buffer = calloc(MAXATOMS*s->boxes->nLocalBoxes,sizeof(real3) * 3);
   int* idBuffer = calloc(MAXATOMS*s->boxes->nLocalBoxes,sizeof(int));
   int numberOfAtoms = getPositionsAndIDs(s, s->boxes->nLocalBoxes,buffer,idBuffer);
   int numberOfMsgs = 0;
   //Sends number of atoms
   // zmq_send(s->sender, (void *)&numberOfAtoms,sizeof(int), ZMQ_SNDMORE);

   //Sends positions of the atoms
   zmq_send(s->sender, (void *)idBuffer, numberOfAtoms*sizeof(int), ZMQ_SNDMORE);
   zmq_send(s->sender, (void *)buffer, numberOfAtoms*sizeof(real3), 0);

   //Update number of bytes sent by this process
   int messageSize = 2 * sizeof(int) + sizeof(long) + numberOfAtoms * (sizeof(real3) + sizeof(int));
   *sentData = *sentData + messageSize;

   //Snippet to print the atom position in a file
   printf("%d_%d : %d Atoms.\n", s->rank, ts, numberOfAtoms);
   char filename[30];
   sprintf(filename, "/tmp/CoMD_%d", getMyRank());
   printf("File = %s\n", filename);
   FILE *f = fopen(filename, "a");
   for(int i=0, j=0; j<numberOfAtoms; i=i+3, j++)
   {
      fprintf(f, "%lf , %lf, %lf\n", buffer[j][0], buffer[j][1], buffer[j][2]);
      fprintf(f, "%d\n", idBuffer[j]);
   }
   fclose(f);

   //DBG info about sent msgs
   
     if (*timeprev != 0) {
     //Time from previous delivery
     period = time - *timeprev;

     //Output info about msg rate

     printf("Rank %d: Message %d -> %d atoms (%lf MiB) - %ld ms from prev (%lf MB/s)\n", s->rank, ts, numberOfAtoms, messageSize*1.0/1024/1024, period, (messageSize*1.0/1000) / period);
   } else {
     printf("Rank %d: Message %d -> %d atoms (%lf MiB)\n", s->rank, ts, numberOfAtoms, messageSize*1.0/1024/1024);
   }

   *timeprev = time;

   free(buffer);
   free(idBuffer);
#endif

   kineticEnergy(s);

   return s->ePotential;
}


long get_current_time_in_ms (void)
{
    long            ms; // Milliseconds
    time_t          s;  // Seconds
    struct timespec spec;

    clock_gettime(CLOCK_REALTIME, &spec);

    s  = spec.tv_sec;
    ms = round(spec.tv_nsec / 1.0e6); // Convert nanoseconds to milliseconds
    if (ms > 999) {
        s++;
        ms = 0;
    }

    ms= ms + s*1.0e3;

    return ms;
}

void computeForce(SimFlat* s)
{
   s->pot->force(s);
}


void advanceVelocity(SimFlat* s, int nBoxes, real_t dt)
{
   for (int iBox=0; iBox<nBoxes; iBox++)
   {
      for (int iOff=MAXATOMS*iBox,ii=0; ii<s->boxes->nAtoms[iBox]; ii++,iOff++)
      {
         s->atoms->p[iOff][0] += dt*s->atoms->f[iOff][0];
         s->atoms->p[iOff][1] += dt*s->atoms->f[iOff][1];
         s->atoms->p[iOff][2] += dt*s->atoms->f[iOff][2];
      }
   }
}

void advancePosition(SimFlat* s, int nBoxes, real_t dt)
{
   for (int iBox=0; iBox<nBoxes; iBox++)
   {
      for (int iOff=MAXATOMS*iBox,ii=0; ii<s->boxes->nAtoms[iBox]; ii++,iOff++)
      {
         int iSpecies = s->atoms->iSpecies[iOff];
         real_t invMass = 1.0/s->species[iSpecies].mass;
         s->atoms->r[iOff][0] += dt*s->atoms->p[iOff][0]*invMass;
         s->atoms->r[iOff][1] += dt*s->atoms->p[iOff][1]*invMass;
         s->atoms->r[iOff][2] += dt*s->atoms->p[iOff][2]*invMass;
      }
   }
}

//Copies the all the positions to buffer and the atom IDs to idBuffer 
int getPositionsAndIDs(SimFlat* s, int nBoxes, real3* buffer, int* idBuffer)
{
  int numberOfAtoms = 0;
   for (int iBox=0; iBox<nBoxes; iBox++)
   {
      for (int iOff=MAXATOMS*iBox,ii=0; ii<s->boxes->nAtoms[iBox]; ii++,iOff++)
      {
        //The Id Buffer
        idBuffer[numberOfAtoms]= s->atoms->gid[iOff];

        //Get the position as floats for the position buffer
        buffer[numberOfAtoms][0] = s->atoms->r[iOff][0];
        buffer[numberOfAtoms][1] = s->atoms->r[iOff][1];
        buffer[numberOfAtoms][2] = s->atoms->r[iOff][2];

        numberOfAtoms++;
      }
   }
   return numberOfAtoms;
}

/// Calculates total kinetic and potential energy across all tasks.  The
/// local potential energy is a by-product of the force routine.
void kineticEnergy(SimFlat* s)
{
   real_t eLocal[2];
   eLocal[0] = s->ePotential;
   eLocal[1] = 0;
   for (int iBox=0; iBox<s->boxes->nLocalBoxes; iBox++)
   {
      for (int iOff=MAXATOMS*iBox,ii=0; ii<s->boxes->nAtoms[iBox]; ii++,iOff++)
      {
         int iSpecies = s->atoms->iSpecies[iOff];
         real_t invMass = 0.5/s->species[iSpecies].mass;
         eLocal[1] += ( s->atoms->p[iOff][0] * s->atoms->p[iOff][0] +
         s->atoms->p[iOff][1] * s->atoms->p[iOff][1] +
         s->atoms->p[iOff][2] * s->atoms->p[iOff][2] )*invMass;
      }
   }

   real_t eSum[2];
   startTimer(commReduceTimer);
   addRealParallel(eLocal, eSum, 2);
   stopTimer(commReduceTimer);

   s->ePotential = eSum[0];
   s->eKinetic = eSum[1];
}

/// \details
/// This function provides one-stop shopping for the sequence of events
/// that must occur for a proper exchange of halo atoms after the atom
/// positions have been updated by the integrator.
///
/// - updateLinkCells: Since atoms have moved, some may be in the wrong
///   link cells.
/// - haloExchange (atom version): Sends atom data to remote tasks.
/// - sort: Sort the atoms.
///
/// \see updateLinkCells
/// \see initAtomHaloExchange
/// \see sortAtomsInCell
void redistributeAtoms(SimFlat* sim)
{
   updateLinkCells(sim->boxes, sim->atoms);

   startTimer(atomHaloTimer);
   haloExchange(sim->atomExchange, sim);
   stopTimer(atomHaloTimer);

   for (int ii=0; ii<sim->boxes->nTotalBoxes; ++ii)
      sortAtomsInCell(sim->atoms, sim->boxes, ii);
}
