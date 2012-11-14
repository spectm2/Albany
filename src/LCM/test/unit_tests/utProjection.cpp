//*****************************************************************//
//    Albany 2.0:  Copyright 2012 Sandia Corporation               //
//    This Software is released under the BSD license detailed     //
//    in the file "license.txt" in the top-level Albany directory  //
//*****************************************************************//
#include <Teuchos_UnitTestHarness.hpp>
#include <Teuchos_ParameterList.hpp>
#include <Epetra_MpiComm.h>
#include <Phalanx.hpp>
#include "PHAL_AlbanyTraits.hpp"
#include "Albany_Utils.hpp"
#include "Albany_StateManager.hpp"
#include "Albany_TmplSTKMeshStruct.hpp"
#include "Albany_STKDiscretization.hpp"
#include "LCM/evaluators/MooneyRivlin.hpp"
#include "LCM/evaluators/SetField.hpp"
#include "Tensor.h"

using namespace std;

namespace {

  TEUCHOS_UNIT_TEST( Projection, Instantiation )
  {
    // Set up the data layout
    const int worksetSize = 1;
    const int numQPts = 1;
    const int numDim = 3;
    Teuchos::RCP<PHX::MDALayout<Cell, QuadPoint> > qp_scalar = Teuchos::rcp(
        new PHX::MDALayout<Cell, QuadPoint>(worksetSize, numQPts));
    Teuchos::RCP<PHX::MDALayout<Cell, QuadPoint, Dim, Dim> > qp_tensor =
        Teuchos::rcp(
            new PHX::MDALayout<Cell, QuadPoint, Dim, Dim>(worksetSize, numQPts,
                numDim, numDim));

    // Instantiate the required evaluators with EvalT = PHAL::AlbanyTraits::Residual and Traits = PHAL::AlbanyTraits

    // The deformation gradient will be set to a specific value, which will provide input to the
    // LameStress evaluator
    Teuchos::ArrayRCP<PHAL::AlbanyTraits::Residual::ScalarT> tensorValue(9);
    tensorValue[0] = 1.010050167084168;
    tensorValue[1] = 0.0;
    tensorValue[2] = 0.0;
    tensorValue[3] = 0.0;
    tensorValue[4] = 0.99750312239746;
    tensorValue[5] = 0.0;
    tensorValue[6] = 0.0;
    tensorValue[7] = 0.0;
    tensorValue[8] = 0.99750312239746;

    // SetField evaluator, which will be used to manually assign a value to the DefGrad field
    Teuchos::ParameterList setFieldParameterList("SetField");
    setFieldParameterList.set<string>("Evaluated Field Name",
        "Deformation Gradient");
    setFieldParameterList.set<Teuchos::RCP<PHX::DataLayout> >(
        "Evaluated Field Data Layout", qp_tensor);
    setFieldParameterList.set<
        Teuchos::ArrayRCP<PHAL::AlbanyTraits::Residual::ScalarT> >(
        "Field Values", tensorValue);
    Teuchos::RCP<LCM::SetField<PHAL::AlbanyTraits::Residual, PHAL::AlbanyTraits> > setField =
        Teuchos::rcp(
            new LCM::SetField<PHAL::AlbanyTraits::Residual, PHAL::AlbanyTraits>(
                setFieldParameterList));

    // LameStress evaluator
    Teuchos::RCP<Teuchos::ParameterList> MRStressParameterList = Teuchos::rcp(
        new Teuchos::ParameterList("Stress"));
    MRStressParameterList->set<string>("DefGrad Name", "Deformation Gradient");
    MRStressParameterList->set<string>("Stress Name", "Stress");
    MRStressParameterList->set<Teuchos::RCP<PHX::DataLayout> >(
        "QP Scalar Data Layout", qp_scalar);
    MRStressParameterList->set<Teuchos::RCP<PHX::DataLayout> >(
        "QP Tensor Data Layout", qp_tensor);
    MRStressParameterList->set<string>("Lame Material Model", "Elastic_New");
    Teuchos::ParameterList& materialModelParametersList =
        MRStressParameterList->sublist("Lame Material Parameters");
    materialModelParametersList.set<double>("c1", 1.0);
    materialModelParametersList.set<double>("c2", -0.75);
    materialModelParametersList.set<double>("c", 1.0);
    Teuchos::RCP<
        LCM::MooneyRivlin<PHAL::AlbanyTraits::Residual, PHAL::AlbanyTraits> > MRStress =
        Teuchos::rcp(
            new LCM::MooneyRivlin<PHAL::AlbanyTraits::Residual,
                PHAL::AlbanyTraits>(*MRStressParameterList));

    // Instantiate a field manager.
    PHX::FieldManager<PHAL::AlbanyTraits> fieldManager;

    // Register the evaluators with the field manager
    fieldManager.registerEvaluator<PHAL::AlbanyTraits::Residual>(setField);
    fieldManager.registerEvaluator<PHAL::AlbanyTraits::Residual>(MRStress);

    // Set the LameStress evaluated fields as required fields
    for (std::vector<Teuchos::RCP<PHX::FieldTag> >::const_iterator it =
        MRStress->evaluatedFields().begin();
        it != MRStress->evaluatedFields().end(); it++)
      fieldManager.requireField<PHAL::AlbanyTraits::Residual>(**it);

    // Call postRegistrationSetup on the evaluators
    PHAL::AlbanyTraits::SetupData setupData = "Test String";
    fieldManager.postRegistrationSetup(setupData);

    // Create a state manager with required fields
    Albany::StateManager stateMgr;
    // Stress and DefGrad are required for all LAME models
    stateMgr.registerStateVariable("Stress", qp_tensor, "dummy", "scalar", 0.0,
        true);
    stateMgr.registerStateVariable("Deformation Gradient", qp_tensor, "dummy",
        "identity", 1.0, true);
    /*// Add material-model specific state variables
     string MRMaterialModelName = MRStressParameterList->get<string>("Lame Material Model");
     std::vector<std::string> MRMaterialModelStateVariableNames = LameUtils::getStateVariableNames(MRMaterialModelName, materialModelParametersList);
     std::vector<double> MRMaterialModelStateVariableInitialValues = LameUtils::getStateVariableInitialValues(MRMaterialModelName, materialModelParametersList);
     for(unsigned int i=0 ; i<MRMaterialModelStateVariableNames.size() ; ++i){
     stateMgr.registerStateVariable(MRMaterialModelStateVariableNames[i],
     qp_scalar,
     Albany::doubleToInitString(MRMaterialModelStateVariableInitialValues[i]),
     true);
     }*/

    // Create a discretization, as required by the StateManager
    Teuchos::RCP<Teuchos::ParameterList> discretizationParameterList =
        Teuchos::rcp(new Teuchos::ParameterList("Discretization"));
    discretizationParameterList->set<int>("1D Elements", worksetSize);
    discretizationParameterList->set<int>("2D Elements", 1);
    discretizationParameterList->set<int>("3D Elements", 1);
    discretizationParameterList->set<string>("Method", "STK3D");
    discretizationParameterList->set<string>("Exodus Output File Name",
        "unitTestOutput.exo"); // Is this required?
    Teuchos::RCP<Epetra_Comm> comm = Teuchos::rcp(
        new Epetra_MpiComm(MPI_COMM_WORLD));
    int numberOfEquations = 3;
    Teuchos::RCP<Albany::GenericSTKMeshStruct> stkMeshStruct = Teuchos::rcp(
        new Albany::TmplSTKMeshStruct<3>(discretizationParameterList, comm));
    stkMeshStruct->setFieldAndBulkData(comm, discretizationParameterList,
        numberOfEquations, stateMgr.getStateInfoStruct(),
        stkMeshStruct->getMeshSpecs()[0]->worksetSize);
    Teuchos::RCP<Albany::AbstractDiscretization> discretization = Teuchos::rcp(
        new Albany::STKDiscretization(stkMeshStruct, comm));

    // Associate the discretization with the StateManager
    stateMgr.setStateArrays(discretization);

    // Create a workset
    PHAL::Workset workset;
    workset.numCells = worksetSize;
    workset.stateArrayPtr = &stateMgr.getStateArray(0);

    // Call the evaluators, evaluateFields() is the function that computes stress based on deformation gradient
    fieldManager.preEvaluate<PHAL::AlbanyTraits::Residual>(workset);
    fieldManager.evaluateFields<PHAL::AlbanyTraits::Residual>(workset);
    fieldManager.postEvaluate<PHAL::AlbanyTraits::Residual>(workset);

    // Pull the stress from the FieldManager
    PHX::MDField<PHAL::AlbanyTraits::Residual::ScalarT, Cell, QuadPoint, Dim,
        Dim> stressField("Stress", qp_tensor);
    fieldManager.getFieldData<PHAL::AlbanyTraits::Residual::ScalarT,
        PHAL::AlbanyTraits::Residual, Cell, QuadPoint, Dim, Dim>(stressField);

    // Assert the dimensions of the stress field
//   std::vector<size_type> stressFieldDimensions;
//   stressField.dimensions(stressFieldDimensions);

    // Record the expected stress, which will be used to check the computed stress
    LCM::Tensor<PHAL::AlbanyTraits::Residual::ScalarT> expectedStress(
        materialModelParametersList.get<double>("Youngs Modulus") * 0.01, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);

    // Check the computed stresses
    typedef PHX::MDField<PHAL::AlbanyTraits::Residual::ScalarT>::size_type size_type;
    for (size_type cell = 0; cell < worksetSize; ++cell) {
      for (size_type qp = 0; qp < numQPts; ++qp) {

        std::cout << "Stress tensor at cell " << cell << ", quadrature point "
            << qp << ":" << endl;
        std::cout << "  " << stressField(cell, qp, 0, 0);
        std::cout << "  " << stressField(cell, qp, 0, 1);
        std::cout << "  " << stressField(cell, qp, 0, 2) << endl;
        std::cout << "  " << stressField(cell, qp, 1, 0);
        std::cout << "  " << stressField(cell, qp, 1, 1);
        std::cout << "  " << stressField(cell, qp, 1, 2) << endl;
        std::cout << "  " << stressField(cell, qp, 2, 0);
        std::cout << "  " << stressField(cell, qp, 2, 1);
        std::cout << "  " << stressField(cell, qp, 2, 2) << endl;

        std::cout << "Expected result:" << endl;
        std::cout << "  " << expectedStress(0, 0);
        std::cout << "  " << expectedStress(0, 1);
        std::cout << "  " << expectedStress(0, 2) << endl;
        std::cout << "  " << expectedStress(1, 0);
        std::cout << "  " << expectedStress(1, 1);
        std::cout << "  " << expectedStress(1, 2) << endl;
        std::cout << "  " << expectedStress(2, 0);
        std::cout << "  " << expectedStress(2, 1);
        std::cout << "  " << expectedStress(2, 2) << endl;

        std::cout << endl;

        double tolerance = 1.0e-15;
        for (size_type i = 0; i < numDim; ++i) {
          for (size_type j = 0; j < numDim; ++j) {
            TEST_COMPARE(
                fabs(stressField(cell, qp, i, j) - expectedStress(i, j)), <=,
                tolerance);
          }
        }

      }
    }
    std::cout << endl;

  }

} // namespace