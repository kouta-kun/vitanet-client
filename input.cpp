//
// Created by kouta on 6/6/21.
//
#include "input.h"
#include <optional>
#include <codecvt>
#include <locale>

std::optional<std::shared_ptr<SDL_Joystick>> joystick{};
static const std::array<std::string, 3> rows{"qwertyuiop", "asdfghjkl", "zxcvbnm"};

std::string getInput(std::string dialogTitle, std::shared_ptr<SDL_Surface> screenSurface) {
    std::string message;

    SDL_Event event;
    SDL_EnableUNICODE(1);

    int charIndex = 0;
    int rowIndex = 0;

    bool writing = true;

    while (writing) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_KEYDOWN:
                    if (event.key.keysym.sym == SDLK_RETURN) {
                        writing = false;
                        break;
                    } else if (event.key.keysym.sym == SDLK_BACKSPACE) {
                        message = message.substr(0, message.size() - 1);
                    } else if ((event.key.keysym.unicode & 0xFF80) == 0) {
                        char ch = event.key.keysym.unicode & 0x7F;
                        message += ch;
                    }
                    break;
                case SDL_JOYBUTTONDOWN:
                    switch (event.jbutton.button) {
                        case 9: // RIGHT_ARROW
                            charIndex += 1;
                            charIndex %= rows[rowIndex].size();
                            break;
                        case 8: // UP_ARROW
                            rowIndex -= 1;
                            rowIndex %= rows.size();
                            break;
                        case 7: // LEFT_ARROW
                            charIndex -= 1;
                            charIndex %= rows[rowIndex].size();
                            break;
                        case 6: // DOWN_ARROW
                            rowIndex += 1;
                            rowIndex %= rows.size();
                            break;
                        case 2: // X_BUTTON
                            message += rows[rowIndex][charIndex];
                            break;
                        case 3: // SQ_BUTTON
                            message = message.substr(0, message.size() - 1);
                            break;
                        case 11: // START_BUTTON
                            writing = false;
                            break;
                    }
                    break;
            }
        }
        SDL_FillRect(screenSurface.get(), nullptr, SDL_MapRGB(screenSurface->format, 255, 255, 255));
        stringRGBA(screenSurface.get(), 0, 0, dialogTitle.c_str(), 255, 0, 0, 255);
        stringRGBA(screenSurface.get(), 0, 12, message.c_str(), 0, 0, 0, 255);
        if (joystick.has_value()) {
            for (int row = 0; row < rows.size(); row++) {
                for (int c = 0; c < rows[row].size(); c++) {
                    auto rowSect = SCREEN_W / rows[row].size();
                    characterRGBA(screenSurface.get(), (c * rowSect) + (rowSect / 2), SCREEN_H - 30 + (10 * row),
                                  rows[row][c], 0, 0, (row == rowIndex && charIndex == c) ? 0 : 255, 255);
                }
            }
        }
        SDL_Flip(screenSurface.get());
    }
    SDL_EnableUNICODE(0);
    return message;
}

bool handleInput(uint64_t userId, std::shared_ptr<SDL_Surface> screenSurf) {
    bool running = true;
    SDL_Event event;
    while (SDL_PollEvent(&event)) {  /* Loop until there are no events left on the queue */
        switch (event.type) {  /* Process the appropiate event type */
            case SDL_QUIT:
                running = false;
                break;
            case SDL_JOYBUTTONDOWN:
                switch (event.jbutton.button) {
                    case 9: // RIGHT_ARROW
                        actorDo(rightAction, userId);
                        break;
                    case 8: // UP_ARROW
                        actorDo(upAction, userId);
                        break;
                    case 7: // LEFT_ARROW
                        actorDo(leftAction, userId);
                        break;
                    case 6: // DOWN_ARROW
                        actorDo(downAction, userId);
                        break;
                    case 0: // TRIANGLE
                        auto msg = getInput("Insert message: ", screenSurf);
                        actorDo(messageAction(msg), userId);
                        break;
                }
                break;
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_w:
                        actorDo(upAction, userId);
                        break;
                    case SDLK_s:
                        actorDo(downAction, userId);
                        break;
                    case SDLK_a:
                        actorDo(leftAction, userId);
                        break;
                    case SDLK_d:
                        actorDo(rightAction, userId);
                        break;
                    case SDLK_t: {
                        auto msg = getInput("Insert message: ", screenSurf);
                        actorDo(messageAction(msg), userId);
                    }
                        break;
                }
                break;
        }
    }
    return running;
}


void inputInit() {
#ifdef __VITA__
    sceSysmoduleLoadModule(SCE_SYSMODULE_APPUTIL);
    sceSysmoduleLoadModule(SCE_SYSMODULE_IME);
    SDL_JoystickEventState(SDL_ENABLE);
    joystick = std::shared_ptr<SDL_Joystick>(SDL_JoystickOpen(0), SDL_JoystickClose);
    auto param = SceAppUtilInitParam{};
    auto bootParam = SceAppUtilBootParam{};
    sceAppUtilInit(&param, &bootParam);
    auto configParam = SceCommonDialogConfigParam{};
    sceCommonDialogSetConfigParam(&configParam);
#endif
}

void inputTerm() {
    joystick = decltype(joystick){};
}