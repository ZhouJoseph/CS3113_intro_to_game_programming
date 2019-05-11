//
//  parse_tile_map.hpp
//  NYUCodebase
//
//  Created by 周凯旋 on 4/9/19.
//  Copyright © 2019 Ivan Safrin. All rights reserved.
//

#ifndef parse_tile_map_h
#define parse_tile_map_h

#include <fstream>
#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#define TILE_SIZE 0.2f
#define ENEMY 81
#define PLAYER 98
using namespace std;
int mapWidth, mapHeight;
unsigned char** levelData;


struct flareEntity{
    flareEntity(int id, float x, float y):id(id), x(x), y(y){}
    int id;
    float x;
    float y;
};
std::vector<flareEntity> objects;

bool readHeader(std::ifstream &stream) {
    string line;
    mapWidth = -1;
    mapHeight = -1;
    while(getline(stream, line)) {
        if(line == "") { break; }
        istringstream sStream(line);
        string key,value;
        getline(sStream, key, '=');
        getline(sStream, value);
        if(key == "width") {
            mapWidth = atoi(value.c_str());
        } else if(key == "height"){
            mapHeight = atoi(value.c_str());
        } }
    if(mapWidth == -1 || mapHeight == -1) {
        return false;
    } else { // allocate our map data
        levelData = new unsigned char*[mapHeight];
        for(int i = 0; i < mapHeight; ++i) {
            levelData[i] = new unsigned char[mapWidth];
        }
        return true;
    }
}

bool readLayerData(std::ifstream &stream) {
    string line;
    while(getline(stream, line)) {
        if(line == "") { break; }
        istringstream sStream(line);
        string key,value;
        getline(sStream, key, '=');
        getline(sStream, value);
        if(key == "data") {
            for(int y = 0; y < mapHeight; y++) {
                getline(stream, line);
                istringstream lineStream(line);
                string tile;
                for(int x=0; x < mapWidth; x++) {
                    getline(lineStream, tile, ',');
                    unsigned char val =  (unsigned char)atoi(tile.c_str());
                    levelData[y][x] = val;
                }
            }
        }
    }
    return true;
}

void placeEntity(const string& type, float placeX, float placeY){
    /* Have the location in the object world --> transform it in the world view */
    cout << type;
    if (type == "enemy"){
        objects.push_back(flareEntity(ENEMY, placeX, placeY));
    }else if (type == "player"){
        objects.push_back(flareEntity(PLAYER, placeX, placeY));
    }
}

bool readEntityData(std::ifstream &stream) {
    string line;
    string type;
    while(getline(stream, line)) {
        if(line == "") { break; }
        istringstream sStream(line);
        string key,value;
        getline(sStream, key, '=');
        getline(sStream, value);
        if(key == "type") {
            type = value;
        } else if(key == "location") {
            istringstream lineStream(value);
            string xPosition, yPosition;
            getline(lineStream, xPosition, ',');
            getline(lineStream, yPosition, ',');
            float placeX = atoi(xPosition.c_str()) * TILE_SIZE;
            float placeY = atoi(yPosition.c_str()) * -TILE_SIZE;
            placeEntity(type, placeX, placeY);
        }
    }
    return true;
}

void parse(const std::string& levelFile){
    ifstream infile(levelFile);
    string line;
    while (getline(infile, line)) {
        if(line == "[header]") {
            if(!readHeader(infile)) {
                return;
            }
        } else if(line == "[layer]") {
            readLayerData(infile);
        } else if(line == "[enemies]" || line == "[player]") {
            readEntityData(infile);
        }
    }
}

#endif /* parse_tile_map_h */
