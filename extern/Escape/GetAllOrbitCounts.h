#ifndef ESCAPE_GETALLORBITCOUNTS_H_
#define ESCAPE_GETALLORBITCOUNTS_H_

#include <algorithm>

#include "Escape/FourVertex.h"
#include "Escape/Utils.h"
#include "Escape/GraphIO.h"
#include "Escape/EdgeHash.h"
#include "Escape/Digraph.h"
#include "Escape/Triadic.h"
#include "Escape/Graph.h"
#include "Escape/Conversion.h"
#include "Escape/FiveTrees.h"
#include "Escape/FiveFromCycleClique.h"
#include "Escape/FiveFromTriangles.h"
#include "Escape/AlmostFiveClique.h"
#include "Escape/FiveCycle.h"
#include "Escape/WedgeCollisions.h"
#include "Escape/TriangleProgram.h"
#include "Escape/OrbitStructure.h"
#include "Escape/ThreeVertexOrbit.h"
#include "Escape/FourVertexOrbit.h"
#include "Escape/FiveVertexOrbit.h"

#include <iostream>
#include <iomanip>
#include <string>
#include <omp.h>

using namespace std;
using namespace Escape;


// This function generates all orbit counts and edge orbit counts for orbits and edge orbits in patterns with 3, 2, and 1 vertex.
// It is a wrapper function that calls the main algorithmic parts

// Input: pointer to CGraph, corresponding DAG, OrbitInfo structure to store orbit counts
// No output: orbit_counts will have all the orbit and edge orbit counts

void get_all_three_orbits(CGraph *cg, CDAG *dag, OrbitInfo &orbit_counts)
{
    double n, m, w;
    TriangleInfo tri_info;

    n = cg->nVertices;
    tri_info = betterWedgeEnumerator(&(dag->outlist));

    for (VertexIdx i = 0; i < n; i++) // looping over vertices
    {
        VertexIdx deg = cg->offsets[i+1] - cg->offsets[i]; // degree of i
        orbit_counts.per_vertex_[0][i] = deg;
        //orbit_counts.per_vertex_[1][i] = 0;
        for (EdgeIdx pos = cg->offsets[i]; pos < cg->offsets[i+1]; pos++) // loop over neighbors of i
        {
        	VertexIdx j = cg->nbors[pos]; // node j is the neighbor of node i
        	VertexIdx deg_j = cg->offsets[j+1] - cg->offsets[j]; // degree of j
        	orbit_counts.per_vertex_[1][i] += deg_j - 1;
        }
        orbit_counts.per_vertex_[2][i] = choosec(deg, 2);
        orbit_counts.per_vertex_[3][i] = tri_info.perVertex[i];
    }
    three_edge_orbit_counter(&(dag->outlist), orbit_counts);
}

// This function generates all node orbit counts for orbits in 4-vertex patterns.
// It is a wrapper function that calls the main algorithmic parts

// Input: pointer to CGraph, corresponding DAG, OrbitInfo structure to store orbit counts
// No output: orbit_counts will have all the orbit and edge orbit counts

void  get_all_four_orbits_node_only(CGraph *cg, CDAG *dag, OrbitInfo &orbit_counts)
{
    double n, m, w;

    n = cg->nVertices;

    for (VertexIdx i = 0; i < n; i++) // loop over vertices
    {
        orbit_counts.per_vertex_[5][i] = orbit_counts.per_vertex_[1][i] * (orbit_counts.per_vertex_[0][i] - 1) - 2 * orbit_counts.per_vertex_[3][i];
        orbit_counts.per_vertex_[7][i] = choosec(orbit_counts.per_vertex_[0][i], 3);
        orbit_counts.per_vertex_[11][i] = orbit_counts.per_vertex_[3][i] * (orbit_counts.per_vertex_[0][i] - 2);

        
        for (EdgeIdx pos = cg->offsets[i]; pos < cg->offsets[i+1]; pos++) // loop over neighbors of i
        {
            VertexIdx j = cg->nbors[pos];
            VertexIdx deg_j = orbit_counts.per_vertex_[0][j]; //degree of j
            orbit_counts.per_vertex_[4][i] += orbit_counts.per_vertex_[1][j];
            orbit_counts.per_vertex_[6][i] += choosec(deg_j - 1, 2);
        }

        orbit_counts.per_vertex_[4][i] -= 2 * orbit_counts.per_vertex_[2][i] + 2 * orbit_counts.per_vertex_[3][i];
    }


    otherFourPatternOrbitCounterNodeOnly(&(dag->outlist), &(dag->inlist), orbit_counts);
    four_cycle_orbit_counter_node_only(&(dag->outlist), &(dag->inlist), orbit_counts);
    fourCliqueOrbitCounterNodeOnly(&(dag->outlist), orbit_counts);
}

