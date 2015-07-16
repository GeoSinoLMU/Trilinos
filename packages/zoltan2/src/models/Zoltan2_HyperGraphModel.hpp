// @HEADER
//
// ***********************************************************************
//
//   Zoltan2: A package of combinatorial algorithms for scientific computing
//                  Copyright 2012 Sandia Corporation
//
// Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
// the U.S. Government retains certain rights in this software.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// 1. Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the Corporation nor the names of the
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY SANDIA CORPORATION "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SANDIA CORPORATION OR THE
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Questions? Contact Karen Devine      (kddevin@sandia.gov)
//                    Erik Boman        (egboman@sandia.gov)
//                    Siva Rajamanickam (srajama@sandia.gov)
//
// ***********************************************************************
//
// @HEADER

/*! \file Zoltan2_HyperGraphModel.hpp
    \brief Defines the HyperGraphModel interface.
*/

#ifndef _ZOLTAN2_HYPERGRAPHMODEL_HPP_
#define _ZOLTAN2_HYPERGRAPHMODEL_HPP_

#include <Zoltan2_Model.hpp>
#include <Zoltan2_ModelHelpers.hpp>
#include <Zoltan2_InputTraits.hpp>
#include <Zoltan2_MatrixAdapter.hpp>
#include <Zoltan2_GraphAdapter.hpp>
#include <Zoltan2_IdentifierAdapter.hpp>
#include <Zoltan2_VectorAdapter.hpp>
#include <Zoltan2_StridedData.hpp>
#include <Zoltan2_MeshAdapter.hpp>

#include <vector>
#include <map>
#include <queue>
#include <Teuchos_Hashtable.hpp>

namespace Zoltan2 {

  /*! \brief Enumerate the views for the pins:
   *    HYPEREDGE_CENTRIC: pins are the global ids of the vertices
   *    VERTEX_CENTRIC: pins are the global ids of the hyperedges
   */
enum CentricView {
  HYPEREDGE_CENTRIC,
  VERTEX_CENTRIC
};

//////////////////////////////////////////////////////////////////////////
/*!  \brief HyperGraphModel defines the interface required for hyper graph models.

    The constructor of the HyperGraphModel can be a global call, requiring
    all processes in the application to call it.  The rest of the
    methods should be local methods.

    The template parameter is an InputAdapter, which is an object that
    provides a uniform interface for models to the user's input data.
*/
template <typename Adapter>
class HyperGraphModel : public Model<Adapter>
{
public:

#ifndef DOXYGEN_SHOULD_SKIP_THIS
  typedef typename Adapter::scalar_t    scalar_t;
  typedef typename Adapter::gno_t       gno_t;
  typedef typename Adapter::lno_t       lno_t;
  typedef typename Adapter::zgid_t       zgid_t;
  typedef typename Adapter::node_t      node_t;
  typedef typename Adapter::user_t      user_t;
  typedef typename Adapter::userCoord_t userCoord_t;
  typedef IdentifierMap<user_t>         idmap_t;
  typedef StridedData<lno_t, scalar_t>  input_t;
#endif

  //!  Destructor
  ~HyperGraphModel() { }

  /*! \brief Constructor
   *
   *  \param  inputAdapter  a pointer to the user's data
   *  \param  env           object containing the parameters
   *  \param  comm          communicator for the problem
   *  \param  modelFlags    a bit map of Zoltan2::GraphModelFlags
   *
   *  All processes in the communicator must call the constructor.
   *  \todo document the model flags that might be set
   */
  
  HyperGraphModel(const RCP<const MatrixAdapter<user_t,userCoord_t> > &ia,
    const RCP<const Environment> &env, const RCP<const Comm<int> > &comm,
    modelFlag_t &modelFlags)
  {
    throw std::runtime_error("Building HyperGraphModel from MatrixAdapter not implemented yet");
  }

  HyperGraphModel(const RCP<const GraphAdapter<user_t,userCoord_t> > &ia,
    const RCP<const Environment> &env, const RCP<const Comm<int> > &comm,
    modelFlag_t &modelFlags)
  {
    throw std::runtime_error("Building HyperGraphModel from GraphAdapter not implemented yet");
  }
  
