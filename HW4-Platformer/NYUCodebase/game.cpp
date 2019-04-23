#include "Necessity.hpp"
#include "parse_tile_map.hpp"


SDL_Window* displayWindow;

enum GameMode{ STATE_MAIN_MENU, STATE_GAME_START, STATE_GAME_OVER };
void programSetUp(ShaderProgram& textured, ShaderProgram& untextured);
void ProcessEvents(bool& done, SDL_Event& event);
GLuint LoadTexture(const char* filePath);


#define FIXED_TIMESTEP 0.0166666f
#define MAX_TIMESTEPS 6

float lerp(float v0, float v1, float t) {
    return (1.0-t)*v0 + t*v1;
}

vector<int> statics = {1,2,3,4,17,18,19,20,33,34,35,36};

struct Entity{
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
        velocity.x = lerp(velocity.x, 0.0f, elapsed * 2.0f);
        velocity.x += acceleration_x * elapsed;
        if( abs(velocity.x) < 0.03f){
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
    
    void collideYBottom(){
        int x = (int)(position.x / TILE_SIZE);
        int y = (int)((position.y - size.y/2) / -TILE_SIZE);

        if(y >= 0){
            if(find(statics.begin(),statics.end(),(int)levelData[y][x]) != statics.end()){
                collidedBottom = true;
                velocity.y = 0.0f;
                position.y -= (position.y - size.y/2) - (-TILE_SIZE * y);
            }
        }

    }
    
    void updateY(float elapsed){
        float gravity = -3.0f;
        collideYTop();
        collideYBottom();
        if(collidedBottom){
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

struct GameState{
    /* member fields */
    GameMode gameMode;
    GLuint spriteSheetID;
    vector<Entity> enemies;
    Entity player;
    SDL_Event event;
    ShaderProgram textured;
    ShaderProgram untextured;
    GLuint fontSheetID;
    bool done;
    bool lose = false;
    float accumulator = 0.0f;
    float elapsed = 0.0f;
    int collectGold = 0;
    int remaining = 45;
    Entity goldTexture;
    Entity key;
    
    
    GameState():
    spriteSheetID(LoadTexture(RESOURCE_FOLDER"arne_sprites.png")),
    fontSheetID(LoadTexture(RESOURCE_FOLDER"pixel_font.png")),
    gameMode(STATE_GAME_START),
    done(false),
    player(PLAYER+1,objects[2].x, objects[2].y, TILE_SIZE, TILE_SIZE)
    {
//        for(int i = 0; i < objects.size() - 1; i++){
//            enemies.push_back(Entity(ENEMY, objects[i].x, objects[i].y, TILE_SIZE, TILE_SIZE));
//        }
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
    
    void renderMainMenu(){

    }
    
    void renderGameLevel(){
        glm::mat4 viewMatrix(1.0f);
        if(player.position.x > 7.0f){
            viewMatrix = glm::translate(viewMatrix, glm::vec3(max(-player.position.x,-LEVEL_WIDTH*TILE_SIZE + 2.0f), min(-player.position.y,1.5f), 0.0f));
        }else{
            viewMatrix = glm::translate(viewMatrix, glm::vec3(min(-player.position.x,-2.0f), min(-player.position.y,1.5f), 0.0f));
        }
        textured.SetViewMatrix(viewMatrix);
        drawTileMap();
        
        player.Draw(textured,spriteSheetID);
        goldTexture = Entity(GOLD+1, player.position.x - 0.08f, player.position.y + 0.17f, TILE_SIZE/1.5, TILE_SIZE/1.5);
        goldTexture.Draw(textured, spriteSheetID);
        Entity playerHead(83, player.position.x, player.position.y + player.size.y, TILE_SIZE, TILE_SIZE);
        playerHead.Draw(textured, spriteSheetID);
        key.Draw(textured, spriteSheetID);
        float space = 0.05f;
        int count = 0;
        for(const auto& c : to_string(collectGold)){
            Entity score((int) c, goldTexture.position.x + 0.07f + space * (count++), goldTexture.position.y, TILE_SIZE / 3, TILE_SIZE / 3);
            score.sprite_count_y = 16;
            score.sprite_count_x = 16;
            score.Draw(textured, fontSheetID);
        }
    }
    
    void renderGameOver(){
        if(!lose){
            key.Draw(textured, spriteSheetID);
            if(key.size.x >= 2.5f){
                float space = 0.1f;
                int count = 0;
                for(const auto& c : string("You get the key!")){
                    Entity score((int) c, key.position.x - key.size.x/2 + TILE_SIZE + space * (count++), key.position.y + key.size.y/2, TILE_SIZE / 2, TILE_SIZE / 2);
                    score.sprite_count_y = 16;
                    score.sprite_count_x = 16;
                    score.Draw(textured, fontSheetID);
                }
                
                count = 0;
                
                for(const auto& c : (string("Gold: ") + to_string(collectGold))){
                    Entity score((int) c, key.position.x - key.size.x/2 + TILE_SIZE + space * (count++), key.position.y + key.size.y/2 - TILE_SIZE, TILE_SIZE / 2, TILE_SIZE / 2);
                    score.sprite_count_y = 16;
                    score.sprite_count_x = 16;
                    score.Draw(textured, fontSheetID);
                }
            }
        }else{
            float space = 0.1f;
            int count = 0;
            for(const auto& c : string("What? Die already?")){
                Entity score((int) c, player.position.x - player.size.x/2 - 3*TILE_SIZE + space * (count++), player.position.y + player.size.y/2, TILE_SIZE / 2, TILE_SIZE / 2);
                score.sprite_count_y = 16;
                score.sprite_count_x = 16;
                score.Draw(textured, fontSheetID);
            }
            count = 0;
            for(const auto& c : string("Press r to restart!")){
                Entity score((int) c, player.position.x - player.size.x/2 - 3*TILE_SIZE + space * (count++), player.position.y + player.size.y/2 - TILE_SIZE, TILE_SIZE / 2, TILE_SIZE / 2);
                score.sprite_count_y = 16;
                score.sprite_count_x = 16;
                score.Draw(textured, fontSheetID);
            }
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
        player.collidedBottom = false;
        processInputGameLevel(elapsed);
    }
    
    void update(float elapsed){
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
    
    void processInputMainMenu(float elapsed){

    }
    
    void processInputGameLevel(float elapsed){
        
        const Uint8 *keys = SDL_GetKeyboardState(NULL);
        float acceleration_x = 0.0f;
        
        player.updateY(elapsed);
        if(keys[SDL_SCANCODE_UP] && !player.jump && player.collidedBottom){
            player.velocity.y = 2.3f;
            player.jump = true;
            player.position.y += 0.01f;
        }
        if(keys[SDL_SCANCODE_RIGHT]){
            acceleration_x = 2.5f;
        }
        else if(keys[SDL_SCANCODE_LEFT]){
            acceleration_x = -2.5f;
        }
        player.updateX(elapsed, acceleration_x);
        int x = (int)(player.position.x / TILE_SIZE);
        int y = (int)(player.position.y / -TILE_SIZE);
        if ( y >= 0 && x >= 0 && y <= LEVEL_HEIGHT && x <= LEVEL_WIDTH ){
            if((levelData[y][x] == 101 || levelData[y][x] == 102)){

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
            }
            if(levelData[y][x] == GOLD){
                levelData[y][x] = 0;
                collectGold++;
                remaining--;
                if(remaining == 0){
                    levelData[5][28] = 0;
                    levelData[5][29] = 0;
                    levelData[5][30] = 0;
                }
            }
            if(levelData[y][x] == KEY + 1){
                
                if(key.index == 0){
                    key = Entity(KEY, player.position.x, player.position.y, TILE_SIZE, TILE_SIZE);
                    gameMode = STATE_GAME_OVER;
                }
            }
        }
    }
    
    void reset(){
        parse(RESOURCE_FOLDER"tile_map.txt");
        player = Entity(PLAYER+1,objects[2].x, objects[2].y, TILE_SIZE, TILE_SIZE);
        gameMode = STATE_GAME_START;
        lose = false;
        accumulator = 0.0f;
        elapsed = 0.0f;
        collectGold = 0;
        remaining = 45;
        key = Entity();
    }
    
    void processInputGameOver(){
        const Uint8 *keys = SDL_GetKeyboardState(NULL);
        if(keys[SDL_SCANCODE_R]){
            reset();
        }
    };
    
};


int main()
{
    /* setup */
    parse(RESOURCE_FOLDER"tile_map.txt");
    
    ShaderProgram textured, untextured;
    programSetUp(textured, untextured);
    GameState gameState = GameState();
    gameState.textured = textured;
    gameState.untextured = untextured;
    float accumulator = 0.0f;
    float lastFrameTicks = 0.0f;
    
    while(!gameState.done){
        glClear(GL_COLOR_BUFFER_BIT);
        float ticks = (float)SDL_GetTicks() / 1000.0f;
        float elapsed = ticks - lastFrameTicks;
        lastFrameTicks = ticks;
        elapsed += accumulator;
        if(elapsed < FIXED_TIMESTEP) {
            accumulator = elapsed;
            continue; }
        while(elapsed >= FIXED_TIMESTEP) {
            gameState.update(FIXED_TIMESTEP);
            elapsed -= FIXED_TIMESTEP;
        }
        ProcessEvents(gameState.done, gameState.event);
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
