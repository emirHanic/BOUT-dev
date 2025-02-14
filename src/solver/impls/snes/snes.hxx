/**************************************************************************
 *
 * Finds the steady-state solution of a set of equations
 * using PETSc for the SNES interface
 *
 **************************************************************************
 * Copyright 2015, 2021 B.D.Dudson
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

#ifdef BOUT_HAS_PETSC

class SNESSolver;

#ifndef __SNES_SOLVER_H__
#define __SNES_SOLVER_H__

#include "mpi.h"

#include <bout_types.hxx>
#include <bout/bout_enum_class.hxx>
#include <bout/solver.hxx>

#include <bout/petsclib.hxx>

#include <petsc.h>
#include <petscsnes.h>

#include <bout/solverfactory.hxx>
namespace {
RegisterSolver<SNESSolver> registersolversnes("snes");
RegisterSolver<SNESSolver> registersolverbeuler("beuler");
} // namespace

BOUT_ENUM_CLASS(BoutSnesEquationForm, pseudo_transient, rearranged_backward_euler,
                backward_euler, direct_newton);

/// Uses PETSc's SNES interface to find a steady state solution to a
/// nonlinear ODE by integrating in time with Backward Euler
class SNESSolver : public Solver {
public:
  explicit SNESSolver(Options* opt = nullptr);
  ~SNESSolver() {}

  /// Initialise solver. Must be called once and only once
  ///
  /// @param[in] nout         Number of outputs
  /// @param[in] tstep        Time between outputs. NB: Not internal timestep
  int init(int nout, BoutReal tstep) override;

  /// Run the simulation
  int run() override;

  /// Nonlinear function. This is called by PETSc SNES object
  /// via a static C-style function. For implicit
  /// time integration this function calculates:
  ///
  ///     f = (x - gamma*G(x)) - rhs
  ///
  ///
  /// @param[in] x  The state vector
  /// @param[out] f  The vector for the result f(x)
  PetscErrorCode snes_function(Vec x, Vec f); ///< Nonlinear function

  /// Preconditioner. Called by PCapply
  /// via a C-style static function.
  ///
  /// @param[in] x  The vector to be operated on
  /// @param[out] f  The result of the operation
  PetscErrorCode precon(Vec x, Vec f);

private:
  BoutReal timestep; ///< Internal timestep
  BoutReal dt;       ///< Current timestep used in snes_function
  BoutReal dt_min_reset; ///< If dt falls below this, reset solve

  int lower_its, upper_its; ///< Limits on iterations for timestep adjustment

  BoutReal out_timestep; ///< Output timestep
  int nsteps;            ///< Number of steps to take

  bool diagnose; ///< Output additional diagnostics

  int nlocal; ///< Number of variables on local processor
  int neq;    ///< Number of variables in total

  /// Form of the equation to solve
  BoutSnesEquationForm equation_form;

  PetscLib lib; ///< Handles initialising, finalising PETSc
  Vec snes_f;   ///< Used by SNES to store function
  Vec snes_x;   ///< Result of SNES
  Vec x0;       ///< Solution at start of current timestep
  Vec delta_x;  ///< Change in solution

  bool predictor;       ///< Use linear predictor?
  Vec x1;               ///< Previous solution
  BoutReal time1{-1.0}; ///< Time of previous solution

  SNES snes;                ///< SNES context
  Mat Jmf;                  ///< Matrix-free Jacobian
  MatFDColoring fdcoloring; ///< Matrix coloring context, used for finite difference
                            ///< Jacobian evaluation
};

#endif // __SNES_SOLVER_H__

#endif // BOUT_HAS_PETSC
