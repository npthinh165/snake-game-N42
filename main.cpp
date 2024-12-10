#include <iostream>
#include <raylib.h>
#include <deque>
#include <raymath.h>
#include <vector>

using namespace std;

static bool allowMove = false;
Color green = {173, 204, 96, 255};
Color darkGreen = {43, 51, 24, 255};

int cellSize = 30;
int cellCount = 25;
int offset = 75;

double lastUpdateTime = 0;


bool ElementInDeque(Vector2 element, deque<Vector2> deque)
{
    for (unsigned int i = 0; i < deque.size(); i++)
    {
        if (Vector2Equals(deque[i], element))
        {
            return true;
        }
    }
    return false;
}

bool EventTriggered(double interval)
{
    double currentTime = GetTime();
    if (currentTime - lastUpdateTime >= interval)
    {
        lastUpdateTime = currentTime;
        return true;
    }
    return false;
}

class Snake
{
public:
    deque<Vector2> body = {Vector2{6, 9}, Vector2{5, 9}, Vector2{4, 9}};
    Vector2 direction = {1, 0};
    bool addSegment = false;

void Draw()
{
    for (unsigned int i = 0; i < body.size(); i++)
    {
        float x = body[i].x;
        float y = body[i].y;
        Rectangle segment = Rectangle{offset + x * cellSize, offset + y * cellSize, (float)cellSize, (float)cellSize};
        DrawRectangleRounded(segment, 0.5, 6, darkGreen);
    }

    Vector2 head = body[0];
    float eyeRadius = 5; 
    Vector2 leftEyePos, rightEyePos;

   
    if (direction.x == 1) 
    {
        leftEyePos = {offset + (head.x + 0.7f) * cellSize, offset + (head.y + 0.3f) * cellSize};
        rightEyePos = {offset + (head.x + 0.7f) * cellSize, offset + (head.y + 0.7f) * cellSize};
    }
    else if (direction.x == -1) 
    {
        leftEyePos = {offset + (head.x + 0.3f) * cellSize, offset + (head.y + 0.3f) * cellSize};
        rightEyePos = {offset + (head.x + 0.3f) * cellSize, offset + (head.y + 0.7f) * cellSize};
    }
    else if (direction.y == 1) 
    {
        leftEyePos = {offset + (head.x + 0.3f) * cellSize, offset + (head.y + 0.7f) * cellSize};
        rightEyePos = {offset + (head.x + 0.7f) * cellSize, offset + (head.y + 0.7f) * cellSize};
    }
    else if (direction.y == -1) 
    {
        leftEyePos = {offset + (head.x + 0.3f) * cellSize, offset + (head.y + 0.3f) * cellSize};
        rightEyePos = {offset + (head.x + 0.7f) * cellSize, offset + (head.y + 0.3f) * cellSize};
    }

    // Vẽ mắt
    DrawCircleV(leftEyePos, eyeRadius, WHITE);
    DrawCircleV(rightEyePos, eyeRadius, WHITE);
}


    void Update()
    {
        body.push_front(Vector2Add(body[0], direction));
        if (addSegment == true)
        {
            addSegment = false;
        }
        else
        {
            body.pop_back();
        }
    }

    void Reset()
    {
        body = {Vector2{6, 9}, Vector2{5, 9}, Vector2{4, 9}};
        direction = {1, 0};
    }
};
class Food
{
public:
    Vector2 position;
    Texture2D currentTexture;
    std::vector<Texture2D> textures;

    Food(deque<Vector2> snakeBody)
    {
        textures.push_back(LoadTexture("Graphics/chicken.png"));
        textures.push_back(LoadTexture("Graphics/pork.png"));
        textures.push_back(LoadTexture("Graphics/apple.png"));
        textures.push_back(LoadTexture("Graphics/banana.png"));
        textures.push_back(LoadTexture("Graphics/grape.png"));

        // Choose a random food texture
        currentTexture = textures[GetRandomValue(0, textures.size() - 1)];
        
        position = GenerateRandomPos(snakeBody);
    }

    ~Food()
    {
        // Unload all textures
        for (Texture2D texture : textures)
        {
            UnloadTexture(texture);
        }
    }

    void Draw()
    {
        float scale = (float)cellSize / currentTexture.width;
    
        DrawTextureEx(currentTexture, Vector2{offset + position.x * cellSize, offset + position.y * cellSize}, 0.0f, scale, WHITE);
    }

