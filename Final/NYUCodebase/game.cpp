#include "Necessity.hpp"
#include "parse_tile_map.hpp"
#include <SDL2_mixer/SDL_mixer.h>


SDL_Window* displayWindow;

enum GameMode{ STATE_MAIN_MENU, STATE_GAME_START, STATE_GAME_OVER };
void programSetUp(ShaderProgram& textured, ShaderProgram& untextured);
void ProcessEvents(bool& done, SDL_Event& event);
GLuint LoadTexture(const char* filePath);


#define FIXED_TIMESTEP 0.008f
#define MAX_TIMESTEPS 6
#define PI 3.14
float lerp(float v0, float v1, float t) {
    return (1.0-t)*v0 + t*v1;
}


/* Statics */
vector<int> statics = {1,2,3,4,5,17,18,19,20,33,34,35,36};

/* Game Music Assets */
struct Sound{
    
    Mix_Chunk *critical = Mix_LoadWAV(RESOURCE_FOLDER"critical.wav");
    Mix_Chunk *gold = Mix_LoadWAV(RESOURCE_FOLDER"gold.wav");
    Mix_Chunk *ascend = Mix_LoadWAV(RESOURCE_FOLDER"ascend.wav");
    
    Mix_Chunk *hit = Mix_LoadWAV(RESOURCE_FOLDER"hit.wav");
    Mix_Chunk *broke = Mix_LoadWAV(RESOURCE_FOLDER"break.wav");
    
    Mix_Music *bg = Mix_LoadMUS(RESOURCE_FOLDER"bg.mp3");
    bool bgFlag = false;
    void playMusic(Mix_Chunk* chunk, int loop){
        Mix_PlayChannel(-1, chunk, loop);
    }
    void playBackground(){
        if(!bgFlag){
            bgFlag = true;
            Mix_PlayMusic(bg, -1);
        }
    }
    
    ~Sound(){
        Mix_FreeChunk(critical);
        Mix_FreeChunk(gold);
        Mix_FreeChunk(ascend);
        Mix_FreeChunk(hit);
        Mix_FreeChunk(broke);
        Mix_FreeMusic(bg);
    }
};

/* Entity: player / enemy / etc */
struct Entity{
    
    float startX;
    float endX;
    
    Entity(){};
    int sprite_count_x = SPRITE_COUNT_X;
    int sprite_count_y = SPRITE_COUNT_Y;
    Entity(int index, float x, float y, float width, float height):index(index), position(x,y,0),size(width,height,0){}
    
