#pragma once
#include <nlohmann/json.hpp>
#include <random>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

struct Meal {
  std::string id;
  std::string name;
  std::string meal_type;
  std::vector<std::string> tags;
  std::vector<std::string> allergens;
  std::vector<std::string> ingredients;
  std::vector<std::string> steps;
};

struct Query {
  std::string meal_type;
  std::vector<std::string> required_tags;
  std::vector<std::string> avoid_allergens;
};

class MealDB {
public:
  bool load_from_file(const std::string& path, std::string& error);
  const std::vector<Meal>& meals() const { return meals_; }

  std::vector<size_t> filter(const Query& q) const;
  size_t random_pick(const std::vector<size_t>& candidates);

  bool load_favorites(const std::string& path, std::string& error);
  bool save_favorites(const std::string& path, std::string& error) const;

  bool add_favorite(const std::string& meal_id);
  bool remove_favorite(const std::string& meal_id);
  std::vector<const Meal*> favorite_meals() const;

private:
  void build_indexes();

  std::vector<Meal> meals_;
  std::unordered_map<std::string, std::vector<size_t>> by_type_;
  std::unordered_map<std::string, std::vector<size_t>> by_tag_;
  std::unordered_set<std::string> favorites_;

  std::mt19937 rng_{std::random_device{}()};
};
