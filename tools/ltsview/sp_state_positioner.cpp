// Author(s): Bas Ploeger
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <cmath>
#include <vector>
#include "cluster.h"
#include "lts.h"
#include "sp_state_positioner.h"
#include "mathutils.h"
#include "rtree.h"
#include "state.h"
#include "transition.h"
#include "vectors.h"


class ClusterStatePositioner
{
  public:
    ClusterStatePositioner(Cluster* c);

    virtual ~ClusterStatePositioner()
    { }

    virtual void positionStates()
    { }

  protected:
    static const float MIN_DELTA_RING = 0.22f;
    static const float MIN_DELTA_SLOT = 0.22f;

    Cluster* cluster;
    float delta_ring;
    std::vector< int > num_ring_slots;
};


class NodeClusterStatePositioner: public ClusterStatePositioner
{
  public:
    NodeClusterStatePositioner(Cluster *c):
      ClusterStatePositioner(c), slot_rtree(NULL)
    { }

    ~NodeClusterStatePositioner();
    void positionStates();

  private:
    void assignStateToNearestSlot(State* state, const Vector2D& position);
    void buildRTree();
    Vector2D sumSuccessorStateVectors(State* state);

    RTree* slot_rtree;
};


class LeafClusterStatePositioner: public ClusterStatePositioner
{
  public:
    LeafClusterStatePositioner(Cluster *c):
      ClusterStatePositioner(c)
    { }

    ~LeafClusterStatePositioner()
    { }

    void positionStates();

  private:
    void computeNumRingStates();
    std::vector< int > num_ring_states;
};


ClusterStatePositioner::ClusterStatePositioner(Cluster* c):
  cluster(c)
{
  int num_rings = 1 + std::floor(cluster->getTopRadius() /
      MIN_DELTA_RING);
  if (num_rings > 1)
  {
    delta_ring = cluster->getTopRadius() / static_cast<float>(num_rings - 1);
  }
  else
  {
    delta_ring = 1.0f;
  }
  num_ring_slots.assign(num_rings, 0);
  for (int ring = 0; ring < num_rings; ++ring)
  {
    int num_slots = std::floor(2.0f * MathUtils::PI * ring * delta_ring /
        MIN_DELTA_SLOT);
    num_ring_slots[ring] = (num_slots > 1) ? num_slots : 1;
  }
}

NodeClusterStatePositioner::~NodeClusterStatePositioner()
{
  if (slot_rtree != NULL)
  {
    delete slot_rtree;
  }
}

void NodeClusterStatePositioner::positionStates()
{
  buildRTree();
  for (int s = 0; s < cluster->getNumStates(); ++s)
  {
    State* state = cluster->getState(s);
    Vector2D position = sumSuccessorStateVectors(state);
    assignStateToNearestSlot(state, position);
  }
}

void NodeClusterStatePositioner::buildRTree()
{
  PackedRTreeBuilder rtree_builder = PackedRTreeBuilder();
  for (size_t ring = 0; ring < num_ring_slots.size(); ++ring)
  {
    float radius = ring * delta_ring;
    int num_slots = num_ring_slots[ring];
    float delta_deg = 360.0f / static_cast<float>(num_slots);
    for (int slot = 0; slot < num_slots; ++slot)
    {
      Vector2D point;
      point.fromPolar(delta_deg * slot, radius);
      rtree_builder.addPoint(point);
    }
  }
  rtree_builder.buildRTree();
  slot_rtree = rtree_builder.getRTree();
}

