#include <SFML/Graphics.hpp> // Biblioteca principal de gráficos do SFML
#include <vector>            // Para uso de std::vector
#include <string>            // Para uso de std::string e std::wstringstream
#include <sstream>           // Para uso de std::wstringstream (strings de texto dinâmico)
#include <iomanip>           // Para formatação de texto (setprecision, fixed, scientific)
#include <cmath>             // Para funções matemáticas (atan2, cos, sin, max)
#include <optional>          // Para o novo sistema de eventos da SFML 3 (std::optional)
#include <limits>            // Para uso de std::numeric_limits
#include <algorithm>         // Para uso de std::clamp

// Define a constante M_PI (Pi) se não estiver definida (necessário para cmath)
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// --- Constantes da Simulação ---

// Parâmetros da janela e do ambiente visual
const unsigned int WINDOW_WIDTH = 1200;
const unsigned int WINDOW_HEIGHT = 800;
const float CONDUCTOR_WIDTH = 1000.f;
const float CONDUCTOR_HEIGHT = 400.f;
const float CONDUCTOR_X = (WINDOW_WIDTH - CONDUCTOR_WIDTH) / 2.f;
const float CONDUCTOR_Y = (WINDOW_HEIGHT - CONDUCTOR_HEIGHT) / 2.f;
const float PARTICLE_RADIUS = 5.f;

// Parâmetros da física de simulação
const double PARTICLE_MASS = 1.0;           // Massa de simulação arbitrária para equilíbrio
const double ELECTRON_CHARGE = -1.602e-19;    // Carga elementar (em Coulombs)
const double FORCE_SCALING_FACTOR = 1.5e18; // Fator de amplificação visual da força
const int CARRIER_INCREASE_RATE = 50;     // Taxa de incremento de portadores por colisão

// Fator de escala APENAS para a visualização dos vetores (setas)
const double VISUAL_FORCE_SCALE = 1.25e18;
// Limite máximo (em pixels) para o comprimento da seta
const float MAX_ARROW_LENGTH = 80.f;

// Fator de escala para a Tensão Hall, calculado para balancear
// o FORCE_SCALING_FACTOR e os outros parâmetros.
const double HALL_VOLTAGE_SCALE = 160.0; 

/**
 * @struct Particle
 * @brief Estrutura que representa um único portador de carga (partícula).
 * Armazena suas propriedades visuais e físicas.
 */
struct Particle
{
    sf::CircleShape shape;  // Representação gráfica da partícula
    sf::Vector2f velocity;  // Vetor de velocidade (componentes x, y)
    double charge;          // Carga elétrica da partícula

    /**
     * @brief Reseta a partícula para a posição inicial (centro da entrada do condutor)
     * e zera sua velocidade.
     */
    void reset()
    {
        float startY = CONDUCTOR_Y + (CONDUCTOR_HEIGHT / 2.f);
        float startX = CONDUCTOR_X;
        shape.setPosition({startX, startY});
        velocity = {0.f, 0.f};
    }
};

/**
 * @brief Desenha uma seta (vetor) na janela de renderização.
 * @param window Referência à janela sf::RenderWindow.
 * @param start Ponto inicial (sf::Vector2f) da seta.
 * @param end Ponto final (sf::Vector2f) da seta.
 * @param color Cor (sf::Color) da seta.
 */
void drawArrow(sf::RenderWindow& window, sf::Vector2f start, sf::Vector2f end, sf::Color color)
{
    // Desenha a linha principal do vetor
    sf::Vertex line[] = { sf::Vertex{start, color}, sf::Vertex{end, color} };
    window.draw(line, 2, sf::PrimitiveType::Lines);

    // Calcula os pontos para a ponta da seta
    float arrowLength = 10.f;
    float angle = std::atan2(end.y - start.y, end.x - start.x);

    sf::Vector2f head1(
        end.x - arrowLength * std::cos(angle - M_PI / 6.f),
        end.y - arrowLength * std::sin(angle - M_PI / 6.f)
    );
    sf::Vector2f head2(
        end.x - arrowLength * std::cos(angle + M_PI / 6.f),
        end.y - arrowLength * std::sin(angle + M_PI / 6.f)
    );

    // Desenha a ponta da seta
    sf::Vertex head[] = {
        sf::Vertex{end, color},
        sf::Vertex{head1, color},
        sf::Vertex{end, color},
        sf::Vertex{head2, color}
    };
    window.draw(head, 4, sf::PrimitiveType::Lines);
}

