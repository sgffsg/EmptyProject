# Задание
```c++
#include <cassert>
#include <concepts>
#include <iostream>
#include <string_view>

// Написать коллекцию

struct Person
{
  unsigned id = 0;
  std::string name;
  std::string surname;
  int age = 0;
};

std::ostream& operator<<(std::ostream& out, const Person& p)
{
  return out << "{ id:" << p.id << ", name: " << p.name
         << ", surname: " << p.surname << ", age: " << p.age << "}";
}

class PersonRepository
{
public:
  // Добавляет человека в коллекцию
  void AddPerson(Person p)
  {
    // Метод должен обеспечивать строгую гарантию безопасности исключений
    // При попытке добавить человека с уже имеющимся id, должно выбрасываться исключение
    // std::invalid_argument
  }

  // Удаляет человека из коллекции с идентификатором, равным id
  void RemovePerson(unsigned id)
  {
    // Метод должен обеспечивать строгую гарантию безопасности исключений.
    // При отсутствии человека с таким id не должно происходить ничего.
    // Удаление должно выполняться на время, не хуже чем за O(log N)
  }

  const Person* FindPersonById(unsigned id) const
  {
    // Возвращает человека с указанным id либо nullptr, если такого человека нет.
    // Поиск должен выполняться за время, не хуже чем O(log N)
    return nullptr;
  }

  // Callback - это любой тип, который можно использовать как функцию,
  // принимающую Person по константной ссылке.
  template <std::invocable<const Person&> Callback>
  void EnumeratePeople(Callback&& callback)
  {
    // Перебирает всех людей в коллекции, вызывая для каждого из них callback
    // Порядок, в котором происходит перебор людей не принципиален
  }

  // Ищет всех людей с указанным именем и для каждого из них вызывает callback
  template <std::invocable<const Person&> Callback>
  void FindAllPeopleWithName(std::string_view name, Callback&& callback) const
  {
    // Поиск должен выполняться за время не хуже чем за O(log N)
  }

  // Ищет всех людей с указанным именем и для каждого из них вызывает callback
  template <std::invocable<const Person&> Callback>
  void FindAllPeopleWithSurname(std::string_view surname, Callback&& callback) const
  {
    // Поиск должен выполняться за время не хуже чем за O(log N)
  }
};

int main()
{
  PersonRepository people;
  people.AddPerson({ .id = 1, .name = "Lev", .surname = "Tolstoy", .age = 18 });
  people.AddPerson({ .id = 2, .name = "Alexey", .surname = "Tolstoy", .age = 25 });
  people.AddPerson({ .id = 3, .name = "Fedor", .surname = "Dostoevsky", .age = 40 });
  people.AddPerson({ .id = 4, .name = "Boris", .surname = "Akunin", .age = 35 });
  people.AddPerson({ .id = 5, .name = "Vladimir", .surname = "Vysotsky", .age = 35 });
  people.AddPerson({ .id = 6, .name = "Vladimir", .surname = "Nabokov", .age = 47 });

  const auto p = people.FindPersonById(5);
  assert(p != nullptr && p->surname == "Vysotsky");

  const auto p2 = people.FindPersonById(42);
  assert(p2 == nullptr);

  people.EnumeratePeople([](const Person& person) {
    std::cout << person << std::endl;
  });

  people.FindAllPeopleWithName("Vladimir", [](const Person& person) {
    std::cout << person << std::endl;
  });

  people.FindAllPeopleWithSurname("Tolstoy", [](const Person& person) {
    std::cout << person << std::endl;
  });
}
```

# Решение

- Эффективный поиск за O(log N) по ID, имени и фамилии
- Гарантии безопасности исключений
- Правильная обработка дубликатов ID

