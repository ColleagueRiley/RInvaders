#define RGFW_IMPLEMENTATION
#define RGFW_BUFFER
#include "RGFW.h"

#include <time.h>

#include "sprites.h"

enum {
    stateDie = 0,
    stateNormal = 1,
    stateShoot = 2,
};

u32 drawSprite(u8* buffer, size_t width, size_t height, u8* sprite, size_t x, i32 y, size_t w, size_t h) {
    u32 collide = 0x00;
    for (i32 y1 = 0; y1 < h; y1++) {
        if (y1 + y < 0) continue;
        if ((y1 + y) > height) continue; 
        
        for (size_t x1 = 0; x1 < w; x1++) {
            size_t index = ((y1 + y) * width * 4) + (x + x1) * 4;
            size_t index2 = (y1 * w * 4) + x1 * 4;
           
            if (sprite[index2 + 3] == 0x00) continue;
           
            u32 collideColor = (((u32)buffer[index]) << 24) | 
                                ((u32)buffer[index + 1] << 16) |
                                ((u32)buffer[index + 2] << 8) |
                                ((u32)buffer[index + 3]);
            
            if (collideColor != 0x00 && collideColor != 0x000000FF)
                collide = collideColor;

            memcpy(&buffer[index], &sprite[index2], 4);
        }
    }
    return collide;
}

void shootBullet(RGFW_window* window, u8* background, RGFW_point* bullet, u8* state, i32 speed) {
    bullet->y += speed;
    
    if (bullet->y + 10 < 0) *state = stateNormal;
    if (bullet->y + 10 >= window->r.h) *state = stateNormal;
    
    u32 collide = drawSprite(window->buffer, window->r.w, window->r.h, bulletSprite, bullet->x, bullet->y, 5, 10);
    switch (collide) {
        case 0x20FF00FF: // bullet coliding with a wall
            u32 color = 0xFF000000;
            
            for (size_t y = 0; y < 10; y++) {
                for (size_t x = 0; x < 5; x++) {
                    size_t index = ((bullet->y + y) * window->r.w * 4) + (bullet->x + x) * 4;
                    memcpy(&background[index], &color, 4);
                }
            }
            
            *state = stateNormal;
            break;
        default: break;
    }
}

#ifdef _MSC_VER
int main(void);
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmd, int nShow) {
    main();
}
#endif 

