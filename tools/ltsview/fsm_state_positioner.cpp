// Author(s): Bas Ploeger
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <algorithm>
#include <limits>
#include <queue>
#include <vector>
#include "cluster.h"
#include "fsm_state_positioner.h"
#include "lts.h"
#include "mathutils.h"
#include "state.h"
#include "transition.h"
#include "vectors.h"

using namespace std;
using namespace MathUtils;

FSMStatePositioner::FSMStatePositioner(LTS *l)
  : StatePositioner(l)
{
  for (Cluster_iterator cluster_it = lts->getClusterIterator();
      ! cluster_it.is_end(); ++cluster_it)
  {
    slot_info[*cluster_it] = new ClusterSlotInfo(*cluster_it);
  }
}

FSMStatePositioner::~FSMStatePositioner()
{
  for (map< Cluster*, ClusterSlotInfo* >::iterator slot_info_it =
      slot_info.begin(); slot_info_it != slot_info.end(); ++slot_info_it)
  {
    delete slot_info_it->second;
  }
}

void FSMStatePositioner::positionStates()
{
  // This algorithm has been based on the one by Frank van Ham, and includes
  // several improvements.
  bottomUpPass();
  topDownPass();
  resolveUnpositioned();
}

void FSMStatePositioner::bottomUpPass()
{
  // Process states bottom-up, keeping edges as short as possible.
  for (Reverse_cluster_iterator ci = lts->getReverseClusterIterator();
      !ci.is_end(); ++ci)
  {
    Cluster* cluster = *ci;
    for (int s = 0; s < cluster->getNumStates(); ++s)
    {
      State* state = cluster->getState(s);
      if (cluster->getNumStates() == 1)
      {
        Vector2D position = Vector2D(0,0);
        requestStatePosition(state, position);
      }
      else if (cluster->getNumDescendants() == 0)
      {
        state->center();
        unpositioned_states.push_back(state);
      }
      else if (cluster->getNumDescendants() == 1)
      {
        vector< State* > successors;
        getSuccessors(state, successors);
        if (allStatesCentered(successors))
        {
          state->center();
          unpositioned_states.push_back(state);
        }
        else
        {
          Vector2D position = sumStateVectorsInSingleCluster(successors);
          requestStatePosition(state, position);
        }
      }
      else
      {
        vector< State* > successors;
        getSuccessors(state, successors);
        Vector2D position = sumStateVectorsInMultipleClusters(
            successors, cluster->getBaseRadius());
        requestStatePosition(state, position);
      }
    }
  }
}

void FSMStatePositioner::topDownPass()
{
  // Assumption: unpositioned_states is sorted bottom-up
  vector< State* > unpositioned_temp;
  for (vector< State* >::reverse_iterator state_it =
      unpositioned_states.rbegin(); state_it != unpositioned_states.rend();
      ++state_it)
  {
    State* state = *state_it;
    if (state->getCluster()->isCentered())
    {
      vector< State* > predecessors;
      getPredecessors(state, predecessors);
      if (allStatesCentered(predecessors))
      {
        unpositioned_temp.push_back(state);
      }
      else
      {
        Vector2D position = sumStateVectorsInSingleCluster(predecessors);
        requestStatePosition(state, position);
      }
    }
    else
    {
      unpositioned_temp.push_back(state);
    }
  }
  unpositioned_states.swap(unpositioned_temp);
}

void FSMStatePositioner::resolveUnpositioned()
{
  for (vector< State* >::iterator state_it = unpositioned_states.begin();
      state_it != unpositioned_states.end(); ++state_it)
  {
    State* state = *state_it;
    ClusterSlotInfo* cs_info = slot_info[state->getCluster()];
    int ring, slot;
    cs_info->findFarthestFreeSlot(ring, slot);
    assignStateToSlot(state, ring, slot);
  }
}

