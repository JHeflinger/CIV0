#include "teams.h"
#include "core/logger.h"

char g_CurrentID = 'K';

char GetRotatingID() {
    g_CurrentID += 1;
    if (g_CurrentID > 'Z') g_CurrentID = 'A';
    return g_CurrentID;
}

Color GetIDColor(char id) {
    int section = 1 + (int)(((float)(id - 'A'))/4.3f);
    float intermediate = ((float)(((int)(id - 'A'))%6)) / 6.0f;
    if (section > 6) section = 6;
    Color color;
    color.a = 255;
    int falling = section%2 == 0;
    intermediate = (falling ? (255.0f*(1.0f - intermediate)) : (255.0f*intermediate));
    // constant
    color.r = (section == 1 || section == 6) ? 255 : 0;
    color.g = (section == 2 || section == 3) ? 255 : 0;
    color.b = (section == 4 || section == 5) ? 255 : 0;
    //intermediate
    color.r = (section == 2 || section == 5) ? (unsigned char)intermediate : color.r;
    color.g = (section == 1 || section == 4) ? (unsigned char)intermediate : color.g;
    color.b = (section == 3 || section == 6) ? (unsigned char)intermediate : color.b;

    return color;
}