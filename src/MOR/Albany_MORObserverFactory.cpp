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

#include "Albany_MORObserverFactory.hpp"

#include "Albany_NOXObserver.hpp"
#include "Albany_SnapshotCollectionObserver.hpp"

#include "Teuchos_ParameterList.hpp"

#include <string>

namespace Albany {

using ::Teuchos::RCP;
using ::Teuchos::rcp;
using ::Teuchos::ParameterList;
using ::Teuchos::sublist;

MORObserverFactory::MORObserverFactory(const RCP<ParameterList> &parentParams) :
  params_(sublist(parentParams, "Model Order Reduction"))
{
  // Nothing to do
}

RCP<NOX::Epetra::Observer> MORObserverFactory::create(const RCP<NOX::Epetra::Observer> &child)
{
  if (collectSnapshots())
  {
    return rcp(new SnapshotCollectionObserver(getSnapParameters(), child));
  }
  else
  {
    return child;
  }
}

bool MORObserverFactory::collectSnapshots() const
{
  return getSnapParameters()->get("Activate", false);
}

RCP<ParameterList> MORObserverFactory::getSnapParameters() const
{
  return sublist(params_, "Snapshot Collection");
}

} // end namespace Albany