    Vector2 GenerateRandomCell()
    {
        float x = GetRandomValue(0, cellCount - 1);
        float y = GetRandomValue(0, cellCount - 1);
        return Vector2{x, y};
    }

    Vector2 GenerateRandomPos(deque<Vector2> snakeBody)
    {
        Vector2 position = GenerateRandomCell();
        while (ElementInDeque(position, snakeBody))
        {
            position = GenerateRandomCell();
        }
        return position;
    }

    void Respawn(deque<Vector2> snakeBody)
    {
        position = GenerateRandomPos(snakeBody);
        currentTexture = textures[GetRandomValue(0, textures.size() - 1)]; // Choose a new random texture
    }
};

class Obstacles {
public:
    std::vector<Vector2> obstaclePositions; // Vị trí các chướng ngại vật

    Obstacles() {}

    // Hàm sinh chướng ngại vật ngẫu nhiên
    void GenerateObstacles(int count, const std::deque<Vector2>& snakeBody, const Vector2& foodPosition)
    {
        obstaclePositions.clear(); // Xóa tất cả các chướng ngại vật cũ
        for (int i = 0; i < count; i++)
        {
            Vector2 obstacle;
            do
            {
                obstacle = Vector2{GetRandomValue(0, cellCount - 1), GetRandomValue(0, cellCount - 1)};
            } while (ElementInDeque(obstacle, snakeBody) || Vector2Equals(obstacle, foodPosition) || ElementInDeque(obstacle, obstaclePositions));
            
            obstaclePositions.push_back(obstacle);
        }
    }

    // Hàm vẽ chướng ngại vật
    void Draw()
    {
        for (const auto& pos : obstaclePositions)
        {
            Rectangle rect = Rectangle{offset + pos.x * cellSize, offset + pos.y * cellSize, (float)cellSize, (float)cellSize};
            DrawRectangleRounded(rect, 0.5, 6, RED); // Vẽ chướng ngại vật màu đỏ
        }
    }

    // Kiểm tra va chạm với chướng ngại vật
    bool CheckCollisionWithObstacles(const Vector2& position) const
    {
        for (const auto& pos : obstaclePositions)
        {
            if (Vector2Equals(position, pos))
            {
                return true;
            }
        }
        return false;
    }

    // Xóa tất cả các chướng ngại vật
    void Clear()
    { 
        obstaclePositions.clear();
    }

private:

    // Hàm kiểm tra một Vector2 có nằm trong deque hay không
    bool ElementInDeque(const Vector2& element, const std::deque<Vector2>& container) const
    {
        for (const auto& item : container)
        {
            if (Vector2Equals(element, item)) return true;
        }
        return false;
    }

    // Hàm kiểm tra một Vector2 có nằm trong vector hay không
    bool ElementInDeque(const Vector2& element, const std::vector<Vector2>& container) const
    {
        for (const auto& item : container)
        {
            if (Vector2Equals(element, item)) return true;
        }
        return false;
    }
};

class Game
{
public:
    Snake snake = Snake();
    Food food = Food(snake.body);
    bool running = true;
    bool isPaused = false;  // Trạng thái tạm dừng
    int score = 0;
    Sound eatSound;
    Sound wallSound;
    double interval;  // Khởi tạo tốc độ ban đầu
    Obstacles obstacles; // Chướng ngại vật
    Game()
    {
        InitAudioDevice();
        eatSound = LoadSound("Sounds/eat.mp3");
        wallSound = LoadSound("Sounds/wall.mp3");
    }

    ~Game()
    {
        UnloadSound(eatSound);
        UnloadSound(wallSound);
        CloseAudioDevice();
    }

    void Draw()
    {
        food.Draw();
        snake.Draw();
        obstacles.Draw(); // Vẽ chướng ngại vật
    }


    void GameOver()
    {
        snake.Reset();
        food.position = food.GenerateRandomPos(snake.body);
        running = false;
        score = 0;
        PlaySound(wallSound);
    }

    void Update()
    {
 
        if (running)
        {
            snake.Update();
            CheckCollisionWithFood();
            CheckCollisionWithEdges();
            CheckCollisionWithTail();
            
            // Kiểm tra va chạm với chướng ngại vật
            if (obstacles.CheckCollisionWithObstacles(snake.body[0]))
            {
                GameOver();  // Gọi GameOver khi va chạm với chướng ngại vật
            }
        }
    }
    void CheckCollisionWithFood()
    {
        if (Vector2Equals(snake.body[0], food.position))
        {
            food.Respawn(snake.body);
            snake.addSegment = true;
            score += 10;  // Tăng điểm lên 10 mỗi lần ăn

            // Tăng tốc độ khi đạt 100 điểm, tối đa đến 0.05
            if (score % 100 == 0 && interval > 0.05)
            {
                interval -= 0.03;  // Giảm interval để tăng tốc độ
            }
            
            PlaySound(eatSound);
        }
    }


