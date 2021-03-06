#include "HelloWorldScene.h"
#include "SimpleAudioEngine.h"
#include "Component.h"

using namespace cocos2d;
using namespace CocosDenshion;
using namespace std;

#pragma mark Default

CCScene* HelloWorld::scene() {
    CCScene *scene = CCScene::create();
    HelloWorld *layer = HelloWorld::create();
    scene->addChild(layer);
    return scene;
}

bool HelloWorld::init() {
    if (!CCLayer::init()) return false;
    
    spriteArray = new CCArray;
    tileArray = new CCArray;
    component = new CCArray;
    
    HelloWorld::addTileMap();
    
    CCSprite *background = CCSprite::create("PuzzleBackgroud.png");
    background->setPosition(ccp(winSize.width/2, winSize.height/2));
    //this->addChild(background, -2, -2);
    
    CCDirector::sharedDirector()->getTouchDispatcher()->addTargetedDelegate(this, 0, true);
    this->scheduleUpdate();
    
    CCObject *object;
    CCARRAY_FOREACH(component, object) {
        Component *cp = (Component*)(object);
        CCLog("ID: %i -Location :%i", cp->getID()+1,cp->getLocation()-300);
        
    }
    
    return true;
}

void HelloWorld::update(float dt) {
    if (swipeRight) {
        HelloWorld::swipedRight(pickedSprite);
        swipeRight = false;
    }
    if (swipeLeft) {
        HelloWorld::swipedLeft(pickedSprite);
        swipeLeft = false;
    }
    if (swipeUp) {
        HelloWorld::swipedUp(pickedSprite);
        swipeUp = false;
    }
    if (swipeDown) {
        HelloWorld::swipedDown(pickedSprite);
        swipeDown = false;
    }
}

#pragma mark Touches

bool HelloWorld::ccTouchBegan(CCTouch *touch, CCEvent *event) {
    CCPoint touchLoc = this->getParent()->convertTouchToNodeSpace(touch);
    CCObject *object = NULL;
    CCARRAY_FOREACH(spriteArray, object) {
        CCSprite *objectSprite = dynamic_cast<CCSprite*>(object);
        CCRect spriteRect = objectSprite->boundingBox();
        if (spriteRect.containsPoint(touchLoc)) {
            CCLog("Object ID: %i", objectSprite->getTag());
            swipeRecognized = false;
            startSwipePoint = touchLoc;
            HelloWorld::setSwipeNavigation();
            CCLog("Start Swipe Point -x%f -y%f", startSwipePoint.x, startSwipePoint.y);
            pickedSprite = objectSprite;
            CCLog(("Picked Sprite position -x%f 0y%f"), pickedSprite->getPositionX(), pickedSprite->getPositionY());
            spriteContained = true;
        }
    }
    return true;
}

void HelloWorld::ccTouchMoved (CCTouch *touch, CCEvent *event) {
    if (spriteContained) {
        CCPoint touchLoc = this->getParent()->convertTouchToNodeSpace(touch);
        movingSwipePoint = touchLoc;
        
        if (!swipeRecognized) swiping = true;
        else swiping = false;
        
        if (swiping == true) {
            CCRect touchRect = CCRect(touchLoc.x, touchLoc.y, 1, 1);
            
            CCRect swipeRightRect = swipeRightSprite->boundingBox();
            CCRect swipeLeftRect = swipeLeftSprite->boundingBox();
            CCRect swipeUpRect = swipeUpSprite->boundingBox();
            CCRect swipeDownRect = swipeDownSprite->boundingBox();
            
            if ((movingSwipePoint.x - startSwipePoint.x > 20) && touchRect.intersectsRect(swipeRightRect)) {
                swipeRecognized = true;
                swipeRight = true;
                HelloWorld::removeSwipeNavigation();
            }
            else if ((startSwipePoint.x - movingSwipePoint.x > 20) && touchRect.intersectsRect(swipeLeftRect)) {
                swipeRecognized = true;
                swipeLeft = true;
                HelloWorld::removeSwipeNavigation();
            }
            else if ((movingSwipePoint.y - startSwipePoint.y > 20) && touchRect.intersectsRect(swipeUpRect)) {
                swipeRecognized = true;
                swipeUp = true;
                HelloWorld::removeSwipeNavigation();
            }
            else if ((startSwipePoint.y - movingSwipePoint.y > 20) && touchRect.intersectsRect(swipeDownRect)) {
                swipeRecognized = true;
                swipeDown = true;
                HelloWorld::removeSwipeNavigation();
            }
            else if (!touchRect.intersectsRect(swipeRightRect) && !touchRect.intersectsRect(swipeLeftRect)
                     && !touchRect.intersectsRect(swipeUpRect) && !touchRect.intersectsRect(swipeDownRect)) swipeRecognized = true;
            
        }
    }
}

