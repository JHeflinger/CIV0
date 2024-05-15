#include "startscene.h"
#include "raylib.h"
#include "data/gconfig.h"
#include "core/logger.h"
#include "network/network.h"
#include "scenes/corescene.h"
#include "core/game.h"

int state = 0; // 0 intial, 1 is host config, 2 is client config
int focus = 0; // 0 nothing, 1 port, 2 boardwidth, 3 boardheight, 4 joinport, 5 ip
int m_pressed = FALSE;
char port_buffer[5];
char width_buffer[6];
char height_buffer[6];
char ip_buffer[15];

int InBox(Vector2 coordinate, int x, int y, int w, int h) {
    return coordinate.x >= x && coordinate.x <= x+w && coordinate.y >= y && coordinate.y <= y+h;
}

void UpdateStartScene() { 
    int xplus = (GetScreenWidth() - 300) / 2;
    int yplus = (GetScreenHeight() - 50) / 2;
    Vector2 m_coords = GetMousePosition();
    if (state == 0) {
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && !m_pressed) {
            m_pressed = TRUE;
            if (InBox(m_coords, 0 + xplus, 200 + yplus, 300, 60)) {
                exit(0);
            }
            if (InBox(m_coords, 0 + xplus, 0 + yplus, 300, 60)) {
                state = 1;
                focus = 0;
            }
            if (InBox(m_coords, 0 + xplus, 100 + yplus, 300, 60)) {
                state = 2;
                focus = 0;
            }
        }
    }
    if (state == 1) {
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && !m_pressed) {
            m_pressed = TRUE;
            if (InBox(m_coords, 0 + xplus, 230 + yplus, 120, 60)) {
                state = 0;
            }
            if (InBox(m_coords, 0 + xplus, 30 + yplus, 300, 40)) {
                focus = 1;
            }
            if (InBox(m_coords, 0 + xplus, 140 + yplus, 110, 40)) {
                focus = 2;
            }
            if (InBox(m_coords, 190 + xplus, 140 + yplus, 110, 40)) {
                focus = 3;
            }
            if (InBox(m_coords, 180 + xplus, 230 + yplus, 120, 60)) {
                uint8_t address[4];
                for (int i = 0; i < 4; i++) address[i] = 0;
                uint16_t port = 0;
                EZN_STATUS status = ezn_str_to_port(&port, port_buffer);
                int width = atoi(width_buffer);
                int height = atoi(height_buffer);
                if (port <= 1024) status = EZN_ERROR;
                if (status == EZN_NONE) {
                    SetupBoard(width, height);
                    ConfigureBoard(width, height);
                    ConfigureNetwork(address, port);
                    InitializeNetwork(SERVER);
                    ChangeScene(CORE);
                }
            }	
        }
        if (focus == 1) {
            int interchar = GetCharPressed();
            if (IsKeyPressed(KEY_BACKSPACE)) {
                for (int i = 4; i >= 0; i--) {
                    if (port_buffer[i] != '\0') {
                        port_buffer[i] = '\0';
                        break;
                    }
                }
            }
            if (interchar != 0) {
                char newchar = (char)interchar;
                if (newchar >= '0' && newchar <= '9') {
                    for (int i = 0; i < 5; i++) {
                        if (port_buffer[i] == '\0') {
                            port_buffer[i] = newchar;
                            break;
                        }
                    }
                }
            }
        }
        if (focus == 2) {
            int interchar = GetCharPressed();
            if (IsKeyPressed(KEY_BACKSPACE)) {
                for (int i = 5; i >= 0; i--) {
                    if (width_buffer[i] != '\0') {
                        width_buffer[i] = '\0';
                        break;
                    }
                }
            }
            if (interchar != 0) {
                char newchar = (char)interchar;
                if (newchar >= '0' && newchar <= '9') {
                    for (int i = 0; i < 6; i++) {
                        if (width_buffer[i] == '\0') {
                            width_buffer[i] = newchar;
                            break;
                        }
                    }
                }
            }
        }
        if (focus == 3) {
            int interchar = GetCharPressed();
            if (IsKeyPressed(KEY_BACKSPACE)) {
                for (int i = 5; i >= 0; i--) {
                    if (height_buffer[i] != '\0') {
                        height_buffer[i] = '\0';
                        break;
                    }
                }
            }
            if (interchar != 0) {
                char newchar = (char)interchar;
                if (newchar >= '0' && newchar <= '9') {
                    for (int i = 0; i < 6; i++) {
                        if (height_buffer[i] == '\0') {
                            height_buffer[i] = newchar;
                            break;
                        }
                    }
                }
            }
        }
    }
    if (state == 2) {
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && !m_pressed) {
            m_pressed = TRUE;
            if (InBox(m_coords, 0 + xplus, 230 + yplus, 120, 60)) {
                state = 0;
            }
            if (InBox(m_coords, 0 + xplus, 30 + yplus, 300, 40)) {
                focus = 4;
            }
            if (InBox(m_coords, 0 + xplus, 140 + yplus, 300, 40)) {
                focus = 5;
            }
            if (InBox(m_coords, 180 + xplus, 230 + yplus, 120, 60)) {
                uint8_t address[4];
                uint16_t port = 0;
                EZN_STATUS status = ezn_str_to_ipaddr(address, ip_buffer);
                if (status == EZN_NONE)
                    ezn_str_to_port(&port, port_buffer);
                if (port <= 1024) status = EZN_ERROR;
                if (status == EZN_NONE) {
                    ConfigureNetwork(address, port);
                    InitializeNetwork(CLIENT);
                    ChangeScene(CORE);
                }
            }	
        }
        if (focus == 4) {
            int interchar = GetCharPressed();
            if (IsKeyPressed(KEY_BACKSPACE)) {
                for (int i = 4; i >= 0; i--) {
                    if (port_buffer[i] != '\0') {
                        port_buffer[i] = '\0';
                        break;
                    }
                }
            }
            if (interchar != 0) {
                char newchar = (char)interchar;
                if (newchar >= '0' && newchar <= '9') {
                    for (int i = 0; i < 5; i++) {
                        if (port_buffer[i] == '\0') {
                            port_buffer[i] = newchar;
                            break;
                        }
                    }
                }
            }
        }
        if (focus == 5) {
            int interchar = GetCharPressed();
            if (IsKeyPressed(KEY_BACKSPACE)) {
                for (int i = 14; i >= 0; i--) {
                    if (ip_buffer[i] != '\0') {
                        ip_buffer[i] = '\0';
                        break;
                    }
                }
            }
            if (interchar != 0) {
                char newchar = (char)interchar;
                if ((newchar >= '0' && newchar <= '9') || newchar == '.') {
                    for (int i = 0; i < 15; i++) {
                        if (ip_buffer[i] == '\0') {
                            ip_buffer[i] = newchar;
                            break;
                        }
                    }
                }
            }
        }
    }
    if (!IsMouseButtonDown(MOUSE_BUTTON_LEFT)) m_pressed = FALSE;
}

