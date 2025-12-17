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
```

## How to Use

The app will prompt you for:

Meal type: breakfast, lunch, dinner, snack (or press Enter to skip)

Keywords: comma-separated (or press Enter to skip)

Example: quick, finger-food

Avoid allergens: comma-separated (or press Enter to skip)

Example: peanut, soy

## Data Format

Meals: data/meals.json
Favorites: data/favorites.json

Meal fields:

id, name, meal_type

tags (keywords), allergens

ingredients, steps

## Notes

Always double-check packaged food labels for allergens.