    void Draw(ShaderProgram& textured, GLuint spriteSheetID) const {
        
        glm::mat4 modelMatrix(1.0f);
        modelMatrix = glm::translate(modelMatrix, position);
        modelMatrix = glm::scale(modelMatrix, size);
        textured.SetModelMatrix(modelMatrix);
        
        
        float u = (float)(((int)index) % sprite_count_x) / (float) sprite_count_x;
        float v = (float)(((int)index) / sprite_count_x) / (float) sprite_count_y;
        float spriteWidth = 1.0/(float)sprite_count_x;
        float spriteHeight = 1.0/(float)sprite_count_y;
        float texCoords[] = {
            u, v+spriteHeight,
            u+spriteWidth, v,
            u, v,
            u+spriteWidth, v,
            u, v+spriteHeight,
            u+spriteWidth, v+spriteHeight
        };
        float vertices[] = {-0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f, 0.5f,  -0.5f,
            -0.5f, 0.5f, -0.5f};
        // draw this data
        glBindTexture(GL_TEXTURE_2D, spriteSheetID);
        glVertexAttribPointer(textured.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
        glEnableVertexAttribArray(textured.positionAttribute);
        glVertexAttribPointer(textured.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
        glEnableVertexAttribArray(textured.texCoordAttribute);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDisableVertexAttribArray(textured.positionAttribute);
        glDisableVertexAttribArray(textured.texCoordAttribute);
    }
    
    void updateX(float elapsed, float acceleration_x){
        
        collideXLeft();
        colllideXRight();
        velocity.x = lerp(velocity.x, 0.0f, elapsed * 3.0f);
        velocity.x += acceleration_x * elapsed;
        if( abs(velocity.x) < 0.01f){
            velocity.x = 0.0f;
        }
        if(velocity.x > 3.0f){
            velocity.x = 3.0f;
        }else if(velocity.x < -3.0f){
            velocity.x = -3.0f;
        }
        position.x += velocity.x * elapsed;
    }
    
    void collideXLeft(){
        int x = (int)((position.x - size.x/2) / TILE_SIZE);
        int y = (int)((position.y) / -TILE_SIZE);
        if(y >= 0){
            if(find(statics.begin(),statics.end(),(int)levelData[y][x]) != statics.end()){
                collidedLeft = true;
                velocity.x = 0.0f;
                position.x -= (position.x - size.x/2) - ((TILE_SIZE * x) + TILE_SIZE);
            }
        }

    }
    
    void colllideXRight(){
        int x = (int)((position.x + size.x/2) / TILE_SIZE);
        int y = (int)((position.y) / -TILE_SIZE);
        if( y >= 0){
            if(find(statics.begin(),statics.end(),(int)levelData[y][x]) != statics.end()){
                collidedRight = true;
                velocity.x = 0.0f;
                position.x -= (position.x + size.x/2) - (TILE_SIZE * x);
            }
        }

    }
    
    void collideYTop(){
        int x = (int)(position.x / TILE_SIZE);
        int y = (int)((position.y + size.y/2) / -TILE_SIZE);
        
        if(y >= 0){
            if(find(statics.begin(),statics.end(),(int)levelData[y][x]) != statics.end()){
                collidedTop = true;
                velocity.y = 0.0f;
                position.y -= (position.y + size.y/2) - (-TILE_SIZE * y) + TILE_SIZE;
            }
        }

    }
    
    void collideYBottom(bool isPlayer){
        int x = (int)(position.x / TILE_SIZE);
        int y = (int)((position.y - size.y/2) / -TILE_SIZE);

        if(y >= 0){
            if(find(statics.begin(),statics.end(),(int)levelData[y][x]) != statics.end()){
                collidedBottom = true;
                velocity.y = isPlayer ? 0.0f : velocity.y;
                position.y -= (position.y - size.y/2) - (-TILE_SIZE * y);
            }
        }

    }
    
    void updateY(float elapsed, bool isPlayer){
        float gravity = -3.0f;
        collideYTop();
        collideYBottom(isPlayer);
        if(collidedBottom && isPlayer){
            jump = false;
        }else{
            velocity.y += gravity * elapsed;
            if(abs(velocity.y) < 0.02f ){
                velocity.y = 0.0f;
            }
            if(velocity.y > 2.3f){
                velocity.y = 2.3f;
            }else if(velocity.y < -2.3f){
                velocity.y = -2.3f;
            }
            position.y += velocity.y * elapsed;
        }
        
    }
    
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 size;
    
    int index = 0;
    bool criticalFlag = false;
    bool jump = false;
    bool collidedTop = false;
    bool collidedBottom = false;
    bool collidedLeft = false;
    bool collidedRight = false;
};


/* Particle systems */
struct Particle{
    Particle(float x, float y):position(x, y, 0.0f), velocity(-0.2f, -0.2f, 0.0f), size(0.2f, 0.2f, 0.0f){}
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 size;
    float lifetime;
};

struct ParticleEmitter {
    
    bool triggered = true;
    
    /* sparse means should I randomly generate (aka, not sparse means a straight line) */
    ParticleEmitter(unsigned int particleCount, float x, float y, bool sparse, float particleSize = 0.2f, float maxLifeTime = 1.0f):
    position(x, y, 0.0f), gravity(-1.0f), maxLifetime(maxLifeTime), sparse(sparse)
    {
        startColor = glm::vec4(241.0f/255.0f, 196.0f/255.0f, 205.0f/255.0f, 1.0f);
        endColor = glm::vec4(48.0f/255.0f, 22.0f/255.0f, 28.0f/255.0f, 1.0f);
        for(int i=0; i < particleCount; i++){
            
            Particle p(position.x, position.y);
            p.size.x = particleSize;
            p.size.y = particleSize;
            p.lifetime = ((float)rand()/(float)RAND_MAX) * maxLifetime;
            
            if(!sparse){
                p.velocity.x = 0.0f;
                p.velocity.y = 0.0f;
            }
            
            if(sparse){
                p.size.x *= ((float)rand()/(float)RAND_MAX);
                p.size.y *= ((float)rand()/(float)RAND_MAX);
                p.velocity.x *= ((float)rand()/(float)RAND_MAX) * ((i%2 == 0) ? 1 : -1);
                p.velocity.y *= ((float)rand()/(float)RAND_MAX);
            }
            
            particles.push_back(p);
        }
    }
    ParticleEmitter(){};
    ~ParticleEmitter(){};
    
    void Update(float elapsed){
        for(int i = 0; i<particles.size(); i++){
            particles[i].position.x += particles[i].velocity.x * elapsed;
            particles[i].position.y += particles[i].velocity.y * elapsed;
            particles[i].velocity.y += gravity.y * elapsed;
            particles[i].lifetime += elapsed;
            if(particles[i].lifetime >= maxLifetime){
                particles[i].lifetime = 0.0f;
                particles[i].position.x = position.x;
                particles[i].position.y = position.y;
                particles[i].velocity.y = 0.2f;
                particles[i].velocity.x = 0.2f;
                if(!sparse){
                    particles[i].velocity.x = 0.0f;
                    particles[i].velocity.y = 0.0f;
                }
                if(sparse){
                    particles[i].velocity.x *= ((float)rand()/(float)RAND_MAX) * ((i%2 == 0) ? 1 : -1);
                    particles[i].velocity.y *= ((float)rand()/(float)RAND_MAX);
                }
                
            }
        }
    }
    
    /* Color doesn't work, so I give up. */
    void Render(ShaderProgram& program){
        if(triggered){
            glm::mat4 modelMatrix(1.0f);
            program.SetModelMatrix(modelMatrix);
            vector<float> vertices;
            vector<float> colors;
            for(int i=0; i < particles.size(); i++) {
                vertices.push_back(particles[i].position.x);
                vertices.push_back(particles[i].position.y);
            }
            for(int i=0; i < particles.size(); i++) {
                float m = (particles[i].lifetime/maxLifetime);
                colors.push_back(lerp(startColor.r, endColor.r, m));
                colors.push_back(lerp(startColor.g, endColor.g, m));
                colors.push_back(lerp(startColor.b, endColor.b, m));
                colors.push_back(lerp(startColor.a, endColor.a, m));
            }
            GLuint colorAttribute = glGetAttribLocation(program.programID, "lerpColor");
            glVertexAttribPointer(colorAttribute, 4, GL_FLOAT, false, 0, colors.data());
            glEnableVertexAttribArray(colorAttribute);
            
            glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices.data());
            glEnableVertexAttribArray(program.positionAttribute);
            glDrawArrays(GL_POINTS, 0, particles.size());
        }
    }
    
    glm::vec3 position;
    glm::vec3 gravity;
    float maxLifetime;
    std::vector<Particle> particles;
    glm::vec4 startColor;
    glm::vec4 endColor;
    bool sparse;
};

struct GameState{
    /* member fields */
    SDL_Event event;
    ShaderProgram textured;
    ShaderProgram untextured;
    
