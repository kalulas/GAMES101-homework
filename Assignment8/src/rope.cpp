#include <iostream>
#include <vector>

#include "CGL/vector2D.h"

#include "mass.h"
#include "rope.h"
#include "spring.h"

namespace CGL {

    Rope::Rope(Vector2D start, Vector2D end, int num_nodes, float node_mass, float k, vector<int> pinned_nodes)
    {
        // TODO (Part 1): Create a rope starting at `start`, ending at `end`, and containing `num_nodes` nodes.

        Vector2D delta = (end - start) / (num_nodes - 1);
        for (int i = 0; i < num_nodes; i++)
        {
            masses.push_back(new Mass(start + delta * i, node_mass, false));

            if (i >= 1)
            {
                springs.push_back(new Spring(masses[i - 1], masses[i], k));
            }
        }

        for (auto &i : pinned_nodes) {
            masses[i]->pinned = true;
        }
    }

    void Rope::simulateEuler(float delta_t, Vector2D gravity)
    {
        for (auto &s : springs)
        {
            // TODO (Part 2): Use Hooke's law to calculate the force on a node
            Vector2D dir = s->m2->position - s->m1->position;
            float cur_length = dir.norm();
            Vector2D force = - s->k * dir.unit() * (cur_length - s->rest_length);
            s->m2->forces += force;
            s->m1->forces += -force;
        }

        const float damping_factor = 0.01;

        for (auto &m : masses)
        {
            if (!m->pinned)
            {
                // TODO (Part 2): Add the force due to gravity, then compute the new velocity and position
                m->forces += gravity * m->mass;
                // TODO (Part 2): Add global damping
                m->forces += -m->velocity * damping_factor;

                Vector2D acceleration = m->forces / m->mass;

                // semi-implicit Euler method
                m->velocity += acceleration * delta_t;
                m->position += m->velocity * delta_t;
            }

            // Reset all forces on each mass
            m->forces = Vector2D(0, 0);
        }
    }

    void Rope::simulateVerlet(float delta_t, Vector2D gravity)
    {
        for (auto &s : springs)
        {
            // TODO (Part 3): Simulate one timestep of the rope using explicit Verlet （solving constraints)
            Vector2D dir = s->m2->position - s->m1->position;
            float cur_length = dir.norm();
            Vector2D adjustment2 = - s->k * dir.unit() * (cur_length - s->rest_length) / 2;
            s->m2->forces += adjustment2;
            Vector2D adjustment1 = - adjustment2;
            s->m1->forces += adjustment1;
        }

        const float damping_factor = 0.00005;

        for (auto &m : masses)
        {
            if (!m->pinned)
            {
                Vector2D temp_position = m->position;
                // TODO (Part 3.1): Set the new position of the rope mass
                // TODO (Part 4): Add global Verlet damping
                m->forces += gravity * m->mass;
                Vector2D acceleration = m->forces / m->mass;
                m->position = m->position + (1 - damping_factor) * (m->position - m->last_position) + acceleration * delta_t * delta_t;
                m->last_position = temp_position;
            }

            // Reset all forces on each mass
            m->forces = Vector2D(0, 0);
        }
    }
}
