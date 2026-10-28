/**
 * WORD BOGGLE GAME CLIENT
 * Modular Client-Side Architecture for ADSL Word Boggle Project
 * 
 * Person B:
 *   - Setup screen (Difficulty & Board Size selection)
 *   - Timer countdown (with auto-end at 00:00)
 *   - Score & words found HUD
 *   - Circular progress rings by word length
 *   - End game screen (AVL-sorted words, found vs missed highlights)
 *   - Print functionality
 * 
 * Person A:
 *   - Board rendering & tile selection (drag & click adjacent neighbors)
 *   - Path highlighting trail & backtracking
 *   - Textbox typing & submit
 */

// Global Game State
const state = {
  difficulty: 'medium',
  size: 4,
  board: [],
  timerSeconds: 240,
  timeRemaining: 240,
  timerInterval: null,
  totalScore: 0,
  foundCount: 0,
  totalWords: 0,
  counts: {},
  foundWords: new Set(),
  endWords: [],
  currentPath: [], // Array of tile indices: [0, 5, 9]
  isSelecting: false
};

// DOM Elements
const $ = (id) => document.getElementById(id);

// --- Initialization ---
document.addEventListener('DOMContentLoaded', () => {
  checkServerHealth();
  initSetupScreen();
  initGameScreen();
  initEndScreen();
});

// ============================================================
// 1. SERVER CONNECTION & HEALTH CHECK
// ============================================================
async function checkServerHealth() {
  const badge = $('server-status');
  try {
    const res = await fetch('/api/ping');
    if (res.ok) {
      badge.textContent = 'Server: Online';
      badge.className = 'server-badge online';
    } else {
      throw new Error();
    }
  } catch (err) {
    badge.textContent = 'Server: Offline';
    badge.className = 'server-badge offline';
  }
}

// ============================================================
// 2. SETUP SCREEN (Person B)
// ============================================================
function initSetupScreen() {
  // Difficulty Selection Pills
  $('difficulty-pills').querySelectorAll('.pill').forEach(btn => {
    btn.addEventListener('click', () => {
      $('difficulty-pills').querySelectorAll('.pill').forEach(p => p.classList.remove('active'));
      btn.classList.add('active');
      state.difficulty = btn.dataset.diff;
    });
  });

  // Size Selection Pills
  $('size-pills').querySelectorAll('.pill').forEach(btn => {
    btn.addEventListener('click', () => {
      $('size-pills').querySelectorAll('.pill').forEach(p => p.classList.remove('active'));
      btn.classList.add('active');
      state.size = parseInt(btn.dataset.size, 10);
    });
  });

  // Start Game Button
  $('btn-start').addEventListener('click', startNewGame);
}

// Start New Game via API POST /api/new
async function startNewGame() {
  try {
    const res = await fetch('/api/new', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({
        size: state.size,
        difficulty: state.difficulty
      })
    });

    if (!res.ok) throw new Error('Failed to create new game');

    const data = await res.json();

    // Populate state from C++ backend response
    state.board = data.board;
    state.size = data.size;
    state.counts = data.counts || {};
    state.totalWords = data.totalWords || 0;
    state.timerSeconds = data.timerSeconds || 180;
    state.timeRemaining = state.timerSeconds;
    state.totalScore = 0;
    state.foundCount = 0;
    state.foundWords.clear();
    state.currentPath = [];

    // Switch views
    switchScreen('screen-game');

    // Initialize HUD and Board
    renderBoard(state.board, state.size);
    renderProgressRings(state.counts);
    updateHUD();
    startTimer();
    clearFeedback();
    $('found-words-chips').innerHTML = '';
    $('word-input').value = '';
    $('word-input').focus();

  } catch (err) {
    alert('Error connecting to C++ server. Please make sure boggle_server.exe is running on port 8080.');
  }
}