  HyperGraphModel(const RCP<const MeshAdapter<user_t> > &ia,
    const RCP<const Environment> &env, const RCP<const Comm<int> > &comm,
                  modelFlag_t &modelflags, CentricView view);
  
  HyperGraphModel(const RCP<const VectorAdapter<userCoord_t> > &ia,
    const RCP<const Environment> &env, const RCP<const Comm<int> > &comm,
    modelFlag_t &flags)
  {
    throw std::runtime_error("cannot build HyperGraphModel from VectorAdapter");
  }

  HyperGraphModel(const RCP<const IdentifierAdapter<user_t> > &ia,
    const RCP<const Environment> &env, const RCP<const Comm<int> > &comm,
    modelFlag_t &flags)
  {
    throw std::runtime_error("cannot build HyperGraphModel from IdentifierAdapter");
  }
  

  /*! \brief Returns the centric view of the hypergraph
   */
  CentricView getCentricView() const {return view_;}

  /*! \brief Returns the number vertices on this process.
   */
  size_t getLocalNumVertices() const { return numLocalVertices_; }

  /*! \brief Returns the global number vertices.
   */
  size_t getGlobalNumVertices() const { return numGlobalVertices_; }

  /*! \brief Returns the number of hyper edges on this process.
   *  These are all hyper edges that have an adjacency to at 
   *  least one on process vertex.
   */
  size_t getLocalNumHyperEdges() const { return numLocalEdges_; }

  /*! \brief Returns the global number hyper edges.
   */
  size_t getGlobalNumHyperEdges() const { return numGlobalEdges_; }

  /*! \brief Returns the local number of pins
   */
  size_t getLocalNumPins() const {return numLocalPins_; }

  /*! \brief Returns the number (0 or greater) of weights per vertex
   */
  int getNumWeightsPerVertex() const { return numWeightsPerVertex_; }

  /*! \brief Returns the number (0 or greater) of weights per edge.
   */
  int getNumWeightsPerHyperEdge() const { return nWeightsPerEdge_; }

  /*! \brief Returns the number (0 or greater) of weights per pins.
   */
  int getNumWeightesPerPin() const {return nWeightsPerPin_;}

  /*! \brief Returns the dimension (0 to 3) of vertex coordinates.
   */
  int getCoordinateDim() const { return vCoordDim_; }

  /*! \brief Sets pointers to this process' vertex Ids and their weights.

      \param Ids will on return point to the list of the global Ids for
        each vertex on this process.
      \param xyz If vertex coordinate data is available, \c xyz
         will on return point to a StridedData object of coordinates.
      \param wgts If vertex weights is available, \c wgts
         will on return point to a StridedData object of weights.
   */
  size_t getVertexList(
    ArrayView<const gno_t> &Ids,
    ArrayView<input_t> &xyz,
    ArrayView<input_t> &wgts) const
  {
    size_t nv = gids_.size();
    Ids = gids_(0, nv);
    xyz = vCoords_.view(0, vCoordDim_);
    wgts = vWeights_.view(0, numWeightsPerVertex_);
    return nv;
  }

  /*! \brief Sets pointers to this process' hyperedge Ids and their weights.

      \param Ids will on return point to the list of the global Ids for
        each hyperedge on this process.
      \param wgts If hyperedge weights is available, \c wgts
         will on return point to a StridedData object of weights.
   */
  size_t getEdgeList(
    ArrayView<const gno_t> &Ids,
    ArrayView<input_t> &wgts) const
  {
    size_t nv = edgeGids_.size();
    Ids = edgeGids_(0, nv);
    wgts = eWeights_.view(0, nWeightsPerEdge_);
    return nv;
  }

  /*! \brief Sets pointers to this process' pins global Ids based on 
    the centric view giveb by getCentricView()

      \param pinIds This is the list of global neighbor Ids corresponding
        to the vertices or hyperedges listed in getVertexList/getEdgeList.
      \param offsets offsets[i] is the offset into pinIds to the start
        of neighbors for ith neighbor.
      \param wgts If pin weights is available, \c wgts
         will on return point to a StridedData object of weights.

      \return The number of ids in the pinIds list.
   */
  size_t getPinList( ArrayView<const gno_t> &pinIds,
    ArrayView<const lno_t> &offsets,
    ArrayView<input_t> &wgts) const
  {
    pinIds = pinGids_(0, numLocalPins_);
    offsets = offsets_.view(0, numLocalVertices_+1);
    wgts = eWeights_.view(0, nWeightsPerPin_);
    return numLocalPins_;
  }


