//
// Created by kouta on 5/6/21.
//

#include "video.hpp"
#include <optional>

const int64_t chunk_w = 32;
const int64_t chunk_h = 32;
const int64_t sprite_size = 16;
static std::map<std::tuple<int32_t, int32_t>, std::shared_ptr<SDL_Surface>> chunkCache{};

static std::optional<std::shared_ptr<SDL_Surface>> playerSprite{};

std::shared_ptr<SDL_Surface> &getPlayerSprite() {
    if (!playerSprite.has_value()) {
        auto response = downloadImage(apiUrl + "chara/");
        playerSprite = std::visit(overloaded{
                [](serverError e) {
                    std::cerr << e.error << std::endl;
                    exit(1);
                    return std::shared_ptr<SDL_Surface>(nullptr);
                },
                [](std::shared_ptr<SDL_Surface> surf) {
                    return surf;
                }
        }, response);
    }
    return playerSprite.value();
}

std::shared_ptr<SDL_Surface> sdlInit() {
    SDL_Init(SDL_INIT_EVERYTHING);
    IMG_Init(IMG_INIT_JPG);
    SDL_Surface *pSurface = SDL_SetVideoMode(SCREEN_W, SCREEN_H, 24, SDL_HWSURFACE | SDL_DOUBLEBUF);
#ifdef __VITA__
    SDL_ShowCursor(SDL_DISABLE);
#endif
    return std::shared_ptr<SDL_Surface>(pSurface, sharedSurfaceDestructor);
}


std::shared_ptr<SDL_Surface> cachedChunk(int32_t chunkX, int32_t chunkY) {
    if (chunkCache.contains({chunkX, chunkY})) return chunkCache[{chunkX, chunkY}];
    return std::visit(overloaded{
                              [&](std::shared_ptr<SDL_Surface> arg) {
                                  chunkCache[{chunkX, chunkY}] = arg;
                                  return arg;
                              },
                              [&](serverError arg) {
                                  std::cout << arg.error << std::endl;
                                  exit(1);
                                  return std::shared_ptr<SDL_Surface>(nullptr);
                              }
                      },
                      getChunkMap(chunkX, chunkY));
}