// ============================================================
// 3. TIMER & HUD (Person B)
// ============================================================
function startTimer() {
  if (state.timerInterval) clearInterval(state.timerInterval);

  updateTimerDisplay();

  state.timerInterval = setInterval(() => {
    state.timeRemaining--;
    updateTimerDisplay();

    if (state.timeRemaining <= 0) {
      clearInterval(state.timerInterval);
      endGame();
    }
  }, 1000);
}

function updateTimerDisplay() {
  const mins = Math.floor(state.timeRemaining / 60);
  const secs = state.timeRemaining % 60;
  const display = `${String(mins).padStart(2, '0')}:${String(secs).padStart(2, '0')}`;
  
  const timerElem = $('hud-timer');
  const timerBox = $('timer-box');
  
  timerElem.textContent = display;

  if (state.timeRemaining <= 30 && state.timeRemaining > 0) {
    timerBox.classList.add('urgent');
  } else {
    timerBox.classList.remove('urgent');
  }
}

function updateHUD() {
  $('hud-score').textContent = state.totalScore;
  $('hud-words').textContent = `${state.foundCount} / ${state.totalWords}`;
}

// ============================================================
// 4. CIRCULAR PROGRESS RINGS BY LENGTH (Person B)
// ============================================================
function renderProgressRings(counts) {
  const container = $('rings-container');
  container.innerHTML = '';

  const radius = 18;
  const circumference = 2 * Math.PI * radius;

  // Buckets: 3, 4, 5, 6, 7, 8+
  const lengths = [3, 4, 5, 6, 7, 8];

  lengths.forEach(len => {
    const pair = counts[String(len)] || [0, 0];
    const found = pair[0];
    const total = pair[1];

    const percent = total > 0 ? (found / total) : 0;
    const strokeDashoffset = circumference - (percent * circumference);
    const isComplete = total > 0 && found >= total;

    const wrapper = document.createElement('div');
    wrapper.className = 'ring-wrapper';
    wrapper.id = `ring-wrapper-${len}`;

    wrapper.innerHTML = `
      <div class="ring-svg-box">
        <svg width="44" height="44" viewBox="0 0 44 44">
          <circle class="ring-bg-circle" cx="22" cy="22" r="${radius}"></circle>
          <circle class="ring-fill-circle ${isComplete ? 'complete' : ''}"
                  id="ring-circle-${len}"
                  cx="22" cy="22" r="${radius}"
                  stroke-dasharray="${circumference}"
                  stroke-dashoffset="${strokeDashoffset}">
          </circle>
        </svg>
        <span class="ring-ratio-text" id="ring-text-${len}">${found}/${total}</span>
      </div>
      <span class="ring-length-label">${len}${len === 8 ? '+' : ''}L</span>
    `;

    container.appendChild(wrapper);
  });
}

function updateProgressRings(counts) {
  const radius = 18;
  const circumference = 2 * Math.PI * radius;

  for (let len = 3; len <= 8; ++len) {
    const pair = counts[String(len)] || [0, 0];
    const found = pair[0];
    const total = pair[1];

    const circle = $(`ring-circle-${len}`);
    const text = $(`ring-text-${len}`);

    if (circle && text) {
      const percent = total > 0 ? (found / total) : 0;
      const strokeDashoffset = circumference - (percent * circumference);
      circle.style.strokeDashoffset = strokeDashoffset;
      text.textContent = `${found}/${total}`;

      if (total > 0 && found >= total) {
        circle.classList.add('complete');
      } else {
        circle.classList.remove('complete');
      }
    }
  }
}

// ============================================================
// 5. INTERACTIVE BOARD & TILE SELECTION (Person A)
// ============================================================
function renderBoard(letters, size) {
  const boardElem = $('board');
  boardElem.className = `boggle-grid grid-${size}`;
  boardElem.innerHTML = '';

  letters.forEach((char, idx) => {
    const tile = document.createElement('div');
    tile.className = 'tile';
    tile.textContent = char;
    tile.dataset.index = idx;
    tile.dataset.row = Math.floor(idx / size);
    tile.dataset.col = idx % size;

    // Pointer events for drag/click selection
    tile.addEventListener('pointerdown', (e) => onTilePointerDown(idx, e));
    tile.addEventListener('pointerenter', () => onTilePointerEnter(idx));

    boardElem.appendChild(tile);
  });
}

