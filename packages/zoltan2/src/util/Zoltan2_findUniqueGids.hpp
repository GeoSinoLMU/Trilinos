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

/*! \file Zoltan2_findUniqueGids.hpp
    \brief Convert keys stored in std::vector to unique Gids stored in 
           std::vector.  
*/

#ifndef _ZOLTAN2_FINDUNIQUEGIDS_HPP_
#define _ZOLTAN2_FINDUNIQUEGIDS_HPP_

#include <Zoltan2_Standards.hpp>
#include <vector>

#include <zoltan_dd_cpp.h>
#include <DD.h>

namespace Zoltan2
{

template <typename key_t, typename gno_t>
size_t findUniqueGids(
  std::vector<key_t> &keys,
  std::vector<gno_t> &gids,
  const Teuchos::Comm<int> &comm
)
{
  // Input:  Vector of keys; key length = key_t.size()
  //         Each key must have the same size.  std::array<gno_t, N> is
  //         an example of a good key_t.
  //         May contain duplicate keys within a processor.
  //         May contain duplicate keys across processors.
  // Input:  Empty vector for holding the results
  // Output: Filled gids vector, containing unique global numbers for
  //         each unique key.  Global numbers are in range [0,#UniqueKeys).
  //
  // Note:  This code uses the Zoltan Distributed Directory to assign the
  //        unique global numbers.  Right now, it hacks into the Zoltan_DD
  //        data structures.  If we like this approach, we can add some
  //        elegance to the Zoltan_DD, allowing operations internal to the
  //        directory.

  size_t num_keys = keys.size();
  key_t dummy;
  size_t num_entries = dummy.size();

  MPI_Comm mpicomm = Teuchos::getRawMpiComm(comm);

  int num_gid = sizeof(gno_t)/sizeof(ZOLTAN_ID_TYPE) * num_entries;
  int num_lid = 0;  // Local IDs not needed
  int num_user = sizeof(gno_t);
  int num_table = num_keys;
  int debug_level = 0;

  Zoltan_DD_Struct *dd = NULL;
  Zoltan_DD_Create(&dd, mpicomm, num_gid, num_lid, num_user, num_table, 
                   debug_level);

  ZOLTAN_ID_PTR ddkeys = new ZOLTAN_ID_TYPE[num_gid * num_keys];
  ZOLTAN_ID_PTR ddnotneeded = NULL;  // Local IDs not needed
  char *ddnewgids = new char[num_user * num_keys];

  // TODO  Need a Zoltan traits class
  if (sizeof(gno_t) > sizeof(ZOLTAN_ID_TYPE))
    throw std::runtime_error("Not ready for sizeof(gno_t) > "
                                           "sizeof(ZOLTAN_ID_TYPE) yet");
  
  // Buffer the keys for Zoltan_DD
  size_t idx = 0;
  for (size_t i = 0; i < num_keys; i++)
    for (size_t v = 0; v < num_entries; v++)
      ddkeys[idx++] = keys[i][v];  // TODO Need Zoltan traits here

  Zoltan_DD_Update(dd, ddkeys, ddnotneeded, ddnewgids, NULL, int(num_keys));

  //////////
  // Insert unique GIDs for DD entries in User data here.

  // Get value of first gid on this rank
  ssize_t nDDEntries = (ssize_t)(dd->nodecnt);
  ssize_t firstIdx;  
  MPI_Scan(&nDDEntries, &firstIdx, 1, MPI_LONG_LONG, MPI_SUM, mpicomm);
  firstIdx -= nDDEntries;  // do not include this rank's entries in prefix sum

  // Loop over all directory entries, updating their userdata with updated gid
  DD_NodeIdx cnt = 0;
  for (DD_NodeIdx i = 0; i < dd->nodelistlen; i++) {
    DD_Node *ptr = &(dd->nodelist[i]);
    if (!(ptr->free)) {
      char *userchar = (char*)(ptr->gid + (dd->gid_length + dd->lid_length));
      gno_t *newgid = (gno_t*) userchar;
      *newgid = gno_t(firstIdx + cnt);
      cnt++;
    }
  }

  ///////////
  // Retrieve the global numbers and put in the result gids vector
  Zoltan_DD_Find(dd, ddkeys, ddnotneeded, ddnewgids, NULL, int(num_keys), NULL);
  gno_t *result = (gno_t *)ddnewgids;
  for (size_t i = 0; i < num_keys; i++)
    gids[i] = result[i];

  delete [] ddkeys;
  delete [] ddnewgids;
  Zoltan_DD_Destroy(&dd);

  ssize_t nUnique = 0;  
  MPI_Allreduce(&nDDEntries, &nUnique, 1, MPI_LONG_LONG, MPI_SUM, mpicomm);
  return size_t(nUnique);
}

}                   // namespace Zoltan2
#endif
