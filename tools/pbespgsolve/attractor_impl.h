// Copyright (c) 2009-2011 University of Twente
// Copyright (c) 2009-2011 Michael Weber <michaelw@cs.utwente.nl>
// Copyright (c) 2009-2011 Maks Verver <maksverver@geocities.com>
// Copyright (c) 2009-2011 Eindhoven University of Technology
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include "attractor.h"
#include <queue>

template<class ForwardIterator, class SetT>
bool is_subset_of(ForwardIterator it, ForwardIterator end, const SetT &set)
{
    for (; it != end; ++it) if (!set.count(*it)) return false;
    return true;
}

template<class SetT, class StrategyT>
void make_attractor_set( const ParityGame &game, ParityGame::Player player,
                         SetT &vertices, StrategyT &strategy )
{
    std::deque<verti> todo(vertices.begin(), vertices.end());
    return make_attractor_set(game, player, vertices, todo, strategy);
}

template<class SetT, class DequeT, class StrategyT>
void make_attractor_set( const ParityGame &game, ParityGame::Player player,
    SetT &vertices, DequeT &todo, StrategyT &strategy )
{
    const StaticGraph &graph = game.graph();

    while (!todo.empty())
    {
        const verti w = todo.front();
        todo.pop_front();

        // Check all predecessors v of w:
        for (StaticGraph::const_iterator it = graph.pred_begin(w);
             it != graph.pred_end(w); ++it)
        {
            const verti v = *it;

            // Skip predecessors that are already in the attractor set:
            if (vertices.find(v) != vertices.end()) continue;

            if (game.player(v) == player)
            {
                // Store strategy for player-controlled vertex:
                strategy[v] = w;
            }
            else  // opponent controls vertex
            if (is_subset_of(graph.succ_begin(v), graph.succ_end(v), vertices))
            {
                // Store strategy for opponent-controlled vertex:
                strategy[v] = NO_VERTEX;
            }
            else
            {
                continue;  // not in the attractor set yet!
            }

            // Add vertex v to the attractor set:
            vertices.insert(v);
            todo.push_back(v);
        }
    }
}
