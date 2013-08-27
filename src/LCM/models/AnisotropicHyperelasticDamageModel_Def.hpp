//*****************************************************************//
//    Albany 2.0:  Copyright 2012 Sandia Corporation               //
//    This Software is released under the BSD license detailed     //
//    in the file "license.txt" in the top-level Albany directory  //
//*****************************************************************//

#include <Intrepid_MiniTensor.h>
#include <Teuchos_TestForException.hpp>
#include <Phalanx_DataLayout.hpp>
#include <typeinfo>

namespace LCM {

  //----------------------------------------------------------------------------
  template<typename EvalT, typename Traits>
  AnisotropicHyperelasticDamageModel<EvalT, Traits>::
  AnisotropicHyperelasticDamageModel(Teuchos::ParameterList* p,
                                     const Teuchos::RCP<Albany::Layouts>& dl):
    LCM::ConstitutiveModel<EvalT,Traits>(p,dl),
    k_f1_(p->get<RealType>("Fiber 1 k", 1.0)),
    q_f1_(p->get<RealType>("Fiber 1 q", 1.0)),
    volume_fraction_f1_(p->get<RealType>("Fiber 1 volume fraction", 0.0)),
    max_damage_f1_(p->get<RealType>("Fiber 1 maximum damage", 1.0)),
    saturation_f1_(p->get<RealType>("Fiber 1 damage saturation", 0.0)),
    k_f2_(p->get<RealType>("Fiber 2 k", 1.0)),
    q_f2_(p->get<RealType>("Fiber 2 q", 1.0)),
    volume_fraction_f2_(p->get<RealType>("Fiber 2 volume fraction", 0.0)),
    max_damage_f2_(p->get<RealType>("Fiber 2 maximum damage", 1.0)),
    saturation_f2_(p->get<RealType>("Fiber 2 damage saturation", 0.0)),
    volume_fraction_m_(p->get<RealType>("Matrix volume fraction", 1.0)),
    max_damage_m_(p->get<RealType>("Matrix maximum damage", 1.0)),
    saturation_m_(p->get<RealType>("Matrix damage saturation", 0.0)),
    direction_f1_(p->get<Teuchos::Array<RealType> >("Fiber 1 Orientation Vector").toVector()),
    direction_f2_(p->get<Teuchos::Array<RealType> >("Fiber 2 Orientation Vector").toVector())
  {
    // define the dependent fields
    this->dep_field_map_.insert( std::make_pair("F", dl->qp_tensor) );
    this->dep_field_map_.insert( std::make_pair("J", dl->qp_scalar) );
    this->dep_field_map_.insert( std::make_pair("Poissons Ratio", dl->qp_scalar) );
    this->dep_field_map_.insert( std::make_pair("Elastic Modulus", dl->qp_scalar) );

    // retrive appropriate field name strings
    std::string cauchy_string = (*field_name_map_)["Cauchy_Stress"];
    std::string matrix_energy_string = (*field_name_map_)["Matrix_Energy"];
    std::string f1_energy_string = (*field_name_map_)["F1_Energy"];
    std::string f2_energy_string = (*field_name_map_)["F2_Energy"];
    std::string matrix_damage_string = (*field_name_map_)["Matrix_Damage"];
    std::string f1_damage_string = (*field_name_map_)["F1_Damage"];
    std::string f2_damage_string = (*field_name_map_)["F2_Damage"];


    // define the evaluated fields
    this->eval_field_map_.insert( std::make_pair(cauchy_string, dl->qp_tensor) );
    this->eval_field_map_.insert( std::make_pair(matrix_energy_string, dl->qp_scalar) );
    this->eval_field_map_.insert( std::make_pair(f1_energy_string, dl->qp_scalar) );
    this->eval_field_map_.insert( std::make_pair(f2_energy_string, dl->qp_scalar) );
    this->eval_field_map_.insert( std::make_pair(matrix_damage_string, dl->qp_scalar) );
    this->eval_field_map_.insert( std::make_pair(f1_damage_string, dl->qp_scalar) );
    this->eval_field_map_.insert( std::make_pair(f2_damage_string, dl->qp_scalar) );

    // define the state variables
    //
    // stress
    this->num_state_variables_++;
    this->state_var_names_.push_back(cauchy_string);
    this->state_var_layouts_.push_back(dl->qp_tensor);
    this->state_var_init_types_.push_back("scalar");
    this->state_var_init_values_.push_back(0.0);
    this->state_var_old_state_flags_.push_back(false);
    this->state_var_output_flags_.push_back(true);
    //
    // matrix energy
    this->num_state_variables_++;
    this->state_var_names_.push_back(matrix_energy_string);
    this->state_var_layouts_.push_back(dl->qp_scalar);
    this->state_var_init_types_.push_back("scalar");
    this->state_var_init_values_.push_back(0.0);
    this->state_var_old_state_flags_.push_back(true);
    this->state_var_output_flags_.push_back(true);
    //
    // fiber 1 energy
    this->num_state_variables_++;
    this->state_var_names_.push_back(f1_energy_string);
    this->state_var_layouts_.push_back(dl->qp_scalar);
    this->state_var_init_types_.push_back("scalar");
    this->state_var_init_values_.push_back(0.0);
    this->state_var_old_state_flags_.push_back(true);
    this->state_var_output_flags_.push_back(true);
    //
    // fiber 2 energy
    this->num_state_variables_++;
    this->state_var_names_.push_back(f2_energy_string);
    this->state_var_layouts_.push_back(dl->qp_scalar);
    this->state_var_init_types_.push_back("scalar");
    this->state_var_init_values_.push_back(0.0);
    this->state_var_old_state_flags_.push_back(true);
    this->state_var_output_flags_.push_back(true);
    //
    // matrix damage
    this->num_state_variables_++;
    this->state_var_names_.push_back(matrix_damage_string);
    this->state_var_layouts_.push_back(dl->qp_scalar);
    this->state_var_init_types_.push_back("scalar");
    this->state_var_init_values_.push_back(0.0);
    this->state_var_old_state_flags_.push_back(true);
    this->state_var_output_flags_.push_back(true);
    //
    // fiber 1 damage
    this->num_state_variables_++;
    this->state_var_names_.push_back(f1_damage_string);
    this->state_var_layouts_.push_back(dl->qp_scalar);
    this->state_var_init_types_.push_back("scalar");
    this->state_var_init_values_.push_back(0.0);
    this->state_var_old_state_flags_.push_back(true);
    this->state_var_output_flags_.push_back(true);
    //
    // fiber 2 damage
    this->num_state_variables_++;
    this->state_var_names_.push_back(f2_damage_string);
    this->state_var_layouts_.push_back(dl->qp_scalar);
    this->state_var_init_types_.push_back("scalar");
    this->state_var_init_values_.push_back(0.0);
    this->state_var_old_state_flags_.push_back(true);
    this->state_var_output_flags_.push_back(true);

  }
  //----------------------------------------------------------------------------
  template<typename EvalT, typename Traits>
  void AnisotropicHyperelasticDamageModel<EvalT, Traits>::
  computeState(typename Traits::EvalData workset,
               std::map<std::string,Teuchos::RCP<PHX::MDField<ScalarT> > > dep_fields,
               std::map<std::string,Teuchos::RCP<PHX::MDField<ScalarT> > > eval_fields)
  {
    bool print = false;
    //if (typeid(ScalarT) == typeid(RealType)) print = true;
    //cout.precision(15);

    // extract dependent MDFields
    PHX::MDField<ScalarT> def_grad        = *dep_fields["F"];
    PHX::MDField<ScalarT> J               = *dep_fields["J"];
    PHX::MDField<ScalarT> poissons_ratio  = *dep_fields["Poissons Ratio"];
    PHX::MDField<ScalarT> elastic_modulus = *dep_fields["Elastic Modulus"];

    // retrive appropriate field name strings
    std::string cauchy_string        = (*field_name_map_)["Cauchy_Stress"];
    std::string matrix_energy_string = (*field_name_map_)["Matrix_Energy"];
    std::string f1_energy_string     = (*field_name_map_)["F1_Energy"];
    std::string f2_energy_string     = (*field_name_map_)["F2_Energy"];
    std::string matrix_damage_string = (*field_name_map_)["Matrix_Damage"];
    std::string f1_damage_string     = (*field_name_map_)["F1_Damage"];
    std::string f2_damage_string     = (*field_name_map_)["F2_Damage"];

    // extract evaluated MDFields
    PHX::MDField<ScalarT> stress    = *eval_fields[cauchy_string];
    PHX::MDField<ScalarT> energy_m  = *eval_fields[matrix_energy_string];
    PHX::MDField<ScalarT> energy_f1 = *eval_fields[f1_energy_string];
    PHX::MDField<ScalarT> energy_f2 = *eval_fields[f2_energy_string];
    PHX::MDField<ScalarT> damage_m  = *eval_fields[matrix_damage_string];
    PHX::MDField<ScalarT> damage_f1 = *eval_fields[f1_damage_string];
    PHX::MDField<ScalarT> damage_f2 = *eval_fields[f2_damage_string];

    // previous state
    Albany::MDArray energy_m_old = 
      (*workset.stateArrayPtr)[matrix_energy_string+"_old"];
    Albany::MDArray energy_f1_old = 
      (*workset.stateArrayPtr)[f1_energy_string+"_old"];
    Albany::MDArray energy_f2_old = 
      (*workset.stateArrayPtr)[f2_energy_string+"_old"];

    ScalarT kappa,mu,Jm53,Jm23,p,I4_f1,I4_f2;
    ScalarT alpha_f1, alpha_f2, alpha_m;

    // Define some tensors for use
    Intrepid::Tensor<ScalarT> I(Intrepid::eye<ScalarT>(num_dims_));
    Intrepid::Tensor<ScalarT> F(num_dims_), s(num_dims_), b(num_dims_), C(num_dims_);
    Intrepid::Tensor<ScalarT> sigma_m(num_dims_), sigma_f1(num_dims_), sigma_f2(num_dims_);
    Intrepid::Tensor<ScalarT> M1dyadM1(num_dims_), M2dyadM2(num_dims_);
    Intrepid::Tensor<ScalarT> S0_f1(num_dims_), S0_f2(num_dims_);

    Intrepid::Vector<ScalarT> M1(num_dims_), M2(num_dims_);


    for (std::size_t cell = 0; cell < workset.numCells; ++cell) {
      for (std::size_t pt = 0; pt < num_pts_; ++pt) {
        // local parameters
        kappa = elastic_modulus(cell, pt)
          / (3. * (1. - 2. * poissons_ratio(cell, pt)));
        mu = elastic_modulus(cell, pt) / (2. * (1. + poissons_ratio(cell, pt)));
        Jm53 = std::pow(J(cell, pt), -5. / 3.);
        Jm23 = std::pow(J(cell, pt), -2. / 3.);
        F.fill( &def_grad(cell, pt, 0, 0));
        
        // compute deviatoric stress
        b = F*Intrepid::transpose(F);
        s = mu * Jm53 * Intrepid::dev(b);
        // compute pressure
        p = 0.5 * kappa * (J(cell, pt) - 1. / (J(cell, pt)));

        sigma_m = s + p*I;

        // compute energy for M
        energy_m(cell, pt) = 0.5 * kappa
          * (0.5 * (J(cell, pt) * J(cell, pt) - 1.0) - std::log(J(cell, pt)))
          + 0.5 * mu * ( Jm23*Intrepid::trace(b) - 3.0);

        // damage term in M
        alpha_m = energy_m_old(cell, pt);
        if (energy_m(cell, pt) > alpha_m) alpha_m = energy_m(cell, pt);

        damage_m(cell, pt) = max_damage_m_ * (1 - std::exp(-alpha_m / saturation_m_));

        //-----------compute stress in Fibers

        // Right Cauchy-Green Tensor C = F^{T} * F
        C = Intrepid::transpose(F)*F;

        // Fiber orientation vectors
        //
        // fiber 1
        for (std::size_t i = 0; i < num_dims_; ++i) {
          M1(i) = direction_f1_[i];
        }
        M1 = M1/norm(M1);

        // fiber 2
        for (std::size_t i = 0; i < num_dims_; ++i) {
          M2(i) = direction_f2_[i];
        }
        M2 = M2/norm(M2);

        // Anisotropic invariants I4 = M_{i} * C * M_{i}
        I4_f1 = Intrepid::dot(M1, Intrepid::dot(C, M1));
        I4_f2 = Intrepid::dot(M2, Intrepid::dot(C, M2));
        M1dyadM1 = Intrepid::dyad(M1, M1);
        M2dyadM2 = Intrepid::dyad(M2, M2);

        // undamaged stress (2nd PK stress)
        S0_f1 = (4.0 * k_f1_ * (I4_f1 - 1.0)
                 * std::exp(q_f1_ * (I4_f1 - 1) * (I4_f1 - 1))) * M1dyadM1;
        S0_f2 = (4.0 * k_f2_ * (I4_f2 - 1.0)
                 * std::exp(q_f2_ * (I4_f2 - 1) * (I4_f2 - 1))) * M2dyadM2;
        
        // compute energy for fibers
        energy_f1(cell, pt) = k_f1_
          * (std::exp(q_f1_ * (I4_f1 - 1) * (I4_f1 - 1)) - 1) / q_f1_;
        energy_f2(cell, pt) = k_f2_
          * (std::exp(q_f2_ * (I4_f2 - 1) * (I4_f2 - 1)) - 1) / q_f2_;

        // Fiber Cauchy stress
        sigma_f1 = (1.0 / J(cell, pt))
          * Intrepid::dot(F, Intrepid::dot(S0_f1, Intrepid::transpose(F)));
        sigma_f2 = (1.0 / J(cell, pt))
          * Intrepid::dot(F, Intrepid::dot(S0_f2, Intrepid::transpose(F)));

        // maximum thermodynamic forces
        alpha_f1 = energy_f1_old(cell, pt);
        alpha_f2 = energy_f2_old(cell, pt);

        if (energy_f1(cell, pt) > alpha_f1) alpha_f1 = energy_f1(cell, pt);

        if (energy_f2(cell, pt) > alpha_f2) alpha_f2 = energy_f2(cell, pt);

        // damage term in fibers
        damage_f1(cell, pt) = max_damage_f1_ * (1 - std::exp(-alpha_f1 / saturation_f1_));
        damage_f2(cell, pt) = max_damage_f2_ * (1 - std::exp(-alpha_f2 / saturation_f2_));

        // total Cauchy stress (M, Fibers)
        for (std::size_t i(0); i < num_dims_; ++i) {
          for (std::size_t j(0); j < num_dims_; ++j) {
            stress(cell, pt, i, j) = 
              volume_fraction_m_ * (1 - damage_m(cell, pt)) * sigma_m(i, j)
              + volume_fraction_f1_ * (1 - damage_f1(cell, pt)) * sigma_f1(i, j)
              + volume_fraction_f2_ * (1 - damage_f2(cell, pt)) * sigma_f2(i, j);
          }
        }

        if (print) {
          std::cout << "  matrix damage: " << damage_m(cell,pt) << std::endl;
          std::cout << "  matrix energy: " << energy_m(cell,pt) << std::endl;
          std::cout << "  fiber1 damage: " << damage_f1(cell,pt) << std::endl;
          std::cout << "  fiber1 energy: " << energy_f1(cell,pt) << std::endl;
          std::cout << "  fiber2 damage: " << damage_f2(cell,pt) << std::endl;
          std::cout << "  fiber2 energy: " << energy_f2(cell,pt) << std::endl;
        }
      } // pt
    } // cell
  }
  //----------------------------------------------------------------------------
} 
