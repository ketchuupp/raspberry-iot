#ifndef FOO_H
#define FOO_H

#include <iostream>

namespace Foo {

class Foo {
public:
    Foo() {}
    void performAction() {
        std::cout << "Foo::performAction\n";
    }

private:
};

}  // namespace Foo

#endif // FOO_H
