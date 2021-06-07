//
// Created by kouta on 6/6/21.
//

#ifndef VITANET_INPUT_H
#define VITANET_INPUT_H

#include <cmath>
#include <SDL/SDL_gfxPrimitives.h>
#include <curlpp/Infos.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/cURLpp.hpp>
#include "video.hpp"
#include "visitor_overload.hpp"
#include "graph.hpp"
#include "request_tools.hpp"
#include "server_conn.hpp"
#include "json.hpp"
#include <variant>
#include <string>
#include <functional>
#include "input.h"

#ifdef __VITA__
#include <vitasdk.h>
#include <psp2/message_dialog.h>
#include <psp2/ime_dialog.h>
#include <psp2/apputil.h>

#endif

std::string getInput(std::string dialogTitle, std::shared_ptr<SDL_Surface> screenSurface);

bool handleInput(uint64_t userId, std::shared_ptr<SDL_Surface> screenSurface);

void inputInit();

void inputTerm();

#endif //VITANET_INPUT_H
