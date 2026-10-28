/**
 * ==========================================================================
 * WORD BOGGLE GAME CLIENT
 * ==========================================================================
 */

// Global Game State
const GameState = {
    size: 4,
    difficulty: "medium",
    boardLetters: [],
    // Person A: Path and selection state
    pathStack: [],        // Sequence of cell indices [0..size^2 - 1]
    currentWord: "",      // String formed by current path
    isDragging: false,    // Whether user is currently drag-selecting
    foundWords: new Set(),
    score: 0,
    timeRemaining: 180,
    timerInterval: null
};

// ==========================================================================
// PERSON A: BOARD TILE DRAG/CLICK, PATH STACK & SUBMIT (/api/submit)
// ==========================================================================

const boardEl = document.getElementById("board");
const wordInput = document.getElementById("word-input");
const submitBtn = document.getElementById("btn-submit-word");
const clearBtn = document.getElementById("btn-clear-path");
const feedbackEl = document.getElementById("submission-feedback");

/**
 * Check if two 1D tile indices are 8-directional neighbors on an N x N grid
 */
function areNeighbors(idx1, idx2, size) {
    const r1 = Math.floor(idx1 / size);
    const c1 = idx1 % size;
    const r2 = Math.floor(idx2 / size);
    const c2 = idx2 % size;

    const rowDiff = Math.abs(r1 - r2);
    const colDiff = Math.abs(c1 - c2);

    return (rowDiff <= 1 && colDiff <= 1) && (idx1 !== idx2);
}

/**
 * Render the N x N board grid of letter tiles (Person A)
 */
function renderBoard(letters, size = 4) {
    GameState.size = size;
    GameState.boardLetters = letters;
    clearPathSelection();

    boardEl.innerHTML = "";
    boardEl.setAttribute("data-size", size);
    boardEl.style.setProperty("--grid-size", size);

    letters.forEach((letter, index) => {
        const tile = document.createElement("div");
        tile.className = "tile";
        tile.dataset.index = index;
        tile.dataset.letter = letter.toUpperCase();
        tile.textContent = letter.toUpperCase();

        // Pointer event listeners for click and drag selection
        tile.addEventListener("pointerdown", (e) => handleTilePointerDown(index, e));
        tile.addEventListener("pointerenter", (e) => handleTilePointerEnter(index, e));

        boardEl.appendChild(tile);
    });
}

/**
 * Handle initial pointer press on a tile
 */
function handleTilePointerDown(index, e) {
    GameState.isDragging = true;
    boardEl.setPointerCapture?.(e.pointerId);

    // If starting a new path or clicking first tile
    if (GameState.pathStack.length === 0 || !GameState.pathStack.includes(index)) {
        clearPathSelection();
        pushTileToPath(index);
    } else if (GameState.pathStack[GameState.pathStack.length - 1] === index) {
        // Clicking head again pops it (backtrack)
        popTileFromPath();
    }
}

/**
 * Handle pointer dragging over neighboring tiles
 */
function handleTilePointerEnter(index, e) {
    if (!GameState.isDragging) return;

    const stack = GameState.pathStack;
    const stackLen = stack.length;

    if (stackLen === 0) {
        pushTileToPath(index);
        return;
    }

    const lastIdx = stack[stackLen - 1];

    // If dragging back to previous tile, treat as backtrack (undo last step)
    if (stackLen >= 2 && stack[stackLen - 2] === index) {
        popTileFromPath();
        return;
    }

    // Otherwise, check if tile is adjacent and not already visited in path
    if (!stack.includes(index) && areNeighbors(lastIdx, index, GameState.size)) {
        pushTileToPath(index);
    }
}

// Global pointer up to end drag
window.addEventListener("pointerup", () => {
    if (GameState.isDragging) {
        GameState.isDragging = false;
    }
});

/**
 * Push tile index onto active path
 */
function pushTileToPath(index) {
    GameState.pathStack.push(index);
    updatePathVisuals();
}

