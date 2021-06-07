//
// Created by kouta on 4/6/21.
//

#include <curlpp/Infos.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/cURLpp.hpp>
#include "json.hpp"
#include <variant>
#include <string>
#include <functional>
#include "server_conn.hpp"
#include "request_tools.hpp"
#include "graph.hpp"
#include <fstream>
#include <filesystem>

std::variant<serverStatus, serverError> getServerStatus() {
    auto response = curlDownloadFile(apiUrl + "status/");
    nlohmann::json j = nlohmann::json::parse(response.body.begin(), response.body.end());
    return deserializeJson<serverStatus>(j);
}

std::variant<std::shared_ptr<SDL_Surface>, serverError> getChunkMap(int32_t x, int32_t y) {
    auto imageUrl = apiUrl + "chunk/" + std::to_string(x) + "/" + std::to_string(y);
    return downloadImage(imageUrl);
}

std::variant<std::shared_ptr<SDL_Surface>, serverError>
downloadImage(const std::string imageUrl) {
    auto imageResponse = curlDownloadFile(imageUrl);
    if (imageResponse.responseCode == 200) {
        SDL_RWops *rwop;
        rwop = SDL_RWFromConstMem(imageResponse.body.data(), imageResponse.body.size());
        SDL_Surface *chunkSurface;
        auto image_type = imageResponse.headers["Content-Type"];
        if (image_type == std::string("image/jpeg"))
            chunkSurface = IMG_LoadJPG_RW(rwop);
        else if (image_type == std::string("image/png"))
            chunkSurface = IMG_LoadPNG_RW(rwop);
        else {
            std::cerr << "Unexpected image type (" << image_type << ")" << std::endl;
            exit(5);
        }
        SDL_FreeRW(rwop);
        return std::shared_ptr<SDL_Surface>(chunkSurface, sharedSurfaceDestructor);
    } else return nlohmann::json::parse(imageResponse.body.begin(), imageResponse.body.end()).get<serverError>();
}

uint64_t connect(std::string playerName) {
    auto response = curlSendFile(apiUrl + "start/" + playerName, "");
    auto jresponse = nlohmann::json::parse(response.body.begin(), response.body.end());
    return jresponse["actor_id"].get<uint64_t>();
}

std::variant<actionResponse, serverError> actorDo(const actorAction &action, uint64_t actorId) {
    nlohmann::json actionJ = action;

    std::string body = actionJ.dump();

    auto response = curlSendFile(apiUrl + "action/" + std::to_string(actorId), body);

    nlohmann::json j = nlohmann::json::parse(response.body.begin(), response.body.end());
//    std::cout << j << std::endl;
    return deserializeJson<actionResponse>(j);
}

std::variant<actorStatus, serverError> getActorStatus(uint64_t actorId) {
    auto response = curlDownloadFile(apiUrl + "self_status/" + std::to_string(actorId));

    nlohmann::json j = nlohmann::json::parse(response.body.begin(), response.body.end());

    return deserializeJson<actorStatus>(j);
}

const actorAction messageAction(std::string message) {
    return actorAction{"MESSAGE", message};
}

