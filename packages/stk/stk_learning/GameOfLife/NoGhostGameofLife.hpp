/*
 * NoGhostGameofLife.h
 *
 *  Created on: Aug 10, 2015
 *      Author: jonchu
 */
#ifndef GAMEOFLIFE_NOGHOSTGAMEOFLIFE_HPP_
#define GAMEOFLIFE_NOGHOSTGAMEOFLIFE_HPP_

#include <gtest/gtest.h>

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <set>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <stdlib.h>

#include <stk_topology/topology.hpp>
#include <stk_mesh/base/BulkData.hpp>
#include <stk_mesh/base/MetaData.hpp>
#include <stk_mesh/base/Comm.hpp>
#include <stk_mesh/base/GetEntities.hpp>
#include <stk_mesh/base/FEMHelpers.hpp>
#include <stk_mesh/base/Field.hpp>
#include <stk_mesh/base/CoordinateSystems.hpp>

#include <stk_util/stk_util/parallel/CommSparse.hpp>
#include <stk_unit_test_utils/ioUtils.hpp>
#include <stk_io/StkMeshIoBroker.hpp>
#include <stk_io/IossBridge.hpp>

#include <time.h>

#include "MeshBuilder.hpp"
#include "EntityKeyHash.hpp"
/*
 * How to use:
 * Basically the same as GameofLife, but make sure the MeshBuilder had no aura.
 */
class NoGhostGameofLife
{
public:
    //basics
    NoGhostGameofLife(MeshBuilder* Mesh, std::string name);

    ~NoGhostGameofLife(){}

    void activate_these_ids(stk::mesh::EntityIdVector& elemIds);

    void run_game_of_life(int numSteps);

    //test functions
    stk::mesh::Entity element_with_id(stk::mesh::EntityId elemId);

    bool is_valid_entity(stk::mesh::Entity entity);

    unsigned num_neighbors(stk::mesh::Entity elem);

    unsigned num_active_elems();

    unsigned num_active_neighbors(stk::mesh::Entity elem);

    //accessors
    inline stk::mesh::BulkData* bulk_data();

    inline unsigned num_procs() const;

    inline unsigned num_elems_on_proc() const;


private:
    //basic stuff
    stk::mesh::BulkData* m_bulkData;
    int m_numProcs;
    stk::mesh::EntityVector m_elements;
    stk::topology m_elemType;

    //game stuff
    ScalarIntField* m_lifeField;
    ScalarIntField* m_neighborField;
    stk::mesh::Part* m_activePart;

    //io
    std::string m_name;
    stk::io::StkMeshIoBroker m_stkIo;
    unsigned m_fileHandler;
    int m_time;

    //book keeping
    std::unordered_map<stk::mesh::Entity, std::unordered_set<stk::mesh::Entity,
    std::hash<stk::mesh::Entity>>, std::hash<stk::mesh::Entity>>
    m_localElementToLocalNeighborElements;

    std::unordered_set<stk::mesh::EntityKey, std::hash<stk::mesh::EntityKey>>
    m_remoteElementKeys;

    std::unordered_map<stk::mesh::EntityKey, std::unordered_set<stk::mesh::EntityKey,
    std::hash<stk::mesh::EntityKey>>, std::hash<stk::mesh::EntityKey>>
    m_remoteElementKeyToLocalNodeKeys;

    std::unordered_map<stk::mesh::EntityKey, int, std::hash<stk::mesh::EntityKey>>
    m_remoteElementKeyToOwningProcessor;

    std::unordered_map<stk::mesh::EntityKey, std::unordered_set<stk::mesh::Entity,
    std::hash<stk::mesh::Entity>>, std::hash<stk::mesh::EntityKey>>
    m_remoteElementKeyToLocalNeighborElements;

    std::unordered_map<stk::mesh::Entity, std::unordered_set<stk::mesh::EntityKey,
    std::hash<stk::mesh::EntityKey>>, std::hash<stk::mesh::Entity>>
    m_localElementToRemoteElementKeys;

    std::vector<stk::mesh::Entity>
    m_localActiveElements;

    std::unordered_set<stk::mesh::Entity, std::hash<stk::mesh::Entity>>
    m_localElementsToVisit;

