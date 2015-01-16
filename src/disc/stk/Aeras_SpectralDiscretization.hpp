//*****************************************************************//
//    Albany 2.0:  Copyright 2012 Sandia Corporation               //
//    This Software is released under the BSD license detailed     //
//    in the file "license.txt" in the top-level Albany directory  //
//*****************************************************************//


#ifndef AERAS_SPECTRALDISCRETIZATION_HPP
#define AERAS_SPECTRALDISCRETIZATION_HPP

#include <vector>
#include <utility>

#include "Teuchos_ParameterList.hpp"
#include "Teuchos_VerboseObject.hpp"


#include "Albany_AbstractDiscretization.hpp"
#include "Albany_AbstractSTKMeshStruct.hpp"
#include "Albany_DataTypes.hpp"

#ifdef ALBANY_EPETRA
#include "Epetra_Comm.h"
#include "Epetra_CrsMatrix.h"
#include "Epetra_Vector.h"
#endif

#include "Albany_NullSpaceUtils.hpp"

// Start of STK stuff
#include <stk_util/parallel/Parallel.hpp>
#include <stk_mesh/base/Types.hpp>
#include <stk_mesh/base/MetaData.hpp>
#include <stk_mesh/base/BulkData.hpp>
#include <stk_mesh/base/Field.hpp>
#include <stk_mesh/base/FieldTraits.hpp>
#ifdef ALBANY_SEACAS
  #include <stk_io/StkMeshIoBroker.hpp>
#endif


namespace Aeras {

#ifdef ALBANY_EPETRA
  typedef shards::Array<GO, shards::NaturalOrder> GIDArray;

  struct DOFsStruct {
    Teuchos::RCP<Epetra_Map> node_map;
    Teuchos::RCP<Epetra_Map> overlap_node_map;
    Teuchos::RCP<Epetra_Map> map;
    Teuchos::RCP<Epetra_Map> overlap_map;
    Albany::NodalDOFManager dofManager;
    Albany::NodalDOFManager overlap_dofManager;
    std::vector< std::vector<LO> > wsElNodeEqID_rawVec;
    std::vector<Albany::IDArray> wsElNodeEqID;
    std::vector< std::vector<GO> > wsElNodeID_rawVec;
    std::vector<GIDArray> wsElNodeID;
  };

  struct NodalDOFsStructContainer {
    typedef std::map<std::pair<std::string,int>,  DOFsStruct >  MapOfDOFsStructs;

    MapOfDOFsStructs mapOfDOFsStructs;
    std::map<std::string, MapOfDOFsStructs::const_iterator> fieldToMap;
    const DOFsStruct& getDOFsStruct(const std::string& field_name) const {return fieldToMap.find(field_name)->second->second;}; //TODO handole errors

    void addEmptyDOFsStruct(const std::string& field_name, const std::string& meshPart, int numComps){
    
      if(numComps != 1)
        mapOfDOFsStructs.insert(make_pair(make_pair(meshPart,1),DOFsStruct()));

      fieldToMap[field_name] = mapOfDOFsStructs.insert(make_pair(make_pair(meshPart,numComps),DOFsStruct())).first;
    }
    
  };
#endif // ALBANY_EPETRA

  class SpectralDiscretization : public Albany::AbstractDiscretization {
  public:

    //! Constructor
    SpectralDiscretization(
       Teuchos::RCP<Albany::AbstractSTKMeshStruct> stkMeshStruct,
       const Teuchos::RCP<const Teuchos_Comm>& commT,
       const Teuchos::RCP<Albany::RigidBodyModes>& rigidBodyModes = Teuchos::null);


    //! Destructor
    ~SpectralDiscretization();

#ifdef ALBANY_EPETRA
    //! Get Epetra DOF map
    Teuchos::RCP<const Epetra_Map> getMap() const;
#endif
    //! Get Tpetra DOF map
    Teuchos::RCP<const Tpetra_Map> getMapT() const;

#ifdef ALBANY_EPETRA
    //! Get Epetra overlapped DOF map
    Teuchos::RCP<const Epetra_Map> getOverlapMap() const;
#endif
    //! Get Tpetra overlapped DOF map
    Teuchos::RCP<const Tpetra_Map> getOverlapMapT() const;

#ifdef ALBANY_EPETRA
    //! Get field DOF map
    Teuchos::RCP<const Epetra_Map> getMap(const std::string& field_name) const;

