// Get the canvas element and its context
const canvas = document.getElementById("gameCanvas");
const ctx = canvas.getContext("2d");

// Set the canvas size
canvas.width = window.innerWidth;
canvas.height = window.innerHeight;

// Game variables
let snake = [{ x: 10, y: 10 }];
let food = generateFood();
let direction = "RIGHT";
let score = 0;
let gameOver = false;
let gamePaused = false;  // Flag to manage game pause
let gameInterval;

// Draw score on the screen
function drawScore() {
    ctx.fillStyle = "white";
    ctx.font = "30px Arial";
    ctx.fillText("Score: " + score, 10, 30);
}

// Draw the snake
function drawSnake() {
    ctx.fillStyle = "lime";
    snake.forEach((segment) => {
        ctx.fillRect(segment.x * 20, segment.y * 20, 20, 20);
    });
}

// Draw the food
function drawFood() {
    ctx.fillStyle = "red";
    ctx.fillRect(food.x * 20, food.y * 20, 20, 20);
}

// Move the snake
function moveSnake() {
    let head = { ...snake[0] };

    if (direction === "UP") head.y--;
    if (direction === "DOWN") head.y++;
    if (direction === "LEFT") head.x--;
    if (direction === "RIGHT") head.x++;

    snake.unshift(head);

    // Check for collision with food
    if (head.x === food.x && head.y === food.y) {
        score++;
        food = generateFood(); // Generate new food
    } else {
        snake.pop(); // Remove the last snake segment
    }

    // Check for collision with walls or itself
    if (
        head.x < 0 || head.x >= canvas.width / 20 || head.y < 0 || head.y >= canvas.height / 20 ||
        snake.slice(1).some((segment) => segment.x === head.x && segment.y === head.y)
    ) {
        gameOver = true;
    }
}

// Generate food at a random position
function generateFood() {
    let x = Math.floor(Math.random() * (canvas.width / 20));
    let y = Math.floor(Math.random() * (canvas.height / 20));
    return { x, y };
}

// Draw the game elements (snake, food, score)
function drawGame() {
    ctx.clearRect(0, 0, canvas.width, canvas.height);  // Clear the canvas

    drawSnake();
    drawFood();
    drawScore();
}

// Handle keyboard input for snake movement and pausing the game
document.addEventListener("keydown", (event) => {
    if (event.key === "ArrowUp" && direction !== "DOWN") {
        direction = "UP";
    } else if (event.key === "ArrowDown" && direction !== "UP") {
        direction = "DOWN";
    } else if (event.key === "ArrowLeft" && direction !== "RIGHT") {
        direction = "LEFT";
    } else if (event.key === "ArrowRight" && direction !== "LEFT") {
        direction = "RIGHT";
    } else if (event.key === "Escape" && !gameOver) {
        gamePaused = !gamePaused; // Toggle pause when ESC is pressed
        document.getElementById("pauseScore").innerText = score; // Display current score
        document.getElementById("pauseScreen").style.display = gamePaused ? "flex" : "none"; // Toggle pause screen
    }
});

// Start the game
function startGame() {
    document.getElementById("loadingScreen").style.display = "none"; // Hide loading screen
    canvas.style.visibility = "visible"; // Show canvas after starting the game
    gameInterval = setInterval(() => {
        if (!gameOver && !gamePaused) {
            moveSnake();
            drawGame();
        } else if (gameOver) {
            clearInterval(gameInterval); // Stop the game
            alert("Game Over! Your score: " + score);
        }
    }, 100); // Game speed
}

// Resume the game when the "Resume Game" button is clicked
document.getElementById("resumeButton").addEventListener("click", () => {
    document.getElementById("pauseScreen").style.display = "none";
    gamePaused = false;
});

// Restart the game when the start button is clicked
document.getElementById("startButton").addEventListener("click", startGame);
