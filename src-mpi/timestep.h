/// \file
/// Leapfrog time integrator

#ifndef __LEAPFROG_H
#define __LEAPFROG_H

#include "CoMDTypes.h"

double timestep(SimFlat* s, int n, int iStep, real_t dt, long* sentData);
void computeForce(SimFlat* s);
void kineticEnergy(SimFlat* s);
// int getPositionsAndIDs(SimFlat* s, int nBoxes, float* buffer, int* idBuffer);
int getPositionsAndIDs(SimFlat* s, int nBoxes, real3* buffer, int* idBuffer);

/// Update local and remote link cells after atoms have moved.
void redistributeAtoms(struct SimFlatSt* sim);

#endif
