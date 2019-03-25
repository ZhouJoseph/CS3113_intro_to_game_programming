#include "Necessity.hpp"
#include "SheetSprite.hpp"
#include "Entity.hpp"

SDL_Window* displayWindow;

enum GameState{ STATE_MAIN_MENU, STATE_GAME_START, STATE_GAME_OVER };
GameState gameState = STATE_MAIN_MENU;

void programSetUp(ShaderProgram& textured, ShaderProgram& untextured);

void ProcessEvents(bool& done, SDL_Event& event);

GLuint LoadTexture(const char* filePath);

void
renderMainMenu(ShaderProgram& program, GLuint fontSheet,
               EntityFont& spaceInvader,EntityFont& title);

void
renderGame (ShaderProgram& textured, const EntityTextured& player,
            const std::vector<EntityTextured>& enemies,
            const std::vector<EntityTextured>& playerBullets,
            const std::vector<EntityTextured>& enemyBullets);

bool shouldRemoveBullet(Entity bullet);

void
updateGame(float& lastFrameTicks, int& lastBulletTick, bool& shootAble, int& lastEnemyMove,
           EntityTextured& player, const SheetSprite& enemyBullet,
           std::vector<EntityTextured>& playerBullets,
           std::vector<EntityTextured>& enemyBullets,
           std::vector<EntityTextured>& enemies);


void
ProcessInput(EntityTextured& player, float& lastFrameTicks,
             bool& done, SDL_Event& event,
             std::vector<EntityTextured>& playerBullets,
             const SheetSprite& enemyBullet);

void DrawSpriteSheetSprite(ShaderProgram &program, int index, int spriteCountX, int spriteCountY);

int main()
{
    
    /* setup */
    ShaderProgram textured, untextured;
    programSetUp(textured, untextured);
    
    GLuint spriteSheet = LoadTexture(RESOURCE_FOLDER"characters_1.png");
    GLuint fontSheet = LoadTexture(RESOURCE_FOLDER"pixel_font.png");
    GLuint enemyBulletSheet = LoadTexture(RESOURCE_FOLDER"laserBlue03.png");
    GLuint playerBulletSheet = LoadTexture(RESOURCE_FOLDER"pixel_font.png");
    
    SheetSprite ufoSprite(spriteSheet, 0, 4/8.0f, 1/12.0f, 1/8.0f, 0.3);
    SheetSprite enemyBullet(enemyBulletSheet, 0, 0, 1.0f, 1.0f, 0.3);
    SheetSprite playerBullet(playerBulletSheet, 0, 0, 1.0f, 1.0f, 0.3);
    SheetSprite playerSprite(spriteSheet, 1/12.0f, 3/8.0f, 1/12.0f, 1/8.0f, 0.3);
    SDL_Event event;
    
    bool done = false;
    
    EntityTextured player(0, -1.2f, 1.0f, 1.0f, playerSprite);
    player.velocity.x = 0.3f;
    player.health = 3;
    
    std::vector<EntityTextured> enemyBullets;
    std::vector<EntityTextured> playerBullets;
    std::vector<EntityTextured> enemies;
    
    float horizontalSpacing = 0.3f;
    float verticalSpacing = 0.2f;
    for (int i = 0; i < 5; ++i) {
        for (int j = 0; j < 11; j++) {
            EntityTextured ufo(-1.7f + horizontalSpacing * j, 1.2f - verticalSpacing * i, 0.85f ,0.85f, ufoSprite);
            ufo.velocity.x = 0.1f;
            enemies.push_back(ufo);
        }
    }

    EntityFont mainMenu(-1.0f, -0.1f, 0.3f, 0.3f, "(Press S to start...)");
    EntityFont title(-1.5f, 0.2f, 0.5f, 0.5f, "Space Invader");
    
    float lastFrameTicks = 0.0f;
    int lastBulletTick = 0;
    int lastEnemyMove = 0;
    
    bool shootAble = true;
    
    srand(static_cast<unsigned int>(time(NULL)));
    
    while (!done) {
        
        glClear(GL_COLOR_BUFFER_BIT);
        /* Here starts the drawing */
        
        ProcessInput(player, lastFrameTicks, done, event, playerBullets, enemyBullet);

        switch (gameState) {
            case STATE_MAIN_MENU:
                renderMainMenu(textured, fontSheet, mainMenu, title);
                break;
            case STATE_GAME_START:
                renderGame(textured, player, enemies, playerBullets, enemyBullets);
                updateGame(lastFrameTicks,lastBulletTick, shootAble, lastEnemyMove,player, enemyBullet, playerBullets, enemyBullets, enemies);
                break;
            case STATE_GAME_OVER:
                done = true;
                break;
        }
        
        /* Here ends the drawing */
        SDL_GL_SwapWindow(displayWindow);
    }
    
    SDL_Quit();
    return 0;
}



