#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp> 
#include <vector>
#include <stdio.h> 
#include <cmath>
#include <map>
#include <ctime>
#include <fstream> 

enum class GameState { MainMenu, Playing, LevelUp, GameOver };
enum class EnemyType { Normal, Tank, Boss };

void saveHighScore(int score, std::string name) {
    std::ofstream file("highscore.txt");
    if (file.is_open()) file << name << " " << score;
    file.close();
}

void loadHighScore(int &score, std::string &name) {
    std::ifstream file("highscore.txt");
    if (file.is_open()) file >> name >> score;
    else { score = 0; name = "Nadie"; }
    file.close();
}

// --- ESTRUCTURAS ---
struct Enemy {
    sf::Sprite sprite;
    float speed, hp, baseScale;
    EnemyType type;
    int currentFrame = 0, rowOffset = 0;
    float animationTimer = 0.0f;
    int frameWidth, frameHeight;

    void init(sf::Texture& tex, sf::Vector2f pos, int playerLevel, EnemyType t) {
        type = t;
        sprite.setTexture(tex);
        if (type == EnemyType::Normal) {
            frameWidth = tex.getSize().x / 4; frameHeight = tex.getSize().y;
            speed = 125.0f; hp = (playerLevel < 6) ? 1.0f : 2.0f; baseScale = 0.8f;
        } else if (type == EnemyType::Tank) {
            frameWidth = tex.getSize().x / 4; frameHeight = tex.getSize().y / 4;
            speed = 75.0f; hp = 3.0f + (playerLevel * 0.5f); baseScale = 2.0f;
        } else if (type == EnemyType::Boss) {
            frameWidth = tex.getSize().x; frameHeight = tex.getSize().y;
            speed = 55.0f; hp = 6.0f + (playerLevel * 2.0f); baseScale = 2.5f; 
        }
        sprite.setOrigin(frameWidth / 2.f, frameHeight / 2.f);
        sprite.setTextureRect(sf::IntRect(0, 0, frameWidth, frameHeight));
        sprite.setScale(baseScale, baseScale);
        sprite.setPosition(pos);
    }

    void updateAnimation(float dt, sf::Vector2f dir) {
        if (type == EnemyType::Boss) {
            if (dir.x > 0) sprite.setScale(baseScale, baseScale);
            else if (dir.x < 0) sprite.setScale(-baseScale, baseScale);
            return;
        }
        animationTimer += dt;
        if (animationTimer >= 0.15f) { animationTimer = 0.0f; currentFrame = (currentFrame + 1) % 4; }
        if (type == EnemyType::Normal) {
            rowOffset = 0; 
            if (dir.x > 0) sprite.setScale(-baseScale, baseScale);
            else if (dir.x < 0) sprite.setScale(baseScale, baseScale);
        } else if (type == EnemyType::Tank) {
            if (std::abs(dir.y) > std::abs(dir.x)) rowOffset = (dir.y > 0) ? 0 : 3;
            else rowOffset = (dir.x > 0) ? 1 : 2;
            if (dir.x > 0) sprite.setScale(-baseScale, baseScale); 
            else if (dir.x < 0) sprite.setScale(baseScale, baseScale);
        }
        sprite.setTextureRect(sf::IntRect(currentFrame * frameWidth, rowOffset * frameHeight, frameWidth, frameHeight));
    }
};

struct Player {
    sf::Sprite sprite; sf::Texture texture;
    int currentFrame = 0, rowOffset = 0;
    float animationTimer = 0.0f;
    int frameWidth, frameHeight;
    float speed = 220.0f, hp = 100.0f, maxHp = 100.0f, baseScale = 1.0f;
    int level = 1, xp = 0, nextLevelXp = 5, bulletCount = 1;
    sf::Vector2f lastDirection = { 0.0f, 1.0f };

    bool load(const std::string& filename) {
        if (!texture.loadFromFile(filename)) return false;
        frameWidth = texture.getSize().x / 4; frameHeight = texture.getSize().y / 4;
        sprite.setTexture(texture);
        baseScale = 45.0f / (float)frameWidth;
        sprite.setOrigin(frameWidth / 2.f, frameHeight / 2.f);
        sprite.setScale(baseScale, baseScale);
        return true;
    }