void HelloWorld::ccTouchEnded(CCTouch *touch, CCEvent *event) {
    HelloWorld::removeSwipeNavigation();
    swiping = false;
    CCPoint touchLoc = this->getParent()->convertTouchToNodeSpace(touch);
    if (HelloWorld::touchPosValidation(touchLoc)) {
        
        CCTMXLayer *layer = map->layerNamed("Grids");
        CCSize layerSize = layer->getLayerSize();
        CCSize tileSize = layer->getMapTileSize();
        CCPoint transPoint = HelloWorld::tileCoorForPosition(touchLoc);
        unsigned int m_gid = layer->tileGIDAt(transPoint);
        CCLog("Tile ID at position : %i", m_gid);
        
    }
    else CCLog("Invalid Position");
}

bool HelloWorld::touchPosValidation(CCPoint touchLoc) {
    if (touchLoc.x < 0 ||
        touchLoc.y < 0 ||
        touchLoc.x >= map->getContentSize().width ||
        touchLoc.y >= map->getContentSize().height) return  false;
    else return true;
}

#pragma mark Swipe Actions

void HelloWorld::swipedRight(CCSprite *sprite) {
    CCPoint spritePosition = sprite->getPosition();
    CCPoint translatePosition = HelloWorld::tileCoorForPosition(spritePosition);
    unsigned int gid = layer->tileGIDAt(translatePosition);
    if (gid != 7 && gid != 14 && gid != 21 && gid != 28 && gid != 35 && gid != 42 && gid != 49 ) {
        CCPoint destination = ccp(spritePosition.x + map->getTileSize().width, spritePosition.y);
        sprite->runAction(CCMoveTo::create(0.3, destination));
        int thisSpriteTag = sprite->getTag();
        
        CCLog("This sprite tag : %i", thisSpriteTag);
        
        swapSprite = (CCSprite*)this->getChildByTag(thisSpriteTag +1);
        swapSprite->runAction(CCMoveTo::create(0.3, sprite->getPosition()));
        HelloWorld::swapTiles(thisSpriteTag, thisSpriteTag+1);
        
        sprite->setTag(thisSpriteTag+1);
        swapSprite->setTag(thisSpriteTag);
        HelloWorld::checkTiles();
    }
}

void HelloWorld::swipedLeft(CCSprite *sprite) {
    CCPoint spritePosition = sprite->getPosition();
    CCPoint translatePosition = HelloWorld::tileCoorForPosition(spritePosition);
    unsigned int gid = layer->tileGIDAt(translatePosition);
    if (gid != 1 && gid != 8 && gid != 15 && gid != 22 && gid != 29 && gid != 36 && gid != 43 ) {
        CCPoint destination = ccp(spritePosition.x - map->getTileSize().width, spritePosition.y);
        sprite->runAction(CCMoveTo::create(0.3, destination));
        int thisSpriteTag = sprite->getTag();
        
        CCLog("This sprite tag : %i", thisSpriteTag);
        
        swapSprite = (CCSprite*)this->getChildByTag(thisSpriteTag -1);
        swapSprite->runAction(CCMoveTo::create(0.3, sprite->getPosition()));
        HelloWorld::swapTiles(thisSpriteTag, thisSpriteTag-1);
        
        sprite->setTag(thisSpriteTag-1);
        swapSprite->setTag(thisSpriteTag);
        HelloWorld::checkTiles();
    }
}