/**
 * Pop the top tile from active path
 */
function popTileFromPath() {
    GameState.pathStack.pop();
    updatePathVisuals();
}

/**
 * Clear the current path stack and reset tile visual states
 */
function clearPathSelection() {
    GameState.pathStack = [];
    GameState.currentWord = "";
    if (wordInput) wordInput.value = "";

    document.querySelectorAll(".tile").forEach(tile => {
        tile.classList.remove("selected", "path-head", "flash-success", "highlight-path");
        const badge = tile.querySelector(".path-badge");
        if (badge) badge.remove();
    });
}

/**
 * Update visual highlights, badges, and the typing box
 */
function updatePathVisuals() {
    const stack = GameState.pathStack;
    let word = "";

    // Clear previous visual highlights
    document.querySelectorAll(".tile").forEach(tile => {
        tile.classList.remove("selected", "path-head");
        const badge = tile.querySelector(".path-badge");
        if (badge) badge.remove();
    });

    stack.forEach((cellIdx, stepIdx) => {
        const tile = boardEl.querySelector(`.tile[data-index="${cellIdx}"]`);
        if (!tile) return;

        tile.classList.add("selected");
        word += tile.dataset.letter;

        // Step number badge (1, 2, 3...)
        const badge = document.createElement("span");
        badge.className = "path-badge";
        badge.textContent = stepIdx + 1;
        tile.appendChild(badge);

        if (stepIdx === stack.length - 1) {
            tile.classList.add("path-head");
        }
    });

    GameState.currentWord = word;
    if (wordInput) {
        wordInput.value = word;
    }
}

/**
 * Briefly highlight a sequence of cell indices on the board (e.g. when word submitted)
 */
function highlightWordPath(pathIndices) {
    if (!pathIndices || pathIndices.length === 0) return;

    pathIndices.forEach(idx => {
        const tile = boardEl.querySelector(`.tile[data-index="${idx}"]`);
        if (tile) {
            tile.classList.add("highlight-path");
        }
    });

    setTimeout(() => {
        pathIndices.forEach(idx => {
            const tile = boardEl.querySelector(`.tile[data-index="${idx}"]`);
            if (tile) {
                tile.classList.remove("highlight-path");
            }
        });
    }, 1200);
}

/**
 * Show temporary feedback below the typing box
 */
function showFeedback(text, type = "success") {
    if (!feedbackEl) return;
    feedbackEl.textContent = text;
    feedbackEl.className = `feedback-msg ${type} show`;

    setTimeout(() => {
        feedbackEl.classList.remove("show");
    }, 2200);
}

/**
 * Shake the input box on invalid / duplicate submissions
 */
function shakeInput() {
    if (!wordInput) return;
    wordInput.classList.remove("shake");
    // Trigger reflow
    void wordInput.offsetWidth;
    wordInput.classList.add("shake");
}

/**
 * Submit word to backend (/api/submit) - Person A
 */
async function submitWord() {
    const rawWord = wordInput ? wordInput.value.trim().toUpperCase() : GameState.currentWord;
    if (!rawWord || rawWord.length < 3) {
        showFeedback("Word must be at least 3 letters", "error");
        shakeInput();
        return;
    }

    try {
        const response = await fetch("/api/submit", {
            method: "POST",
            headers: { "Content-Type": "application/json" },
            body: JSON.stringify({ word: rawWord })
        });

        if (!response.ok) {
            throw new Error(`Server returned HTTP ${response.status}`);
        }

        const data = await response.json();

        if (data.status === "VALID") {
            showFeedback(`+${data.pointsAwarded} pts! "${data.word}"`, "success");

            // Update score & count displays
            GameState.score = data.totalScore;
            updateHUD(data.totalScore, data.foundCount);

            // Add chip to drawer
            addFoundWordChip(data.word, data.pointsAwarded);

            // Flash tiles or highlight path returned by backend
            if (data.path && data.path.length > 0) {
                highlightWordPath(data.path);
            }

            clearPathSelection();
        } else if (data.status === "DUPLICATE") {
            showFeedback(`Already found "${data.word}"`, "warning");
            shakeInput();
            if (data.path) highlightWordPath(data.path);
        } else {
            showFeedback(data.message || `"${rawWord}" is not valid`, "error");
            shakeInput();
        }
    } catch (err) {
        console.error("Submission error:", err);
        showFeedback("Connection error to server", "error");
    }
}