Vector2D FSMStatePositioner::sumStateVectorsInSingleCluster(
    vector< State* > &states)
{
  Vector2D sum_vector = Vector2D(0, 0);
  for (vector< State* >::iterator state_it = states.begin(); state_it !=
      states.end(); ++state_it)
  {
    State* state = *state_it;
    if ( ! state->isCentered() )
    {
      sum_vector += Vector2D::fromPolar(state->getPositionAngle(),
          state->getPositionRadius());
    }
  }
  return sum_vector;
}

Vector2D FSMStatePositioner::sumStateVectorsInMultipleClusters(
    vector< State* > &states, float rim_radius)
{
  Vector2D sum_vector = Vector2D(0, 0);
  for (vector< State* >::iterator state_it = states.begin(); state_it !=
      states.end(); ++state_it)
  {
    State* state = *state_it;
    if (state->getCluster()->isCentered())
    {
      if (!state->isCentered())
      {
        sum_vector += Vector2D::fromPolar(state->getPositionAngle(),
            state->getPositionRadius());
      }
    }
    else
    {
      sum_vector += Vector2D::fromPolar(state->getCluster()->getPosition(),
          rim_radius);
      if (!state->isCentered())
      {
        sum_vector += Vector2D::fromPolar(state->getPositionAngle() +
            state->getCluster()->getPosition(), state->getPositionRadius());
      }
    }
  }
  return sum_vector;
}

bool FSMStatePositioner::allStatesCentered(vector< State* > &states)
{
  for (vector< State* >::iterator state = states.begin(); state !=
      states.end(); ++state)
  {
    if ( ! (**state).isCentered() )
    {
      return false;
    }
  }
  return true;
}

void FSMStatePositioner::getPredecessors(State* state, vector< State* >&
    predecessors)
{
  for (int t = 0; t < state->getNumInTransitions(); ++t)
  {
    State* predecessor = state->getInTransition(t)->getBeginState();
    if (predecessor->getRank() == state->getRank() - 1)
    {
      predecessors.push_back(predecessor);
    }
  }
}

void FSMStatePositioner::getSuccessors(State* state, vector< State* >&
    successors)
{
  for (int t = 0; t < state->getNumOutTransitions(); ++t)
  {
    State* successor = state->getOutTransition(t)->getEndState();
    if (successor->getRank() == state->getRank() + 1)
    {
      successors.push_back(successor);
    }
  }
}

void FSMStatePositioner::assignStateToSlot(State* state, int ring, int slot)
{
  ClusterSlotInfo* cs_info = slot_info[state->getCluster()];
  cs_info->occupySlot(ring, slot);
  if (ring == 0)
  {
    state->center();
  }
  else
  {
    float angle, radius;
    cs_info->getPolarCoordinates(ring, slot, angle, radius);
    state->setPositionAngle(angle);
    state->setPositionRadius(radius);
  }
}

void FSMStatePositioner::requestStatePosition(State* state, Vector2D& position)
{
  ClusterSlotInfo* cs_info = slot_info[state->getCluster()];
  int ring, slot;
  cs_info->findNearestSlot(position, ring, slot);
  cs_info->findNearestFreeSlot(ring, slot);
  assignStateToSlot(state, ring, slot);
}



const float ClusterSlotInfo::MIN_DELTA_RING = 0.22f;
const float ClusterSlotInfo::MIN_DELTA_SLOT = 0.22f;

ClusterSlotInfo::ClusterSlotInfo(Cluster* cluster)
{
  int num_rings = 1 + static_cast<int>(cluster->getTopRadius() /
      MIN_DELTA_RING);
  if (num_rings > 1)
  {
    delta_ring = cluster->getTopRadius() / static_cast<float>(num_rings - 1);
  }
  else
  {
    delta_ring = 1.0f;
  }

  for (int ring = 0; ring < num_rings; ++ring)
  {
    float circumference = 2 * PI * ring * delta_ring;
    // max with 1 to ensure that ring 0 also has a slot
    int slots = max(1, static_cast<int>(circumference / MIN_DELTA_SLOT));
    num_slots.push_back(slots);
  }
}