// This function generates all orbit counts and edge orbit counts for orbits and edge orbits in 4-vertex patterns.
// It is a wrapper function that calls the main algorithmic parts

// Input: pointer to CGraph, corresponding DAG, OrbitInfo structure to store orbit counts
// No output: orbit_counts will have all the orbit and edge orbit counts

void  get_all_four_orbits(CGraph *cg, CDAG *dag, OrbitInfo &orbit_counts)
{
	double n, m, w;

    n = cg->nVertices;

    for (VertexIdx i = 0; i < n; i++) // loop over vertices
    {
      	orbit_counts.per_vertex_[5][i] = orbit_counts.per_vertex_[1][i] * (orbit_counts.per_vertex_[0][i] - 1) - 2 * orbit_counts.per_vertex_[3][i];
      	orbit_counts.per_vertex_[7][i] = choosec(orbit_counts.per_vertex_[0][i], 3);
      	orbit_counts.per_vertex_[11][i] = orbit_counts.per_vertex_[3][i] * (orbit_counts.per_vertex_[0][i] - 2);

        
        for (EdgeIdx pos = cg->offsets[i]; pos < cg->offsets[i+1]; pos++) // loop over neighbors of i
        {
            VertexIdx j = cg->nbors[pos];
        	VertexIdx deg_j = orbit_counts.per_vertex_[0][j]; //degree of j
        	orbit_counts.per_vertex_[4][i] += orbit_counts.per_vertex_[1][j];
        	orbit_counts.per_vertex_[6][i] += choosec(deg_j - 1, 2);
        }

        orbit_counts.per_vertex_[4][i] -= 2 * orbit_counts.per_vertex_[2][i] + 2 * orbit_counts.per_vertex_[3][i];
    }


    otherFourPatternOrbitCounter(&(dag->outlist), &(dag->inlist), orbit_counts);
    four_cycle_orbit_counter(&(dag->outlist), &(dag->inlist), orbit_counts);
    fourCliqueOrbitCounter(&(dag->outlist), orbit_counts);
}



void get_all_five_orbits(CGraph *cg, CDAG *dag, OrbitInfo &orbit_counts, int parallelism_enabled)
{

 	double n, m, w;

    n = cg->nVertices;

    TriangleInfo tri_info = betterWedgeEnumerator(&(dag->outlist));
    TriangleList allTris = storeAllTriangles(cg,tri_info.total);
        
    FiveTreeOrbitCounter(&(dag->outlist), &(dag->inlist), orbit_counts);

    #pragma omp parallel sections if(parallelism_enabled)
    {
        #pragma omp section
        {
            FiveCycleOrbitCounter(&(dag->outlist), &(dag->inlist), orbit_counts);
        }
        #pragma omp section
        {
            WedgeCollisionOrbitCounter(cg, &(dag->outlist), &(dag->inlist), orbit_counts);
        }
        #pragma omp section
        {
            Pattern51OrbitCounter(cg, &(dag->outlist), &(dag->inlist), orbit_counts);
        }
        #pragma omp section
        {
            OtherFiveOrbitsCounter(&(dag->outlist), &(dag->inlist), orbit_counts);
        }
        #pragma omp section
        {
            Pattern36OrbitCounter(&(dag->outlist), &(dag->inlist), orbit_counts);
        }
        #pragma omp section
        {
            ChordalWedgeColOrbitCounter(cg, &(dag->outlist), &(dag->inlist), &allTris, orbit_counts);
        }
        #pragma omp section
        {
            Pattern66OrbitCounter(&(dag->outlist), &(dag->inlist), orbit_counts);
        }
        #pragma omp section
        {
            AlmostFiveCliqueOrbitCounter(cg, &(dag->outlist), &(dag->inlist), orbit_counts);
        }
        #pragma omp section
        {        
            FiveCliqueOrbitCounter(&(dag->outlist), &(dag->inlist), orbit_counts);
        }
    }
    
}
#endif
