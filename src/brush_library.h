#pragma once
#include <memory>
#include <string>
#include <vector>

#include "brush.h"

constexpr int brushThumbSize = 40;

// Owns every available brush plus an RGB preview of each (for the picker).
class BrushLibrary
{
public:
    // Loads the default PNG brushes from `dir` and adds a procedural SoftBrush.
    void loadDefaults(const std::string& dir);

    int count() const { return (int)brushes.size(); }
    const Brush& get(int i) const { return *brushes[i]; }
    const std::vector<unsigned char>& thumb(int i) const { return thumbs[i]; }

private:
    void add(std::unique_ptr<Brush> brush);

    std::vector<std::unique_ptr<Brush>> brushes;
    std::vector<std::vector<unsigned char>> thumbs;
};
