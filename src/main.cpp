#include "meal_db.hpp"
#include "util.hpp"

#include <iostream>
#include <string>
#include <vector>

static bool is_skip_token(std::string s) {
  trim_inplace(s);
  s = to_lower(s);
  return s.empty() || s == "blank" || s == "none" || s == "n/a" || s == "na" || s == "skip";
}

static std::string read_line(const std::string& prompt) {
  std::cout << prompt;
  std::string line;
  std::getline(std::cin, line);
  return line;
}

static std::vector<std::string> parse_csv(std::string s) {
  std::vector<std::string> out;
  if (is_skip_token(s)) return out;

  std::string cur;
  for (char ch : s) {
    if (ch == ',') {
      trim_inplace(cur);
      if (!cur.empty()) out.push_back(cur);
      cur.clear();
    } else {
      cur.push_back(ch);
    }
  }
  trim_inplace(cur);
  if (!cur.empty()) out.push_back(cur);

  return out;
}

static void pause_for_enter() {
  std::cout << "\nPress Enter to continue...";
  std::string tmp;
  std::getline(std::cin, tmp);
}

static void print_meal(const Meal& m) {
  std::cout << "\n=== " << m.name << " ===\n";
  std::cout << "ID: " << m.id << "\n";
  std::cout << "Type: " << m.meal_type << "\n";

  if (!m.tags.empty()) {
    std::cout << "Attributes: ";
    for (size_t i = 0; i < m.tags.size(); ++i) {
      std::cout << m.tags[i] << (i + 1 < m.tags.size() ? ", " : "");
    }
    std::cout << "\n";
  }

  if (!m.allergens.empty()) {
    std::cout << "Allergens: ";
    for (size_t i = 0; i < m.allergens.size(); ++i) {
      std::cout << m.allergens[i] << (i + 1 < m.allergens.size() ? ", " : "");
    }
    std::cout << "\n";
  }

  if (!m.ingredients.empty()) {
    std::cout << "\nIngredients:\n";
    for (const auto& ing : m.ingredients) std::cout << " - " << ing << "\n";
  }

  if (!m.steps.empty()) {
    std::cout << "\nSteps:\n";
    for (size_t i = 0; i < m.steps.size(); ++i) {
      std::cout << (i + 1) << ") " << m.steps[i] << "\n";
    }
  }
}

static void print_menu() {
  std::cout << "\nMenu\n"
            << "  1) Generate a random meal (with filters)\n"
            << "  2) List matching meals (with filters)\n"
            << "  3) Show favorites\n"
            << "  4) Add favorite by meal ID\n"
            << "  5) Remove favorite by meal ID\n"
            << "  0) Quit\n";
}

static Query prompt_query() {
  Query q;

  std::string type = read_line("Meal type (breakfast/lunch/dinner/snack, Enter to skip): ");
  if (!is_skip_token(type)) {
    trim_inplace(type);
    q.meal_type = type;
  }

  std::string attrs = read_line(
      "Meal attributes (e.g., quick, no-cook, finger-food) (comma-separated, Enter to skip): ");
  q.required_tags = parse_csv(attrs);

  std::string avoid = read_line(
      "Avoid allergens (e.g., peanuts, soy, dairy) (comma-separated, Enter to skip): ");
  q.avoid_allergens = parse_csv(avoid);

  return q;
}

int main() {
  std::cout << "Toddler Meal Generator\n";

  MealDB db;
  std::string err;

  if (!db.load_from_file("data/meals.json", err)) {
    std::cerr << "Error loading meals: " << err << "\n";
    return 1;
  }

  // Favorites live in repo root by default.
  db.load_favorites("favorites.json", err);

  while (true) {
    print_menu();
    std::string choice = read_line("Select: ");
    trim_inplace(choice);

    if (choice == "0") {
      db.save_favorites("favorites.json", err);
      std::cout << "Bye.\n";
      break;
    }

    if (choice == "1") {
      Query q = prompt_query();
      auto candidates = db.filter(q);

      if (candidates.empty()) {
        std::cout << "\nNo meals matched those filters.\n";
        pause_for_enter();
        continue;
      }

      size_t pick = db.random_pick(candidates);
      const Meal& meal = db.meals()[pick];
      print_meal(meal);

      pause_for_enter();
      continue;
    }

    if (choice == "2") {
      Query q = prompt_query();
      auto candidates = db.filter(q);

      if (candidates.empty()) {
        std::cout << "\nNo meals matched those filters.\n";
        pause_for_enter();
        continue;
      }

      std::cout << "\nMatching meals (" << candidates.size() << "):\n";
      for (size_t idx : candidates) {
        const Meal& m = db.meals()[idx];
        std::cout << " - " << m.name << "  [" << m.id << "]\n";
      }

      pause_for_enter();
      continue;
    }

    if (choice == "3") {
      auto favs = db.favorite_meals();
      if (favs.empty()) {
        std::cout << "\nNo favorites saved yet.\n";
        pause_for_enter();
        continue;
      }

      std::cout << "\nFavorites:\n";
      for (const Meal* m : favs) {
        std::cout << " - " << m->name << "  [" << m->id << "]\n";
      }

      pause_for_enter();
      continue;
    }

    if (choice == "4") {
      std::string id = read_line("Enter meal ID to favorite: ");
      trim_inplace(id);

      if (id.empty()) {
        std::cout << "No ID entered.\n";
        pause_for_enter();
        continue;
      }

      if (db.add_favorite(id)) {
        db.save_favorites("favorites.json", err);
        std::cout << "Saved favorite: " << id << "\n";
      } else {
        std::cout << "Could not favorite '" << id << "' (check the ID).\n";
      }

      pause_for_enter();
      continue;
    }

    if (choice == "5") {
      std::string id = read_line("Enter meal ID to remove from favorites: ");
      trim_inplace(id);

      if (id.empty()) {
        std::cout << "No ID entered.\n";
        pause_for_enter();
        continue;
      }

      if (db.remove_favorite(id)) {
        db.save_favorites("favorites.json", err);
        std::cout << "Removed favorite: " << id << "\n";
      } else {
        std::cout << "That ID wasn't in favorites.\n";
      }

      pause_for_enter();
      continue;
    }

    std::cout << "Invalid option.\n";
    pause_for_enter();
  }

  return 0;
}