    void CheckCollisionWithEdges()
    {
        if (snake.body[0].x == cellCount || snake.body[0].x == -1)
        {
            GameOver();
        }
        if (snake.body[0].y == cellCount || snake.body[0].y == -1)
        {
            GameOver();
        }
    }

    void CheckCollisionWithTail()
    {
        deque<Vector2> headlessBody = snake.body;
        headlessBody.pop_front();
        if (ElementInDeque(snake.body[0], headlessBody))
        {
            GameOver();
        }
    }

    void SetDifficulty(int difficulty)
    {
        switch (difficulty)
        {
        case 0:  // Easy
            interval = 0.2;
            obstacles.Clear();
            break;
        case 1:  // Normal
            interval = 0.1;
            obstacles.Clear();
            break;
        case 2:  // Hard
            interval = 0.1;
            // Sinh chướng ngại vật
            obstacles.Clear();
            obstacles.GenerateObstacles(10, snake.body, food.position); // 10 chướng ngại vật
            break;
        }
    }
    // Hàm xử lý sự kiện nút "Back"

};

int main()
{
    InitWindow(2 * offset + cellSize * cellCount, 2 * offset + cellSize * cellCount, "Retro Snake");
    SetTargetFPS(60);

    Game game = Game();
    bool showMenu = true;
    bool showInstructions = false;
    bool showDifficultyMenu = false;

        // Kích thước mới của các nút start và instructions
    float largeButtonWidth = 400.0f;
    float largeButtonHeight = 100.0f;

    // Kích thước các nút còn lại
    float smallButtonWidth = 300.0f;
    float smallButtonHeight = 75.0f;

    // Tính toán vị trí và kích thước của cửa sổ
    float windowWidth = (float)(2 * offset + cellSize * cellCount);
    float windowHeight = (float)(2 * offset + cellSize * cellCount);

    // Xác định vị trí X để các nút căn giữa
    float largeButtonX = (windowWidth - largeButtonWidth) / 2.0f;
    float smallButtonX = (windowWidth - smallButtonWidth) / 2.0f;

    // Khoảng cách dọc giữa các nút
    float buttonSpacing = 50.0f;

    // Tính toán vị trí Y của các nút
    float startButtonY = (windowHeight - (largeButtonHeight * 2 + buttonSpacing)) / 2.0f;
    float instructionsButtonY = startButtonY + largeButtonHeight + buttonSpacing;

    float easyButtonY = (windowHeight - (smallButtonHeight * 3 + buttonSpacing * 2)) / 2.0f;
    float normalButtonY = easyButtonY + smallButtonHeight + buttonSpacing;
    float hardButtonY = normalButtonY + smallButtonHeight + buttonSpacing;
    // Định nghĩa các nút Pause và Reset
    float pauseButtonX = offset - 5;  // Nút tạm dừng
    float pauseButtonY = windowHeight - smallButtonHeight - (float)offset + 80.0f;

    float resetButtonX = offset - 5 + smallButtonWidth + 20.0f; // Nút Reset
    float resetButtonY = windowHeight - smallButtonHeight - (float)offset + 80.0f;
    float buttonWidth = 120.0f;  // Chiều rộng mỗi nút
    float buttonHeight = 40.0f;  // Chiều cao mỗi nút

    float totalButtonWidth = 3 * buttonWidth + 2 * buttonSpacing;  // Tổng chiều rộng các nút và khoảng cách

    float buttonsStartX = (windowWidth - totalButtonWidth) / 2.0f; // Vị trí X bắt đầu của các nút
    float buttonsY = windowHeight - offset - buttonHeight + 50.0f; // Đặt bên trên bảng điểm


    // Định nghĩa các nút
    Rectangle startButton = {largeButtonX, startButtonY, largeButtonWidth, largeButtonHeight};
    Rectangle instructionsButton = {largeButtonX, instructionsButtonY, largeButtonWidth, largeButtonHeight};
    Rectangle easyButton = {smallButtonX, easyButtonY, smallButtonWidth, smallButtonHeight};
    Rectangle normalButton = {smallButtonX, normalButtonY, smallButtonWidth, smallButtonHeight};
    Rectangle hardButton = {smallButtonX, hardButtonY, smallButtonWidth, smallButtonHeight};
    Rectangle pauseButton = {buttonsStartX, buttonsY, buttonWidth, buttonHeight};
    Rectangle resetButton = {buttonsStartX + buttonWidth + buttonSpacing, buttonsY, buttonWidth, buttonHeight};
    // Nút back (giữ nguyên vị trí góc phải dưới)
    float backButtonX = windowWidth - smallButtonWidth - (float)offset + 100.0f ;
    float backButtonY = windowHeight - smallButtonHeight - (float)offset + 90.0f; // Dịch xuống thêm một chút
    Rectangle backButton = {backButtonX, backButtonY, smallButtonWidth - 100.0f, smallButtonHeight - 30.0f};


    while (WindowShouldClose() == false)
    {
        Vector2 mousePos = GetMousePosition();
        bool mouseOverStart = CheckCollisionPointRec(mousePos, startButton);
        bool mouseOverInstructions = CheckCollisionPointRec(mousePos, instructionsButton);
        bool mouseOverEasy = CheckCollisionPointRec(mousePos, easyButton);
        bool mouseOverNormal = CheckCollisionPointRec(mousePos, normalButton);
        bool mouseOverHard = CheckCollisionPointRec(mousePos, hardButton);
        bool mouseOverBack = CheckCollisionPointRec(mousePos, backButton);

        if (!showMenu) {
            // Lấy vị trí chuột hiện tại
            Vector2 mousePos = GetMousePosition();

            // Xử lý nút Pause
            bool mouseOverPause = CheckCollisionPointRec(mousePos, pauseButton);
            if (mouseOverPause && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                game.isPaused = !game.isPaused;  // Đảo trạng thái Pause
            }

            // Xử lý nút Reset
            bool mouseOverReset = CheckCollisionPointRec(mousePos, resetButton);
            if (mouseOverReset && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                game.snake.Reset();               // Reset trạng thái rắn
                game.food.Respawn(game.snake.body); // Đặt lại vị trí thức ăn
                game.running = false;             // Đưa game về trạng thái dừng
                game.score = 0;                   // Reset điểm số
                game.isPaused = false;            // Đảm bảo không bị Pause sau Reset
            }
        }

        if (showMenu)
        {
            BeginDrawing();
            ClearBackground(green);

            // Vẽ menu chính
            DrawText("King of Snakes", (windowWidth - MeasureText("Retro Snake Game", 50)) / 2.0f, offset, 50, darkGreen);

                // Nút Start
            DrawRectangleRec(startButton, DARKGRAY);
            DrawText("Start Game", startButton.x + (largeButtonWidth - MeasureText("Start Game", 30)) / 2.0f, startButton.y + 35, 30, WHITE);

            // Nút Instructions
            DrawRectangleRec(instructionsButton, DARKGRAY);
            DrawText("Instructions", instructionsButton.x + (largeButtonWidth - MeasureText("Instructions", 30)) / 2.0f, instructionsButton.y + 35, 30, WHITE);

            // Chọn bắt đầu trò chơi
            if (mouseOverStart && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                showDifficultyMenu = true;
                showMenu = false;
            }

            // Xem hướng dẫn
            if (mouseOverInstructions && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                showInstructions = true;
                showMenu = false;
            }

            EndDrawing();
        }
        else if (showDifficultyMenu)
        {
            BeginDrawing();
            ClearBackground(green);

            // Tiêu đề
            DrawText("Choose Difficulty", (windowWidth - MeasureText("Choose Difficulty", 50)) / 2.0f, offset, 50, darkGreen);

            // Nút Easy
            DrawRectangleRec(easyButton, DARKGRAY);
            DrawText("Easy", easyButton.x + (smallButtonWidth - MeasureText("Easy", 25)) / 2.0f, easyButton.y + 25, 25, WHITE);

            // Nút Normal
            DrawRectangleRec(normalButton, DARKGRAY);
            DrawText("Normal", normalButton.x + (smallButtonWidth - MeasureText("Normal", 25)) / 2.0f, normalButton.y + 25, 25, WHITE);

            // Nút Hard
            DrawRectangleRec(hardButton, DARKGRAY);
            DrawText("Hard", hardButton.x + (smallButtonWidth - MeasureText("Hard", 25)) / 2.0f, hardButton.y + 25, 25, WHITE);
                // Vẽ nút Back
            DrawRectangleRec(backButton, DARKGRAY);
            DrawText("Back", backButton.x + 70, backButton.y + 15, 20, WHITE);

            if (mouseOverBack && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                showDifficultyMenu = false;  // Thoát menu độ khó
                showMenu = true;  // Quay về menu chính
            }

            // Lựa chọn dễ
            if (mouseOverEasy && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                game.SetDifficulty(0);  // Dễ
                showDifficultyMenu = false;
                game.running = true;

            }

            // Lựa chọn bình thường
            if (mouseOverNormal && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                game.SetDifficulty(1);  // Bình thường
                showDifficultyMenu = false;
                game.running = true;
            }

            // Lựa chọn khó
            if (mouseOverHard && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                game.SetDifficulty(2);  // Khó
                showDifficultyMenu = false;
                game.running = true;
            }

            EndDrawing();
        }
        else if (showInstructions)
        {   BeginDrawing();
            ClearBackground(green);

            // Vẽ hướng dẫn
            DrawText("Instructions", offset, offset, 40, darkGreen);
            DrawText("Use arrow keys to move the snake.", offset, offset + 60, 20, darkGreen);
            DrawText("Avoid hitting the walls or your own tail!", offset, offset + 90, 20, darkGreen);
            DrawText("Press 'Back' to return to the menu", offset, offset + 120, 20, darkGreen);

            // Vẽ nút quay lại
            DrawRectangleRec(backButton, DARKGRAY);
            DrawText("Back", backButton.x + 70, backButton.y + 15, 20, WHITE);

            if (mouseOverBack && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                showInstructions = false;
                showMenu = true;
            }

            EndDrawing();
        }
        else
        {
            BeginDrawing();
            ClearBackground(green);  
            DrawRectangleRec(backButton, DARKGRAY);
            DrawText("Back", backButton.x + 70, backButton.y + 15, 20, WHITE);

            if (mouseOverBack && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                showMenu = true;  // Quay lại menu chính
                game.snake.Reset();               // Reset trạng thái rắn
                game.food.Respawn(game.snake.body); // Đặt lại vị trí thức ăn
                game.running = false;             // Đưa game về trạng thái dừng
                game.score = 0;                   // Reset điểm số
                game.isPaused = false;  
            }

            if (!game.isPaused) {
                if (EventTriggered(game.interval)) { // Chỉ cập nhật khi không bị tạm dừng
                    allowMove = true;
                    game.Update();
                }
            }


            // Xử lý phím di chuyển
            if (IsKeyPressed(KEY_UP) && game.snake.direction.y != 1 && allowMove)
            {
                game.snake.direction = {0, -1};
                game.running = true;
                allowMove = false;
            }
            if (IsKeyPressed(KEY_DOWN) && game.snake.direction.y != -1 && allowMove)
            {
                game.snake.direction = {0, 1};
                game.running = true;
                allowMove = false;
            }
            if (IsKeyPressed(KEY_LEFT) && game.snake.direction.x != 1 && allowMove)
            {
                game.snake.direction = {-1, 0};
                game.running = true;
                allowMove = false;
            }
            if (IsKeyPressed(KEY_RIGHT) && game.snake.direction.x != -1 && allowMove)
            {
                game.snake.direction = {1, 0};
                game.running = true;
                allowMove = false;
            }

            // Giao diện game
            DrawRectangleLinesEx(Rectangle{(float)offset - 5, (float)offset - 5, (float)cellSize * cellCount + 10, (float)cellSize * cellCount + 10}, 5, darkGreen);
            DrawText("Retro Snake", offset - 5, 20, 40, darkGreen);
            DrawText(TextFormat("%i", game.score), offset - 5, offset + cellSize * cellCount + 10, 40, darkGreen);
            game.Draw();
            EndDrawing();
            // Vẽ nút Pause
            DrawRectangleRec(pauseButton, DARKGRAY);
            DrawText(game.isPaused ? "Resume" : "Pause", pauseButton.x +30, pauseButton.y + 10, 20, WHITE);

            // Vẽ nút Reset
            DrawRectangleRec(resetButton, DARKGRAY);
            DrawText("Reset", resetButton.x +30, resetButton.y + 10, 20, WHITE);
        }

        if (IsKeyPressed(KEY_ENTER))
        {
            showMenu = false;  // Bắt đầu game khi nhấn Enter
            game.running = true;  // Đảm bảo game được bắt đầu
        }

    }
    CloseWindow();
    return 0;
}