    //! Get field overlapped DOF map
    Teuchos::RCP<const Epetra_Map> getOverlapMap(const std::string& field_name) const;
#endif

#ifdef ALBANY_EPETRA
    //! Get Epetra Jacobian graph
    Teuchos::RCP<const Epetra_CrsGraph> getJacobianGraph() const;
#endif
    //! Get Tpetra Jacobian graph
    Teuchos::RCP<const Tpetra_CrsGraph> getJacobianGraphT() const;

#ifdef ALBANY_EPETRA
    //! Get Epetra overlap Jacobian graph
    Teuchos::RCP<const Epetra_CrsGraph> getOverlapJacobianGraph() const;
#endif
    //! Get Tpetra overlap Jacobian graph
    Teuchos::RCP<const Tpetra_CrsGraph> getOverlapJacobianGraphT() const;

#ifdef ALBANY_EPETRA
    //! Get Epetra Node map
    Teuchos::RCP<const Epetra_Map> getNodeMap() const; 
    //! Get overlapped Node map
    Teuchos::RCP<const Epetra_Map> getOverlapNodeMap() const;
#endif
    //! Get Tpetra Node map
    Teuchos::RCP<const Tpetra_Map> getNodeMapT() const; 
    Teuchos::RCP<const Tpetra_Map> getOverlapNodeMapT() const;

    //! Get Node set lists (typedef in Albany_AbstractDiscretization.hpp)
    const Albany::NodeSetList& getNodeSets() const { return nodeSets; };
    const Albany::NodeSetCoordList& getNodeSetCoords() const { return nodeSetCoords; };

    //! Get Side set lists (typedef in Albany_AbstractDiscretization.hpp)
    const Albany::SideSetList& getSideSets(const int workset) const { return sideSets[workset]; };

    //! Get connectivity map from elementGID to workset
    Albany::WsLIDList& getElemGIDws() { return elemGIDws; };

    //! Get map from (Ws, El, Local Node) -> NodeLID
    const Albany::WorksetArray<Teuchos::ArrayRCP<Teuchos::ArrayRCP<Teuchos::ArrayRCP<LO> > > >::type& getWsElNodeEqID() const;

    //! Get map from (Ws, Local Node) -> NodeGID
    const Albany::WorksetArray<Teuchos::ArrayRCP<Teuchos::ArrayRCP<GO> > >::type& getWsElNodeID() const;

#ifdef ALBANY_EPETRA
    //! Get IDArray for (Ws, Local Node, nComps) -> (local) NodeLID, works for both scalar and vector fields
    const std::vector<Albany::IDArray>& getElNodeEqID(const std::string& field_name) const
        {return nodalDOFsStructContainer.getDOFsStruct(field_name).wsElNodeEqID;}
#endif

    //! Retrieve coodinate vector (num_used_nodes * 3)
    const Teuchos::ArrayRCP<double>& getCoordinates() const;
    void setCoordinates(const Teuchos::ArrayRCP<const double>& c);
    void setReferenceConfigurationManager(const Teuchos::RCP<AAdapt::rc::Manager>& rcm);

    const Albany::WorksetArray<Teuchos::ArrayRCP<Teuchos::ArrayRCP<double*> > >::type& getCoords() const;
    const Albany::WorksetArray<Teuchos::ArrayRCP<double> >::type& getSphereVolume() const;

    //! Print the coordinates for debugging

    void printCoords() const;

    //! Get stateArrays
    Albany::StateArrays& getStateArrays() {return stateArrays;}

    //! Get nodal parameters state info struct
    const Albany::StateInfoStruct& getNodalParameterSIS() const
      {return stkMeshStruct->getFieldContainer()->getNodalParameterSIS();}

