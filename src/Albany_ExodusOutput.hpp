/********************************************************************\
*            Albany, Copyright (2010) Sandia Corporation             *
*                                                                    *
* Notice: This computer software was prepared by Sandia Corporation, *
* hereinafter the Contractor, under Contract DE-AC04-94AL85000 with  *
* the Department of Energy (DOE). All rights in the computer software*
* are reserved by DOE on behalf of the United States Government and  *
* the Contractor as provided in the Contract. You are authorized to  *
* use this computer software for Governmental purposes but it is not *
* to be released or distributed to the public. NEITHER THE GOVERNMENT*
* NOR THE CONTRACTOR MAKES ANY WARRANTY, EXPRESS OR IMPLIED, OR      *
* ASSUMES ANY LIABILITY FOR THE USE OF THIS SOFTWARE. This notice    *
* including this sentence must appear on any copies of this software.*
*    Questions to Andy Salinger, agsalin@sandia.gov                  *
\********************************************************************/

#ifndef ALBANY_EXODUSOUTPUT_HPP
#define ALBANY_EXODUSOUTPUT_HPP

#include "Epetra_Vector.h"
#include "Teuchos_RCP.hpp"

namespace Teuchos {
  class Time;
}

namespace Albany {

class AbstractDiscretization;
class STKDiscretization;

class ExodusOutput {
public:
   void writeSolution(double stamp, const Epetra_Vector &solution);

   explicit ExodusOutput(const Teuchos::RCP<AbstractDiscretization> &disc);

private:
   Teuchos::RCP<STKDiscretization> stkDisc_;

   Teuchos::RCP<Teuchos::Time> exoOutTime_;

   // Disallow copy and assignment
   ExodusOutput(const ExodusOutput &);
   ExodusOutput &operator=(const ExodusOutput &);
};

}

#endif //ALBANY_EXODUSOUTPUT_HPP