    void updateAnimation(float dt, bool isMoving, sf::Vector2f moveDir) {
        if (isMoving) {
            if (std::abs(moveDir.y) > std::abs(moveDir.x)) rowOffset = (moveDir.y > 0) ? 0 : 1;
            else rowOffset = (moveDir.x > 0) ? 2 : 3;
            animationTimer += dt;
            if (animationTimer >= 0.1f) { animationTimer = 0.0f; currentFrame = (currentFrame + 1) % 4; }
            if (moveDir.x > 0) sprite.setScale(baseScale, baseScale);
            else if (moveDir.x > 0) sprite.setScale(-baseScale, baseScale);
        } else { currentFrame = 0; }
        sprite.setTextureRect(sf::IntRect(currentFrame * frameWidth, rowOffset * frameHeight, frameWidth, frameHeight));
    }

    void reset() {
        speed = 220.0f; hp = 100.0f; maxHp = 100.0f; level = 1; xp = 0; nextLevelXp = 5;
        sprite.setPosition(400, 300); bulletCount = 1; lastDirection = { 0.0f, 1.0f };
    }
};

struct Bullet { sf::CircleShape shape; sf::Vector2f velocity; float lifetime = 1.5f; };
struct Gem { sf::Sprite sprite; };

// --- MAIN ---
int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "Survivor Arsenal");
    window.setFramerateLimit(60);
    srand(static_cast<unsigned>(time(0)));

    // Variables de Score y Nombre
    int currentScore = 0, highScore = 0;
    std::string highScoreName = "", playerName = "";
    loadHighScore(highScore, highScoreName);

    // Audio
    sf::Music menuMusic, actionMusic;
    menuMusic.openFromFile("menuTheme.ogg"); actionMusic.openFromFile("music.ogg");
    menuMusic.setLoop(true); actionMusic.setLoop(true);
    menuMusic.setVolume(40.f); actionMusic.setVolume(30.f);
    menuMusic.play();

    sf::SoundBuffer shootBuff, hitBuff, gameOverBuff;
    sf::Sound shootSound, hitSound, gameOverSound;
   
    if (gameOverBuff.loadFromFile("gameOver.wav")) gameOverSound.setBuffer(gameOverBuff);

    bool gameOverSoundPlayed = false;

    // Texturas
    std::map<EnemyType, sf::Texture> enemyTextures;
    sf::Texture texMenu, texGem, texGrass;
    texMenu.loadFromFile("menu_bg.png");
    enemyTextures[EnemyType::Normal].loadFromFile("normal.png");
    enemyTextures[EnemyType::Tank].loadFromFile("tank.png");
    enemyTextures[EnemyType::Boss].loadFromFile("Boss.png");
    texGem.loadFromFile("gem.png");
    texGrass.loadFromFile("Grass.png");
    texGrass.setRepeated(true);

    sf::Font font;
    bool hasFont = font.loadFromFile("/home/adrian/CONTRA/pixeloid_sans/PixeloidSans-Bold.ttf");

    Player p; p.load("player.png"); p.reset();
    sf::Sprite spriteMenu(texMenu);
    spriteMenu.setScale(800.f / texMenu.getSize().x, 600.f / texMenu.getSize().y);
    sf::Sprite bg(texGrass);
    bg.setTextureRect({0, 0, 20000, 20000}); bg.setPosition(-10000, -10000);

    sf::Text uiMain, uiTimer, uiMessage, uiScore;
    if (hasFont) {
        uiMain.setFont(font); uiMain.setCharacterSize(18); uiMain.setPosition(10, 10);
        uiTimer.setFont(font); uiTimer.setCharacterSize(22); uiTimer.setPosition(370, 10); uiTimer.setFillColor(sf::Color::Yellow);
        uiMessage.setFont(font); uiMessage.setCharacterSize(25); uiMessage.setOutlineThickness(2);
        uiScore.setFont(font); uiScore.setCharacterSize(18); uiScore.setPosition(620, 10);
    }

    sf::RectangleShape hpBack(sf::Vector2f(200, 12)), hpFront(sf::Vector2f(200, 12)), overlay(sf::Vector2f(800, 600));
    hpBack.setFillColor(sf::Color(50, 50, 50)); hpBack.setPosition(10, 40);
    hpFront.setFillColor(sf::Color::Red); hpFront.setPosition(10, 40);
    overlay.setFillColor(sf::Color(0, 0, 0, 180));

    std::vector<Enemy> enemies; std::vector<Bullet> bullets; std::vector<Gem> gems;
    sf::Clock spawnClock, shootClock, damageClock, dtClock, gameTimer;
    float spawnInterval = 1.0f;
    GameState state = GameState::MainMenu;

    while (window.isOpen()) {
        float dt = dtClock.restart().asSeconds();
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();

            if (state == GameState::MainMenu && event.type == sf::Event::TextEntered) {
                if (event.text.unicode < 128) {
                    if (event.text.unicode == 8 && !playerName.empty()) playerName.pop_back();
                    else if (event.text.unicode != 8 && event.text.unicode != 32 && playerName.size() < 12) 
                        playerName += static_cast<char>(event.text.unicode);
                }
            }

            if (event.type == sf::Event::KeyPressed) {
                if (state == GameState::MainMenu && event.key.code == sf::Keyboard::Space) {
                    if (playerName.empty()) playerName = "Player1";
                    state = GameState::Playing; 
                    p.reset(); enemies.clear(); gems.clear(); bullets.clear(); 
                    currentScore = 0; gameTimer.restart();
                    menuMusic.stop(); actionMusic.play();
                    gameOverSoundPlayed = false;
                }
                if (state == GameState::LevelUp) {
                    if (event.key.code == sf::Keyboard::Num1) { p.bulletCount++; state = GameState::Playing; }
                    if (event.key.code == sf::Keyboard::Num2) { p.speed += 35.f; state = GameState::Playing; }
                    if (event.key.code == sf::Keyboard::Num3) { p.hp = p.maxHp; state = GameState::Playing; }
                }
                if (state == GameState::GameOver && event.key.code == sf::Keyboard::Escape) {
                    state = GameState::MainMenu;
                    gameOverSound.stop(); menuMusic.play(); 
                }
            }
        }

        if (state == GameState::Playing) {
           
            sf::Vector2f moveDir(0.0f, 0.0f);
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) moveDir.y -= 1;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) moveDir.y += 1;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) moveDir.x -= 1;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) moveDir.x += 1;

            if (moveDir.x != 0 || moveDir.y != 0) {
                float len = std::sqrt(moveDir.x*moveDir.x + moveDir.y*moveDir.y);
                p.sprite.move((moveDir/len) * p.speed * dt);
                p.updateAnimation(dt, true, moveDir);
                p.lastDirection = moveDir/len; 
            } else p.updateAnimation(dt, false, moveDir);

            sf::Vector2f fireDir(0.0f, 0.0f);
            bool isAiming = false;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) { fireDir.y -= 1; isAiming = true; }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) { fireDir.y += 1; isAiming = true; }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) { fireDir.x -= 1; isAiming = true; }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) { fireDir.x += 1; isAiming = true; }
            if (isAiming) {
                float len = std::sqrt(fireDir.x*fireDir.x + fireDir.y*fireDir.y);
                p.lastDirection = fireDir/len;
            }

            if (spawnClock.getElapsedTime().asSeconds() > spawnInterval) {
                float angle = (rand() % 360) * 3.14159f / 180.f;
                sf::Vector2f pos = p.sprite.getPosition() + sf::Vector2f(std::cos(angle)*550, std::sin(angle)*550);
                EnemyType t = (rand() % 100 < 10) ? EnemyType::Boss : (rand() % 100 < 30 ? EnemyType::Tank : EnemyType::Normal);
                Enemy e; e.init(enemyTextures[t], pos, p.level, t);
                enemies.push_back(e); spawnClock.restart();
            }

            if (shootClock.getElapsedTime().asSeconds() > 0.55f) {
                shootSound.play(); 
                for (int i = 0; i < p.bulletCount; i++) {
                    Bullet b; b.shape.setRadius(5); b.shape.setFillColor(sf::Color::Cyan); b.shape.setPosition(p.sprite.getPosition());
                    float spread = (i - (p.bulletCount - 1) / 2.0f) * 0.25f;
                    float cs = std::cos(spread), sn = std::sin(spread);
                    b.velocity = sf::Vector2f(p.lastDirection.x * cs - p.lastDirection.y * sn, p.lastDirection.x * sn + p.lastDirection.y * cs) * 550.f;
                    bullets.push_back(b);
                }
                shootClock.restart();
            }

        
            for (int i = 0; i < (int)bullets.size(); i++) {
                bullets[i].shape.move(bullets[i].velocity * dt);
                bullets[i].lifetime -= dt;
                if (bullets[i].lifetime <= 0) { bullets.erase(bullets.begin() + i); i--; continue; }
                for (int j = 0; j < (int)enemies.size(); j++) {
                    if (bullets[i].shape.getGlobalBounds().intersects(enemies[j].sprite.getGlobalBounds())) {
                        enemies[j].hp -= 1.0f; bullets.erase(bullets.begin() + i); i--;
                        if (enemies[j].hp <= 0) {
                            if (enemies[j].type == EnemyType::Normal) currentScore += 10;
                            else if (enemies[j].type == EnemyType::Tank) currentScore += 50;
                            else if (enemies[j].type == EnemyType::Boss) currentScore += 500;
                            Gem g; g.sprite.setTexture(texGem); g.sprite.setScale(0.3f, 0.3f);
                            g.sprite.setOrigin(g.sprite.getLocalBounds().width/2.f, g.sprite.getLocalBounds().height/2.f);
                            g.sprite.setPosition(enemies[j].sprite.getPosition());
                            gems.push_back(g); enemies.erase(enemies.begin() + j);
                        }
                        break;
                    }
                }
            }

       
            for (int i = 0; i < (int)enemies.size(); i++) {
                sf::Vector2f d = p.sprite.getPosition() - enemies[i].sprite.getPosition();
                float dist = std::sqrt(d.x*d.x + d.y*d.y);
                sf::Vector2f dir = (dist > 0) ? d/dist : sf::Vector2f(0,0);
                enemies[i].sprite.move(dir * enemies[i].speed * dt);
                enemies[i].updateAnimation(dt, dir);
                if (enemies[i].sprite.getGlobalBounds().intersects(p.sprite.getGlobalBounds()) && damageClock.getElapsedTime().asSeconds() > 0.6f) {
                    p.hp -= (enemies[i].type == EnemyType::Boss ? 35.0f : 10.0f);
                    hitSound.play(); damageClock.restart(); 
                    if (p.hp <= 0) { 
                        state = GameState::GameOver; actionMusic.stop(); 
                        if (!gameOverSoundPlayed) { gameOverSound.play(); gameOverSoundPlayed = true; }
                        if (currentScore > highScore) { highScore = currentScore; highScoreName = playerName; saveHighScore(highScore, highScoreName); }
                    }
                }
            }

       
            for (int i = 0; i < (int)gems.size(); i++) {
                if (p.sprite.getGlobalBounds().intersects(gems[i].sprite.getGlobalBounds())) {
                    p.xp++; gems.erase(gems.begin() + i); i--;
                    if (p.xp >= p.nextLevelXp) { state = GameState::LevelUp; p.level++; p.xp = 0; p.nextLevelXp += 5; spawnInterval *= 0.98f; }
                }
            }
        }

       
       // --- RENDER ---
        window.clear(sf::Color(10, 10, 10));
        
        if (state == GameState::MainMenu) {
            window.setView(window.getDefaultView());
            window.draw(spriteMenu);
            
            if (hasFont) {
                // 1. LÓGICA DE ANIMACIÓN (Ondulación y Parpadeo)
                float time = gameTimer.getElapsedTime().asSeconds();
                float offsetY = std::sin(time * 2.5f) * 12.0f; // Movimiento arriba/abajo
                
                // Calculamos el brillo (Alpha) de 100 a 255 para que nunca desaparezca del todo
                int alpha = static_cast<int>(155 + (std::sin(time * 4.0f) + 1.0f) * 0.5f * 100.0f);

                // 2. CAMBIO DE COLOR DINÁMICO
                // Si el score actual supera al record, se vuelve dorado
                sf::Color textColor = sf::Color(255, 255, 255, alpha); // Blanco por defecto
                if (currentScore > highScore && currentScore > 0) {
                    textColor = sf::Color(255, 215, 0, alpha); // Dorado (Gold)
                }

                // 3. CONSTRUCCIÓN DEL TEXTO
                std::string menuText = "INGRESA TU NOMBRE: " + playerName + "_\n\n";
                menuText += "[ESPACIO] PARA EMPEZAR\n\n";
                menuText += "ULTIMO RECORD: " + highScoreName + " - " + std::to_string(highScore);
                
                if (currentScore > 0) {
                    menuText += "\nTU PUNTUACION ANTERIOR: " + std::to_string(currentScore);
                }

                uiMessage.setString(menuText);
                uiMessage.setFillColor(textColor); 
                uiMessage.setOutlineColor(sf::Color(0, 0, 0, alpha));
                uiMessage.setOutlineThickness(2.0f);

                // 4. POSICIÓN AJUSTABLE
                // Modifica 150 (X) y 320 (Y) para moverlo por la pantalla
                uiMessage.setPosition(150.0f, 320.0f + offsetY); 
                
                window.draw(uiMessage);
            }
        }else {


         
         

if (state == GameState::MainMenu) {
    window.setView(window.getDefaultView());
    window.draw(spriteMenu);

    if (hasFont) {
      
        float posX = 150.0f; 
        float posY = 350.0f;

       
        float time = gameTimer.getElapsedTime().asSeconds();
        
       
        float offsetY = std::sin(time * 2.5f) * 12.0f; 
        
      
        int alpha = static_cast<int>((std::sin(time * 3.5f) + 1.0f) * 0.5f * 255.0f);

       
        uiMessage.setString("ESCRIBE TU NOMBRE: " + playerName + "_\n\n[ESPACIO] EMPEZAR\n\nRECORD: " + highScoreName + " - " + std::to_string(highScore));
        uiMessage.setFillColor(sf::Color(255, 255, 255, alpha)); 
        uiMessage.setPosition(posX, posY + offsetY);           
        window.draw(uiMessage);
    }
}
        
            window.setView(sf::View(p.sprite.getPosition(), { 800, 600 }));
            window.draw(bg);
            for (auto& g : gems) window.draw(g.sprite);
            for (auto& b : bullets) window.draw(b.shape);
            for (auto& e : enemies) window.draw(e.sprite);
            window.draw(p.sprite);

            window.setView(window.getDefaultView());
            hpFront.setSize({(p.hp / p.maxHp) * 200.f, 12});
            window.draw(hpBack); window.draw(hpFront);
            if (hasFont) {
                uiMain.setString("NIVEL: " + std::to_string(p.level) + " XP: " + std::to_string(p.xp));
                uiScore.setString("SCORE: " + std::to_string(currentScore));
                int s = (int)gameTimer.getElapsedTime().asSeconds();
                uiTimer.setString(std::to_string(s/60) + ":" + (s%60 < 10 ? "0" : "") + std::to_string(s%60));
                
                window.draw(uiMain); window.draw(uiScore); window.draw(uiTimer);
            }
            if (state == GameState::LevelUp || state == GameState::GameOver) {
                window.draw(overlay);
                uiMessage.setString(state == GameState::LevelUp ? "LEVEL UP!\n1.Mas Balas\n2.35% Mas Velocidad\n3. Curar" : "GAME OVER\nSCORE: " + std::to_string(currentScore) + "\n[ESC] MENU");
                uiMessage.setPosition(150, 200); window.draw(uiMessage);
            }
        }
        window.display();
    }
    return 0;
}