int CalculateLength(char* str) {
    int width = 0;
    int ind = 0;
    while (str[ind] != '\0') {
        if (str[ind] == '.') {
            width += 2;
        } else {
            width += 16;
        }
        ind++;
    }
    return width;
}

void DrawStartScene() {
    BeginDrawing();

    ClearBackground(CLITERAL(Color){ 40, 40, 40, 255 });

    DrawText("Jason Heflinger's", (GetScreenWidth() - 260) / 2, (GetScreenHeight() - 400) / 2, 30, RAYWHITE);
    DrawText("CIVILIZATION 0", (GetScreenWidth() - 740) / 2, (GetScreenHeight() - 300) / 2, 96, RAYWHITE);

    int xplus = (GetScreenWidth() - 300) / 2;
    int yplus = (GetScreenHeight() - 50) / 2;
    if (state == 0) {
        DrawRectangle(10 + xplus, 10 + yplus, 300, 60, DARKGRAY);
        DrawRectangle(0 + xplus, 0 + yplus, 300, 60, GRAY);
        DrawText("HOST", 100 + xplus, 10 + yplus, 38, RAYWHITE);

        DrawRectangle(10 + xplus, 110 + yplus, 300, 60, DARKGRAY);
        DrawRectangle(0 + xplus, 100 + yplus, 300, 60, GRAY);
        DrawText("JOIN", 105 + xplus, 110 + yplus, 38, RAYWHITE);

        DrawRectangle(10 + xplus, 210 + yplus, 300, 60, DARKGRAY);
        DrawRectangle(0 + xplus, 200 + yplus, 300, 60, GRAY);
        DrawText("QUIT", 105 + xplus, 210 + yplus, 38, RAYWHITE);
    }
    if (state == 1) {
        DrawText("PORT", 110 + xplus, -10 + yplus, 30, RAYWHITE);
        DrawRectangle(0 + xplus, 30 + yplus, 300, 40, LIGHTGRAY);
        DrawText(port_buffer, (GetScreenWidth()/2) - (CalculateLength(port_buffer)/2), 35 + yplus, 30, DARKGRAY);
        DrawText("BOARD SIZE", 55 + xplus, 90 + yplus, 30, RAYWHITE);
        DrawRectangle(0 + xplus, 140 + yplus, 110, 40, LIGHTGRAY);
        DrawText(width_buffer, (GetScreenWidth()/2) - 95 - (CalculateLength(width_buffer)/2), 145 + yplus, 30, DARKGRAY);
        DrawRectangle(190 + xplus, 140 + yplus, 110, 40, LIGHTGRAY);
        DrawText(height_buffer, (GetScreenWidth()/2) + 95 - (CalculateLength(height_buffer)/2), 145 + yplus, 30, DARKGRAY);
        DrawText("by", 135 + xplus, 150 + yplus, 30, RAYWHITE);
        DrawRectangle(10 + xplus, 240 + yplus, 120, 60, DARKGRAY);
        DrawRectangle(190 + xplus, 240 + yplus, 120, 60, DARKGRAY);
        DrawRectangle(0 + xplus, 230 + yplus, 120, 60, GRAY);
        DrawRectangle(180 + xplus, 230 + yplus, 120, 60, GRAY);
        DrawText("BACK", 10 + xplus, 240 + yplus, 38, RAYWHITE);
        DrawText("GO", 215 + xplus, 240 + yplus, 38, RAYWHITE);
    }
    if (state == 2) {
        DrawText("PORT", 110 + xplus, -10 + yplus, 30, RAYWHITE);
        DrawRectangle(0 + xplus, 30 + yplus, 300, 40, LIGHTGRAY);
        DrawText(port_buffer, (GetScreenWidth()/2) - (CalculateLength(port_buffer)/2), 35 + yplus, 30, DARKGRAY);
        DrawText("IP", 135 + xplus, 90 + yplus, 30, RAYWHITE);
        DrawRectangle(0 + xplus, 140 + yplus, 300, 40, LIGHTGRAY);
        DrawText(ip_buffer, (GetScreenWidth()/2) - (CalculateLength(ip_buffer)/2), 145 + yplus, 30, DARKGRAY);
        DrawRectangle(10 + xplus, 240 + yplus, 120, 60, DARKGRAY);
        DrawRectangle(190 + xplus, 240 + yplus, 120, 60, DARKGRAY);
        DrawRectangle(0 + xplus, 230 + yplus, 120, 60, GRAY);
        DrawRectangle(180 + xplus, 230 + yplus, 120, 60, GRAY);
        DrawText("BACK", 10 + xplus, 240 + yplus, 38, RAYWHITE);
        DrawText("GO", 215 + xplus, 240 + yplus, 38, RAYWHITE);
    }

    EndDrawing();
}