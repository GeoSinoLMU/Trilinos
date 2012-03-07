// @HEADER
// ***********************************************************************
//
//         Zoltan2: Sandia Partitioning Ordering & Coloring Library
//
//                Copyright message goes here.   TODO
//
// ***********************************************************************
//
// Testing of CoordinateModel
//

#include <Zoltan2_CoordinateModel.hpp>
#include <Zoltan2_BasicCoordinateInput.hpp>
#include <Zoltan2_TestHelpers.hpp>

#include <set>

#include <Teuchos_Comm.hpp>
#include <Teuchos_DefaultComm.hpp>
#include <Teuchos_ArrayView.hpp>
#include <Teuchos_OrdinalTraits.hpp>

#include <Tpetra_CrsMatrix.hpp>

using namespace std;
using Teuchos::RCP;
using Teuchos::Comm;
using Teuchos::DefaultComm;

void testCoordinateModel(std::string fname, gno_t xdim, gno_t ydim, gno_t zdim,
  const RCP<const Comm<int> > &comm, bool consecutiveIds)
{
  int rank = comm->getRank();
  int fail = 0, gfail = 0;

  RCP<const Zoltan2::Environment> default_env = 
    Zoltan2::getDefaultEnvironment();

  //////////////////////////////////////////////////////////////
  // Use an Tpetra::CrsMatrix for the user data.
  //////////////////////////////////////////////////////////////
  typedef Tpetra::CrsMatrix<scalar_t, lno_t, gno_t> tcrsMatrix_t;
  
  UserInputForTests *input;
  if (fname.size() > 0)
    input = new UserInputForTests(fname, comm);
  else
    input = new UserInputForTests(xdim,ydim,zdim,comm);

  RCP<tcrsMatrix_t > M = input->getTpetraCrsMatrix();
  lno_t nLocalIds = M->getNodeNumRows();
  gno_t nGlobalIds =  M->getGlobalNumRows();

  ArrayView<const gno_t> idList = M->getRowMap()->getNodeElementList();
  std::set<gno_t> idSet(idList.begin(), idList.end());

  //////////////////////////////////////////////////////////////
  // Create an CoordinateModel with this input
  //////////////////////////////////////////////////////////////

  typedef Zoltan2::XpetraCrsMatrixInput<tcrsMatrix_t> adapter_t;
  typedef Zoltan2::MatrixInput<tcrsMatrix_t> base_adapter_t;
  typedef Zoltan2::StridedInput<lno_t, scalar_t> input_t;

  RCP<const adapter_t> ia = Teuchos::rcp(new adapter_t(M));
  
  Zoltan2::CoordinateModel<base_adapter_t> *model = NULL;
  const base_adapter_t *base_ia = ia.get();

  try{
    model = new Zoltan2::CoordinateModel<base_adapter_t>(
      base_ia, default_env, comm, consecutiveIds);
  }
  catch (std::exception &e){
    std::cerr << rank << ") " << e.what() << std::endl;
    fail = 1;
  }

  gfail = globalFail(comm, fail);

  if (gfail)
    printFailureCode(comm, fail);
  
  // Test the CoordinateModel interface

  if (model->getLocalNumCoordinates() != nLocalIds)
    fail = 2;

  if (!fail && model->getGlobalNumCoordinates() != nGlobalIds)
    fail = 3;

  // For now, MatrixInput does not implement weights
  if (!fail && model->getCoordinateWeightDim() !=  0)
    fail = 4;

  gfail = globalFail(comm, fail);

  if (gfail)
    printFailureCode(comm, fail);
  
  ArrayView<const gno_t> gids;
  ArrayView<input_t> wgts;
  
  model->getCoordinateList(gids, wgts);

  if (!fail && gids.size() != nLocalIds)
    fail = 5;

  if (!fail && wgts.size() != 0)
    fail = 6;

  for (lno_t i=0; !fail && i < nLocalIds; i++){
    std::set<gno_t>::iterator next = idSet.find(gids[i]);
    if (next == idSet.end())
      fail = 7;
  }

  if (!fail && consecutiveIds){
    bool inARow = Zoltan2::CoordinateTraits<gno_t>::areConsecutive(
      gids.getRawPtr(), nLocalIds);

    if (!inARow)
      fail = 8;
  }

  gfail = globalFail(comm, fail);

  if (gfail)
    printFailureCode(comm, fail);

  delete model;
  delete input;
}

int main(int argc, char *argv[])
{
  Teuchos::GlobalMPISession session(&argc, &argv);
  Teuchos::RCP<const Teuchos::Comm<int> > comm =
    Teuchos::DefaultComm<int>::getComm();

  int rank = comm->getRank();

  std::string nullString;
  std::vector<std::string> mtxFiles;
  
  mtxFiles.push_back("../data/simple.mtx");
  bool wishConsecutiveIds = true;

  for (unsigned int fileNum=0; fileNum < mtxFiles.size(); fileNum++){

    if (rank == 0){
      std::cout << mtxFiles[fileNum];
      std::cout << ", consecutive IDs not requested" << std::endl;
    }
    testCoordinateModel(mtxFiles[fileNum], 0,0,0,comm, !wishConsecutiveIds);

    if (rank == 0){
      std::cout << mtxFiles[fileNum];
      std::cout << ", consecutive IDs are requested" << std::endl;
    }
    testCoordinateModel(mtxFiles[fileNum], 0,0,0,comm,  wishConsecutiveIds);
  }

  if (rank == 0){
    std::cout << "5x5x5 mesh";
    std::cout << ", consecutive IDs not requested" << std::endl;
  }
  testCoordinateModel(nullString, 5, 5, 5, comm, !wishConsecutiveIds);

  if (rank == 0){
    std::cout << "5x5x5 mesh";
    std::cout << ", consecutive IDs are requested" << std::endl;
  }
  testCoordinateModel(nullString, 5, 5, 5, comm, wishConsecutiveIds);

  if (rank==0) std::cout << "PASS" << std::endl;

  return 0;
}
