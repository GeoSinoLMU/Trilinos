// @HEADER
// ****************************************************************************
//                Tempus: Copyright (2017) Sandia Corporation
//
// Distributed under BSD 3-clause license (See accompanying file Copyright.txt)
// ****************************************************************************
// @HEADER

#include "Teuchos_UnitTestHarness.hpp"
#include "Teuchos_XMLParameterListHelpers.hpp"
#include "Teuchos_TimeMonitor.hpp"
#include "Teuchos_DefaultComm.hpp"

#include "Tempus_config.hpp"
#include "Tempus_IntegratorBasic.hpp"
#include "Tempus_IntegratorForwardSensitivity.hpp"
#include "Tempus_IntegratorPseudoTransientForwardSensitivity.hpp"

#include "Thyra_VectorStdOps.hpp"
#include "Thyra_MultiVectorStdOps.hpp"

#include "../TestModels/SinCosModel.hpp"
#include "../TestModels/SteadyQuadraticModel.hpp"
#include "../TestUtils/Tempus_ConvergenceTestUtils.hpp"

#include "Stratimikos_DefaultLinearSolverBuilder.hpp"
#include "Thyra_LinearOpWithSolveFactoryHelpers.hpp"
#include "Thyra_DefaultMultiVectorProductVector.hpp"

#include <vector>
#include <fstream>
#include <sstream>
#include <limits>