Vector2D NodeClusterStatePositioner::sumSuccessorStateVectors(State* state)
{
  Vector2D sum_vector = Vector2D(0, 0);
  for (int t = 0; t < state->getNumOutTransitions(); ++t)
  {
    State* successor = state->getOutTransition(t)->getEndState();
    if (successor->getRank() == state->getRank() + 1)
    {
      if (successor->getCluster()->isCentered())
      {
        if (!successor->isCentered())
        {
          Vector2D v_state;
          v_state.fromPolar(successor->getPositionAngle(),
              successor->getPositionRadius());
          sum_vector += v_state;
        }
      }
      else
      {
        Vector2D v_cluster;
        v_cluster.fromPolar(successor->getCluster()->getPosition(),
            cluster->getBaseRadius());
        Vector2D v_state = Vector2D(0, 0);
        if (!successor->isCentered())
        {
          v_state.fromPolar(successor->getPositionAngle() +
              successor->getCluster()->getPosition(),
              successor->getPositionRadius());
        }
        sum_vector += v_cluster + v_state;
      }
    }
  }
  return sum_vector;
}

void NodeClusterStatePositioner::assignStateToNearestSlot(State* state,
    const Vector2D& position)
{
  Vector2D assigned_position = position;
  slot_rtree->findNearestNeighbour(position);
  if (slot_rtree->hasFoundNeighbour())
  {
    assigned_position = slot_rtree->foundNeighbour();
  }
  float angle, radius;
  assigned_position.toPolar(angle, radius);
  if (radius < 0.5f * delta_ring)
  {
    state->center();
  }
  else
  {
    state->setPositionAngle(angle);
    state->setPositionRadius(radius);
  }
  slot_rtree->deletePoint(assigned_position);
}

void LeafClusterStatePositioner::positionStates()
{
  computeNumRingStates();
  int state_begin = 0;
  if (num_ring_states[0] == 1)
  {
    cluster->getState(0)->center();
    ++state_begin;
  }
  for (size_t ring = 1; ring < num_ring_states.size(); ++ring)
  {
    float radius = delta_ring * ring;
    int num_states = num_ring_states[ring];
    float delta_deg = 360.0f / static_cast<float>(num_states);
    float angle = (ring % 2 == 1) ? 0.0f : 0.5f * delta_deg;
    for (int s = state_begin; s < state_begin + num_states; ++s)
    {
      cluster->getState(s)->setPositionAngle(angle);
      cluster->getState(s)->setPositionRadius(radius);
      angle += delta_deg;
    }
    state_begin += num_states;
  }
}

void LeafClusterStatePositioner::computeNumRingStates()
{
  int total_slots = 0;
  for (size_t ring = 0; ring < num_ring_slots.size(); ++ring)
  {
    total_slots += num_ring_slots[ring];
  }
  int todo_states = cluster->getNumStates();
  num_ring_states.assign(num_ring_slots.size(), 0);
  for (size_t ring = 0; ring < num_ring_slots.size(); ++ring)
  {
    if (ring == num_ring_slots.size() - 1)
    {
      num_ring_states[ring] = todo_states;
    }
    else
    {
      float rel_slots = static_cast<float>(num_ring_slots[ring]) /
        static_cast<float>(total_slots);
      int num_states = MathUtils::round_to_int(cluster->getNumStates() *
          rel_slots);
      num_ring_states[ring] = num_states;
      todo_states -= num_states;
    }
  }
}


SinglePassStatePositioner::SinglePassStatePositioner(LTS *l)
  : StatePositioner(l)
{
}

SinglePassStatePositioner::~SinglePassStatePositioner()
{
}

void SinglePassStatePositioner::positionStates()
{
  for (Reverse_cluster_iterator ci = lts->getReverseClusterIterator();
      !ci.is_end(); ++ci)
  {
    Cluster* cluster = *ci;
    ClusterStatePositioner* cs_positioner;
    if (cluster->getNumDescendants() == 0 || (cluster->getNumDescendants() == 1
          && cluster->getDescendant(0)->getNumStates() == 1))
    {
      cs_positioner = new LeafClusterStatePositioner(cluster);
    }
    else
    {
      cs_positioner = new NodeClusterStatePositioner(cluster);
    }
    cs_positioner->positionStates();
    delete cs_positioner;
  }
}