    //! Retrieve Vector (length num worksets) of element block names
    const Albany::WorksetArray<std::string>::type&  getWsEBNames() const;
    //! Retrieve Vector (length num worksets) of physics set index
    const Albany::WorksetArray<int>::type&  getWsPhysIndex() const;

#ifdef ALBANY_EPETRA
    void writeSolution(const Epetra_Vector& soln, const double time, const bool overlapped = false);
#endif
   
   void writeSolutionT(const Tpetra_Vector& solnT, const double time, const bool overlapped = false);
   void writeSolutionToMeshDatabaseT(const Tpetra_Vector &solutionT, const double time, const bool overlapped = false);
   void writeSolutionToFileT(const Tpetra_Vector& solnT, const double time, const bool overlapped = false);

#ifdef ALBANY_EPETRA 
    Teuchos::RCP<Epetra_Vector> getSolutionField(const bool overlapped=false) const;
#endif
    //Tpetra analog
    Teuchos::RCP<Tpetra_Vector> getSolutionFieldT(const bool overlapped=false) const;

    int getSolutionFieldHistoryDepth() const;
#ifdef ALBANY_EPETRA
    Teuchos::RCP<Epetra_MultiVector> getSolutionFieldHistory() const;
    Teuchos::RCP<Epetra_MultiVector> getSolutionFieldHistory(int maxStepCount) const;
    void getSolutionFieldHistory(Epetra_MultiVector &result) const;

    void setResidualField(const Epetra_Vector& residual);
#endif
    //Tpetra analog
    void setResidualFieldT(const Tpetra_Vector& residualT);

    // Retrieve mesh struct
    Teuchos::RCP<Albany::AbstractSTKMeshStruct> getSTKMeshStruct() {return stkMeshStruct;}
    Teuchos::RCP<Albany::AbstractMeshStruct> getMeshStruct() const {return stkMeshStruct;}

    //! Flag if solution has a restart values -- used in Init Cond
    bool hasRestartSolution() const {return stkMeshStruct->hasRestartSolution();}

    //! STK supports MOR
    virtual bool supportsMOR() const { return true; }

    //! If restarting, convenience function to return restart data time
    double restartDataTime() const {return stkMeshStruct->restartDataTime();}

    //! After mesh modification, need to update the element connectivity and nodal coordinates
    void updateMesh(bool shouldTransferIPData = false);

    //! Function that transforms an STK mesh of a unit cube (for FELIX problems)
    void transformMesh();

    //! Close current exodus file in stk_io and create a new one for an adapted mesh and new results
    void reNameExodusOutput(std::string& filename);

   //! Get number of spatial dimensions
    int getNumDim() const { return stkMeshStruct->numDim; }

    //! Get number of total DOFs per node
    int getNumEq() const { return neq; }

    //! Locate nodal dofs in non-overlapping vectors using local indexing
    int getOwnedDOF(const int inode, const int eq) const;

    //! Locate nodal dofs in overlapping vectors using local indexing
    int getOverlapDOF(const int inode, const int eq) const;

    //! Locate nodal dofs using global indexing
    GO getGlobalDOF(const GO inode, const int eq) const;


    //! used when NetCDF output on a latitude-longitude grid is requested.
    // Each struct contains a latitude/longitude index and it's parametric
    // coordinates in an element.
    struct interp {
      std::pair<double, double> parametric_coords;
      std::pair<unsigned, unsigned> latitude_longitude;
    };

    const stk::mesh::MetaData& getSTKMetaData(){ return metaData; }

    const stk::mesh::BulkData& getSTKBulkData(){ return bulkData; }

  private:

    //! Private to prohibit copying
    SpectralDiscretization(const SpectralDiscretization&);

    //! Private to prohibit copying
    SpectralDiscretization& operator=(const SpectralDiscretization&);

    inline GO gid(const stk::mesh::Entity node) const;

#ifdef ALBANY_EPETRA
    // Copy values from STK Mesh field to given Epetra_Vector
    void getSolutionField(Epetra_Vector &result, bool overlapped=false) const;
#endif
    // Copy values from STK Mesh field to given Tpetra_Vector
    void getSolutionFieldT(Tpetra_Vector &resultT, bool overlapped=false) const;

#ifdef ALBANY_EPETRA
    //! Copy field from STK Mesh field to given Epetra_Vector
    void getField(Epetra_Vector &field_vector, const std::string& field_name) const;

