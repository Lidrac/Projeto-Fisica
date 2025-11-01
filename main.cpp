#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <optional>
#include <limits>

// --- Constantes da Simulação (parâmetros do seu último código) ---
const unsigned int WINDOW_WIDTH = 1200;
const unsigned int WINDOW_HEIGHT = 800;
const float CONDUCTOR_WIDTH = 1000.f;
const float CONDUCTOR_HEIGHT = 400.f;
const float CONDUCTOR_X = (WINDOW_WIDTH - CONDUCTOR_WIDTH) / 2.f;
const float CONDUCTOR_Y = (WINDOW_HEIGHT - CONDUCTOR_HEIGHT) / 2.f;
const float PARTICLE_RADIUS = 5.f;

const double PARTICLE_MASS = 1.0; 

const double ELECTRON_CHARGE = -1.602e-19;
const double FORCE_SCALING_FACTOR = 1.5e18;
const int CARRIER_INCREASE_RATE = 50;

// --- Estrutura para representar a nossa única partícula ativa ---
struct Particle
{
    sf::CircleShape shape;
    sf::Vector2f velocity;
    double charge;

    void reset()
    {
        float startY = CONDUCTOR_Y + (CONDUCTOR_HEIGHT / 2.f);
        float startX = CONDUCTOR_X;
        shape.setPosition({startX, startY});
        velocity = {0.f, 0.f};
    }
};

