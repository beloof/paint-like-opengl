#pragma once
#include <cstddef>
#include <deque>
#include <vector>

#include "canvas.h"

// Undo/redo for canvas edits: one entry per edit (a stroke, a clear).
//
// Each entry stores only the pixels of the rectangle that changed, as they were
// before the edit. Undo and redo swap that rectangle with the canvas, so a single
// buffer serves both directions and memory stays proportional to what was drawn.
class History
{
public:
    explicit History(std::size_t maxBytes = 128u * 1024u * 1024u,
                     std::size_t maxSteps = 200)
        : maxBytes(maxBytes), maxSteps(maxSteps) {}

    // Usage: begin(canvas); ...edit the canvas...; commit(canvas);
    void begin(const Canvas& canvas);
    void commit(const Canvas& canvas); // records what changed (nothing if nothing did)

    bool undo(Canvas& canvas);
    bool redo(Canvas& canvas);

    bool canUndo() const { return !undoStack.empty(); }
    bool canRedo() const { return !redoStack.empty(); }

    void clear();

private:
    struct Patch
    {
        int x, y, w, h;
        std::vector<unsigned char> data; // RGB of the rectangle, tightly packed
    };

    static void swapWithCanvas(Patch& patch, Canvas& canvas);
    void dropRedo();
    void trim();

    std::vector<unsigned char> before; // canvas as it was at begin()
    bool pending = false;

    std::deque<Patch> undoStack;
    std::deque<Patch> redoStack;
    std::size_t bytes = 0; // total size of both stacks
    std::size_t maxBytes;
    std::size_t maxSteps;
};