void HelloWorld::swipedUp(CCSprite *sprite) {
    CCPoint spritePosition = sprite->getPosition();
    CCPoint translatePosition = HelloWorld::tileCoorForPosition(spritePosition);
    unsigned int gid = layer->tileGIDAt(translatePosition);
    if (gid >= 7) {
        CCPoint destination = ccp(spritePosition.x, spritePosition.y + map->getTileSize().height);
        sprite->runAction(CCMoveTo::create(0.3, destination));
        int thisSpriteTag = sprite->getTag();
        
        CCLog("This sprite tag : %i", thisSpriteTag);
        
        swapSprite = (CCSprite*)this->getChildByTag(thisSpriteTag -7);
        swapSprite->runAction(CCMoveTo::create(0.3, sprite->getPosition()));
        HelloWorld::swapTiles(thisSpriteTag, thisSpriteTag-7);
        
        sprite->setTag(thisSpriteTag-7);
        swapSprite->setTag(thisSpriteTag);
        HelloWorld::checkTiles();
    }
}

void HelloWorld::swipedDown(CCSprite *sprite) {
    CCPoint spritePosition = sprite->getPosition();
    CCPoint translatePosition = HelloWorld::tileCoorForPosition(spritePosition);
    unsigned int gid = layer->tileGIDAt(translatePosition);
    if (gid < 43) {
        CCPoint destination = ccp(spritePosition.x, spritePosition.y - map->getTileSize().height);
        sprite->runAction(CCMoveTo::create(0.3, destination));
        int thisSpriteTag = sprite->getTag();
        
        CCLog("This sprite tag : %i", thisSpriteTag);
        
        swapSprite = (CCSprite*)this->getChildByTag(thisSpriteTag +7);
        swapSprite->runAction(CCMoveTo::create(0.3, sprite->getPosition()));
        HelloWorld::swapTiles(thisSpriteTag, thisSpriteTag+7);
        
        sprite->setTag(thisSpriteTag+7);
        swapSprite->setTag(thisSpriteTag);
        HelloWorld::checkTiles();
    }
}

#pragma mark Tile Functions

void HelloWorld::addTileMap () {
    map = CCTMXTiledMap::create("AnotherMap.tmx");
    this->addChild(map, -1, -1);
    layer = map->layerNamed("Grids");
    HelloWorld::createFixture(layer);
    
    CCArray *pChildrenArray = map->getChildren();
    CCObject *pObject = NULL;
    CCSpriteBatchNode *child = NULL;
    CCARRAY_FOREACH(pChildrenArray, pObject) {
        child = ((CCSpriteBatchNode *)pObject);
        if (!child) break;
        child->getTexture()->setAntiAliasTexParameters();
    }
}

void HelloWorld::createFixture(CCTMXLayer* layer) {
    CCSize layerSize = layer->getLayerSize();
    for (int y = 0; y < layerSize.height; y++) {
        for (int x = 0; x < layerSize.width; x++) {
            tileSprite = layer->tileAt(ccp(x, y));
            unsigned int m_gid = layer->tileGIDAt(ccp(x, y));
            tileSprite->setTag(x);
            tileArray->addObject(tileSprite);
            
            char stringID[10];
            sprintf(stringID, "%d", m_gid);
            CCLabelTTF *labelID = CCLabelTTF::create(stringID, "Arial", 20);
            
            labelID->setPosition(ccp(tileSprite->getPositionX() + tileSprite->getContentSize().width/2,
                                     tileSprite->getPositionY() + tileSprite->getContentSize().height/2));
            this->addChild(labelID, 0, 99);
            
            char tileName[7][20];
            strcpy(tileName[0], "Candy");
            strcpy(tileName[1], "Candy2");
            strcpy(tileName[2], "Candy3");
            strcpy(tileName[3], "Candy4");
            strcpy(tileName[4], "Candy5");
            strcpy(tileName[5], "Candy6");
            strcpy(tileName[6], "Candy7");
            int randomTile = rand() %7;
            char spriteName[100];
            sprintf(spriteName, "%s.png", tileName[randomTile]);
            CCSprite *randomTileSprite = CCSprite::create(spriteName);
            randomTileSprite->setPosition(ccp(tileSprite->getPositionX() + tileSprite->getContentSize().width/2,
                                              tileSprite->getPositionY() + tileSprite->getContentSize().height/2));
            int mgidTag = (300 + m_gid);
            this->addChild(randomTileSprite, 3, mgidTag);
            spriteArray->addObject(randomTileSprite);
            
            tileDimentionArray[m_gid] = randomTile;
            
            Component *cp =new Component(randomTile,mgidTag);
            component->addObject(cp);
        }
    }
}

