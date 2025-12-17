# Toddler Meal Generator (C++ / CMake)

A command-line app that generates toddler-friendly meal ideas from a JSON meal database. Supports filters (meal type, keywords, allergens to avoid) and a favorites list.

## Features
- Random meal generation with filters
- List all meals that match filters
- Favorites saved locally (`data/favorites.json`)
- Meals stored in JSON (`data/meals.json`) so it’s easy to edit

## Build & Run (Windows / PowerShell)

From the project root folder:

```powershell
cmake -S . -B build
cmake --build build --config Debug
.\build\Debug\tmg.exe