//
// Created by kouta on 4/6/21.
//


#ifndef __SERVER_CONN_HPP
#define __SERVER_CONN_HPP

#include <curlpp/Infos.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/cURLpp.hpp>
#include <SDL/SDL_image.h>
#include "json.hpp"
#include <variant>
#include <string>
#include <functional>

const std::string apiUrl = "http://koutarou.uy:4242/";

struct actorMessage {
    uint64_t timestamp;
    std::string msg;
};

struct actorInfo {
    std::array<int32_t, 2> position;
    bool enabled;
    uint64_t creation_time;
    std::string name;
    std::vector<actorMessage> messages;
};

struct serverStatus {
    std::map<std::string, actorInfo> actors;
    uint64_t world_frame;
};

struct actorStatus {
    uint64_t height;
    std::string type;
};

struct actorAction {
    std::string act;
    std::string args;
};

const actorAction upAction{"MOVE", "UP"};
const actorAction downAction{"MOVE", "DOWN"};
const actorAction leftAction{"MOVE", "LEFT"};
const actorAction rightAction{"MOVE", "RIGHT"};

const actorAction messageAction(std::string message);

struct actionResponse {
    uint64_t actor_id;
    actorAction action;
};

struct serverError {
    std::string error;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(actorAction, act, args)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(actionResponse, actor_id, action)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(actorMessage, timestamp, msg)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(actorInfo, position, enabled, creation_time, name, messages)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(serverError, error)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(serverStatus, actors, world_frame)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(actorStatus, height, type)


template<typename T>
std::variant<T, serverError> deserializeJson(const nlohmann::json &j) {
    if (j.contains("error")) return j.get<serverError>();
    else return j.get<T>();
}

std::variant<serverStatus, serverError> getServerStatus();

std::variant<actorStatus, serverError> getActorStatus(uint64_t actorId);

std::variant<std::shared_ptr<SDL_Surface>, serverError> getChunkMap(int x, int y);

uint64_t connect(std::string playerName);

std::variant<std::shared_ptr<SDL_Surface>, serverError>
downloadImage(const std::string imageUrl);

std::variant<actionResponse, serverError> actorDo(const actorAction &action, uint64_t actorId);

#endif