    // Copy field vector into STK Mesh field
    void setField(const Epetra_Vector &field_vector, const std::string& field_name, bool overlapped=false);

    Teuchos::RCP<Epetra_MultiVector> getSolutionFieldHistoryImpl(int stepCount) const;
    void getSolutionFieldHistoryImpl(Epetra_MultiVector &result) const;

    // Copy solution vector from Epetra_Vector into STK Mesh
    // Here soln is the local (non overlapped) solution
    void setSolutionField(const Epetra_Vector& soln);
#endif
    //Tpetra version of above
    void setSolutionFieldT(const Tpetra_Vector& solnT);

    // Copy solution vector from Epetra_Vector into STK Mesh
    // Here soln is the local + neighbor (overlapped) solution
#ifdef ALBANY_EPETRA
    void setOvlpSolutionField(const Epetra_Vector& soln);
#endif
    //Tpetra version of above
    void setOvlpSolutionFieldT(const Tpetra_Vector& solnT);

    int nonzeroesPerRow(const int neq) const;
    double monotonicTimeLabel(const double time);

#ifdef ALBANY_EPETRA
    void computeNodalEpetraMaps(bool overlapped);
#endif

    //! Process STK mesh for Owned nodal quantitites
    void computeOwnedNodesAndUnknowns();
    //! Process coords for ML
    void setupMLCoords();
    //! Process STK mesh for Overlap nodal quantitites
    void computeOverlapNodesAndUnknowns();
    //! Process STK mesh for CRS Graphs
    void computeGraphs();
    //! Process STK mesh for Workset/Bucket Info
    void computeWorksetInfo();
    //! Process STK mesh for NodeSets
    void computeNodeSets();
    //! Process STK mesh for SideSets
    void computeSideSets();
    //! Call stk_io for creating exodus output file
    void setupExodusOutput();
    //! Call stk_io for creating NetCDF output file
    void setupNetCDFOutput();
#ifdef ALBANY_EPETRA
    int processNetCDFOutputRequest(const Epetra_Vector&);
#endif
    int processNetCDFOutputRequestT(const Tpetra_Vector&);

    //! Find the local side id number within parent element
    unsigned determine_local_side_id( const stk::mesh::Entity elem , stk::mesh::Entity side );
    //! Call stk_io for creating exodus output file
    Teuchos::RCP<Teuchos::FancyOStream> out;

    //! Convert the stk mesh on this processor to a nodal graph using SEACAS
    void meshToGraph();

    void writeCoordsToMatrixMarket() const;

    double previous_time_label;

  protected:


    //! Stk Mesh Objects
    stk::mesh::MetaData& metaData;
    stk::mesh::BulkData& bulkData;

#ifdef ALBANY_EPETRA
    //! Epetra communicator
    Teuchos::RCP<const Epetra_Comm> comm;
#endif

    //! Tpetra communicator and Kokkos node
    Teuchos::RCP<const Teuchos::Comm<int> > commT;

    //! Unknown map and node map
    Teuchos::RCP<const Tpetra_Map> node_mapT; 
    Teuchos::RCP<const Tpetra_Map> mapT; 

    //! Overlapped unknown map and node map
    Teuchos::RCP<const Tpetra_Map> overlap_mapT; 
    Teuchos::RCP<const Tpetra_Map> overlap_node_mapT; 

#ifdef ALBANY_EPETRA
    Teuchos::RCP<Epetra_Map> node_map;
    Teuchos::RCP<Epetra_Map> map;
    Teuchos::RCP<Epetra_Map> overlap_node_map;
    Teuchos::RCP<Epetra_Map> overlap_map;

    NodalDOFsStructContainer nodalDOFsStructContainer;
#endif


    //! Jacobian matrix graph
    Teuchos::RCP<Tpetra_CrsGraph> graphT; 

    //! Overlapped Jacobian matrix graph
    Teuchos::RCP<Tpetra_CrsGraph> overlap_graphT; 

    //! Processor ID
    unsigned int myPID;