    std::unordered_set<stk::mesh::EntityKey, std::hash<stk::mesh::EntityKey>>
    m_remoteElementKeysToVisit;

    //whatever
    bool is_element_active(stk::mesh::Entity elem);

    void activate_element(stk::mesh::Entity elem);

    void deactivate_element(stk::mesh::Entity elem);

   //constructor
    void get_elements();
    void confirm_no_ghosting_ghosting();
    void create_element_connectivity_maps();
        void create_local_element_to_local_element_map();
            void create_map_for_this_element(stk::mesh::Entity elem);
                void add_this_nodes_elements_to_this_elements_map(stk::mesh::Entity elem,
                                                              stk::mesh::Entity node);
        void create_remote_element_key_maps();
            void fill_buffer_with_local_element_keys_and_remote_node_keys(stk::CommSparse& buffer);
                void fill_buffer_with_this_elements_info(stk::mesh::Entity elem, stk::CommSparse& buffer);
                    void fill_map_with_this_elements_nodes(stk::mesh::Entity elem, std::unordered_map<int,
                                                           std::unordered_set<stk::mesh::EntityKey,
                                                           std::hash<stk::mesh::EntityKey>>>& map);
                    void fill_buffer_with_map_info(stk::mesh::Entity elem, stk::CommSparse& buffer,
                                                   std::unordered_map<int,std::unordered_set
                                                   <stk::mesh::EntityKey,std::hash
                                                   <stk::mesh::EntityKey>>>& map);
            void unpack_remote_elem_key_info_from_buffer(stk::CommSparse& buffer);
                void unpack_remote_info_from_this_processor(int proc, stk::CommBuffer& buf);
            void create_map_of_remote_elem_keys_to_local_elements();
                void map_this_remote_element_key_with_this_nodes_elements(stk::mesh::EntityKey remoteKey,
                                                                          stk::mesh::EntityKey nodeKey);
        void create_local_element_to_remote_element_key_map();
            void fill_buffer_with_local_neighbors_of_remote_keys(stk::CommSparse& buffer);
                void fill_buffer_with_local_neighbors_of_remote_element_key(stk::mesh::EntityKey remoteKey,
                                                                        stk::CommSparse& buffer);
            void unpack_local_and_remote_key_info_from_each_processor(stk::CommSparse& buffer);
                void unpack_local_and_remote_keys_from_buffer(stk::CommBuffer& buf);
    void write_output_mesh();

    //activate elements
    void activate_element_id(stk::mesh::EntityId elemId);

    //GoL
    void run_game_of_life_step();
        void determine_elements_to_check();
            void refresh_element_maps();
                void get_elements_to_visit();
            void communicate_remote_element_keys_to_check(stk::CommSparse& buffer);
            void recieve_local_element_keys_to_check(stk::CommSparse& buffer);
        void update_neighbor_values_with_local_elements();
        void update_neighbor_values_with_remote_elements();
            void send_num_active_neighbors_of_remote_elem_keys(stk::CommSparse& buffer);
                void pack_number_of_local_neighbors_of_remote_element_into_buffer(stk::CommSparse& buffer,
                                                                       stk::mesh::EntityKey remoteKey);
                    int count_local_active_neighbors_for_remote_element_key(stk::mesh::EntityKey remoteKey);
                    void pack_num_active_neighbors_into_buffer(stk::CommSparse& buffer, int numActive,
                                                           stk::mesh::EntityKey remoteKey);
            void recieve_num_active_neighbors_of_local_elements(stk::CommSparse& buffer);
                void update_local_element_with_remote_neighbor_data(stk::CommBuffer& buf);
        void update_element_membership();
            void update_tri_membership(stk::mesh::Entity elem);
            void update_quad_membership(stk::mesh::Entity elem);
            void update_hex_membership(stk::mesh::Entity elem);
        void write_output_step();
};

//accessors
inline stk::mesh::BulkData* NoGhostGameofLife::bulk_data()
{
    return m_bulkData;
}
inline unsigned NoGhostGameofLife::num_procs() const
{
    return m_numProcs;
}
inline unsigned NoGhostGameofLife::num_elems_on_proc() const
{
    return m_elements.size();
}



#endif /* GAMEOFLIFE_NOGHOSTGAMEOFLIFE_HPP_ */
