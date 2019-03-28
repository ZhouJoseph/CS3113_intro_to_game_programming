//
//  Entity.hpp
//  NYUCodebase
//
//  Created by 周凯旋 on 3/16/19.
//  Copyright © 2019 Ivan Safrin. All rights reserved.
//

#ifndef Entity_hpp
#define Entity_hpp
#include "ShaderProgram.h"
#include <math.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "SheetSprite.hpp"
#include <vector>

struct Entity{
    Entity(){};
    Entity(float x, float y, float width, float height);
    virtual void Draw(ShaderProgram& program) const;
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 size;
    int health;
};

struct EntityTextured:Entity{
    EntityTextured(){};
    EntityTextured(float x, float y, float width, float height, const SheetSprite& sprite);
    virtual void Draw(ShaderProgram &program) const;
    void shootBullet(std::vector<EntityTextured>& bulletVector, const SheetSprite& bullet, float verticalVelocity);
    SheetSprite sprite;
    
};

struct EntityFont:Entity{
    EntityFont(float x, float y, float width, float height, const std::string& text);
    virtual void Draw(ShaderProgram &program, GLuint fontTexture, float spacing) const;
    void DrawText(ShaderProgram& program, GLuint fontTexture, float size, float spacing) const;
    std::string text;
};



#endif /* Entity_hpp */