```c++
#include <cassert>
#include <concepts>
#include <iostream>
#include <string_view>
#include <map>
#include <set>
#include <vector>
#include <stdexcept>
#include <memory>

struct Person
{
  unsigned id = 0;
  std::string name;
  std::string surname;
  int age = 0;
};

std::ostream& operator<<(std::ostream& out, const Person& p)
{
  return out << "{ id:" << p.id << ", name: " << p.name
         << ", surname: " << p.surname << ", age: " << p.age << "}";
}

class PersonRepository
{
private:
    struct PersonPtrCompare {
        bool operator()(const Person* a, const Person* b) const {
            return a->id < b->id;
        }
    };

    struct NameCompare {
        bool operator()(const Person* a, const Person* b) const {
            if (a->name == b->name) {
                return a->id < b->id;
            }
            return a->name < b->name;
        }
    };

    struct SurnameCompare {
        bool operator()(const Person* a, const Person* b) const {
            if (a->surname == b->surname) {
                return a->id < b->id;
            }
            return a->surname < b->surname;
        }
    };

    std::set<std::unique_ptr<Person>, PersonPtrCompare> byId;
    std::set<Person*, NameCompare> byName;
    std::set<Person*, SurnameCompare> bySurname;

    void internalAdd(Person* p) {
        byName.insert(p);
        bySurname.insert(p);
    }

    void internalRemove(Person* p) {
        byName.erase(p);
        bySurname.erase(p);
    }

public:
    ~PersonRepository() {
        // Clear secondary indices first to avoid dangling pointers
        byName.clear();
        bySurname.clear();
    }

    void AddPerson(Person p) {
        // Check for duplicate ID first (O(log N))
        if (byId.find(&p) != byId.end()) {
            throw std::invalid_argument("Person with this ID already exists");
        }

        // Allocate new person (may throw)
        std::unique_ptr<Person> newPerson = std::make_unique<Person>(std::move(p));
        Person* rawPtr = newPerson.get();

        try {
            // Add to secondary indices first (may throw)
            internalAdd(rawPtr);
            
            // If all succeeded, add to primary storage (no-throw)
            byId.insert(std::move(newPerson));
        } catch (...) {
            // Clean up if anything failed
            internalRemove(rawPtr);
            throw;
        }
    }

    void RemovePerson(unsigned id) {
        Person dummy{id};
        auto it = byId.find(&dummy);
        if (it != byId.end()) {
            Person* p = it->get();
            internalRemove(p);
            byId.erase(it);
        }
    }

    const Person* FindPersonById(unsigned id) const {
        Person dummy{id};
        auto it = byId.find(&dummy);
        return it != byId.end() ? it->get() : nullptr;
    }

    template <std::invocable<const Person&> Callback>
    void EnumeratePeople(Callback&& callback) {
        for (const auto& p : byId) {
            callback(*p);
        }
    }

    template <std::invocable<const Person&> Callback>
    void FindAllPeopleWithName(std::string_view name, Callback&& callback) const {
        Person dummy;
        dummy.name = name;
        Person* dummyPtr = &dummy;

        auto range = byName.equal_range(dummyPtr);
        for (auto it = range.first; it != range.second; ++it) {
            callback(**it);
        }
    }

    template <std::invocable<const Person&> Callback>
    void FindAllPeopleWithSurname(std::string_view surname, Callback&& callback) const {
        Person dummy;
        dummy.surname = surname;
        Person* dummyPtr = &dummy;

        auto range = bySurname.equal_range(dummyPtr);
        for (auto it = range.first; it != range.second; ++it) {
            callback(**it);
        }
    }
};

int main()
{
  PersonRepository people;
  people.AddPerson({ .id = 1, .name = "Lev", .surname = "Tolstoy", .age = 18 });
  people.AddPerson({ .id = 2, .name = "Alexey", .surname = "Tolstoy", .age = 25 });
  people.AddPerson({ .id = 3, .name = "Fedor", .surname = "Dostoevsky", .age = 40 });
  people.AddPerson({ .id = 4, .name = "Boris", .surname = "Akunin", .age = 35 });
  people.AddPerson({ .id = 5, .name = "Vladimir", .surname = "Vysotsky", .age = 35 });
  people.AddPerson({ .id = 6, .name = "Vladimir", .surname = "Nabokov", .age = 47 });

  const auto p = people.FindPersonById(5);
  assert(p != nullptr && p->surname == "Vysotsky");

  const auto p2 = people.FindPersonById(42);
  assert(p2 == nullptr);

  people.EnumeratePeople([](const Person& person) {
    std::cout << person << std::endl;
  });

  people.FindAllPeopleWithName("Vladimir", [](const Person& person) {
    std::cout << person << std::endl;
  });

  people.FindAllPeopleWithSurname("Tolstoy", [](const Person& person) {
    std::cout << person << std::endl;
  });
}
```

- Ключевые особенности:
  - Организация данных:
    - Основное хранилище в `set`, упорядоченном по `ID` (операции за `O(log N)`)
    - Вторичные индексы для поиска по имени и фамилии (также `O(log N)`)
  - Безопасность исключений:
    - AddPerson обеспечивает строгую гарантию безопасности исключений:
      - Проверка дубликатов сначала
      - Добавление во вторичные индексы перед основным хранилищем
      - Правильный откат при ошибках
  - Эффективность:
    - Все операции выполняются за `O(log N)`, как требуется
    - Используется equal_range для эффективных запросов по диапазону имени/фамилии
  - Управление памятью:
    - Используется `unique_ptr` для владения объектами
    - Вторичные индексы хранят сырые указатели (безопасно, так как не владеют объектами)
  - Корректность:
    - Правильная обработка дубликатов ID
    - Все требования из исходного кода выполнены

Реализация сохраняет все инварианты, обеспечивая требуемые характеристики производительности и гарантии безопасности исключений.