    //! Number of equations (and unknowns) per node
    const unsigned int neq;

    //! Number of elements on this processor
    unsigned int numMyElements;

    //! node sets stored as std::map(string ID, int vector of GIDs)
    Albany::NodeSetList nodeSets;
    Albany::NodeSetCoordList nodeSetCoords;

    //! side sets stored as std::map(string ID, SideArray classes) per workset (std::vector across worksets)
    std::vector<Albany::SideSetList> sideSets;

    //! Connectivity array [workset, element, local-node, Eq] => LID
    Albany::WorksetArray<Teuchos::ArrayRCP<Teuchos::ArrayRCP<Teuchos::ArrayRCP<LO> > > >::type wsElNodeEqID;

    //! Connectivity array [workset, element, local-node] => GID
    Albany::WorksetArray<Teuchos::ArrayRCP<Teuchos::ArrayRCP<GO> > >::type wsElNodeID;

    mutable Teuchos::ArrayRCP<double> coordinates;
    Albany::WorksetArray<std::string>::type wsEBNames;
    Albany::WorksetArray<int>::type wsPhysIndex;
    Albany::WorksetArray<Teuchos::ArrayRCP<Teuchos::ArrayRCP<double*> > >::type coords;
    Albany::WorksetArray<Teuchos::ArrayRCP<double> >::type sphereVolume;

    //! Connectivity map from elementGID to workset and LID in workset
    Albany::WsLIDList  elemGIDws;

    // States: vector of length worksets of a map from field name to shards array
    Albany::StateArrays stateArrays;
    std::vector<std::vector<std::vector<double> > > nodesOnElemStateVec;

    //! list of all owned nodes, saved for setting solution
    std::vector< stk::mesh::Entity > ownednodes ;
    std::vector< stk::mesh::Entity > cells ;

    //! list of all overlap nodes, saved for getting coordinates for mesh motion
    std::vector< stk::mesh::Entity > overlapnodes ;

    //! Number of elements on this processor
    int numOwnedNodes;
    int numOverlapNodes;
    GO numGlobalNodes;

    // Needed to pass coordinates to ML.
    Teuchos::RCP<Albany::RigidBodyModes> rigidBodyModes;

    int netCDFp;
    size_t netCDFOutputRequest;
    std::vector<int> varSolns;
    Albany::WorksetArray<Teuchos::ArrayRCP<std::vector<interp> > >::type interpolateData;

    // Storage used in periodic BCs to un-roll coordinates. Pointers saved for destructor.
    std::vector<double*>  toDelete;

    Teuchos::RCP<Albany::AbstractSTKMeshStruct> stkMeshStruct;

    // Used in Exodus writing capability
#ifdef ALBANY_SEACAS
    Teuchos::RCP<stk::io::StkMeshIoBroker> mesh_data;

    int outputInterval;

    size_t outputFileIdx;
#endif
    bool interleavedOrdering;

  private:

    Teuchos::RCP<Tpetra_CrsGraph> nodalGraph;


    // find the location of "value" within the first "count" locations of "vector"
    ssize_t in_list(const std::size_t value, std::size_t count, std::size_t *vector) {

      for(std::size_t i=0; i < count; i++) {
        if(vector[i] == value)
          return i;
      }
       return -1;
    }

    ssize_t in_list(const std::size_t value, const Teuchos::Array<GO>& vector) {
      for (std::size_t i=0; i < vector.size(); i++)
        if (vector[i] == value)
          return i;
      return -1;
    }

    ssize_t in_list(const std::size_t value, const std::vector<std::size_t>& vector) {
      for (std::size_t i=0; i < vector.size(); i++)
        if (vector[i] == value)
          return i;
      return -1;
    }

    ssize_t entity_in_list(const stk::mesh::Entity& value,
                           const std::vector<stk::mesh::Entity>& vec) {
      for (std::size_t i = 0; i < vec.size(); i++)
        if (bulkData.identifier(vec[i]) == bulkData.identifier(value))
          return i;
      return -1;
    }

    void printVertexConnectivity();

  };

}

#endif // AERAS_SPECTRALDISCRETIZATION_HPP