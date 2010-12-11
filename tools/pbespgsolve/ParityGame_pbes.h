#include <mcrl2/pbes/parity_game_generator.h>

template <typename Container>
void ParityGame::assign_pbes( mcrl2::pbes_system::pbes<Container> &pbes,
    verti *goal_vertex, StaticGraph::EdgeDirection edge_dir )
{
    /* NOTE: this code assumes the vertices generated by parity_game_generator
             are numbered from 2 to num_vertices-1 with no gaps, with 0 and 1
             representing true and false (respectively) and 2 representing the
             initial condition. */

    if (goal_vertex) *goal_vertex = 2;

    // Generate min-priority parity game
    mcrl2::pbes_system::parity_game_generator pgg(pbes, true, true);

    // Build the edge list
    StaticGraph::edge_list edges;
    verti begin = 0, end = 3;
    for (verti v = begin; v < end; ++v)
    {
        std::set<size_t> deps = pgg.get_dependencies(v);
        for ( std::set<size_t>::const_iterator it = deps.begin();
                it != deps.end(); ++it )
        {
            verti w = (verti)*it;
            assert(w >= begin);
            if (w >= end) end = w + 1;
            edges.push_back(std::make_pair(v - begin, w - begin));
        }
    }

    // Determine maximum prioirity
    int max_prio = 0;
    for (verti v = begin; v < end; ++v)
    {
        max_prio = std::max(max_prio, (int)pgg.get_priority(v));
    }

    // Assign vertex info and recount cardinalities
    reset(end - begin, max_prio + 1);
    for (verti v = begin; v < end; ++v)
    {
        bool and_op = pgg.get_operation(v) ==
                        mcrl2::pbes_system::parity_game_generator::PGAME_AND;
        vertex_[v - begin].player = and_op ? PLAYER_ODD : PLAYER_EVEN;
        vertex_[v - begin].priority = pgg.get_priority(v);
    }
    recalculate_cardinalities(end - begin);

    // Assign graph
    graph_.assign(edges, edge_dir);
}