namespace Tempus_Test {

using Teuchos::RCP;
using Teuchos::ParameterList;
using Teuchos::sublist;
using Teuchos::getParametersFromXmlFile;

using Tempus::IntegratorBasic;
using Tempus::SolutionHistory;
using Tempus::SolutionState;

// ************************************************************
// ************************************************************
void test_sincos_fsa(const bool use_combined_method,
                     const bool use_dfdp_as_tangent,
                     Teuchos::FancyOStream &out, bool &success)
{
  std::vector<double> StepSize;
  std::vector<double> ErrorNorm;
  const int nTimeStepSizes = 7;
  double dt = 0.2;
  double order = 0.0;
  Teuchos::RCP<const Teuchos::Comm<int> > comm =
    Teuchos::DefaultComm<int>::getComm();
  Teuchos::RCP<Teuchos::FancyOStream> my_out =
    Teuchos::fancyOStream(Teuchos::rcpFromRef(std::cout));
  my_out->setProcRankAndSize(comm->getRank(), comm->getSize());
  my_out->setOutputToRootOnly(0);
  for (int n=0; n<nTimeStepSizes; n++) {

    // Read params from .xml file
    RCP<ParameterList> pList =
      getParametersFromXmlFile("Tempus_BackwardEuler_SinCos.xml");

    // Setup the SinCosModel
    RCP<ParameterList> scm_pl = sublist(pList, "SinCosModel", true);
    scm_pl->set("Use DfDp as Tangent", use_dfdp_as_tangent);
    RCP<SinCosModel<double> > model =
      Teuchos::rcp(new SinCosModel<double>(scm_pl));

    dt /= 2;

    // Setup sensitivities
    RCP<ParameterList> pl = sublist(pList, "Tempus", true);
    ParameterList& sens_pl = pl->sublist("Sensitivities");
    if (use_combined_method)
      sens_pl.set("Sensitivity Method", "Combined");
    else {
      sens_pl.set("Sensitivity Method", "Staggered");
      sens_pl.set("Reuse State Linear Solver", true);
    }
    sens_pl.set("Use DfDp as Tangent", use_dfdp_as_tangent);

    // Setup the Integrator and reset initial time step
    pl->sublist("Default Integrator")
       .sublist("Time Step Control").set("Initial Time Step", dt);
    RCP<Tempus::IntegratorForwardSensitivity<double> > integrator =
      Tempus::integratorForwardSensitivity<double>(pl, model);
    order = integrator->getStepper()->getOrder();

    // Initial Conditions
    double t0 = pl->sublist("Default Integrator")
      .sublist("Time Step Control").get<double>("Initial Time");
    RCP<const Thyra::VectorBase<double> > x0 =
      model->getExactSolution(t0).get_x();
    const int num_param = model->get_p_space(0)->dim();
    RCP<Thyra::MultiVectorBase<double> > DxDp0 =
      Thyra::createMembers(model->get_x_space(), num_param);
    for (int i=0; i<num_param; ++i)
      Thyra::assign(DxDp0->col(i).ptr(),
                    *(model->getExactSensSolution(i, t0).get_x()));
    integrator->setInitialState(t0, x0, Teuchos::null, Teuchos::null,
                                DxDp0, Teuchos::null, Teuchos::null);

    // Integrate to timeMax
    bool integratorStatus = integrator->advanceTime();
    TEST_ASSERT(integratorStatus)

    // Test if at 'Final Time'
    double time = integrator->getTime();
    double timeFinal =pl->sublist("Default Integrator")
       .sublist("Time Step Control").get<double>("Final Time");
    TEST_FLOATING_EQUALITY(time, timeFinal, 1.0e-14);

    // Time-integrated solution and the exact solution
    RCP<const Thyra::VectorBase<double> > x = integrator->getX();
    RCP<const Thyra::MultiVectorBase<double> > DxDp = integrator->getDxDp();
    RCP<const Thyra::VectorBase<double> > x_exact =
      model->getExactSolution(time).get_x();
    RCP<Thyra::MultiVectorBase<double> > DxDp_exact =
      Thyra::createMembers(model->get_x_space(), num_param);
    for (int i=0; i<num_param; ++i)
      Thyra::assign(DxDp_exact->col(i).ptr(),
                    *(model->getExactSensSolution(i, time).get_x()));

    // Plot sample solution and exact solution
    if (comm->getRank() == 0 && n == nTimeStepSizes-1) {
      typedef Thyra::DefaultMultiVectorProductVector<double> DMVPV;

      std::ofstream ftmp("Tempus_BackwardEuler_SinCos_Sens.dat");
      RCP<const SolutionHistory<double> > solutionHistory =
        integrator->getSolutionHistory();
      RCP< Thyra::MultiVectorBase<double> > DxDp_exact_plot =
        Thyra::createMembers(model->get_x_space(), num_param);
      for (int i=0; i<solutionHistory->getNumStates(); i++) {
        RCP<const SolutionState<double> > solutionState = (*solutionHistory)[i];
        double time = solutionState->getTime();
        RCP<const DMVPV> x_prod_plot =
          Teuchos::rcp_dynamic_cast<const DMVPV>(solutionState->getX());
        RCP<const Thyra::VectorBase<double> > x_plot =
          x_prod_plot->getMultiVector()->col(0);
        RCP<const Thyra::MultiVectorBase<double> > DxDp_plot =
          x_prod_plot->getMultiVector()->subView(Teuchos::Range1D(1,num_param));
        RCP<const Thyra::VectorBase<double> > x_exact_plot =
          model->getExactSolution(time).get_x();
        for (int j=0; j<num_param; ++j)
          Thyra::assign(DxDp_exact_plot->col(j).ptr(),
                        *(model->getExactSensSolution(j, time).get_x()));
        ftmp << std::fixed << std::setprecision(7)
             << time
             << std::setw(11) << get_ele(*(x_plot), 0)
             << std::setw(11) << get_ele(*(x_plot), 1);
        for (int j=0; j<num_param; ++j)
          ftmp << std::setw(11) << get_ele(*(DxDp_plot->col(j)), 0)
               << std::setw(11) << get_ele(*(DxDp_plot->col(j)), 1);
        ftmp << std::setw(11) << get_ele(*(x_exact_plot), 0)
             << std::setw(11) << get_ele(*(x_exact_plot), 1);
        for (int j=0; j<num_param; ++j)
          ftmp << std::setw(11) << get_ele(*(DxDp_exact_plot->col(j)), 0)
               << std::setw(11) << get_ele(*(DxDp_exact_plot->col(j)), 1);
        ftmp << std::endl;
      }
      ftmp.close();
    }

    // Calculate the error
    RCP<Thyra::VectorBase<double> > xdiff = x->clone_v();
    RCP<Thyra::MultiVectorBase<double> > DxDpdiff = DxDp->clone_mv();
    Thyra::V_StVpStV(xdiff.ptr(), 1.0, *x_exact, -1.0, *(x));
    Thyra::V_VmV(DxDpdiff.ptr(), *DxDp_exact, *DxDp);
    StepSize.push_back(dt);
    double L2norm = Thyra::norm_2(*xdiff);
    L2norm *= L2norm;
    Teuchos::Array<double> L2norm_DxDp(num_param);
    Thyra::norms_2(*DxDpdiff, L2norm_DxDp());
    for (int i=0; i<num_param; ++i)
      L2norm += L2norm_DxDp[i]*L2norm_DxDp[i];
    L2norm = std::sqrt(L2norm);
    ErrorNorm.push_back(L2norm);

    *my_out << " n = " << n << " dt = " << dt << " error = " << L2norm
            << std::endl;

  }

  // Check the order and intercept
  double slope = computeLinearRegressionLogLog<double>(StepSize, ErrorNorm);
  *my_out << "  Stepper = BackwardEuler" << std::endl;
  *my_out << "  =========================" << std::endl;
  *my_out << "  Expected order: " << order << std::endl;
  *my_out << "  Observed order: " << slope << std::endl;
  *my_out << "  =========================" << std::endl;
  TEST_FLOATING_EQUALITY( slope, order, 0.015 );
  TEST_FLOATING_EQUALITY( ErrorNorm[0], 0.163653, 1.0e-4 );

  if (comm->getRank() == 0) {
    std::ofstream ftmp("Tempus_BackwardEuler_SinCos_Sens-Error.dat");
    double error0 = 0.8*ErrorNorm[0];
    for (int n=0; n<nTimeStepSizes; n++) {
      ftmp << StepSize[n]  << "   " << ErrorNorm[n] << "   "
           << error0*(StepSize[n]/StepSize[0]) << std::endl;
    }
    ftmp.close();
  }

}

TEUCHOS_UNIT_TEST(BackwardEuler, SinCos_Combined_FSA)
{
  test_sincos_fsa(true, false, out, success);
}

TEUCHOS_UNIT_TEST(BackwardEuler, SinCos_Combined_FSA_Tangent)
{
  test_sincos_fsa(true, true, out, success);
}

TEUCHOS_UNIT_TEST(BackwardEuler, SinCos_Staggered_FSA)
{
  test_sincos_fsa(false, false, out, success);
}

TEUCHOS_UNIT_TEST(BackwardEuler, SinCos_Staggered_FSA_Tangent)
{
  test_sincos_fsa(false, true, out, success);
}

// ************************************************************
// ************************************************************
void test_pseudotransient_fsa(const bool use_dfdp_as_tangent,
                              Teuchos::FancyOStream &out, bool &success)
{
  // Read params from .xml file
  RCP<ParameterList> pList =
    getParametersFromXmlFile("Tempus_BackwardEuler_SteadyQuadratic.xml");

  // Setup the SteadyQuadraticModel
  RCP<ParameterList> scm_pl = sublist(pList, "SteadyQuadraticModel", true);
  scm_pl->set("Use DfDp as Tangent", use_dfdp_as_tangent);
  RCP<SteadyQuadraticModel<double> > model =
    Teuchos::rcp(new SteadyQuadraticModel<double>(scm_pl));

  // Setup sensitivities
  RCP<ParameterList> pl = sublist(pList, "Tempus", true);
  ParameterList& sens_pl = pl->sublist("Sensitivities");
  sens_pl.set("Use DfDp as Tangent", use_dfdp_as_tangent);
  sens_pl.set("Reuse State Linear Solver", true);
  sens_pl.set("Force W Update", true); // Have to do this because for this
  // model the solver seems to be overwriting the matrix

  // Setup the Integrator
  RCP<Tempus::IntegratorPseudoTransientForwardSensitivity<double> > integrator =
    Tempus::integratorPseudoTransientForwardSensitivity<double>(pl, model);

  // Integrate to timeMax
  bool integratorStatus = integrator->advanceTime();
  TEST_ASSERT(integratorStatus);

  // Test if at 'Final Time'
  double time = integrator->getTime();
  double timeFinal =pl->sublist("Default Integrator")
    .sublist("Time Step Control").get<double>("Final Time");
  TEST_FLOATING_EQUALITY(time, timeFinal, 1.0e-14);

  // Time-integrated solution and the exact solution
  RCP<const Thyra::VectorBase<double> > x_vec = integrator->getX();
  RCP<const Thyra::MultiVectorBase<double> > DxDp_vec = integrator->getDxDp();
  const double x = Thyra::get_ele(*x_vec, 0);
  const double dxdb = Thyra::get_ele(*(DxDp_vec->col(0)), 0);
  const double x_exact = model->getSteadyStateSolution();
  const double dxdb_exact = model->getSteadyStateSolutionSensitivity();

  TEST_FLOATING_EQUALITY( x,    x_exact,    1.0e-6 );
  TEST_FLOATING_EQUALITY( dxdb, dxdb_exact, 1.0e-6 );
}

TEUCHOS_UNIT_TEST(BackwardEuler, SteadyQuadratic_PseudoTransient_FSA)
{
  test_pseudotransient_fsa(false, out, success);
}

TEUCHOS_UNIT_TEST(BackwardEuler, SteadyQuadratic_PseudoTransient_FSA_Tangent)
{
  test_pseudotransient_fsa(true, out, success);
}

} // namespace Tempus_Test
