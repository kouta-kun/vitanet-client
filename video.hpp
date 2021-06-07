//
// Created by kouta on 5/6/21.
//

#ifndef VITANET_VIDEO_HPP
#define VITANET_VIDEO_HPP

#include <cmath>

static const int SCREEN_H = 544;
static const int SCREEN_W = 960;
static const int SQUARED_SCREEN_SIZE = std::min(SCREEN_H, SCREEN_W);

#include <SDL/SDL_gfxPrimitives.h>
#include <curlpp/Infos.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/cURLpp.hpp>
#include "visitor_overload.hpp"
#include "graph.hpp"
#include "request_tools.hpp"
#include "server_conn.hpp"
#include "json.hpp"
#include <variant>
#include <string>
#include <functional>

std::shared_ptr<SDL_Surface> sdlInit();

void drawFrame(std::shared_ptr<SDL_Surface> screenSurface, serverStatus worldStatus, actorStatus ourStatus,
               uint64_t playerId);

#endif //VITANET_VIDEO_HPP
