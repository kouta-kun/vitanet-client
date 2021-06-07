#include <functional>
#include <string>
#include <variant>

#include "json.hpp"
#include "server_conn.hpp"
#include "request_tools.hpp"
#include "graph.hpp"
#include "visitor_overload.hpp"
#include "video.hpp"
#include "input.h"
#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/Infos.hpp>
#include <SDL/SDL_gfxPrimitives.h>
#include <cmath>

bool mainLoop(uint64_t userId, const std::shared_ptr<SDL_Surface> &screenSurface, serverStatus &worldStatus,
              actorStatus &ourStatus) {
    bool running;
    running = handleInput(userId, screenSurface);

    auto worldVariant = getServerStatus();
    std::visit(overloaded{
            [](serverError arg) {
                std::cout << "ERROR:" << arg.error << std::endl;
            },
            [&](serverStatus arg) {
                worldStatus = arg;
            }
    }, worldVariant);

    auto ourVariant = getActorStatus(userId);
    std::visit(overloaded{
            [](serverError arg) {
                std::cout << "ERROR:" << arg.error << std::endl;
            },
            [&](actorStatus arg) {
                ourStatus = arg;
            }
    }, ourVariant);

    drawFrame(screenSurface, worldStatus, ourStatus, userId);
    return running;
}

int main() {
    netInit();
    httpInit();
    auto screenSurface = sdlInit();
    inputInit();

    std::string playerName = getInput("Player name", screenSurface);
    auto userId = connect(playerName);
    serverStatus worldStatus;
    actorStatus ourStatus;

    bool running = true;
    while (running) {
        running = mainLoop(userId, screenSurface, worldStatus, ourStatus);
    }

    inputTerm();
    netTerm();
    httpTerm();
}