// Check 8-directional adjacency
function isAdjacent(idx1, idx2, size) {
  const r1 = Math.floor(idx1 / size);
  const c1 = idx1 % size;
  const r2 = Math.floor(idx2 / size);
  const c2 = idx2 % size;

  return Math.abs(r1 - r2) <= 1 && Math.abs(c1 - c2) <= 1 && idx1 !== idx2;
}

function onTilePointerDown(index, event) {
  event.preventDefault();
  state.isSelecting = true;
  state.currentPath = [index];
  updatePathHighlights();
  updateInputFromPath();
}

function onTilePointerEnter(index) {
  if (!state.isSelecting) return;

  const path = state.currentPath;
  const lastIndex = path[path.length - 1];

  // Going back / backtracking: if user drags onto the second-to-last tile
  if (path.length >= 2 && path[path.length - 2] === index) {
    path.pop();
    updatePathHighlights();
    updateInputFromPath();
    return;
  }

  // Already visited in current word
  if (path.includes(index)) return;

  // Check adjacency
  if (isAdjacent(lastIndex, index, state.size)) {
    path.push(index);
    updatePathHighlights();
    updateInputFromPath();
  }
}

window.addEventListener('pointerup', () => {
  if (state.isSelecting) {
    state.isSelecting = false;
    // If path formed a word >= 3 letters, auto-submit
    const word = getWordFromPath();
    if (word.length >= 3) {
      submitWord(word);
    }
    clearSelection();
  }
});

function updatePathHighlights() {
  const tiles = $('board').querySelectorAll('.tile');
  tiles.forEach(t => {
    const idx = parseInt(t.dataset.index, 10);
    if (state.currentPath.includes(idx)) {
      t.classList.add('selected');
    } else {
      t.classList.remove('selected', 'path-active');
    }
  });
}

function clearSelection() {
  state.currentPath = [];
  updatePathHighlights();
}

function getWordFromPath() {
  return state.currentPath.map(idx => state.board[idx]).join('').toLowerCase();
}

function updateInputFromPath() {
  $('word-input').value = getWordFromPath().toUpperCase();
}

// ============================================================
// 6. TYPING INPUT & SUBMISSION (Person A)
// ============================================================
function initGameScreen() {
  // Submit button
  $('btn-submit').addEventListener('click', () => {
    const word = $('word-input').value.trim();
    if (word) submitWord(word);
  });

  // Enter key submission
  $('word-input').addEventListener('keydown', (e) => {
    if (e.key === 'Enter') {
      const word = $('word-input').value.trim();
      if (word) submitWord(word);
    }
  });

  // Clear button
  $('btn-clear').addEventListener('click', () => {
    $('word-input').value = '';
    clearSelection();
    clearFeedback();
  });

  // Early end game button
  $('btn-end-early').addEventListener('click', () => {
    if (confirm('Are you sure you want to end the game now?')) {
      if (state.timerInterval) clearInterval(state.timerInterval);
      endGame();
    }
  });
}

async function submitWord(word) {
  try {
    const res = await fetch('/api/submit', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ word: word })
    });

    if (!res.ok) throw new Error('Submission error');

    const data = await res.json();

    if (data.status === 'VALID') {
      showFeedback(`+${data.pointsAwarded} pts! "${data.word.toUpperCase()}"`, 'valid');
      state.totalScore = data.totalScore;
      state.foundCount = data.foundCount;
      state.foundWords.add(data.word.toLowerCase());

      // Highlight path on board if returned by backend
      if (data.path && data.path.length > 0) {
        highlightCellPath(data.path);
      }

      // Add to found chips preview
      addWordChip(data.word.toUpperCase());

      // Update HUD & Progress rings
      updateHUD();
      if (data.counts) {
        state.counts = data.counts;
        updateProgressRings(data.counts);
      }

    } else if (data.status === 'DUPLICATE') {
      showFeedback(`Already found "${data.word.toUpperCase()}"`, 'duplicate');
    } else {
      showFeedback(`"${data.word.toUpperCase()}" is not on this board`, 'invalid');
    }

    $('word-input').value = '';
    clearSelection();

  } catch (err) {
    showFeedback('Error submitting word', 'invalid');
  }
}