    GameMode gameMode;
    
    Entity playerEmitter;
    Entity player;
    Entity goldTexture;
    Entity key;
    vector<Entity> enemies;
    
    ParticleEmitter emit;
    ParticleEmitter mainMenu1, mainMenu2;
    
    GLuint spriteSheetID;
    GLuint fontSheetID;
    
    bool done = false;
    bool lose = false;
    bool shrinking = false;
    float accumulator = 0.0f;
    float elapsed = 0.0f;
    int collectGold = 0;
    int remaining = 45;
    
    Sound sound;

    GameState():
    spriteSheetID(LoadTexture(RESOURCE_FOLDER"arne_sprites.png")),
    fontSheetID(LoadTexture(RESOURCE_FOLDER"pixel_font.png")),
    gameMode(STATE_MAIN_MENU),
    player(PLAYER+1,objects[2].x, objects[2].y, TILE_SIZE, TILE_SIZE),
    mainMenu1(150, 0.5f, -2.7f, true),
    mainMenu2(150, 3.5f, -2.7f, true)
    {
        
        mainMenu2.gravity.y = 1.0f;
        for(int i = 0; i < objects.size() - 1; i++){
            enemies.push_back(Entity(ENEMY, objects[i].x, objects[i].y, TILE_SIZE, TILE_SIZE));
            enemies[i].velocity.x = 0.5f;
        }
        enemies[0].startX = enemies[0].position.x - 0.1f;
        enemies[0].endX = enemies[0].position.x + 0.9f;
        enemies[1].position.y += 0.5f;
        enemies[1].startX = enemies[1].position.x - 0.5f;
        enemies[1].endX = enemies[1].position.x + 0.3f;
    }
    
