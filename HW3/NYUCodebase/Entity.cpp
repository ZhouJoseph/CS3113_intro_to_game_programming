//
//  Entity.cpp
//  NYUCodebase
//
//  Created by 周凯旋 on 3/16/19.
//  Copyright © 2019 Ivan Safrin. All rights reserved.
//

#include "Entity.hpp"
#include <vector>

Entity::Entity(float x, float y, float width, float height):position(x,y,0),size(width,height,0),health(100){}

void Entity::Draw(ShaderProgram& program) const {
    glm::mat4 modelMatrix(1.0f);
    modelMatrix = glm::translate(modelMatrix, position);
    modelMatrix = glm::scale(modelMatrix, size);
    program.SetModelMatrix(modelMatrix);
    
    float vertices[] = {-0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5};
    glEnableVertexAttribArray(program.positionAttribute);
    glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glDisableVertexAttribArray(program.positionAttribute);
}


EntityTextured::EntityTextured(float x, float y, float width, float height, const SheetSprite& sprite):
Entity(x,y,width,height),sprite(sprite){}
void EntityTextured::Draw(ShaderProgram &program) const {
    Entity::Draw(program);
    sprite.Draw(program);
}

void EntityTextured::shootBullet(std::vector<EntityTextured>& bulletVector, const SheetSprite& bullet, float verticalVelocity){
    EntityTextured shot(position.x, position.y, 0.15f, 0.35f, bullet);
    shot.velocity.y = verticalVelocity;
    bulletVector.push_back(shot);
}


EntityFont::EntityFont(float x, float y, float width, float height, const std::string& text):
Entity(x,y,width,height), text(text){}
void EntityFont::Draw(ShaderProgram &program, GLuint fontTexture, float spacing) const {
    Entity::Draw(program);
    DrawText(program, fontTexture, size.x, spacing);
}
void EntityFont::DrawText(ShaderProgram& program, GLuint fontTexture, float size, float spacing) const {
    
    float texture_size = 1.0/16.0f;
    std::vector<float> vertexData;
    std::vector<float> texCoordData;
    for(int i=0; i < text.size(); i++) {
        int spriteIndex = (int)text[i];
        float texture_x = (float)(spriteIndex % 16) / 16.0f;
        float texture_y = (float)(spriteIndex / 16) / 16.0f;
        vertexData.insert(vertexData.end(), {
            ((size+spacing) * i) + (-0.5f * size), 0.5f * size,
            ((size+spacing) * i) + (-0.5f * size), -0.5f * size,
            ((size+spacing) * i) + (0.5f * size), 0.5f * size,
            ((size+spacing) * i) + (0.5f * size), -0.5f * size,
            ((size+spacing) * i) + (0.5f * size), 0.5f * size,
            ((size+spacing) * i) + (-0.5f * size), -0.5f * size,
        });
        texCoordData.insert(texCoordData.end(), {
            texture_x, texture_y,
            texture_x, texture_y + texture_size,
            texture_x + texture_size, texture_y,
            texture_x + texture_size, texture_y + texture_size,
            texture_x + texture_size, texture_y,
            texture_x, texture_y + texture_size,
        });
    }
    glBindTexture(GL_TEXTURE_2D, fontTexture);
    glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
    glEnableVertexAttribArray(program.positionAttribute);
    glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
    glEnableVertexAttribArray(program.texCoordAttribute);
    glBindTexture(GL_TEXTURE_2D, fontTexture);
    glDrawArrays(GL_TRIANGLES, 0, text.size()*6);
    glDisableVertexAttribArray(program.positionAttribute);
    glDisableVertexAttribArray(program.texCoordAttribute);
}