function highlightCellPath(path) {
  const tiles = $('board').querySelectorAll('.tile');
  path.forEach(idx => {
    if (tiles[idx]) tiles[idx].classList.add('path-active');
  });

  setTimeout(() => {
    path.forEach(idx => {
      if (tiles[idx]) tiles[idx].classList.remove('path-active');
    });
  }, 1200);
}

function addWordChip(word) {
  const chip = document.createElement('span');
  chip.className = 'word-chip';
  chip.textContent = word;
  $('found-words-chips').prepend(chip);
}

function showFeedback(msg, type) {
  const bar = $('feedback-bar');
  bar.textContent = msg;
  bar.className = `feedback-bar ${type}`;
}

function clearFeedback() {
  const bar = $('feedback-bar');
  bar.className = 'feedback-bar hide';
  bar.textContent = '';
}

// ============================================================
// 7. END GAME SCREEN (Person B)
// ============================================================
function initEndScreen() {
  // Play Again Button
  $('btn-play-again').addEventListener('click', () => {
    switchScreen('screen-setup');
  });

  // Print Button
  $('btn-print').addEventListener('click', () => {
    window.print();
  });

  // Filter Tabs
  $('screen-end').querySelectorAll('.tab').forEach(tab => {
    tab.addEventListener('click', () => {
      $('screen-end').querySelectorAll('.tab').forEach(t => t.classList.remove('active'));
      tab.classList.add('active');
      filterEndWords(tab.dataset.filter);
    });
  });
}

async function endGame() {
  if (state.timerInterval) clearInterval(state.timerInterval);

  try {
    const res = await fetch('/api/end');
    if (!res.ok) throw new Error('Failed to retrieve end game statistics');

    const data = await res.json();
    state.endWords = data.words || [];

    // Render Stats
    $('end-score').textContent = data.totalScore;
    $('end-found-count').textContent = `${data.foundCount} / ${data.totalCount}`;

    const percent = data.totalCount > 0
      ? Math.round((data.foundCount / data.totalCount) * 100)
      : 0;
    $('end-percent').textContent = `${percent}%`;

    // Counts for tabs
    const foundCount = state.endWords.filter(w => w.found).length;
    const missedCount = state.endWords.length - foundCount;

    $('count-all').textContent = state.endWords.length;
    $('count-found').textContent = foundCount;
    $('count-missed').textContent = missedCount;

    // Render word badges
    filterEndWords('all');

    // Switch to end screen
    switchScreen('screen-end');

  } catch (err) {
    alert('Error fetching end game summary.');
  }
}

// Render word badges with filter (all, found, missed)
function filterEndWords(filter) {
  const container = $('end-word-list');
  container.innerHTML = '';

  const words = state.endWords.filter(item => {
    if (filter === 'found') return item.found;
    if (filter === 'missed') return !item.found;
    return true; // 'all'
  });

  words.forEach(item => {
    const badge = document.createElement('span');
    badge.className = `word-badge ${item.found ? 'found' : 'missed'}`;
    badge.innerHTML = `
      <span>${item.word.toUpperCase()}</span>
      <span class="pts">+${item.points}</span>
    `;
    container.appendChild(badge);
  });
}

// Screen Switcher Helper
function switchScreen(screenId) {
  ['screen-setup', 'screen-game', 'screen-end'].forEach(id => {
    const el = $(id);
    if (id === screenId) {
      el.classList.remove('hide');
    } else {
      el.classList.add('hide');
    }
  });
}