// Person A Event Listeners
if (submitBtn) {
    submitBtn.addEventListener("click", submitWord);
}

if (clearBtn) {
    clearBtn.addEventListener("click", clearPathSelection);
}

if (wordInput) {
    wordInput.addEventListener("keydown", (e) => {
        if (e.key === "Enter") {
            submitWord();
        } else if (e.key === "Escape") {
            clearPathSelection();
        }
    });

    // Synchronize manual typing with currentWord
    wordInput.addEventListener("input", (e) => {
        GameState.currentWord = e.target.value.toUpperCase();
    });
}

// ==========================================================================
// PERSON B: HUD, SETUP API (/api/new), TIMER, PROGRESS RINGS & END SCREEN
// ==========================================================================

function updateHUD(score, foundCount) {
    const scoreVal = document.getElementById("score-display");
    const foundVal = document.getElementById("found-count-display");
    const drawerCount = document.getElementById("drawer-found-count");

    if (scoreVal) scoreVal.textContent = score;
    if (foundVal) foundVal.textContent = foundCount;
    if (drawerCount) drawerCount.textContent = foundCount;
}

function addFoundWordChip(word, points) {
    const container = document.getElementById("found-words-chips");
    if (!container) return;

    const chip = document.createElement("span");
    chip.className = "btn btn-secondary";
    chip.style.cssText = "padding: 0.25rem 0.6rem; font-size: 0.85rem; cursor: pointer;";
    chip.textContent = `${word} (${points})`;

    // Clicking chip highlights its path on board
    chip.addEventListener("click", async () => {
        try {
            const resp = await fetch("/api/submit", {
                method: "POST",
                headers: { "Content-Type": "application/json" },
                body: JSON.stringify({ word: word })
            });
            const d = await resp.json();
            if (d.path) highlightWordPath(d.path);
        } catch (_) {}
    });

    container.prepend(chip);
}

/**
 * Initialize game by calling /api/new
 */
async function startNewGame(size = 4, difficulty = "medium") {
    try {
        const resp = await fetch("/api/new", {
            method: "POST",
            headers: { "Content-Type": "application/json" },
            body: JSON.stringify({ size: parseInt(size), difficulty: difficulty })
        });

        if (!resp.ok) throw new Error("Failed to start new game");
        const data = await resp.json();

        // Render board using Person A's board generator
        renderBoard(data.board, data.size);
        updateHUD(0, 0);

        console.log(`[Game] Started new ${data.size}x${data.size} game (${data.difficulty}). Total words: ${data.totalWords}`);
    } catch (err) {
        console.warn("Could not connect to /api/new, using demo board:", err);
        // Fallback demo board for offline testing
        const demoLetters = ["C", "A", "T", "S", "R", "E", "A", "D", "L", "I", "N", "E", "B", "O", "A", "T"];
        renderBoard(demoLetters, 4);
    }
}

// Person B Setup Button Listener
const startBtn = document.getElementById("btn-start-game");
if (startBtn) {
    startBtn.addEventListener("click", () => {
        const size = document.getElementById("select-size")?.value || 4;
        const diff = document.getElementById("select-difficulty")?.value || "medium";

        document.getElementById("screen-setup")?.classList.remove("active");
        document.getElementById("screen-game")?.classList.add("active");

        startNewGame(size, diff);
    });
}

// Auto-initialize demo / default board on page load
window.addEventListener("DOMContentLoaded", () => {
    startNewGame(4, "medium");
});

