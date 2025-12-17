#include "meal_db.hpp"
#include "util.hpp"

#include <fstream>
#include <unordered_set>

using nlohmann::json;

static bool read_json_file(const std::string& path, json& out, std::string& err) {
  std::ifstream in(path);
  if (!in) {
    err = "Could not open file: " + path;
    return false;
  }
  try {
    in >> out;
  } catch (const std::exception& e) {
    err = std::string("JSON parse error: ") + e.what();
    return false;
  }
  return true;
}

bool MealDB::load_from_file(const std::string& path, std::string& error) {
  json j;
  if (!read_json_file(path, j, error)) return false;

  if (!j.is_array()) {
    error = "Expected a JSON array at top-level";
    return false;
  }

  meals_.clear();
  for (const auto& item : j) {
    Meal m;
    m.id = item.value("id", "");
    m.name = item.value("name", "");
    m.meal_type = item.value("meal_type", "");

    // Accept either "attributes" (preferred) or legacy "tags"
    const char* key = nullptr;
    if (item.contains("attributes") && item["attributes"].is_array()) key = "attributes";
    else if (item.contains("tags") && item["tags"].is_array()) key = "tags";

    if (key) {
      for (const auto& x : item[key]) m.tags.push_back(x.get<std::string>());
    }

    if (item.contains("allergens") && item["allergens"].is_array())
      for (const auto& x : item["allergens"]) m.allergens.push_back(x.get<std::string>());

    if (item.contains("ingredients") && item["ingredients"].is_array())
      for (const auto& x : item["ingredients"]) m.ingredients.push_back(x.get<std::string>());

    if (item.contains("steps") && item["steps"].is_array())
      for (const auto& x : item["steps"]) m.steps.push_back(x.get<std::string>());

    if (!m.id.empty() && !m.name.empty()) meals_.push_back(std::move(m));
  }

  build_indexes();
  return true;
}

void MealDB::build_indexes() {
  by_type_.clear();
  by_tag_.clear();

  for (size_t i = 0; i < meals_.size(); ++i) {
    const Meal& m = meals_[i];

    if (!m.meal_type.empty()) {
      by_type_[to_lower(m.meal_type)].push_back(i);
    }

    for (const auto& t : m.tags) {
      if (!t.empty()) by_tag_[to_lower(t)].push_back(i);
    }
  }
}

std::vector<size_t> MealDB::filter(const Query& q) const {
  std::vector<size_t> candidates;

  // Start pool: by type if given, else all
  if (!q.meal_type.empty()) {
    auto it = by_type_.find(to_lower(q.meal_type));
    if (it == by_type_.end()) return {};
    candidates = it->second;
  } else {
    candidates.reserve(meals_.size());
    for (size_t i = 0; i < meals_.size(); ++i) candidates.push_back(i);
  }

  // Required tags (aka "attributes")
  for (const auto& tag : q.required_tags) {
    if (tag.empty()) continue;

    auto it = by_tag_.find(to_lower(tag));
    if (it == by_tag_.end()) return {};

    std::unordered_set<size_t> allowed(it->second.begin(), it->second.end());
    std::vector<size_t> next;
    next.reserve(candidates.size());
    for (size_t idx : candidates) {
      if (allowed.count(idx)) next.push_back(idx);
    }
    candidates.swap(next);

    if (candidates.empty()) return {};
  }

  // Avoid allergens
  if (!q.avoid_allergens.empty()) {
    std::vector<size_t> next;
    next.reserve(candidates.size());

    for (size_t idx : candidates) {
      const Meal& m = meals_[idx];
      bool ok = true;
      for (const auto& a : q.avoid_allergens) {
        if (a.empty()) continue;
        if (contains_ci(m.allergens, a)) { ok = false; break; }
      }
      if (ok) next.push_back(idx);
    }
    candidates.swap(next);
  }

  return candidates;
}

size_t MealDB::random_pick(const std::vector<size_t>& candidates) {
  std::uniform_int_distribution<size_t> dist(0, candidates.size() - 1);
  return candidates[dist(rng_)];
}

// Favorites (simple JSON: {"favorites":["meal_001","meal_002"]})
bool MealDB::load_favorites(const std::string& path, std::string& error) {
  favorites_.clear();

  std::ifstream in(path);
  if (!in) return true; // ok if not present yet

  json j;
  try { in >> j; }
  catch (...) { error = "Could not parse favorites JSON"; return false; }

  if (j.contains("favorites") && j["favorites"].is_array()) {
    for (const auto& x : j["favorites"]) favorites_.insert(x.get<std::string>());
  }
  return true;
}

bool MealDB::save_favorites(const std::string& path, std::string& error) const {
  json j;
  j["favorites"] = json::array();
  for (const auto& id : favorites_) j["favorites"].push_back(id);

  std::ofstream out(path);
  if (!out) { error = "Could not write favorites file"; return false; }
  out << j.dump(2);
  return true;
}

bool MealDB::add_favorite(const std::string& meal_id) {
  return favorites_.insert(meal_id).second;
}

bool MealDB::remove_favorite(const std::string& meal_id) {
  return favorites_.erase(meal_id) > 0;
}

std::vector<const Meal*> MealDB::favorite_meals() const {
  std::vector<const Meal*> out;
  for (const auto& m : meals_) {
    if (favorites_.count(m.id)) out.push_back(&m);
  }
  return out;
}
