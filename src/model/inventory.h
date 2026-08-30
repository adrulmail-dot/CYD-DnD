#pragma once
#include <Arduino.h>
#include <vector>

struct InventoryItem {
    String name;
    String catalogSlug;   // "" if custom/free-form item
    String kind;           // "weapon" | "armor" | "magic" | "gear"
    float weight = 0;      // pounds, per unit
    int quantity = 1;
    bool equipped = false;
    String notes = "";
};

class Inventory {
public:
    std::vector<InventoryItem> items;

    float totalWeight() const {
        float sum = 0;
        for (const auto &it : items) sum += it.weight * it.quantity;
        return sum;
    }

    // 5e default carry capacity = strength score * 15 lb.
    float carryCapacity(int strengthScore) const {
        return strengthScore * 15.0f;
    }

    void add(const InventoryItem &item) { items.push_back(item); }

    void remove(size_t index) {
        if (index < items.size()) items.erase(items.begin() + index);
    }

    void toggleEquipped(size_t index) {
        if (index < items.size()) items[index].equipped = !items[index].equipped;
    }
};
