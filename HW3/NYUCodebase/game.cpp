#include "Necessity.hpp"
#include "SheetSprite.hpp"
#include "Entity.hpp"

SDL_Window* displayWindow;

enum GameMode{ STATE_MAIN_MENU, STATE_GAME_START, STATE_GAME_OVER };

void programSetUp(ShaderProgram& textured, ShaderProgram& untextured);
void ProcessEvents(bool& done, SDL_Event& event);
GLuint LoadTexture(const char* filePath);
bool shouldRemoveBullet(Entity bullet);


struct GameState{
    
    GameState():
    mainMenu(-1.0f, -0.1f, 0.3f, 0.3f, "(Press S to start...)"),
    title(-1.5f, 0.2f, 0.5f, 0.5f, "Space Invader"),
    scoreFont(-1.8f, 1.4f, 0.3f, 0.3f, "Score: 0"),
    healthFont(-1.8f, -1.4f, 0.3f, 0.3f, "Health: 3"),
    ufoSprite(LoadTexture(RESOURCE_FOLDER"characters_1.png"), 0, 4/8.0f, 1/12.0f, 1/8.0f, 0.3),
    enemyBullet(LoadTexture(RESOURCE_FOLDER"laserBlue03.png"), 0, 0, 1.0f, 1.0f, 0.3),
    playerBullet(LoadTexture(RESOURCE_FOLDER"laserGreen03.png"), 0, 0, 1.0f, 1.0f, 0.3),
    playerSprite(LoadTexture(RESOURCE_FOLDER"characters_1.png"), 1/12.0f, 3/8.0f, 1/12.0f, 1/8.0f, 0.3),
    gameMode(STATE_MAIN_MENU),
    lastFrameTicks(0.0f),
    lastBulletTick(0),
    shootAble(true),
    score(0),
    done(false)
    {
        player = EntityTextured(0, -1.2f, 1.0f, 1.0f, playerSprite);
        player.health = 3;
        player.velocity.x = 0.6f;
        float horizontalSpacing = 0.3f;
        float verticalSpacing = 0.2f;
        for (int i = 0; i < 5; ++i) {
            for (int j = 0; j < 11; j++) {
                EntityTextured ufo(-1.7f + horizontalSpacing * j, 1.2f - verticalSpacing * i, 0.85f ,0.85f, ufoSprite);
                ufo.velocity.x = 0.1f;
                enemies.push_back(ufo);
            }
        }
    }
    
    void renderMainMenu(){
        GLuint fontSheet = LoadTexture(RESOURCE_FOLDER"pixel_font.png");
        mainMenu.Draw(program, fontSheet, 0.01f);
        title.Draw(program, fontSheet, 0.01f);
    }
    
    void renderGameLevel(){
        player.Draw(program);
        for(auto& ufo : enemies)
            ufo.Draw(program);
        for(const auto& bullet : playerBullets)
            bullet.Draw(program);
        for(const auto& bullet : enemyBullets)
            bullet.Draw(program);
        GLuint fontSheet = LoadTexture(RESOURCE_FOLDER"pixel_font.png");
        scoreFont.text = "Score: " + std::to_string(score);
        healthFont.text = "Health: " + std::to_string(player.health);
        scoreFont.Draw(program, fontSheet, 0.01f);
        healthFont.Draw(program, fontSheet, 0.01f);
    }
    
    void renderGameOver(){
        EntityFont finalScore(-1.1f, -0.5f, 0.3f, 0.3f, scoreFont.text);
        finalScore.Draw(program, LoadTexture(RESOURCE_FOLDER"pixel_font.png"), 0.01f);
        std::string displayText = (enemies.size() == 0) ? "WIN!" : "LOSE";
        EntityFont result(-0.9f, 0.0f, 0.8f, 0.8f, displayText);
        result.Draw(program, LoadTexture(RESOURCE_FOLDER"pixel_font.png"), 0.01f);
        EntityFont restart = mainMenu;
        restart.position.x = -1.1f;
        restart.position.y = -0.8f;
        restart.Draw(program, LoadTexture(RESOURCE_FOLDER"pixel_font.png"), 0.01f);
    }
    
    void render(){
        switch(gameMode){
            case STATE_MAIN_MENU:
                renderMainMenu();
                break;
            case STATE_GAME_START:
                renderGameLevel();
                break;
            case STATE_GAME_OVER:
                renderGameOver();
                break;
        }
    }
    
    void updateMainMenu(){
        float ticks = (float)SDL_GetTicks() / 1000.0f;
        lastFrameTicks = ticks;
    }
    
    void updateGameOver(){}
    
    void updateGameLevel(){
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
        
        
        for(auto& bullet : playerBullets)
            bullet.position.y += bullet.velocity.y * elapsed;
        for(auto& bullet : enemyBullets)
            bullet.position.y += bullet.velocity.y * elapsed;
        
        /* check if the enemies touch the bound */
        for(const auto& ufo: enemies){
            if(ufo.position.x < minXPosition){ minXPosition = ufo.position.x; }
            if(ufo.position.x > maxXPosition){ maxXPosition = ufo.position.x; }
        }
        
        /* switch the velocity and move the ufo down a level*/
        if(maxXPosition > 1.7f || minXPosition < -1.7f){
            for(auto& ufo: enemies){
                if(maxXPosition > 1.7f)
                    ufo.position.x -= 0.01f;
                else
                    ufo.position.x += 0.01f;
                ufo.velocity.x *= -1;
                ufo.position.y -= 0.1f;
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
                    score += 10;
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
                gameMode = STATE_GAME_OVER;
            }
        }
        /* check if enemy hits a player */
        for(auto& ufo : enemies){
            float distanceX = abs(ufo.position.x - player.position.x) - (ufo.size.x * ufo.sprite.size + player.size.x * player.sprite.size)/2;
            float distanceY = abs(ufo.position.y - player.position.y) - (ufo.size.y * ufo.sprite.size + player.size.y * player.sprite.size)/2;
            if(distanceX < -0.1f && distanceY < -0.1f){
                gameMode = STATE_GAME_OVER;
            }
        }
        
        if(shootAble && enemies.size() != 0){
            int index = rand() % enemies.size();
            enemies[index].shootBullet(enemyBullets, enemyBullet, -1.0f);
        }
        if(enemies.size() == 0){ gameMode = STATE_GAME_OVER; }
        enemyBullets.erase(std::remove_if(enemyBullets.begin(), enemyBullets.end(), shouldRemoveBullet), enemyBullets.end());
        playerBullets.erase(std::remove_if(playerBullets.begin(), playerBullets.end(), shouldRemoveBullet), playerBullets.end());
    }
    
