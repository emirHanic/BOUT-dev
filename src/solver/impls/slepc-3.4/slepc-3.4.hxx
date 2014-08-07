/**************************************************************************
 * Interface to SLEPc solver
 * NOTE: This class needs tidying, generalising to use FieldData interface
 *
 **************************************************************************
 * Copyright 2010 B.D.Dudson, S.Farley, M.V.Umansky, X.Q.Xu
 *
 * Contact: Ben Dudson, bd512@york.ac.uk
 *
 * This file is part of BOUT++.
 *
 * BOUT++ is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * BOUT++ is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with BOUT++.  If not, see <http://www.gnu.org/licenses/>.
 *
 **************************************************************************/

#ifdef BOUT_HAS_SLEPC_3_4
//Hacked together by <DD>
class SlepcSolver;

#ifndef __SLEPC_SOLVER_H__
#define __SLEPC_SOLVER_H__

#include <slepc.h>

#include <field2d.hxx>
#include <field3d.hxx>
#include <vector2d.hxx>
#include <vector3d.hxx>
#include "../../solverfactory.hxx"
#include <bout/solver.hxx>

#include <vector>
#include <bout/slepclib.hxx>
#include <bout/petsclib.hxx>

#define OPT_SIZE 40

//Define a name to use with SolverType to indicate SlepcSolver
//is in charge of advancing fields
#define SOLVERSLEPCSELF "self"

using std::vector;

class SlepcSolver : public Solver {
 public:
  SlepcSolver();
  ~SlepcSolver();

  int advanceStep(Mat &matOperator, Vec &inData, Vec &outData);

  //These contain slepc specific code and call the advanceSolver code
  int init(bool restarting, int NOUT, BoutReal TIMESTEP);
  int run();

  ////////////////////////////////////////
  /// OVERRIDE
  ///      Here we override *all* other
  ///      virtual functions in order to 
  ///      pass through control to the 
  ///      actual solver (advanceSolver)
  ///      This is only required if allow
  ///      use of additional solver
  ////////////////////////////////////////

  //////Following overrides all just pass through to advanceSolver

  //Override virtual add functions in order to pass through to advanceSolver
  void add(Field2D &v, const char* name){
    if(selfSolve){Solver::add(v,name);}else{advanceSolver->add(v,name);}
  }
  void add(Field3D &v, const char* name){
    if(selfSolve){Solver::add(v,name);}else{advanceSolver->add(v,name);}
  }
  void add(Vector2D &v, const char* name){
    if(selfSolve){Solver::add(v,name);}else{advanceSolver->add(v,name);}
  }
  void add(Vector3D &v, const char* name){
    if(selfSolve){Solver::add(v,name);}else{advanceSolver->add(v,name);}
  }

  //Set operations
  void setJacobian(Jacobian j){advanceSolver->setJacobian(j);}
  void setSplitOperator(rhsfunc fC, rhsfunc fD){
    if(selfSolve){Solver::setSplitOperator(fC,fD);}else{advanceSolver->setSplitOperator(fC,fD);}
  }

  //Constraints
  bool constraints(){
    if(selfSolve){return false;}else{return advanceSolver->constraints();}
  }
  void constraint(Field2D &v, Field2D &C_v, const char* name){
    advanceSolver->constraint(v,C_v,name);
  }
  void constraint(Field3D &v, Field3D &C_v, const char* name){
    advanceSolver->constraint(v,C_v,name);
  }
  void constraint(Vector2D &v, Vector2D &C_v, const char* name){
    advanceSolver->constraint(v,C_v,name);
  }
  void constraint(Vector3D &v, Vector3D &C_v, const char* name){
    advanceSolver->constraint(v,C_v,name);
  }

  //Override count operations
  int n2Dvars() const{
    if(selfSolve){return Solver::n2Dvars();}else{return advanceSolver->n2Dvars();}
  }
  int n3Dvars() const{
    if(selfSolve){return Solver::n3Dvars();}else{return advanceSolver->n3Dvars();}
  }
  //Time steps
  void setMaxTimestep(BoutReal dt){advanceSolver->setMaxTimestep(dt);}
  BoutReal getCurrentTimestep(){return advanceSolver->getCurrentTimestep();}

private:
  MPI_Comm comm;
  EPS eps; //Slepc solver handle
  Mat shellMat; //"Shell" matrix operator
  Solver* advanceSolver; //Pointer to actual solver used to advance fields
  void setAdvanceSolver(Options *opts=NULL); //Used to set the advanceSolver pointer
  void copySettingsToAdvanceSolver();

  void vecToFields(Vec &inVec);
  void fieldsToVec(Vec &outVec);
  
  void createShellMat();
  void createEPS();

  void readOptions();
  void analyseResults();
  void slepcToBout(PetscScalar &reEigIn, PetscScalar &imEigIn,
		   BoutReal &reEigOut, BoutReal &imEigOut);

  SlepcLib slib;// Handles initialize / finalize

  bool selfSolve; //If true then we don't create an advanceSolver
  //For selfSolve=true
  BoutReal *f0;
  BoutReal *f1;

  //Timestep details
  int nout;
  BoutReal tstep;

  //Used for SLEPc options
  int nEig,maxIt;
  PetscReal tol,target;
  //Generic options
  bool useInitial; //If true then set the first vector in subspace to be initial conditions
  bool debugMonitor;

  //Grid size
  PetscInt localSize;
};


#endif // __SLEPC_SOLVER_H__

#endif

#ifndef __SLEPC_SOLVER_H__
#define __SLEPC_SOLVER_H__

#include "../emptysolver.hxx"
typedef EmptySolver SlepcSolver;

#endif
