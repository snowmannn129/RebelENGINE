#include <benchmark/benchmark.h>
#include "ui/widgets/ListView.h"
#include <vector>
#include <string>
#include <random>

using namespace RebelCAD::UI;
using json = nlohmann::json;

// Performance benchmarks for ListView
static void BM_ListViewAddItems(benchmark::State& state) {
    ListView list("benchmark_list");
    std::vector<std::string> items;
    for (int i = 0; i < state.range(0); i++) {
        items.push_back("Item " + std::to_string(i));
    }
    
    for (auto _ : state) {
        list.addItems(items);
        list.clearItems();
    }
}
BENCHMARK(BM_ListViewAddItems)->Range(8, 8<<10);

static void BM_ListViewSelection(benchmark::State& state) {
    ListView list("benchmark_list");
    std::vector<std::string> items;
    for (int i = 0; i < state.range(0); i++) {
        items.push_back("Item " + std::to_string(i));
    }
    list.addItems(items);
    list.setSelectionMode(ListView::SelectionMode::Multiple);
    
    for (auto _ : state) {
        for (size_t i = 0; i < items.size(); i += 2) {
            list.addToSelection(i);
        }
        list.clearSelection();
    }
}
BENCHMARK(BM_ListViewSelection)->Range(8, 8<<10);

static void BM_ListViewScroll(benchmark::State& state) {
    ListView list("benchmark_list");
    std::vector<std::string> items;
    for (int i = 0; i < 1000; i++) {
        items.push_back("Item " + std::to_string(i));
    }
    list.addItems(items);
    
    for (auto _ : state) {
        for (float pos = 0; pos < 1000; pos += 10) {
            list.setScrollPosition(pos);
        }
    }
}
BENCHMARK(BM_ListViewScroll);

// Benchmark item removal performance
static void BM_ListViewRemoveItems(benchmark::State& state) {
    ListView list("benchmark_list");
    std::vector<std::string> items;
    for (int i = 0; i < state.range(0); i++) {
        items.push_back("Item " + std::to_string(i));
    }
    
    for (auto _ : state) {
        state.PauseTiming(); // Don't count setup time
        list.addItems(items);
        state.ResumeTiming();
        
        // Remove items from random positions
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, state.range(0) - 1);
        for (int i = state.range(0) - 1; i >= 0; i--) {
            list.removeItem(dis(gen) % (i + 1));
        }
    }
}
BENCHMARK(BM_ListViewRemoveItems)->Range(8, 8<<10);

// Benchmark style application performance
static void BM_ListViewStyleUpdate(benchmark::State& state) {
    ListView list("benchmark_list");
    std::vector<std::string> items(100, "Test Item");
    list.addItems(items);
    
    json style = {
        {"itemHeight", 20.0f},
        {"backgroundColor", "#FFFFFF"},
        {"selectedColor", "#0000FF"},
        {"textColor", "#000000"},
        {"fontSize", 12.0f},
        {"padding", 5.0f}
    };
    
    for (auto _ : state) {
        style["itemHeight"] = float(state.iterations() % 30 + 10);
        list.applyStyle(style);
    }
}
BENCHMARK(BM_ListViewStyleUpdate);

// Benchmark event handling performance
static void BM_ListViewEventHandling(benchmark::State& state) {
    ListView list("benchmark_list");
    std::vector<std::string> items(state.range(0), "Test Item");
    list.addItems(items);
    list.setSelectionMode(ListView::SelectionMode::Multiple);
    
    size_t callbackCount = 0;
    list.onSelectionChanged([&callbackCount](const std::set<size_t>&) {
        callbackCount++;
    });
    
    for (auto _ : state) {
        for (size_t i = 0; i < 100; i++) {
            list.setSelectedIndex(i % state.range(0));
        }
    }
    
    state.counters["CallbacksTriggered"] = benchmark::Counter(callbackCount);
}
BENCHMARK(BM_ListViewEventHandling)->Range(8, 8<<10);

// Benchmark focus management
static void BM_ListViewFocusManagement(benchmark::State& state) {
    ListView list("benchmark_list");
    std::vector<std::string> items(100, "Test Item");
    list.addItems(items);
    
    for (auto _ : state) {
        for (int i = 0; i < state.range(0); i++) {
            list.setFocus(true);
            list.setFocus(false);
        }
    }
}
BENCHMARK(BM_ListViewFocusManagement)->Range(8, 8<<10);

// Benchmark mixed operations (simulates real-world usage)
static void BM_ListViewMixedOperations(benchmark::State& state) {
    ListView list("benchmark_list");
    std::vector<std::string> items;
    std::random_device rd;
    std::mt19937 gen(rd());
    
    for (auto _ : state) {
        state.PauseTiming();
        items.clear();
        for (int i = 0; i < 100; i++) {
            items.push_back("Item " + std::to_string(i));
        }
        list.addItems(items);
        state.ResumeTiming();
        
        // Mix of operations
        for (int i = 0; i < state.range(0); i++) {
            switch (i % 5) {
                case 0: // Add item
                    list.addItem("New Item " + std::to_string(i));
                    break;
                case 1: // Remove random item
                    if (!items.empty()) {
                        list.removeItem(gen() % items.size());
                    }
                    break;
                case 2: // Select/deselect
                    list.setSelectedIndex(i % 100);
                    break;
                case 3: // Scroll
                    list.setScrollPosition(float(i % 100));
                    break;
                case 4: // Focus toggle
                    list.setFocus(i % 2 == 0);
                    break;
            }
        }
        
        state.PauseTiming();
        list.clearItems();
        state.ResumeTiming();
    }
}
BENCHMARK(BM_ListViewMixedOperations)->Range(8, 8<<10);

BENCHMARK_MAIN();
