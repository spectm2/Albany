//*****************************************************************//
//    Albany 2.0:  Copyright 2012 Sandia Corporation               //
//    This Software is released under the BSD license detailed     //
//    in the file "license.txt" in the top-level Albany directory  //
//*****************************************************************//

#if !defined(LCM_LinearHMCModel_hpp)
#define LCM_LinearHMCModel_hpp

#include "Phalanx_config.hpp"
#include "Phalanx_Evaluator_WithBaseImpl.hpp"
#include "Phalanx_Evaluator_Derived.hpp"
#include "Phalanx_MDField.hpp"
#include "Albany_Layouts.hpp"
#include "LCM/models/ConstitutiveModel.hpp"

namespace LCM
{

//! \brief Constitutive Model Base Class
template<typename EvalT, typename Traits>
class LinearHMCModel: public LCM::ConstitutiveModel<EvalT, Traits>
{
public:

  typedef typename EvalT::ScalarT ScalarT;
  typedef typename EvalT::MeshScalarT MeshScalarT;

  using ConstitutiveModel<EvalT, Traits>::num_dims_;
  using ConstitutiveModel<EvalT, Traits>::num_pts_;
  using ConstitutiveModel<EvalT, Traits>::field_name_map_;

  ///
  /// Constructor
  ///
  LinearHMCModel(Teuchos::ParameterList* p,
      const Teuchos::RCP<Albany::Layouts>& dl);

  ///
  /// Virtual Destructor
  ///
  virtual
  ~LinearHMCModel()
  {};

  ///
  /// Method to compute the state (e.g. energy, stress, tangent)
  ///
  virtual
  void
  computeState(typename Traits::EvalData workset,
      std::map<std::string, Teuchos::RCP<PHX::MDField<ScalarT> > > dep_fields,
      std::map<std::string, Teuchos::RCP<PHX::MDField<ScalarT> > > eval_fields);

  virtual
  void
  computeStateParallel(typename Traits::EvalData workset,
      std::map<std::string, Teuchos::RCP<PHX::MDField<ScalarT> > > dep_fields,
      std::map<std::string, Teuchos::RCP<PHX::MDField<ScalarT> > > eval_fields);

private:

  ///
  /// Private to prohibit copying
  ///
  LinearHMCModel(const LinearHMCModel&);

  ///
  /// Private to prohibit copying
  ///
  LinearHMCModel& operator=(const LinearHMCModel&);

  ///
  /// material parameters
  ///
  RealType C11, C33, C12, C23, C44, C66;
  std::vector<RealType> lengthScale;
  std::vector<RealType> betaParameter;

  ///
  /// model parameters
  ///
  int numMicroScales;

  ///
  /// independent field names
  ///
  std::string macroStrainName;
  std::vector<std::string> strainDifferenceName;
  std::vector<std::string> microStrainGradientName;
  
  ///
  /// evaluated field names
  ///
  std::string macroStressName;
  std::vector<std::string> microStressName;
  std::vector<std::string> doubleStressName;

  typedef PHX::MDField<ScalarT,Cell,QuadPoint,Dim,Dim> HMC2Tensor;
  typedef PHX::MDField<ScalarT,Cell,QuadPoint,Dim,Dim,Dim> HMC3Tensor;

};
}

#endif