void HelloWorld::letThereBeACandy(CCTMXLayer *layer) {
    CCSprite *candy = CCSprite::create("Candy.png");
    this->addChild(candy, 1, 101);
    
    CCSprite *candy2 = CCSprite::create("Candy.png");
    this->addChild(candy2, 1, 102);
    
    CCSprite *tempSprite = CCSprite::create();
    tempSprite = layer->tileAt(ccp(3, 3));
    CCLog("Temp Sprite tag %i", tempSprite->getTag());

    CCSprite *tempSprite2 = CCSprite::create();
    tempSprite2 = layer->tileAt(ccp(5, 5));
    CCLog("Temp Sprite tag %i", tempSprite2->getTag());
    
    candy->setPosition(ccp(tempSprite->getPositionX() + (tempSprite->getContentSize().width /2),
                           tempSprite->getPositionY() + (tempSprite->getContentSize().height /2)));
    
    candy2->setPosition(ccp(tempSprite2->getPositionX() + (tempSprite2->getContentSize().width /2),
                            tempSprite2->getPositionY() + (tempSprite2->getContentSize().height /2)));
    spriteArray->addObject(candy2);
    spriteArray->addObject(candy);
}

CCPoint HelloWorld::tileCoorForPosition(CCPoint position) {
    float x = position.x / map->getTileSize().width;
    float y = ((winSize.height - position.y)/(map->getTileSize().height)) -1;
    CCPoint transPos = ccp(floor(x), floor(y));
    return transPos;
}

CCPoint HelloWorld::positionForTileCoor(CCPoint tileCoor) {
    int x = (tileCoor.x * map->getTileSize().width) + map->getTileSize().width/2;
    int y = (map->getContentSize().height * map->getTileSize().height) - (tileCoor.y * map->getTileSize().height) - map->getTileSize().height/2;
    return ccp(x, y);
}

CCSprite *HelloWorld::getRightTileByTag(int tag) {
    return NULL;
}

void HelloWorld::swapTiles(int spriteTag, int swapSpriteTag) {

}

void HelloWorld::checkTiles() {
    
}

void HelloWorld::checkTileCombo(CCSprite *sprite) {
    
}

#pragma mark Swipe Navigation

void HelloWorld::setSwipeNavigation() {
    swipeRightSprite = CCSprite::create("SwipeLeftRight.png");
    swipeRightSprite->setPosition(ccp((startSwipePoint.x + swipeRightSprite->getContentSize().width / 2), startSwipePoint.y));
    swipeRightSprite->setOpacity(0);
    swipeRightSprite->setTag(200);
    this->addChild(swipeRightSprite);
    
    swipeLeftSprite = CCSprite::create("SwipeLeftRight.png");
    swipeLeftSprite->setPosition(ccp((startSwipePoint.x - swipeLeftSprite->getContentSize().width / 2), startSwipePoint.y));
    swipeLeftSprite->setOpacity(0);
    swipeLeftSprite->setTag(201);
    this->addChild(swipeLeftSprite);
    
    swipeUpSprite = CCSprite::create("SwipeUpDown.png");
    swipeUpSprite->setPosition(ccp(startSwipePoint.x, startSwipePoint.y + (swipeUpSprite->getContentSize().height / 2)));
    swipeUpSprite->setOpacity(0);
    swipeUpSprite->setTag(202);
    this->addChild(swipeUpSprite);
    
    swipeDownSprite = CCSprite::create("SwipeUpDown.png");
    swipeDownSprite->setPosition(ccp(startSwipePoint.x, startSwipePoint.y - (swipeUpSprite->getContentSize().height / 2)));
    swipeDownSprite->setOpacity(0);
    swipeDownSprite->setTag(203);
    this->addChild(swipeDownSprite);
    
}

void HelloWorld::removeSwipeNavigation() {
    if (this->getChildByTag(200))this->removeChildByTag(200);
    if (this->getChildByTag(201))this->removeChildByTag(201);
    if (this->getChildByTag(202))this->removeChildByTag(202);
    if (this->getChildByTag(203))this->removeChildByTag(203);
}

#pragma mark Debug

void HelloWorld::propertiesCheck(CCDictionary *singleTile) {

}

void HelloWorld::menuCloseCallback(CCObject* pSender) {
    CCDirector::sharedDirector()->end();
    
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    exit(0);
#endif
}