void drawFrame(std::shared_ptr<SDL_Surface> screenSurface, serverStatus worldStatus, actorStatus ourStatus,
               uint64_t playerId) {
    auto &info = worldStatus.actors[std::to_string(playerId)];
    auto[x, y] = info.position;

    auto leftBorder = int32_t(floor(float(x - chunk_w) / float(chunk_w))) * chunk_w;
    auto rightBorder = int32_t(ceil(float(x + chunk_w) / float(chunk_w))) * chunk_w;
    auto topBorder = int32_t(floor(float(y - chunk_h) / float(chunk_h))) * chunk_h;
    auto bottomBorder = int32_t(ceil(float(y + chunk_h) / float(chunk_h))) * chunk_h;

    SDL_FillRect(screenSurface.get(), nullptr, 0);

    std::map<std::tuple<int32_t, int32_t>, std::shared_ptr<SDL_Surface>> imageMap;

    for (auto x = leftBorder; x < rightBorder; x++)
        for (auto y = topBorder; y < bottomBorder; y++) {
            if (((x - (chunk_w / 2)) % chunk_w == 0) && ((y - (chunk_h / 2)) % chunk_h == 0)) {
                imageMap[{x, y}] = cachedChunk(int32_t(floor(float(x - (chunk_w / 2)) / float(chunk_w))),
                                               int32_t(floor(float(y - (chunk_w / 2)) / float(chunk_h))));
            }
        }

    for (auto it : imageMap) {
        auto[cX, cY] = it.first;
        auto relX = (cX - x);
        auto relY = (cY - y);
        auto target_rect = SDL_Rect{
                .x=((relX - ((chunk_w) / 2)) * sprite_size) + (SQUARED_SCREEN_SIZE / 2) +
                   ((SCREEN_W - SQUARED_SCREEN_SIZE) / 2),
                .y=((relY - ((chunk_h) / 2)) * sprite_size) + (SQUARED_SCREEN_SIZE / 2) +
                   ((SCREEN_H - SQUARED_SCREEN_SIZE) / 2),
                .w=chunk_w * sprite_size,
                .h=chunk_h * sprite_size
        };
        SDL_BlitSurface(it.second.get(), nullptr,
                        screenSurface.get(), &target_rect
        );
    }

    auto &ptr = getPlayerSprite();

    for (auto &[id, actor] : worldStatus.actors) {
        auto[aX, aY] = actor.position;
        if (aX > leftBorder && aX < rightBorder && aY > topBorder && aY < bottomBorder) {
            auto frameIndex = (worldStatus.world_frame - actor.creation_time) % 4;
            auto actorId = std::stoull(id);
            auto userIndex = actorId % (ptr->h / 16);
            auto relX = aX - x;
            auto relY = aY - y;

            auto target_rect = SDL_Rect{
                    .x=((relX) * sprite_size) + (SQUARED_SCREEN_SIZE / 2) + (SCREEN_W - SQUARED_SCREEN_SIZE) / 2,
                    .y=((relY) * sprite_size) + (SQUARED_SCREEN_SIZE / 2) + (SCREEN_H - SQUARED_SCREEN_SIZE) / 2,
                    .w= sprite_size,
                    .h=sprite_size
            };

            auto source_rect = SDL_Rect{
                    .x=(frameIndex * sprite_size),
                    .y=(userIndex * sprite_size),
                    .w=sprite_size,
                    .h=sprite_size
            };

            SDL_BlitSurface(ptr.get(), &source_rect, screenSurface.get(), &target_rect);
#ifndef NDEBUG
            if (actorId == playerId) {
                stringRGBA(screenSurface.get(), target_rect.x, target_rect.y - 30,
                           (std::to_string(aX) + "," + std::to_string(aY)).c_str(),
                           255, 255, 0, 255);
                stringRGBA(screenSurface.get(), target_rect.x, target_rect.y - 20,
                           ("HEIGHT: " + std::to_string(ourStatus.height) + ", ON: " + ourStatus.type).c_str(),
                           255, 255, 0, 255);
            }
#endif
            stringRGBA(screenSurface.get(), target_rect.x, target_rect.y - 10,
                       (actor.name + "(" + id + ")").c_str(),
                       255, 255, 0, 255);
            for (auto i = 0; i < actor.messages.size(); i++) {
                stringRGBA(screenSurface.get(),
                           target_rect.x + 18, target_rect.y + (actor.messages.size() - i) * 10,
                           actor.messages[i].msg.c_str(),
                           255, 255, 0, 255);
            }
        }
    }

#ifndef NDEBUG
    for (auto it : imageMap) {
        auto[cX, cY] = it.first;

        auto relX = (cX - x) * sprite_size;
        auto relY = (cY - y) * sprite_size;
        auto chunk_w_screen = chunk_w * sprite_size;

        rectangleRGBA(screenSurface.get(),
                      (relX - chunk_w_screen / 2) + (SQUARED_SCREEN_SIZE / 2),
                      (relY - chunk_w_screen / 2) + (SQUARED_SCREEN_SIZE / 2),
                      (relX + chunk_w_screen / 2) + (SQUARED_SCREEN_SIZE / 2),
                      (relY + chunk_w_screen / 2) + (SQUARED_SCREEN_SIZE / 2),
                      255, 0, 0, 255);
        auto chunk_x = int64_t(floor(float(cX) / float(chunk_w)));
        auto chunk_y = int64_t(floor(float(cY) / float(chunk_w)));
        stringRGBA(screenSurface.get(), relX, relY, (std::to_string(chunk_x) + std::to_string(chunk_y)).c_str(), 255, 0,
                   0, 255);
    }
#endif
    SDL_Flip(screenSurface.get());
}