  ////////////////////////////////////////////////////
  // The Model interface.
  ////////////////////////////////////////////////////

  size_t getLocalNumObjects() const { return numLocalVertices_; }

  size_t getGlobalNumObjects() const { return numGlobalVertices_; }

private:

  struct GhostCell {
    lno_t lid; //Assumes lno_t is signed (-1 corresponds to not on this process)
    gno_t gid;
    unsigned int dist;
    GhostCell(lno_t l,gno_t g, unsigned int d) {lid=l;gid=g;dist=d;}
    bool operator<(const struct GhostCell& other) const {return dist>other.dist;}
  };
  template <typename AdapterWithCoords>
  void shared_GetVertexCoords(const AdapterWithCoords *ia);
  

  const RCP<const Environment > env_;
  const RCP<const Comm<int> > comm_;

  CentricView view_;

  ArrayRCP<const gno_t> gids_;        // vertices of input graph

  int numWeightsPerVertex_;
  ArrayRCP<input_t> vWeights_;

  int vCoordDim_;
  ArrayRCP<input_t> vCoords_;

  ArrayRCP<const gno_t> edgeGids_;
 
  int nWeightsPerEdge_;
  ArrayRCP<input_t> eWeights_;

  ArrayRCP<const gno_t> pinGids_;
  ArrayRCP<const lno_t> offsets_;

  int nWeightsPerPin_;
  ArrayRCP<input_t> pWeights_;

  // For convenience

  size_t numLocalVertices_;
  size_t numGlobalVertices_;
  size_t numLocalEdges_;
  size_t numGlobalEdges_;
  size_t numLocalPins_;
  
