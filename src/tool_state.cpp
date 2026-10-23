#include "tool_state.h"

const unsigned char palette[paletteCount][3] = {
    {0, 0, 0},       {128, 128, 128}, {136, 84, 43},  {230, 40, 40},
    {255, 150, 0},   {250, 220, 0},   {0, 160, 0},    {0, 190, 190},
    {0, 0, 255},     {20, 60, 120},   {130, 50, 200}, {255, 105, 180},
};

static const unsigned char white[3] = { 255, 255, 255 };

const unsigned char* ToolState::color() const
{
    return eraser ? white : palette[colorIndex];
}
