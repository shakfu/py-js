// clang++ -std=c++20 -o test_span_vec test_span_vec.cpp

#include <iostream>
#include <vector>
#include <span>

int main() {
    // Create a vector with some integers
    std::vector<int> numbers = {10, 20, 30, 40, 50};
    
    // Print the original vector
    std::cout << "Original vector: ";
    for (const auto& num : numbers) {
        std::cout << num << " ";
    }
    std::cout << std::endl;
    
    // Create a span that views the entire vector
    std::span<int> fullSpan(numbers);
    
    // Create a span that skips the first element
    std::span<int> offsetSpan(numbers.data() + 1, numbers.size() - 1);
    // Alternatively: std::span<int> offsetSpan = fullSpan.subspan(1);
    
    // Print the offset span
    std::cout << "Offset span (skipping first element): ";
    for (const auto& num : offsetSpan) {
        std::cout << num << " ";
    }
    std::cout << std::endl;
    
    // Modify elements through the span
    for (auto& num : offsetSpan) {
        num *= 2;
    }
    
    // Print the modified vector (changes are reflected in the original vector)
    std::cout << "Modified vector after doubling values through the span: ";
    for (const auto& num : numbers) {
        std::cout << num << " ";
    }
    std::cout << std::endl;
    
    return 0;
}