  // For debugging
  void print();

};


////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////
//TODO get the weights for vertices and hyperedges
//GFD Do we need weights for pins too? First adjacency weights
template <typename Adapter>
HyperGraphModel<Adapter>::HyperGraphModel(
  const RCP<const MeshAdapter<user_t> > &ia,
  const RCP<const Environment> &env,
  const RCP<const Comm<int> > &comm,
  modelFlag_t &modelFlags,
  CentricView view):
       env_(env),
       comm_(comm),
       view_(view),
       gids_(),
       numWeightsPerVertex_(0),
       vWeights_(),
       vCoordDim_(0),
       vCoords_(),
       edgeGids_(),
       nWeightsPerEdge_(0),
       eWeights_(),
       pinGids_(),
       offsets_(),
       nWeightsPerPin_(0),
       pWeights_(),
       numLocalVertices_(0),
       numGlobalVertices_(0),
       numLocalEdges_(0),
       numGlobalEdges_(0),
       numLocalPins_(0)
{
  env_->timerStart(MACRO_TIMERS, "HyperGraphModel constructed from MeshAdapter");

  int me = comm_->getRank();

  // This HyperGraphModel is built with vertices == ia->getPrimaryEntityType()
  // and hyperedges == ia->getAdjacencyEntityType() from MeshAdapter.

  std::string model_type("traditional");
  const Teuchos::ParameterList &pl = env->getParameters();
  const Teuchos::ParameterEntry *pe2 = pl.getEntryPtr("hypergraph_model_type");
  if (pe2){
    model_type = pe2->getValue<std::string>(&model_type);
  }

  // Get the hypergraph from the input adapter

  Zoltan2::MeshEntityType primaryEType = ia->getPrimaryEntityType();
  Zoltan2::MeshEntityType adjacencyEType = ia->getAdjacencyEntityType();

  // Get the IDs of the primary entity type; these are hypergraph vertices
  
  zgid_t const *vtxIds=NULL;
  try {
    numLocalVertices_ = ia->getLocalNumOf(primaryEType);
    ia->getIDsViewOf(primaryEType, vtxIds);
    numGlobalVertices_ = ia->getGlobalNumOf(primaryEType);
  }
  Z2_FORWARD_EXCEPTIONS;

  gids_ = arcp<const gno_t>(vtxIds, 0, numLocalVertices_, false);

  // Get the IDs of the adjacency entity type; these are hypergraph hyperedges
  if (model_type=="traditional") {
    zgid_t const *edgeIds=NULL;
    try {
      numLocalEdges_ = ia->getLocalNumOf(adjacencyEType);
      ia->getIDsViewOf(adjacencyEType, edgeIds);
      numGlobalEdges_ = ia->getGlobalNumOf(adjacencyEType);
    }
    Z2_FORWARD_EXCEPTIONS;
    
    edgeGids_ = arcp<const gno_t>(edgeIds, 0, numLocalEdges_, false);
  }
  else if (model_type=="ghosting") {
    numLocalEdges_ = numLocalVertices_;
    edgeGids_ = arcp<const gno_t>(vtxIds, 0, numLocalVertices_, false);
    numGlobalEdges_ = numGlobalVertices_;
  }
 
  Zoltan2::MeshEntityType primaryPinType = primaryEType;
  Zoltan2::MeshEntityType adjacencyPinType = adjacencyEType;
  size_t numPrimaryPins = numLocalVertices_;
  if (view_==HYPEREDGE_CENTRIC) {
    primaryPinType = adjacencyEType;
    adjacencyPinType = primaryEType;
    numPrimaryPins = numLocalEdges_;
  }
  if (model_type=="traditional") {
    
    zgid_t const *nborIds=NULL;
    lno_t const *offsets=NULL;
    
    try {
      ia->getAdjsView(primaryPinType,adjacencyPinType,offsets,nborIds);
    }
    Z2_FORWARD_EXCEPTIONS;
    
    numLocalPins_ = offsets[numPrimaryPins];

    pinGids_ = arcp<const gno_t>(nborIds, 0, numLocalPins_, false);
    offsets_ = arcp<const lno_t>(offsets, 0, numPrimaryPins + 1, false);
  }
  else if (model_type=="ghosting") {

    //Find local ghosting with second adjacency
    if(!me) std::cout<<"\n\nPHASE ONE\n\n";
    std::map<gno_t,lno_t> lid_mapping;
    for (size_t i=0;i<numLocalVertices_;i++)
      lid_mapping[gids_[i]]=i;
    primaryPinType=primaryEType;
    adjacencyPinType =ia->getSecondAdjacencyEntityType();
    unsigned int layers=2;
    const lno_t* offsets;
    const zgid_t* adjacencyIds;
    //size_t numSecondAdj;
    if (!ia->avail2ndAdjs(primaryPinType,adjacencyPinType)) {
      Zoltan2::get2ndAdjsViewFromAdjs<user_t>(ia,primaryPinType, adjacencyPinType,
                                          offsets, adjacencyIds);
    }
    else {
      //numSecondAdj = ia->getLocalNum2ndAdjs(primaryPinType,adjacencyPinType);
      ia->get2ndAdjsView(primaryPinType,adjacencyPinType,offsets,adjacencyIds);
    }
    std::map<gno_t,std::map<gno_t,unsigned int> > ghosts;
    for (size_t i=0;i<numLocalVertices_;i++) {
      std::priority_queue<GhostCell> dist_queue;
      dist_queue.push(GhostCell(static_cast<lno_t>(i),gids_[i],0));
      ghosts[gids_[i]][gids_[i]]=0;
      while (!dist_queue.empty()) {
        GhostCell g = dist_queue.top();
        dist_queue.pop();
        if (g.lid==-1)
          continue;
        if (g.dist==layers)
          break;
        for (int j =offsets[g.lid];j<offsets[g.lid+1];j++) {
          if (ghosts[gids_[i]].find(adjacencyIds[j])==ghosts[gids_[i]].end()) {
            if (lid_mapping.find(adjacencyIds[j])==lid_mapping.end())
              dist_queue.push(GhostCell(-1,adjacencyIds[j],g.dist+1));
            else
              dist_queue.push(GhostCell(lid_mapping[adjacencyIds[j]],adjacencyIds[j],g.dist+1));
            ghosts[gids_[i]][adjacencyIds[j]] = g.dist+1;
          }
        }
      }
    }

    //Share off process ghosts
    if (!me) std::cout<<"\n\nPHASE TWO\n\n";
    std::map<gno_t,std::map<gno_t,unsigned int> > global_ghosts;
    for (size_t i=0;i<numGlobalVertices_;i++) {
      //Tell everyone how many ghosts this process has of the ith entity
      size_t num =0;
      if (ghosts.find(i)!=ghosts.end())
        num = ghosts[i].size();
      size_t *nums = new size_t[comm->getSize()];
      gatherAll(*comm,1,&num,comm->getSize(),nums);
      
      //Send to those that also have the ith entity all of this processes ghosts
      int num_messages=0;
      for (int j=0;j<comm->getSize();j++) {
        if (j==me || nums[j]==0 || num==0)
          continue;
        num_messages+=nums[j];
        typename std::map<gno_t,unsigned int>::iterator itr;
        for (itr=ghosts[i].begin();itr!=ghosts[i].end();itr++) {
          gno_t* send_vals = new gno_t[2];
          send_vals[0] = itr->first;
          send_vals[1] = itr->second;
          ArrayRCP<const gno_t> rcp_send_vals = arcp(send_vals,0,2,true);
          isend(*comm,rcp_send_vals,j);
        }
      }
      delete nums;

      //Receive
      while (num_messages>0) {
        ArrayRCP<gno_t> rcp_recv_vals(2);
        typedef Teuchos::CommRequest<int> request_t;
        RCP< request_t > request = ireceive(*comm,rcp_recv_vals,-1);
        comm->wait(Teuchos::Ptr<RCP<request_t> >(&request));
      
        typename std::map<gno_t,unsigned int>::iterator itr = ghosts[i].find(rcp_recv_vals[0]);
        if (itr==ghosts[i].end()||rcp_recv_vals[1]<static_cast<gno_t>(itr->second)) {
          global_ghosts[i][rcp_recv_vals[0]] = rcp_recv_vals[1];
          if (me) std::cout<<"Received: "<<i<<" "<<rcp_recv_vals[0]<<" "<<rcp_recv_vals[1]<<std::endl;
        }
        num_messages--;
      }
      
    }
    
    //update local ghosting information with new global ghosts
    if (!me) std::cout<<"\n\nPHASE THREE\n\n";
    for (size_t i=0;i<numLocalVertices_;i++) {//for each local entity
      typename std::map<gno_t,unsigned int>::iterator itr;
      for (itr=ghosts[gids_[i]].begin();itr!=ghosts[gids_[i]].end();itr++) { //for each ghost of this entity
        if (itr->second<layers) {
          typename std::map<gno_t,unsigned int>::iterator global_itr;
          for (global_itr=global_ghosts[itr->first].begin();
               global_itr!=global_ghosts[itr->first].end(); global_itr++) { //for each global ghost of the ghost entity

            //
            typename std::map<gno_t,unsigned int>::iterator local_itr 
              = ghosts[gids_[i]].find(global_itr->first);
            if (i==0&&me)
              std::cout<<global_itr->first<<" "<<global_itr->second<<' '<<local_itr->first<<" "<<local_itr->second<<std::endl;
            if (global_itr->second+itr->second<=layers &&
                (local_itr==ghosts[gids_[i]].end() ||
                 global_itr->second+itr->second<local_itr->second)) {
              ghosts[gids_[i]][global_itr->first]=global_itr->second+itr->second;
              std::cout<<me<<" New Ghost value: "<<gids_[i]<<" "<<local_itr->first<<" "<<global_itr->second+itr->second<<"\n";
            }
          }
        }
      }
    }

    //Finally make the pins
    if (!me) std::cout<<"\n\nPHASE FOUR\n\n";
    for (size_t i=0;i<numLocalVertices_;i++) {//for each local entity
      numLocalPins_+=ghosts[gids_[i]].size();
    }
    gno_t* temp_pins = new gno_t[numLocalPins_];
    lno_t* temp_offsets = new lno_t[numLocalVertices_+1];
    gno_t j=0;
    for (size_t i=0;i<numLocalVertices_;i++) {//for each local entity
      temp_offsets[i]=j;
      typename std::map<gno_t,unsigned int>::iterator itr;
      for (itr=ghosts[gids_[i]].begin();itr!=ghosts[gids_[i]].end();itr++) { //for each ghost of this entity
        temp_pins[j]=itr->first;
        j++;
        
      }
    }
    temp_offsets[numLocalVertices_]=numLocalPins_;
    
    pinGids_ = arcp<const gno_t>(temp_pins,0,numLocalPins_,true);
    offsets_ = arcp<const lno_t>(temp_offsets,0,numLocalVertices_+1,true);

    if (!me) std::cout<<"\n\nGhosting complete!\n\n";
  }
  // Get edge weights
  /*
  nWeightsPerEdge_ = ia->getNumWeightsPer2ndAdj(primaryEType, secondAdjEType);

  if (nWeightsPerEdge_ > 0){
    input_t *wgts = new input_t [nWeightsPerEdge_];
    eWeights_ = arcp(wgts, 0, nWeightsPerEdge_, true);
  }

  for (int w=0; w < nWeightsPerEdge_; w++){
    const scalar_t *ewgts=NULL;
    int stride=0;

    ia->get2ndAdjWeightsView(primaryEType, secondAdjEType,
			     ewgts, stride, w);

    ArrayRCP<const scalar_t> wgtArray(ewgts, 0, numLocalEdges_, false);
    eWeights_[w] = input_t(wgtArray, stride);
  }
  */


  typedef MeshAdapter<user_t> adapterWithCoords_t;
  shared_GetVertexCoords<adapterWithCoords_t>(&(*ia));

  env_->timerStop(MACRO_TIMERS, "HyperGraphModel constructed from MeshAdapter");
  print();
}

//////////////////////////////////////////////////////////////////////////

template <typename Adapter>
template <typename AdapterWithCoords>
void HyperGraphModel<Adapter>::shared_GetVertexCoords(const AdapterWithCoords *ia)
{
  // get Vertex coordinates from input adapter

  vCoordDim_ = ia->getDimension();

  if (vCoordDim_ > 0){
    input_t *coordInfo = new input_t [vCoordDim_];
    env_->localMemoryAssertion(__FILE__, __LINE__, vCoordDim_, coordInfo);

    for (int dim=0; dim < vCoordDim_; dim++){
      const scalar_t *coords=NULL;
      int stride=0;
      ia->getCoordinatesView(coords, stride, dim);
      ArrayRCP<const scalar_t> coordArray = arcp(coords, 0,
                                                 stride*numLocalVertices_,
                                                 false);
      coordInfo[dim] = input_t(coordArray, stride);
    }

    vCoords_ = arcp<input_t>(coordInfo, 0, vCoordDim_, true);
  }
}

//////////////////////////////////////////////////////////////////////////
template <typename Adapter>
void HyperGraphModel<Adapter>::print()
{
  if (env_->getDebugLevel() < VERBOSE_DETAILED_STATUS)
    return;

  std::ostream *os = env_->getDebugOStream();
  
  int me = comm_->getRank();
  std::string fn(" ");

  *os << me << fn
      << " Nvtx  " << gids_.size()
      << " Nedge " << edgeGids_.size()
      << " NPins " << numLocalPins_
      << " NVWgt " << numWeightsPerVertex_
      << " NEWgt " << nWeightsPerEdge_
      << " NPWgt " << nWeightsPerPin_
      << " CDim  " << vCoordDim_
      << std::endl;

  for (lno_t i = 0; i < gids_.size(); i++) {
    *os << me << fn << i << " VTXGID " << gids_[i];
    if (view_==VERTEX_CENTRIC) {
      *os <<":";
      for (lno_t j = offsets_[i]; j< offsets_[i+1];j++)
        *os <<" "<<pinGids_[j];
    }
    *os << std::endl;
  }
  for (lno_t i = 0; i<edgeGids_.size();i++) {
    *os << me << fn << i << " EDGEGID " << edgeGids_[i];
    if (view_==HYPEREDGE_CENTRIC) {
      *os <<":";
      for (lno_t j = offsets_[i]; j< offsets_[i+1];j++)
        *os <<" "<<pinGids_[j];
    }
    *os << std::endl;
  }
  if (vCoordDim_) {
    for (lno_t i = 0; i < gids_.size(); i++) {
      *os << me << fn << i << " COORDS " << gids_[i] << ": ";
      for (int j = 0; j < vCoordDim_; j++)
         *os << vCoords_[j][i] << " ";
      *os << std::endl;
    }
  }
  else
    *os << me << fn << "NO COORDINATES AVAIL " << std::endl;
}

}   // namespace Zoltan2


#endif

