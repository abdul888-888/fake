# Facebook Qt Desktop

This is a Qt 6 Widgets desktop shell around the existing C++ Facebook backend.
The backend headers are preserved under `Backend/`; GUI code lives under `src/`.

## Requirements

- Qt 6
- CMake 3.20+
- A C++20 compiler

## Build With Qt Creator

1. Open Qt Creator.
2. Choose **File > Open File or Project**.
3. Open `CMakeLists.txt` from this folder.
4. Select a Qt 6 Desktop kit.
5. Configure, build, and run.

## Build From A Qt Command Prompt

```powershell
cd C:\Users\HP\Documents\Codex\2026-06-27\computer-plugin-computer-use-openai-bundled\outputs\FacebookQt
cmake -S . -B build -DCMAKE_PREFIX_PATH="C:\Qt\6.x.x\msvc2022_64"
cmake --build build --config Release
.\build\Release\FacebookQtDesktop.exe
```

Change `CMAKE_PREFIX_PATH` to your installed Qt folder.

## Project Structure

- `Backend/` existing business logic and linked-list data structures.
- `src/Controllers/` Qt-facing service layer.
- `src/Views/` main window and stacked pages.
- `src/Widgets/` reusable post and user cards.
- `src/Utils/` UI helper functions.
- `Resources/` QSS theme and SVG icons.
- `Forms/` Qt Designer starter forms for login and signup.

## Data Files

The backend persists data using text files in the application's working directory:

- `users.txt`
- `friends.txt`
- `requests.txt`
- `posts.txt`
- `likes.txt`
- `comments.txt`
