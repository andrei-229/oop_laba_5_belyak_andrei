// main.cpp
// Demo for ReusingMemoryResource + PmrStack

#include <iostream>
#include <string>
#include <memory_resource>
#include "../include/memory_resource_reuse.hpp"
#include "../include/stack.hpp"

struct Person
{
    int id;
    std::pmr::string name;
    double score;

    Person(int id_, const std::pmr::string &name_, double s) : id(id_), name(name_), score(s) {}
};

int main()
{
    ReusingMemoryResource rmr;

    std::cout << "--- Integer stack demo ---\n";
    {
        PmrStack<int> s(&rmr);
        for (int i = 1; i <= 5; ++i)
            s.push(i * 10);
        std::cout << "stack size: " << s.size() << "\n";
        for (auto it = s.begin(); it != s.end(); ++it)
        {
            std::cout << *it << " ";
        }
        std::cout << "\nPop two\n";
        s.pop();
        s.pop();
        std::cout << "stack size: " << s.size() << "\n";
    }

    std::cout << "free blocks after int stack out of scope: in_use=" << rmr.in_use_count()
              << " free=" << rmr.free_count() << "\n";

    std::cout << "--- Complex type demo (Person) ---\n";
    {
        PmrStack<Person> ps(&rmr);
        std::pmr::string n1("Alice", &rmr);
        std::pmr::string n2("Bob", &rmr);
        std::pmr::string n3("Carol", &rmr);

        ps.emplace(1, n1, 10.5);
        ps.emplace(2, n2, 20.25);
        ps.emplace(3, n3, 15.75);

        std::cout << "person stack size: " << ps.size() << "\n";
        for (auto it = ps.begin(); it != ps.end(); ++it)
        {
            const Person &p = *it;
            std::cout << "id=" << p.id << " name=" << p.name << " score=" << p.score << "\n";
        }
    }

    std::cout << "At end: in_use=" << rmr.in_use_count() << " free=" << rmr.free_count() << "\n";
    std::cout << "Program end; ReusingMemoryResource destructor will free remaining memory.\n";
    return 0;
}