int main(void) {
    srand(time(NULL));

    RGFW_window* window = RGFW_createWindow("RInvaders", RGFW_RECT(0, 0, 380, 480), RGFW_windowCenter | RGFW_windowNoResize);
    
    u8* background = (u8*)malloc(380 * 480 * 4);
    for (size_t i = 0; i < 4; i++)
        drawSprite(background, window->r.w, window->r.h, wallSprite, 40 + (i * 80), window->r.h - 100, 40, 40);

    RGFW_rect player = RGFW_RECT(40, window->r.h - 30, 20, 20);
    u8 playerState = stateNormal;
    
    RGFW_rect alien = RGFW_RECT(30, 30, 11, 5);
    u8 alienState[11 * 5] = {stateNormal};
    memset(alienState, stateNormal, sizeof(alienState));

    i8 alienDir = 1;

    float startTime = RGFW_getTime();
    size_t frameCount = 0;

    RGFW_point bullet = RGFW_POINT(0, 0);
    RGFW_point alienBullet[11 * 5];
    size_t alienCount = 11 * 5;

    u8 alienSpriteState = 0;

    while (RGFW_window_shouldClose(window) == RGFW_FALSE) {
        while (RGFW_window_checkEvent(window));
        if (playerState != stateDie || alienCount <= 0) {
            if (RGFW_isPressed(window, RGFW_left))
                player.x -= 3;
            if (RGFW_isPressed(window, RGFW_right))
                player.x += 3;
            if (RGFW_isPressed(window, RGFW_space) && playerState != stateShoot) {
                playerState = stateShoot;
                bullet = RGFW_POINT(player.x + 5, player.y - 11);
            }
        } else  if (RGFW_isPressed(window, RGFW_return)) {
            if (RGFW_isPressed(window, RGFW_shiftL) || RGFW_isPressed(window, RGFW_shiftR)) {
                for (size_t i = 0; i < 4; i++)
                    drawSprite(background, window->r.w, window->r.h, wallSprite, 40 + (i * 80), window->r.h - 100, 40, 40);
                memset(alienState, stateNormal, sizeof(alienState));
                alienCount = 11 * 5;
            } else {
                for (size_t i = 0; i < sizeof(alienState); i++) {
                    if (alienState[i] == stateShoot)
                        alienState[i] = stateNormal;
                }
            }

            player = RGFW_RECT(40, window->r.h - 30, 20, 20);
            playerState = stateNormal;
            alien = RGFW_RECT(30, 30, 11, 5);
            alienDir = 1;
        }

        if (player.x > window->r.w) {
            player.x = player.x - window->r.w;
        } else if (player.x < 0) {
            player.x = window->r.w + player.x;
        }
        
        memcpy(window->buffer, background, window->r.w * window->r.h * 4);
            
        if (playerState == stateShoot) {
            shootBullet(window, background, &bullet, &playerState, -5);
        }
        
        if (alienCount > 0) {
            for (size_t x = 0; x < alien.w; x++) {
                for (size_t y = 0; y < alien.h; y++) {
                    size_t index = (y * alien.w) + x;
                    if (alienState[index] == stateDie) continue;
                    

                    size_t spriteIndex = ((y * 2) + alienSpriteState) * 20;
    //                if (y == 0) spriteIndex -= 1;

                    spriteIndex = (spriteIndex * 20 * 4);  

                    u32 collide = drawSprite(window->buffer, window->r.w, window->r.h, &alienSprite[spriteIndex], (x * 25) + alien.x, (y * 25) + alien.y, 20, 20);
                    switch (collide) {
                        case 0xFFFFFFFF: // coliding with a bullet
                            if (playerState == stateShoot) {
                                alienState[index] = stateDie;
                                alienCount -= 1;
                                playerState = stateNormal;
                            }
                            break;
                        default: break;
                    }

                    if (alienState[index] == stateShoot) {
                        shootBullet(window, background, &alienBullet[index], &alienState[index], 5);
                    }
                }
            }
                
            if ((frameCount % 40) == 0) {
                size_t shots = rand() % 3;
                for (size_t i = 0; i < shots; i++) {
                    size_t x = rand() % alien.w;
                    size_t y = rand() % alien.h;
                    size_t index = (y * alien.w) + x;

                    if (alienState[index] == stateDie) {
                        i--;
                        continue;
                    }

                    alienState[index] = stateShoot;
                    alienBullet[index] = RGFW_POINT(((x * 25) + alien.x) - 5, ((y + 25) + alien.x) + 36);
                }
            }
            
                if ((frameCount % 40) == 0){
                    alienSpriteState = 1;
                }
                if ((frameCount % 50) == 0 && playerState != stateDie) {
                    alienSpriteState = 0;
                    if (alienDir == -1 && (alien.x <= 5 && alien.y + (alien.h * 20) < window->r.h - 30)) {
                        alien.y += 20;
                        alienDir = 1;
                    } else if (alienDir == 1 && ((alien.x + (alien.w * 25)) >= (window->r.w - 15) && alien.y + (alien.h * 20) < window->r.h - 30)) {
                        alien.y += 20;
                        alienDir = -1;
                    } else { 
                        alien.x += alienDir * 5;
                    }
                }

            if (alien.y + (alien.h * 20) >= window->r.h - 40)
            playerState = stateDie;
        }

        u32 collide = drawSprite(window->buffer, window->r.w, window->r.h, lonicSprite, player.x, player.y, player.w, player.h);
        switch (collide) {
            case 0xFFFFFFFF: // coliding with a bullet
                playerState = stateDie;
                break; 
            default: break;
        }

        RGFW_window_swapBuffers(window);
        RGFW_checkFPS(startTime, frameCount, 60);
        frameCount++;
    }

    free(background);

    RGFW_window_close(window);
}