    void drawTileMap(){
        std::vector<float> vertexData;
        std::vector<float> texCoordData;
        glm::mat4 modelMatrix(1.0f);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f,0.0f,0.0f));
        textured.SetModelMatrix(modelMatrix);
        int count = 0;
        for(int y=0; y < LEVEL_HEIGHT; y++) {
            for(int x=0; x < LEVEL_WIDTH; x++) {
                if((int)levelData[y][x] == 0){ continue; }
                
                count++;
                float u = (float)(((int)levelData[y][x] - 1) % SPRITE_COUNT_X) / (float) SPRITE_COUNT_X;
                float v = (float)(((int)levelData[y][x] - 1) / SPRITE_COUNT_X) / (float) SPRITE_COUNT_Y;
                if(levelData[y][x] == 49){
                    levelData[y][x] = 50;
                }else if(levelData[y][x] == 50){
                    levelData[y][x] = 51;
                }else if(levelData[y][x] == 51){
                    levelData[y][x] = 0;
                }
                
                if(levelData[y][x] == 5 && remaining == 0){
                    levelData[y][x] = 0;
                }
                
                
                float spriteWidth = 1.0f/(float)SPRITE_COUNT_X;
                float spriteHeight = 1.0f/(float)SPRITE_COUNT_Y;
                vertexData.insert(vertexData.end(), {
                    TILE_SIZE * x, -TILE_SIZE * y,
                    TILE_SIZE * x, (-TILE_SIZE * y)-TILE_SIZE,
                    (TILE_SIZE * x)+TILE_SIZE, (-TILE_SIZE * y)-TILE_SIZE,
                    TILE_SIZE * x, -TILE_SIZE * y,
                    (TILE_SIZE * x)+TILE_SIZE, (-TILE_SIZE * y)-TILE_SIZE,
                    (TILE_SIZE * x)+TILE_SIZE, -TILE_SIZE * y
                });
                texCoordData.insert(texCoordData.end(), {
                    u, v,
                    u, v+(spriteHeight),
                    u+spriteWidth, v+(spriteHeight),
                    u, v,
                    u+spriteWidth, v+(spriteHeight),
                    u+spriteWidth, v
                });
            }
        }
        glBindTexture(GL_TEXTURE_2D, spriteSheetID);
        glVertexAttribPointer(textured.positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
        glEnableVertexAttribArray(textured.positionAttribute);
        glVertexAttribPointer(textured.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
        glEnableVertexAttribArray(textured.texCoordAttribute);
        glDrawArrays(GL_TRIANGLES, 0, count*6);
        glDisableVertexAttribArray(textured.positionAttribute);
        glDisableVertexAttribArray(textured.texCoordAttribute);
    }
    
    
    void drawText(const string& text, float x, float y){
        float space = 0.1f;
        int count = 0;
        for(const auto& c : text){
            Entity chara((int) c, x + space * (count++), y, TILE_SIZE / 2, TILE_SIZE / 2);
            chara.sprite_count_y = 16;
            chara.sprite_count_x = 16;
            chara.Draw(textured, fontSheetID);
        }
    }
    
    
    /* pressed is used to tell if the user press the space or not in the mainmenu */
    bool pressed = false;
    /* startingY is for the topest text's y position */
    float startingY = -1.5f;
    void renderMainMenu(){
        sound.playBackground();
        
        drawText("Welcome", 1.7f, startingY);
        drawText("(press Space)", 1.4f, startingY - 0.2f);
        processInputMainMenu();
        
        if(pressed){
            startingY = lerp(startingY, -0.3f, 0.02f);
            drawText("Platformer: GAME OF KEYS", 0.8f, startingY - 0.6f);
            drawText("      move: left / right", 0.8f, startingY - 0.8f);
            drawText("      jump: up / fly: space", 0.8f, startingY - 1.0f);
            drawText("      quit: esc", 0.8f, startingY - 1.2f);
            drawText("     press S to start", 0.8f, startingY - 1.7f);
            drawText(" CS3113 Kaixuan Zhou", 1.0f, startingY - 1.9f);
            drawText("  Prof. Ivan Safrin", 1.0f, startingY - 2.1f);
        }
        
        mainMenu1.Render(untextured);
        mainMenu2.Render(untextured);
    }
    
    void renderGameLevel(){
        /* Camera following player */
        glm::mat4 viewMatrix(1.0f);
        if(player.position.x > 7.0f){
            viewMatrix = glm::translate(viewMatrix, glm::vec3(max(-player.position.x,-LEVEL_WIDTH*TILE_SIZE + 2.0f), min(-player.position.y,1.5f), 0.0f));
        }else{
            viewMatrix = glm::translate(viewMatrix, glm::vec3(min(-player.position.x,-2.0f), min(-player.position.y,1.5f), 0.0f));
        }
        textured.SetViewMatrix(viewMatrix);
        untextured.SetViewMatrix(viewMatrix);
        
        /* tile map */
        drawTileMap();
        
        /* player and the #gold on player's head */
        player.Draw(textured,spriteSheetID);
        goldTexture = Entity(GOLD+1, player.position.x - 0.08f, player.position.y + 0.17f, TILE_SIZE/1.5, TILE_SIZE/1.5);
        goldTexture.Draw(textured, spriteSheetID);
        Entity playerHead(83, player.position.x, player.position.y + player.size.y, TILE_SIZE, TILE_SIZE);
        playerHead.Draw(textured, spriteSheetID);
        
        
        /* can player fly? */
        if(flyingCount == 0){
            playerEmitter.Draw(textured, spriteSheetID);
        }
        
        key.Draw(textured, spriteSheetID);
        
        /* number of collected golds */
        float space = 0.05f;
        int count = 0;
        for(const auto& c : to_string(collectGold)){
            Entity score((int) c, goldTexture.position.x + 0.07f + space * (count++), goldTexture.position.y, TILE_SIZE / 3, TILE_SIZE / 3);
            score.sprite_count_y = 16;
            score.sprite_count_x = 16;
            score.Draw(textured, fontSheetID);
        }
        
        /* particle systems */
        emit.Render(untextured);
        
        
        // enemies
        for(const auto& e : enemies){
            e.Draw(textured, spriteSheetID);
        }
        
    }
    
    void renderGameOver(){
        if(!lose){
            key.Draw(textured, spriteSheetID);
            if(key.size.x >= 2.5f){
                drawText("You get the key!", key.position.x - key.size.x/2 + TILE_SIZE, key.position.y + key.size.y/2);
                drawText(string("Gold: ") + to_string(collectGold), key.position.x - key.size.x/2 + TILE_SIZE, key.position.y + key.size.y/2 - TILE_SIZE);
                drawText("Press r to continue!", key.position.x - key.size.x/2 + TILE_SIZE, key.position.y + key.size.y/2 - TILE_SIZE - TILE_SIZE);
            }
            if(currentLevel == 4){
                drawText(string("You are done!") + to_string(collectGold), key.position.x - key.size.x/2 + TILE_SIZE, key.position.y + key.size.y/2 - TILE_SIZE - TILE_SIZE);
            }
        }else{
            drawText("What? Die already?", player.position.x - player.size.x/2 - 3*TILE_SIZE, player.position.y + player.size.y + player.size.y);
            drawText("Press r to continue!", player.position.x - player.size.x/2 - 3*TILE_SIZE, player.position.y + player.size.y + player.size.y - TILE_SIZE);
            drawText("Press esc to quit:(", player.position.x - player.size.x/2 - 3*TILE_SIZE, player.position.y + player.size.y + player.size.y - TILE_SIZE - TILE_SIZE);
        }
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
    
    void updateMainMenu(float elapsed){
        mainMenu1.position.y += elapsed * 0.5f;
        if(mainMenu1.position.y > 0.3f){
            mainMenu1.position.y = -2.7f;
        }
        mainMenu1.Update(elapsed);
        mainMenu2.position.y -= elapsed * 0.5f;
        if(mainMenu2.position.y < -3.0f){
            mainMenu2.position.y = 0.3f;
        }
        mainMenu2.Update(elapsed);
    }
    
    void updateGameOver(float elapsed){
        if(!lose){
            if(key.size.x < 2.5f){
                key.size.x += 0.02f;
                key.size.y += 0.02f;
            }
        }
        processInputGameOver();
    }
    
    void updateGameLevel(float elapsed){
        /* turns out that this looks good for particle systems */
        playerEmitter.position.x = player.position.x - player.size.x/2;
        if(speedUpCount == 0){
            playerEmitter.position.y = player.position.y + player.size.y/2;
        }else{
            playerEmitter.position.y = player.position.y;
        }
        emit.position.x = playerEmitter.position.x - 0.02f;
        emit.position.y = playerEmitter.position.y - playerEmitter.size.y/2;
        emit.Update(elapsed);
        
        /* init every collided to false */
        player.collidedBottom = false;
        player.collidedTop = false;
        player.collidedLeft = false;
        player.collidedRight = false;
        for(auto& e : enemies){
            e.collidedBottom = false;
            e.collidedTop = false;
            e.collidedLeft = false;
            e.collidedRight = false;
        }
        processInputGameLevel(elapsed);
        
        if(speedUpCount == 0 && playerEmitter.size.y < 0.5f){
            playerEmitter.size.y += elapsed;
        }
        
        if(player.collidedTop){
            
            int x = (int)(player.position.x / TILE_SIZE);
            int y = (int)((player.position.y + player.size.y) / -TILE_SIZE);
            
            if((int)levelData[y][x] == 5){
                levelData[y][x] = 0;
                sound.playMusic(sound.hit, 0.5);
            }
        }
        
        /* Game AI */
        if(currentLevel == 3){
            for(auto& e : enemies){
                float oldVel;
                if(e.position.x > player.position.x && e.velocity.x > 0.0f){
                    e.velocity.x *= -1;
                }
                if(e.position.x < player.position.x && e.velocity.x < 0.0f){
                    e.velocity.x *= -1;
                }
                oldVel = e.velocity.x;
                e.collideXLeft();
                e.colllideXRight();
                e.collideYBottom(false);
                e.velocity.x = oldVel;
                if(e.collidedRight || e.collidedLeft || (e.position.y < player.position.y && e.collidedBottom)){
                    e.velocity.y = 1.5f;
                }
                e.updateY(elapsed,false);
                e.position.x += e.velocity.x * elapsed;
            }
        }else{
            /* for other level, it's just simple AI */
            for(auto& e : enemies){
                e.updateY(elapsed, false);
                if(e.position.x > e.endX){
                    e.velocity.x *= -1;
                    e.position.x -= 0.02f;
                }
                if(e.position.x < e.startX){
                    e.velocity.x *= -1;
                    e.position.x += 0.02f;
                }
                e.position.x += e.velocity.x * elapsed;
            }
        }

        /* if a player is hit by enemy, auto die */
        for(auto& e : enemies){
            float distanceX = abs(e.position.x - player.position.x) - (e.size.x + player.size.x)/2;
            float distanceY = abs(e.position.y - player.position.y) - (e.size.y+ player.size.y)/2;
            if(distanceX < -0.1f && distanceY < -0.1f){
                lose = true;
                gameMode = STATE_GAME_OVER;
            }
        }
    }
    
    void update(float elapsed){
        if(!shrinking && !player.jump){
            if(player.size.y < 0.22f){
                player.size.y += elapsed / 5;

            }else{
                shrinking = true;
            }
        }else{
            if(player.size.y > 0.2f){
                player.size.y -= elapsed / 5;

            }else{
                shrinking = false;
            }
        }
        
        switch(gameMode){
            case STATE_MAIN_MENU:
                updateMainMenu(elapsed);
                break;
            case STATE_GAME_START:
                updateGameLevel(elapsed);
                break;
            case STATE_GAME_OVER:
                updateGameOver(elapsed);
                break;
        }
    }
    
    void processInputMainMenu(){
        const Uint8 *keys = SDL_GetKeyboardState(NULL);
        if(keys[SDL_SCANCODE_SPACE]){
            pressed = true;
        }
        if(keys[SDL_SCANCODE_S]){
            gameMode = STATE_GAME_START;
        }
    }

    int flyingCount = 5;
    int speedUpCount = 5;
    void processInputGameLevel(float elapsed){
        
        const Uint8 *keys = SDL_GetKeyboardState(NULL);
        float acceleration_x = 0.0f;
        
        player.updateY(elapsed, true);
        if(keys[SDL_SCANCODE_UP] && !player.jump && player.collidedBottom){
            player.velocity.y = 2.3f;
            player.jump = true;
            player.position.y += 0.01f;
        }
        
        emit.triggered = false;
        
        
        if(keys[SDL_SCANCODE_RIGHT]){
            acceleration_x = 2.5f;
        }else if(keys[SDL_SCANCODE_LEFT]){
            acceleration_x = -2.5f;
        }
        /* If fly */
        if(flyingCount == 0){
            if(keys[SDL_SCANCODE_SPACE]){
                emit.triggered = true;
                if(speedUpCount == 0){
                    player.velocity.y = 1.5f;
                    acceleration_x *= 1.5f;
                    
                }else{
                    player.velocity.y = 0.5f;
                }
            }
        }


        
        player.updateX(elapsed, acceleration_x);
        int x = (int)(player.position.x / TILE_SIZE);
        int y = (int)(player.position.y / -TILE_SIZE);
        if ( y >= 0 && x >= 0 && y <= LEVEL_HEIGHT && x <= LEVEL_WIDTH ){
            if((levelData[y][x] == 101 || levelData[y][x] == 102)){
                
                glClearColor(255.0f/255.0f, 0.0f/255.0f, 0.0f/255.0f, 0.5f);
                sound.playMusic(sound.critical, 0.5);
                if(!player.criticalFlag){
                    if(collectGold < 3){
                        lose = true;
                        gameMode = STATE_GAME_OVER;
                    }
                    collectGold = max(0,collectGold-3);
                }
                player.criticalFlag = true;
                
            }else{
                player.criticalFlag = false;
                glClearColor(86.0f/255.0f, 152.0f/255.0f, 195.0f/255.0f, 1.0f);
            }
            if(levelData[y][x] == GOLD){
                sound.playMusic(sound.gold, 1);
                levelData[y][x] = 49;
                collectGold++;
                remaining--;
                if(remaining == 0 && currentLevel == 1){
                    levelData[5][28] = 0;
                    levelData[5][29] = 0;
                    levelData[5][30] = 0;
                }
                if(remaining == 0 && currentLevel == 2){
                    for(int i = 0; i < 7; i++){
                        levelData[1][i+20] = 5;
                        sound.playMusic(sound.broke, 0.5);
                    }
                    for(int i = 2; i < 7; i++){
                        levelData[i][20] = 5;
                        levelData[i][26] = 5;
                        sound.playMusic(sound.broke, 0.5);
                    }
                    for(int i = 0; i < 7; i++){
                        levelData[7][i+20] = 5;
                        sound.playMusic(sound.broke, 0.5);
                    }
                }
                if(remaining == 0 && currentLevel == 3){
                    sound.playMusic(sound.ascend, 0.5);
                    key = Entity(KEY, player.position.x, player.position.y, TILE_SIZE, TILE_SIZE);
                    currentLevel++;
                    gameMode = STATE_GAME_OVER;
                }
                
            }
            if(levelData[y][x] == KEY + 1){
                if(key.index == 0){
                    sound.playMusic(sound.ascend, 0.5);
                    key = Entity(KEY, player.position.x, player.position.y, TILE_SIZE, TILE_SIZE);
                    gameMode = STATE_GAME_OVER;
                    currentLevel++;
                }
            }
            /* flying object */
            if(levelData[y][x] == 38){
                levelData[y][x] = 0;
                sound.playMusic(sound.ascend, 0.5);
                flyingCount--;
            }
            /* speedup object */
            if(levelData[y][x] == 43){
                levelData[y][x] = 0;
                sound.playMusic(sound.ascend, 0.5);
                speedUpCount--;
                if(speedUpCount == 0){
                    for(int y=0; y < LEVEL_HEIGHT; y++) {
                        for(int x=0; x < LEVEL_WIDTH; x++) {
                            if((int)levelData[y][x] == 0){
                                levelData[y][x] = (char)GOLD;
                                if(remaining == -1){
                                    remaining = 1;
                                }else{
                                    remaining++;
                                }
                            }
                        }
                    }
                }
            }
        }
        
    }
    
    int currentLevel = 1;
    
    void reset(){
        objects.clear();
        parse(RESOURCE_FOLDER"tile_map" + to_string(currentLevel) + ".txt");
        player = Entity(PLAYER+1,objects[2].x, objects[2].y, TILE_SIZE, TILE_SIZE);
        gameMode = STATE_GAME_START;
        lose = false;
        accumulator = 0.0f;
        elapsed = 0.0f;
        collectGold = 0;
        key = Entity();
        
        playerEmitter = Entity(69, player.position.x - player.size.x/2, player.position.y, TILE_SIZE, TILE_SIZE);

        enemies.clear();
        if(currentLevel == 1){
            for(int i = 0; i < objects.size() - 1; i++){
                enemies.push_back(Entity(ENEMY, objects[i].x, objects[i].y, TILE_SIZE, TILE_SIZE));
                enemies[i].velocity.x = 0.5f;
            }
            enemies[0].startX = enemies[0].position.x - 0.1f;
            enemies[0].endX = enemies[0].position.x + 0.9f;
            enemies[1].position.y += 0.5f;
            enemies[1].startX = enemies[1].position.x - 0.5f;
            enemies[1].endX = enemies[1].position.x + 0.3f;
            remaining = 45;
        }
        if(currentLevel == 2){
            emit = ParticleEmitter(100, -1.0f, 1.0f, true, 0.4f, 0.5f);
            for(int i = 0; i < objects.size() - 1; i++){
                enemies.push_back(Entity(ENEMY, objects[i].x, objects[i].y, TILE_SIZE, TILE_SIZE));
            }
            enemies[0].velocity.x = 2.5f;
            enemies[0].startX = enemies[0].position.x - 1.8f;
            enemies[0].endX = enemies[0].position.x + 4.4f;
            enemies[1].velocity.x = 0.5f;
            enemies[1].position.y += 0.3f;
            enemies[1].startX = enemies[1].position.x - 0.5f;
            enemies[1].endX = enemies[1].position.x + 0.9f;
            remaining = 38;
            flyingCount = 5;
        }
        if(currentLevel == 3){
            for(int i = 0; i < objects.size() - 1; i++){
                enemies.push_back(Entity(ENEMY, objects[i].x, objects[i].y, TILE_SIZE, TILE_SIZE));
            }
            enemies[0].velocity.x = 0.7f;
            enemies[0].startX = enemies[0].position.x - 1.8f;
            enemies[0].endX = enemies[0].position.x + 4.4f;
            enemies[1].velocity.x = 0.45f;
            enemies[1].position.y += 0.3f;
            enemies[1].startX = enemies[1].position.x - 0.5f;
            enemies[1].endX = enemies[1].position.x + 0.9f;
            speedUpCount = 5;
            remaining = -1;
        }
    }
    
    void processInputGameOver(){
        const Uint8 *keys = SDL_GetKeyboardState(NULL);
        if(keys[SDL_SCANCODE_R]){
            reset();
        }
        if(keys[SDL_SCANCODE_ESCAPE]){
            done = true;
        }
    };
    
};


int main()
{
    /* setup */
    parse(RESOURCE_FOLDER"tile_map1.txt");
    Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 4096 );
    ShaderProgram textured, untextured;
    programSetUp(textured, untextured);
    GameState gameState = GameState();
    gameState.textured = textured;
    gameState.untextured = untextured;
    float accumulator = 0.0f;
    float lastFrameTicks = 0.0f;
    
    while(!gameState.done){
        ProcessEvents(gameState.done, gameState.event);
        
        glClear(GL_COLOR_BUFFER_BIT);
        float ticks = (float)SDL_GetTicks() / 1000.0f;
        float elapsed = ticks - lastFrameTicks;
        lastFrameTicks = ticks;
        elapsed += accumulator;
        if(elapsed < FIXED_TIMESTEP) {
            accumulator = elapsed;
            continue;
        }
        while(elapsed >= FIXED_TIMESTEP) {
            gameState.update(FIXED_TIMESTEP);
            elapsed -= FIXED_TIMESTEP;
        }
        accumulator = elapsed;
        gameState.render();
        gameState.update(elapsed);
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
    /* Translate the view matrix */
    viewMatrix = glm::translate(viewMatrix, glm::vec3(-2.0f, 1.5f, 0.0f));
    
    textured.SetViewMatrix(viewMatrix);
    textured.SetProjectionMatrix(projectionMatrix);
    untextured.SetViewMatrix(viewMatrix);
    untextured.SetProjectionMatrix(projectionMatrix);
    glClearColor(86.0f/255.0f, 152.0f/255.0f, 195.0f/255.0f, 1.0f);
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