void programSetUp(ShaderProgram& textured, ShaderProgram& untextured){
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
    
#ifdef _WINDOWS
    glewInit();
#endif
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    untextured.Load(RESOURCE_FOLDER"vertex.glsl", RESOURCE_FOLDER"fragment.glsl");
    textured.Load(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glm::mat4 viewMatrix = glm::mat4(1.0f);
    glm::mat4 projectionMatrix = glm::ortho(-2.0f, 2.0f, -1.5f, 1.5f, -1.0f, 1.0f);
    textured.SetViewMatrix(viewMatrix);
    textured.SetProjectionMatrix(projectionMatrix);
    untextured.SetViewMatrix(viewMatrix);
    untextured.SetProjectionMatrix(projectionMatrix);
    //    glClearColor(254.0f/255.0f, 223.0f/255.0f, 225.0f/255.0f, 1.0f);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glUseProgram(textured.programID);
    glUseProgram(untextured.programID);
}


void ProcessEvents(bool& done, SDL_Event& event){
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
            done = true;
        }
    }
}

GLuint LoadTexture(const char* filePath) {
    int w, h, comp;
    unsigned char* image = stbi_load(filePath, &w, &h, &comp, STBI_rgb_alpha);
    
    if (image == NULL) {
        std::cout << "Unable to load image. Make sure the path is correct\n";
        assert(false);
    }
    
    GLuint retTexture;
    glGenTextures(1, &retTexture);
    glBindTexture(GL_TEXTURE_2D, retTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    stbi_image_free(image);
    return retTexture;
}

void ProcessInput(EntityTextured& player, float& lastFrameTicks,
                  bool& done, SDL_Event& event,
                  std::vector<EntityTextured>& playerBullets,
                  const SheetSprite& enemyBullet)
{
    ProcessEvents(done, event);
    float ticks = (float)SDL_GetTicks() / 1000.0f;
    float elapsed = ticks - lastFrameTicks;
    const Uint8 *keys = SDL_GetKeyboardState(NULL);
    switch (gameState) {
        case STATE_MAIN_MENU:
            if(keys[SDL_SCANCODE_S])
                gameState = STATE_GAME_START;
            break;
        case STATE_GAME_START:
            if(keys[SDL_SCANCODE_LEFT]){
                if(player.position.x > -1.8f)
                    player.position.x -= player.velocity.x * elapsed;
            }else if (keys[SDL_SCANCODE_RIGHT]){
                if(player.position.x < 1.8f)
                    player.position.x += player.velocity.x * elapsed;
            }
            /* Player Shooting! */
            if(keys[SDL_SCANCODE_SPACE]){
                if(playerBullets.size() == 0)
                {player.shootBullet(playerBullets, enemyBullet, 2.0f);}
            }
            break;
        case STATE_GAME_OVER:
            done = true;
            break;
    }
}

void
updateGame(float& lastFrameTicks, int& lastBulletTick, bool& shootAble, int& lastEnemyMove,
           EntityTextured& player, const SheetSprite& enemyBullet,
           std::vector<EntityTextured>& playerBullets,
           std::vector<EntityTextured>& enemyBullets,
           std::vector<EntityTextured>& enemies)
{
    float ticks = (float)SDL_GetTicks() / 1000.0f;
    float elapsed = ticks - lastFrameTicks;
    lastFrameTicks = ticks;
    float minXPosition = 1.7f;
    float maxXPosition = -1.7f;
    
    
    if((int)ticks % 1 == 0 && (int)ticks != lastBulletTick){
        lastBulletTick = ticks;
        shootAble = true;
    }else{
        shootAble = false;
    }
    
    
    if((int)ticks % 7 == 0 && (int)ticks != (int)lastEnemyMove){
        lastEnemyMove = ticks;
        for(auto& ufo : enemies){
            ufo.position.y -= 0.1f;
        }
    }
    
    for(auto& bullet : playerBullets)
        bullet.position.y += bullet.velocity.y * elapsed;
    for(auto& bullet : enemyBullets)
        bullet.position.y += bullet.velocity.y * elapsed;
    
    /* check if the enemies touch the bound */
    for(const auto& ufo: enemies){
        if(ufo.position.x < minXPosition){ minXPosition = ufo.position.x; }
        if(ufo.position.x > maxXPosition){ maxXPosition = ufo.position.x; }
    }
    
    /* switch the velocity */
    if(maxXPosition > 1.7f || minXPosition < -1.7f){
        for(auto& ufo: enemies){
            if(maxXPosition > 1.7f)
                ufo.position.x -= 0.01f;
            else
                ufo.position.x += 0.01f;
            ufo.velocity.x *= -1;
        }
        maxXPosition = -1.7f;
        minXPosition = 1.7f;
    }
    
    /* Draw the enemies */
    for(auto& ufo : enemies){
        //                    ufo.Draw(textured);
        ufo.position.x += ufo.velocity.x * elapsed;
    }
    
    for(auto& bullet : playerBullets){
        for(int j = 0; j < enemies.size(); j++){
            auto target = enemies[j];
            float distanceX = abs(bullet.position.x - target.position.x) - (bullet.size.x * bullet.sprite.size + target.size.x * target.sprite.size)/2;
            float distanceY = abs(bullet.position.y - target.position.y) - (bullet.size.y * bullet.sprite.size + target.size.y * target.sprite.size)/2;
            if(distanceX < -0.08f && distanceY < -0.05f){
                playerBullets.pop_back();
                enemies.erase(enemies.begin() + j);
                break;
            }
        }
    }
    /* Check if a bullet hits a player */
    for(int i = 0; i < enemyBullets.size(); ++i){
        auto bullet = enemyBullets[i];
        if(player.health > 0){
            float distanceX = abs(bullet.position.x - player.position.x) - (bullet.size.x * bullet.sprite.size + player.size.x * player.sprite.size)/2;
            float distanceY = abs(bullet.position.y - player.position.y) - (bullet.size.y * bullet.sprite.size + player.size.y * player.sprite.size)/2;
            if(distanceX < -0.05f && distanceY < -0.05f){
                player.health -= 1;
                enemyBullets.erase(enemyBullets.begin() + i);
            }
        }else{
            gameState = STATE_GAME_OVER;
        }
    }
    /* check if enemy hits a player */
    for(auto& ufo : enemies){
        float distanceX = abs(ufo.position.x - player.position.x) - (ufo.size.x * ufo.sprite.size + player.size.x * player.sprite.size)/2;
        float distanceY = abs(ufo.position.y - player.position.y) - (ufo.size.y * ufo.sprite.size + player.size.y * player.sprite.size)/2;
        if(distanceX < -0.1f && distanceY < -0.1f){
            gameState = STATE_GAME_OVER;
        }
    }
    
    if(shootAble && enemies.size() != 0){
        int index = rand() % enemies.size();
        enemies[index].shootBullet(enemyBullets, enemyBullet, -1.0f);
    }
    enemyBullets.erase(std::remove_if(enemyBullets.begin(), enemyBullets.end(), shouldRemoveBullet), enemyBullets.end());
    playerBullets.erase(std::remove_if(playerBullets.begin(), playerBullets.end(), shouldRemoveBullet), playerBullets.end());
}

void
renderMainMenu(ShaderProgram& program, GLuint fontSheet,
               EntityFont& spaceInvader,EntityFont& title)
{
    spaceInvader.Draw(program, fontSheet, 0.01f);
    title.Draw(program, fontSheet, 0.01f);
}

void
renderGame (ShaderProgram& textured, const EntityTextured& player,
            const std::vector<EntityTextured>& enemies,
            const std::vector<EntityTextured>& playerBullets,
            const std::vector<EntityTextured>& enemyBullets)
{
    player.Draw(textured);
    for(auto& ufo : enemies)
        ufo.Draw(textured);
    for(const auto& bullet : playerBullets)
        bullet.Draw(textured);
    for(const auto& bullet : enemyBullets)
        bullet.Draw(textured);
}

bool shouldRemoveBullet(Entity bullet) {
    if(bullet.position.y > 1.5f || bullet.position.y < -1.5f) {
        return true;
    } else {
        return false;
    }
}