/**
 * @brief Desenha a representação do Campo Magnético (B) no condutor.
 * Símbolos 'X' indicam um campo entrando na tela.
 * @param window Janela de renderização.
 * @param font Fonte para desenhar o símbolo 'X'.
 * @param bounds Limites do condutor (sf::FloatRect).
 */
void drawBField(sf::RenderWindow& window, sf::Font& font, sf::FloatRect bounds)
{
    sf::Text bSymbol(font, "X", 20);
    bSymbol.setFillColor(sf::Color(255, 255, 255, 50)); // Cor cinza-claro, 50% transparente

    for (float x = bounds.position.x + 50; x < bounds.position.x + bounds.size.x; x += 100)
    {
        for (float y = bounds.position.y + 50; y < bounds.position.y + bounds.size.y; y += 100)
        {
            bSymbol.setPosition({x, y});
            window.draw(bSymbol);
        }
    }
}


int main()
{
    // Inicializa o gerador de números aleatórios
    srand(time(0));

    // --- 1. Inicialização e Carregamento de Recursos ---
    
    sf::RenderWindow window(sf::VideoMode({WINDOW_WIDTH, WINDOW_HEIGHT}), "Simulacao do Efeito Hall");
    window.setFramerateLimit(60); // Limita a 60 quadros por segundo

    // Carrega a fonte para a UI
    sf::Font font;
    if (!font.openFromFile("font.ttf"))
    {
        // Falha no carregamento, encerra o programa
        return -1;
    }

    // Configura os elementos de texto da UI
    sf::Text infoText(font, "Loading...", 20);
    infoText.setPosition({10, 10});
    infoText.setFillColor(sf::Color::White);

    sf::Text controlsText(font, L"Controles:\nCima/Baixo: Campo B | Esquerda/Direita: Corrente I\n'S': Trocar portador | 'R': Resetar | 'Espaço': Pausar/Rodar", 18);
    controlsText.setPosition({10, WINDOW_HEIGHT - 120});
    controlsText.setFillColor(sf::Color(200, 200, 200));

    sf::Text calculationsText(font, L"Cálculos da última corrida aparecerão aqui.", 18);
    calculationsText.setPosition({10, WINDOW_HEIGHT - 60});
    calculationsText.setFillColor(sf::Color::Yellow);

    // Configura o objeto visual do condutor
    sf::RectangleShape conductorShape({CONDUCTOR_WIDTH, CONDUCTOR_HEIGHT});
    conductorShape.setPosition({CONDUCTOR_X, CONDUCTOR_Y});
    conductorShape.setFillColor(sf::Color(50, 50, 50));
    conductorShape.setOutlineThickness(2.f);
    conductorShape.setOutlineColor(sf::Color::White);

    // Elementos da UI para a legenda de forças
    sf::Text legendTitle(font, L"Legenda:", 16);
    legendTitle.setPosition({WINDOW_WIDTH - 200, 10});
    legendTitle.setFillColor(sf::Color::White);

    sf::RectangleShape fm_key({20, 3}); // Chave de cor para Força Magnética
    fm_key.setFillColor(sf::Color::Red);
    fm_key.setPosition({WINDOW_WIDTH - 200, 40});

    sf::Text fm_text(font, L": Força Magnética (Fm)", 15);
    fm_text.setPosition({WINDOW_WIDTH - 175, 35});
    fm_text.setFillColor(sf::Color::White);

    sf::RectangleShape fe_key({20, 3}); // Chave de cor para Força Elétrica
    fe_key.setFillColor(sf::Color::Blue);
    fe_key.setPosition({WINDOW_WIDTH - 200, 60});

    sf::Text fe_text(font, L": Força Elétrica (Fe)", 15);
    fe_text.setPosition({WINDOW_WIDTH - 175, 55});
    fe_text.setFillColor(sf::Color::White);


    // --- 2. Variavéis de Estado da Simulação ---

    // A única partícula ativa na simulação
    Particle activeParticle;
    activeParticle.shape.setRadius(PARTICLE_RADIUS);
    activeParticle.reset();

    // Parâmetros da física controlados pelo usuário
    bool areElectrons = true;
    double magneticField = 2.0;
    double currentFactor = 200.0;
    double hallVoltage = 0.0;
    
    // Contadores de colisão ("memória" do sistema)
    int totalChargeTop = 0;
    int totalChargeBottom = 0;

    // Estado de pausa
    bool isPaused = false;

    // Variáveis para exibir os resultados da última corrida
    float lastXDeflection = 0.f;
    float lastYDeflection = 0.f;

    // Variáveis para armazenar forças para visualização
    double currentMagneticForce = 0.0;
    double currentElectricForce = 0.0;

    // Função Lambda para configurar o tipo de partícula (elétron ou lacuna)
    auto setupParticleType = [&](Particle& p) {
        if (areElectrons) {
            p.charge = ELECTRON_CHARGE;
            p.shape.setFillColor(sf::Color::Cyan);
        } else {
            p.charge = -ELECTRON_CHARGE; // Carga positiva
            p.shape.setFillColor(sf::Color::Red);
        }
    };
    
    setupParticleType(activeParticle); // Configuração inicial
    sf::Clock clock; // Relógio para calcular o delta time (dt)

    // --- 3. Loop Principal da Simulação ---
    while (window.isOpen())
    {
        // Calcula o tempo passado desde o último quadro
        sf::Time dt = clock.restart();

        // --- 4. Processamento de Eventos (Input do Usuário) ---
        if (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();

            // Verifica se uma tecla foi pressionada
            if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
            {
                // Controles dos parâmetros da física
                if (keyPressed->code == sf::Keyboard::Key::Up) magneticField += 0.2;
                if (keyPressed->code == sf::Keyboard::Key::Down) magneticField -= 0.2;
                if (keyPressed->code == sf::Keyboard::Key::Right) currentFactor += 0.1;
                if (keyPressed->code == sf::Keyboard::Key::Left) currentFactor = std::max(0.1, currentFactor - 0.1);
                
                // Controle de Pausa
                if (keyPressed->code == sf::Keyboard::Key::Space)
                {
                    isPaused = !isPaused; // Inverte o estado de pausa
                }

                // Controles de Reset da Simulação
                if (keyPressed->code == sf::Keyboard::Key::S || keyPressed->code == sf::Keyboard::Key::R)
                {
                    if (keyPressed->code == sf::Keyboard::Key::S) areElectrons = !areElectrons;
                    // Zera todo o estado da simulação
                    totalChargeTop = 0;
                    totalChargeBottom = 0;
                    hallVoltage = 0;
                    currentFactor = 200.0;
                    lastXDeflection = 0.f;
                    lastYDeflection = 0.f;
                    activeParticle.reset();
                    setupParticleType(activeParticle);
                }
            }
        }

        // --- 5. Bloco de Atualização da Física (só executa se não estiver pausado) ---
        if (!isPaused)
        {
            // Define a velocidade horizontal (corrente)
            activeParticle.velocity.x = currentFactor;

            // --- Cálculo das Forças ---
            // Campo Elétrico (E = V/d)
            double electricField = hallVoltage / CONDUCTOR_HEIGHT;
            // Força Elétrica (Fe = qE)
            currentElectricForce = activeParticle.charge * electricField;
            // Força Magnética (Fm = qvB)
            currentMagneticForce = activeParticle.charge * activeParticle.velocity.x * magneticField;
            // Força total amplificada para visualização
            double totalForce = (currentMagneticForce + currentElectricForce) * FORCE_SCALING_FACTOR;
            
            // --- Atualização do Movimento ---
            // Aceleração (a = F/m)
            double acceleration_y = -totalForce / PARTICLE_MASS;
            
            // Atualiza a velocidade vertical (v = v0 + at)
            activeParticle.velocity.y += acceleration_y * dt.asSeconds();
            // Move a partícula (p = p0 + vt)
            activeParticle.shape.move(activeParticle.velocity * dt.asSeconds());
            
            sf::Vector2f pos = activeParticle.shape.getPosition();
            
            bool runIsOver = false;

            // --- Detecção de Fim da "Corrida" ---
            if (pos.y < CONDUCTOR_Y) // Colidiu no topo
            {
                totalChargeTop += CARRIER_INCREASE_RATE;
                runIsOver = true;
                
                // Armazena os dados da colisão para exibição
                lastXDeflection = pos.x - CONDUCTOR_X;
                lastYDeflection = -CONDUCTOR_HEIGHT / 2.f; 
            }
            else if (pos.y > CONDUCTOR_Y + CONDUCTOR_HEIGHT - 2 * PARTICLE_RADIUS) // Colidiu embaixo
            {
                totalChargeBottom += CARRIER_INCREASE_RATE;
                runIsOver = true;

                // Armazena os dados da colisão para exibição
                lastXDeflection = pos.x - CONDUCTOR_X;
                lastYDeflection = CONDUCTOR_HEIGHT / 2.f;
            }
            else if (pos.x > CONDUCTOR_X + CONDUCTOR_WIDTH) // Saiu pela direita (atingiu o equilíbrio)
            {
                runIsOver = true;

                // Armazena os dados da travessia para exibição
                lastXDeflection = CONDUCTOR_WIDTH;
                lastYDeflection = pos.y - (CONDUCTOR_Y + CONDUCTOR_HEIGHT / 2.f);
            }

            // Se a corrida terminou, recalcula a Tensão Hall e reseta a partícula
            if (runIsOver)
            {
                double chargeDifference = totalChargeBottom - totalChargeTop;
                
                // A Tensão Hall  é proporcional apenas ao acúmulo de carga,
                // com um fator de escala para balancear a simulação.
                hallVoltage = chargeDifference * HALL_VOLTAGE_SCALE * (areElectrons ? -1.0 : 1.0); 
                
                activeParticle.reset();
            }
        }

        // --- 6. Bloco de Renderização (sempre executa) ---
        
        // Atualiza os textos da UI
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

        // Limpa a tela para o próximo quadro
        window.clear(sf::Color(20, 20, 40));

        // Desenha os elementos do cenário
        window.draw(conductorShape);
        drawBField(window, font, conductorShape.getGlobalBounds());

        // Desenha os elementos dinâmicos
        sf::Vector2f particlePos = activeParticle.shape.getPosition();
        float midY = CONDUCTOR_Y + CONDUCTOR_HEIGHT / 2.f;

        // Lógica para desenhar o vetor da Corrente Convencional (I)
        sf::Text I_label(font, "I (Corrente Convencional)", 15);
        if (areElectrons)
        {
            // Elétrons (v_d) para a direita, então I (convencional) é para a ESQUERDA
            drawArrow(window, {CONDUCTOR_X + 120, midY}, {CONDUCTOR_X + 20, midY}, sf::Color::Yellow);
            I_label.setPosition({CONDUCTOR_X + 130, midY - 10});
        }
        else
        {
            // Lacunas (v_d) para a direita, então I (convencional) é para a DIREITA
            drawArrow(window, {CONDUCTOR_X + 20, midY}, {CONDUCTOR_X + 120, midY}, sf::Color::Yellow);
            I_label.setPosition({CONDUCTOR_X + 20, midY + 5});
        }
        window.draw(I_label);
        
        // Desenha os vetores de força (apenas se a simulação estiver rodando)
        if (!isPaused)
        {
            // Fator de escala para visualização das setas
            float forceVizScale = 1.25e18; 
            
            // Seta da Força Magnética (Fm) - Vermelha
            float fm_y = std::clamp(
                static_cast<float>(-(currentMagneticForce) * forceVizScale), 
                -MAX_ARROW_LENGTH, 
                MAX_ARROW_LENGTH
            );
            drawArrow(window, particlePos, {particlePos.x, particlePos.y + fm_y}, sf::Color::Red);

            // Seta da Força Elétrica (Fe) - Azul
            float fe_y = std::clamp(
                static_cast<float>(-(currentElectricForce) * forceVizScale), 
                -MAX_ARROW_LENGTH, 
                MAX_ARROW_LENGTH
            );
            drawArrow(window, particlePos, {particlePos.x, particlePos.y + fe_y}, sf::Color::Blue);
        }

        // Desenha a partícula e a UI
        window.draw(activeParticle.shape);
        window.draw(infoText);
        window.draw(controlsText);
        window.draw(calculationsText);

        // Desenha os elementos da legenda
        window.draw(legendTitle);
        window.draw(fm_key);
        window.draw(fm_text);
        window.draw(fe_key);
        window.draw(fe_text);
        
        // Exibe o quadro finalizado na janela
        window.display();
    }

    return 0;
}