inline void ClusterSlotInfo::occupySlot(int ring, int slot)
{
  occupied_slots.insert(Slot(ring, slot));
}

inline int ClusterSlotInfo::getNumRings()
{
  return num_slots.size();
}

inline int ClusterSlotInfo::getNumSlots(int ring)
{
  return num_slots[ring];
}

void ClusterSlotInfo::getPolarCoordinates(int ring, int slot, float &angle,
    float &radius)
{
  radius = delta_ring * ring;
  angle = rad_to_deg(2 * PI * slot / num_slots[ring]);
}

Vector2D ClusterSlotInfo::getVector(int ring, int slot)
{
  float angle, radius;
  getPolarCoordinates(ring, slot, angle, radius);
  return Vector2D::fromPolar(angle, radius);
}

void ClusterSlotInfo::findNearestSlot(Vector2D &position, int &ring, int
    &slot)
{
  float angle, radius;
  position.toPolar(angle, radius);
  ring = min(round_to_int(radius / delta_ring), getNumRings() - 1);
  slot = round_to_int(num_slots[ring] * deg_to_rad(angle) / (2 * PI)) %
    getNumSlots(ring);
}

void ClusterSlotInfo::findNearestFreeSlot(int &ring, int &slot)
{
  SlotSet visited_slots;
  queue< Slot > to_visit_slots;
  to_visit_slots.push(Slot(ring, slot));
  while (! to_visit_slots.empty())
  {
    Slot slot_coord = to_visit_slots.front();
    ring = slot_coord.ring;
    slot = slot_coord.slot;
    if (occupied_slots.find(slot_coord) == occupied_slots.end())
    {
      return;
    }
    for (int s = slot - 1; s < slot + 3; s += 2)
    {
      int next_slot = s % getNumSlots(ring);
      if (s < 0)
      {
        next_slot += getNumSlots(ring);
      }
      if (visited_slots.find(Slot(ring, next_slot)) == visited_slots.end())
      {
        to_visit_slots.push(Slot(ring, next_slot));
      }
    }
    for (int next_ring = ring - 1; next_ring < ring + 3; next_ring += 2)
    {
      if (0 <= next_ring && next_ring < getNumRings())
      {
        int next_slot = 0;
        if (next_ring > 0)
        {
          next_slot = round_to_int(static_cast<float>(slot) /
              static_cast<float>(getNumSlots(ring)) * getNumSlots(next_ring)) %
            getNumSlots(ring);
        }
        if (visited_slots.find(Slot(next_ring, next_slot)) == visited_slots.end())
        {
          to_visit_slots.push(Slot(next_ring, next_slot));
        }
      }
    }
    visited_slots.insert(Slot(ring, slot));
    to_visit_slots.pop();
  }
}

void ClusterSlotInfo::findFarthestFreeSlot(int &ring, int &slot)
{
  if (occupied_slots.empty())
  {
    ring = getNumRings() - 1;
    slot = 0;
    return;
  }
  float max_min_distance = 0.0f;
  for (int r = 0; r < getNumRings(); ++r)
  {
    for (int s = 0; s < getNumSlots(r); ++s)
    {
      Vector2D slot_vector = getVector(r, s);
      float min_distance = numeric_limits< float >::max();
      for (SlotSet::iterator occupied_slot = occupied_slots.begin();
          occupied_slot != occupied_slots.end(); ++occupied_slot)
      {
        Vector2D delta_vector = slot_vector - getVector(occupied_slot->ring,
            occupied_slot->slot);
        min_distance = min(min_distance, delta_vector.length());
      }
      if (min_distance > max_min_distance)
      {
        max_min_distance = min_distance;
        ring = r;
        slot = s;
      }
    }
  }
}
