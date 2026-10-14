#include <iostream>
#include <vector>

void byValue(std::vector<int> v)             { v.push_back(99); }
void byRef(std::vector<int>& v)              { v.push_back(99); }
void byConstRef(const std::vector<int>& v)   { std::cout << "size inside: " << v.size() << '\n'; }
void byPointer(std::vector<int>* v)          { if (v != nullptr) { v->push_back(99); } }

int main() {
    std::vector<int> data{1, 2, 3};

    byValue(data);
    std::cout << "after byValue:   " << data.size() << '\n';

    byRef(data);
    std::cout << "after byRef:     " << data.size() << '\n';

    byConstRef(data);

    byPointer(&data);
    std::cout << "after byPointer: " << data.size() << '\n';

    byPointer(nullptr);   // вказівник може бути «порожнім», а посилання — ні
}

// after byValue: 3
// after byRef: 4
// size inside: 4
// after byPointer: 5