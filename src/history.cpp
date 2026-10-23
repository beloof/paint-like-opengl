#include "history.h"

#include <algorithm>
#include <cstring>

void History::begin(const Canvas& canvas)
{
    if (pending) commit(canvas); // an earlier edit was never committed; keep it
    before = canvas.getPixels();
    pending = true;
}

void History::commit(const Canvas& canvas)
{
    if (!pending) return;
    pending = false;

    const std::vector<unsigned char>& now = canvas.getPixels();
    if (before.size() != now.size()) return;

    const int w = canvas.width();
    const int h = canvas.height();
    const std::size_t stride = (std::size_t)w * 3;

    // Smallest rectangle containing every pixel that differs.
    int minX = w, minY = h, maxX = -1, maxY = -1;
    for (int y = 0; y < h; y++)
    {
        const unsigned char* a = &before[y * stride];
        const unsigned char* b = &now[y * stride];
        if (std::memcmp(a, b, stride) == 0) continue;

        minY = std::min(minY, y);
        maxY = y;
        for (int x = 0; x < w; x++)
        {
            if (a[x * 3] != b[x * 3] || a[x * 3 + 1] != b[x * 3 + 1] || a[x * 3 + 2] != b[x * 3 + 2])
            {
                minX = std::min(minX, x);
                maxX = std::max(maxX, x);
            }
        }
    }
    if (maxX < 0) return; // nothing changed: no history entry

    Patch patch;
    patch.x = minX;
    patch.y = minY;
    patch.w = maxX - minX + 1;
    patch.h = maxY - minY + 1;
    patch.data.resize((std::size_t)patch.w * patch.h * 3);
    for (int row = 0; row < patch.h; row++)
    {
        std::memcpy(&patch.data[(std::size_t)row * patch.w * 3],
                    &before[((std::size_t)(patch.y + row) * w + patch.x) * 3],
                    (std::size_t)patch.w * 3);
    }

    dropRedo(); // a new edit makes the undone ones unreachable
    bytes += patch.data.size();
    undoStack.push_back(std::move(patch));
    trim();
}

void History::swapWithCanvas(Patch& patch, Canvas& canvas)
{
    std::vector<unsigned char> current = canvas.readRect(patch.x, patch.y, patch.w, patch.h);
    canvas.writeRect(patch.x, patch.y, patch.w, patch.h, patch.data);
    patch.data.swap(current);
}

bool History::undo(Canvas& canvas)
{
    if (undoStack.empty()) return false;
    Patch patch = std::move(undoStack.back());
    undoStack.pop_back();
    swapWithCanvas(patch, canvas);
    redoStack.push_back(std::move(patch));
    return true;
}

bool History::redo(Canvas& canvas)
{
    if (redoStack.empty()) return false;
    Patch patch = std::move(redoStack.back());
    redoStack.pop_back();
    swapWithCanvas(patch, canvas);
    undoStack.push_back(std::move(patch));
    return true;
}

void History::dropRedo()
{
    for (const Patch& p : redoStack)
        bytes -= p.data.size();
    redoStack.clear();
}

// Forget the oldest edits once over the step or memory budget.
void History::trim()
{
    while (!undoStack.empty() && (undoStack.size() > maxSteps || bytes > maxBytes))
    {
        bytes -= undoStack.front().data.size();
        undoStack.pop_front();
    }
}

void History::clear()
{
    undoStack.clear();
    redoStack.clear();
    before.clear();
    bytes = 0;
    pending = false;
}