int main()
{
    srand(time(0));

    sf::RenderWindow window(sf::VideoMode({WINDOW_WIDTH, WINDOW_HEIGHT}), "Simulacao do Efeito Hall");
    window.setFramerateLimit(60);

    sf::Font font;
    if (!font.openFromFile("font.ttf"))
    {
        return -1;
    }

    sf::Text infoText(font, "Loading...", 20);
    infoText.setPosition({10, 10});
    infoText.setFillColor(sf::Color::White);

    sf::Text controlsText(font, L"Controles:\nCima/Baixo: Campo B | Esquerda/Direita: Corrente I\n'S': Trocar portador | 'R': Resetar | 'Espaço': Pausar/Rodar", 18);
    controlsText.setPosition({10, WINDOW_HEIGHT - 120});
    controlsText.setFillColor(sf::Color(200, 200, 200));

    sf::Text calculationsText(font, L"Cálculos da última corrida aparecerão aqui.", 18);
    calculationsText.setPosition({10, WINDOW_HEIGHT - 60});
    calculationsText.setFillColor(sf::Color::Yellow);

    sf::RectangleShape conductorShape({CONDUCTOR_WIDTH, CONDUCTOR_HEIGHT});
    conductorShape.setPosition({CONDUCTOR_X, CONDUCTOR_Y});
    conductorShape.setFillColor(sf::Color(50, 50, 50));
    conductorShape.setOutlineThickness(2.f);
    conductorShape.setOutlineColor(sf::Color::White);

    Particle activeParticle;
    activeParticle.shape.setRadius(PARTICLE_RADIUS);
    activeParticle.reset();

    bool areElectrons = true;
    double magneticField = 2.0;
    double currentFactor = 200.0;
    double hallVoltage = 0.0;
    
    int totalChargeTop = 0;
    int totalChargeBottom = 0;

    bool isPaused = false;

    float lastXDeflection = 0.f;
    float lastYDeflection = 0.f;

    auto setupParticleType = [&](Particle& p) {
        if (areElectrons) {
            p.charge = ELECTRON_CHARGE;
            p.shape.setFillColor(sf::Color::Cyan);
        } else {
            p.charge = -ELECTRON_CHARGE;
            p.shape.setFillColor(sf::Color::Red);
        }
    };
    
    setupParticleType(activeParticle);

    sf::Clock clock;

    while (window.isOpen())
    {
        sf::Time dt = clock.restart();

        if (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();

            if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
            {
                if (keyPressed->code == sf::Keyboard::Key::Up) magneticField += 0.2;
                if (keyPressed->code == sf::Keyboard::Key::Down) magneticField -= 0.2;
                if (keyPressed->code == sf::Keyboard::Key::Right) currentFactor += 0.1;
                if (keyPressed->code == sf::Keyboard::Key::Left) currentFactor = std::max(0.1, currentFactor - 0.1);
                
                if (keyPressed->code == sf::Keyboard::Key::Space)
                {
                    isPaused = !isPaused;
                }

                if (keyPressed->code == sf::Keyboard::Key::S || keyPressed->code == sf::Keyboard::Key::R)
                {
                    if (keyPressed->code == sf::Keyboard::Key::S) areElectrons = !areElectrons;
                    totalChargeTop = 0;
                    totalChargeBottom = 0;
                    hallVoltage = 0;
                    lastXDeflection = 0.f;
                    lastYDeflection = 0.f;
                    activeParticle.reset();
                    setupParticleType(activeParticle);
                }
            }
        }

        if (!isPaused)
        {
            activeParticle.velocity.x = currentFactor;

            double electricField = hallVoltage / CONDUCTOR_HEIGHT;
            double electricForce = activeParticle.charge * electricField;
            double magneticForce = activeParticle.charge * activeParticle.velocity.x * magneticField;
            double totalForce = (magneticForce + electricForce) * FORCE_SCALING_FACTOR;
            
            double acceleration_y = -totalForce / PARTICLE_MASS;
            
            activeParticle.velocity.y += acceleration_y * dt.asSeconds();
            activeParticle.shape.move(activeParticle.velocity * dt.asSeconds());
            
            sf::Vector2f pos = activeParticle.shape.getPosition();
            
            bool runIsOver = false;


            if (pos.y < CONDUCTOR_Y) // Colidiu no topo
            {
                totalChargeTop += CARRIER_INCREASE_RATE;
                runIsOver = true;
                
                // O deslocamento X é a distância percorrida até a colisão
                lastXDeflection = pos.x - CONDUCTOR_X;
                // O deslocamento Y é a distância fixa do centro até a parede de cima
                lastYDeflection = -CONDUCTOR_HEIGHT / 2.f; 
            }
            else if (pos.y > CONDUCTOR_Y + CONDUCTOR_HEIGHT - 2 * PARTICLE_RADIUS) // Colidiu embaixo
            {
                totalChargeBottom += CARRIER_INCREASE_RATE;
                runIsOver = true;

                lastXDeflection = pos.x - CONDUCTOR_X;
                // O deslocamento Y é a distância fixa do centro até a parede de baixo
                lastYDeflection = CONDUCTOR_HEIGHT / 2.f;
            }
            else if (pos.x > CONDUCTOR_X + CONDUCTOR_WIDTH) // Saiu pela direita (sem colidir)
            {
                runIsOver = true;

                // O deslocamento X é a largura total do condutor
                lastXDeflection = CONDUCTOR_WIDTH;
                // O deslocamento Y é a posição final real da partícula
                lastYDeflection = pos.y - (CONDUCTOR_Y + CONDUCTOR_HEIGHT / 2.f);
            }

            if (runIsOver)
            {
                double chargeDifference = totalChargeBottom - totalChargeTop;
                hallVoltage = chargeDifference * magneticField * currentFactor * (areElectrons ? -1.0 : 1.0); 
                activeParticle.reset();
            }
        }

        // --- ATUALIZAÇÃO E RENDERIZAÇÃO ---
        
        std::wstringstream info_ss;
        info_ss << std::fixed << std::setprecision(2);
        info_ss << L"Campo Magnético (B): " << magneticField << L" T\n";
        info_ss << L"Fator Corrente (I): " << currentFactor << L"\n";
        info_ss << L"Portadores Acumulados: " << totalChargeBottom + totalChargeTop << L"\n\n";
        info_ss << std::scientific << std::setprecision(3);
        info_ss << L"Tensão Hall (V_H): " << hallVoltage << L" V";
        infoText.setString(info_ss.str());

        std::wstringstream calc_ss;
        calc_ss << std::fixed << std::setprecision(1);
        calc_ss << L"Última Corrida -> Deslocamento X: " << lastXDeflection << L" pixels | Deslocamento Y: " << lastYDeflection << L" pixels";
        calculationsText.setString(calc_ss.str());

        window.clear(sf::Color(20, 20, 40));
        window.draw(conductorShape);
        window.draw(activeParticle.shape);
        window.draw(infoText);
        window.draw(controlsText);
        window.draw(calculationsText);
        window.display();
    }

    return 0;
}