    void update(){
        switch(gameMode){
            case STATE_MAIN_MENU:
                updateMainMenu();
                break;
            case STATE_GAME_START:
                updateGameLevel();
                break;
            case STATE_GAME_OVER:
                updateGameOver();
                break;
        }
    }
    
    void processInputMainMenu(){
        ProcessEvents(done, event);
        const Uint8 *keys = SDL_GetKeyboardState(NULL);
        if(keys[SDL_SCANCODE_S])
            gameMode = STATE_GAME_START;
    }
    
    void processInputGameLevel(){
        ProcessEvents(done, event);
        float ticks = (float)SDL_GetTicks() / 1000.0f;
        float elapsed = ticks - lastFrameTicks;
        const Uint8 *keys = SDL_GetKeyboardState(NULL);
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
            {player.shootBullet(playerBullets, playerBullet, 2.5f);}
        }
    }
    
    void reset(){
        mainMenu = EntityFont(-1.0f, -0.1f, 0.3f, 0.3f, "(Press S to start...)");
        title = EntityFont(-1.5f, 0.2f, 0.5f, 0.5f, "Space Invader");
        scoreFont = EntityFont(-1.8f, 1.4f, 0.3f, 0.3f, "Score: 0");
        healthFont = EntityFont(-1.8f, -1.4f, 0.3f, 0.3f, "Health: 3");
        ufoSprite = SheetSprite(LoadTexture(RESOURCE_FOLDER"characters_1.png"), 0, 4/8.0f, 1/12.0f, 1/8.0f, 0.3);
        enemyBullet = SheetSprite(LoadTexture(RESOURCE_FOLDER"laserBlue03.png"), 0, 0, 1.0f, 1.0f, 0.3);
        playerBullet = SheetSprite(LoadTexture(RESOURCE_FOLDER"laserGreen03.png"), 0, 0, 1.0f, 1.0f, 0.3);
        playerSprite = SheetSprite(LoadTexture(RESOURCE_FOLDER"characters_1.png"), 1/12.0f, 3/8.0f, 1/12.0f, 1/8.0f, 0.3);
        gameMode = STATE_MAIN_MENU;
        lastFrameTicks = 0.0f;
        lastBulletTick = 0;
        shootAble = true;
        score = 0;
        player = EntityTextured(0, -1.2f, 1.0f, 1.0f, playerSprite);
        player.health = 3;
        player.velocity.x = 0.6f;
        enemies.clear();
        enemyBullets.clear();
        playerBullets.clear();
        float horizontalSpacing = 0.3f;
        float verticalSpacing = 0.2f;
        for (int i = 0; i < 5; ++i) {
            for (int j = 0; j < 11; j++) {
                EntityTextured ufo(-1.7f + horizontalSpacing * j, 1.2f - verticalSpacing * i, 0.85f ,0.85f, ufoSprite);
                ufo.velocity.x = 0.1f;
                enemies.push_back(ufo);
            }
        }
    }
    
    void processInputGameOver(){
        ProcessEvents(done, event);
        float ticks = (float)SDL_GetTicks() / 1000.0f;
        lastFrameTicks = ticks;
        const Uint8 *keys = SDL_GetKeyboardState(NULL);
        if(keys[SDL_SCANCODE_S]){
            reset();
        }
    };
    
    void processInput(){
        switch (gameMode) {
            case STATE_MAIN_MENU:
                processInputMainMenu();
                break;
            case STATE_GAME_START:
                processInputGameLevel();
                break;
            case STATE_GAME_OVER:
                processInputGameOver();
                break;
        }
    }
    ShaderProgram program;
    GameMode gameMode;
    SDL_Event event;
    EntityTextured player;
    EntityFont mainMenu;
    EntityFont title;
    EntityFont scoreFont;
    EntityFont healthFont;
    SheetSprite ufoSprite;
    SheetSprite enemyBullet;
    SheetSprite playerBullet;
    SheetSprite playerSprite;
    std::vector<EntityTextured> enemyBullets;
    std::vector<EntityTextured> playerBullets;
    std::vector<EntityTextured> enemies;
    float lastFrameTicks;
    int lastBulletTick;
    int score;
    bool shootAble;
    bool done;
};


int main()
{
    /* setup */
    ShaderProgram textured, untextured;
    programSetUp(textured, untextured);
    
    srand(static_cast<unsigned int>(time(NULL)));

    GameState gameState;
    gameState.program = textured;
    
    while (!gameState.done) {
        
        glClear(GL_COLOR_BUFFER_BIT);
        /* Here starts the drawing */
        gameState.processInput();
        gameState.render();
        gameState.update();
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

bool shouldRemoveBullet(Entity bullet) {
    if(bullet.position.y > 1.5f || bullet.position.y < -1.5f) {
        return true;
    } else {
        return false;
    }
}
