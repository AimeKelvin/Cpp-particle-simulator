#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>

const float PI = 3.14159265f;

struct Particle {
    sf::CircleShape shape;
    sf::Vector2f velocity;
    float radius;

    Particle(float x, float y, float r) : radius(r) {
        shape.setRadius(radius);
        shape.setOrigin(radius, radius);  // center origin for easier collision
        shape.setPosition(x, y);
        shape.setFillColor(sf::Color(rand() % 256, rand() % 256, rand() % 256));

        // Random velocity
        velocity = sf::Vector2f((rand() % 200 - 100) / 50.f, (rand() % 200 - 100) / 50.f);
    }

    sf::Vector2f getPosition() const {
        return shape.getPosition();
    }
};

bool checkCollision(const Particle& a, const Particle& b) {
    float dx = a.getPosition().x - b.getPosition().x;
    float dy = a.getPosition().y - b.getPosition().y;
    float distance = std::sqrt(dx * dx + dy * dy);
    return distance < (a.radius + b.radius);
}

void resolveCollision(Particle& a, Particle& b) {
    sf::Vector2f delta = b.getPosition() - a.getPosition();
    float dist = std::sqrt(delta.x * delta.x + delta.y * delta.y);
    if (dist == 0.f) return; // avoid division by zero

    // Minimum translation distance to push balls apart after intersecting
    float overlap = 0.5f * (dist - a.radius - b.radius);

    // Displace current balls so they are no longer overlapping
    a.shape.move(overlap * (delta.x / dist), overlap * (delta.y / dist));
    b.shape.move(-overlap * (delta.x / dist), -overlap * (delta.y / dist));

    // Compute normalized collision vector
    sf::Vector2f norm = delta / dist;

    // Relative velocity
    sf::Vector2f relVel = b.velocity - a.velocity;

    // Velocity along the normal
    float velAlongNorm = relVel.x * norm.x + relVel.y * norm.y;

    if (velAlongNorm > 0)
        return; // particles moving away, no need to resolve

    // Elastic collision response (assuming equal mass)
    float restitution = 1.0f; // perfectly elastic

    float j = -(1 + restitution) * velAlongNorm / 2; // impulse scalar

    sf::Vector2f impulse = j * norm;

    a.velocity -= impulse;
    b.velocity += impulse;
}

int main() {
    srand(static_cast<unsigned int>(time(nullptr)));

    sf::RenderWindow window(sf::VideoMode(800, 600), "Particle Interaction Simulator");
    window.setFramerateLimit(60);

    const int particleCount = 50;
    const float radius = 10.f;
    const float gravity = 0.2f;

    std::vector<Particle> particles;

    // Spawn particles at random positions
    for (int i = 0; i < particleCount; ++i) {
        float x = radius + rand() % (800 - int(2 * radius));
        float y = radius + rand() % (600 - int(2 * radius));
        particles.emplace_back(x, y, radius);
    }

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event))
            if (event.type == sf::Event::Closed)
                window.close();

        // Update physics
        for (auto& p : particles) {
            p.velocity.y += gravity; // apply gravity

            sf::Vector2f pos = p.getPosition();
            // Boundary collision with window edges (simulate cups)
            if (pos.x - radius < 0.f) {
                p.velocity.x = std::abs(p.velocity.x); // bounce right
                p.shape.setPosition(radius, pos.y);
            }
            else if (pos.x + radius > 800.f) {
                p.velocity.x = -std::abs(p.velocity.x); // bounce left
                p.shape.setPosition(800.f - radius, pos.y);
            }
            if (pos.y - radius < 0.f) {
                p.velocity.y = std::abs(p.velocity.y); // bounce down
                p.shape.setPosition(pos.x, radius);
            }
            else if (pos.y + radius > 600.f) {
                p.velocity.y = -std::abs(p.velocity.y); // bounce up
                p.shape.setPosition(pos.x, 600.f - radius);
            }

            p.shape.move(p.velocity);
        }

        // Check collisions between particles
        for (size_t i = 0; i < particles.size(); ++i) {
            for (size_t j = i + 1; j < particles.size(); ++j) {
                if (checkCollision(particles[i], particles[j])) {
                    resolveCollision(particles[i], particles[j]);
                }
            }
        }

        window.clear(sf::Color::Black);
        for (const auto& p : particles)
            window.draw(p.shape);
        window.display();
    }

    return 0;
}
