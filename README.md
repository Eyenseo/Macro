## What is this?

This is the result of my [bachelor thesis](https://github.com/Eyenseo/Bachelorthesis). There are some bugs present - I will not fix them as this repository is meant to represent what I developed and implemented in three months.

Have a look around if you are interested in _recursive decent parsers_, _abstract syntax trees_ and _interpreter_ written with C++11/14 and some features from C++17.
The code will not compile though - I have not provided the used libraries because most of them are proprietary - sorry =/

The [`IndentStream`](https://github.com/Eyenseo/Macro/blob/master/src/cad/macro/IndentStream.cpp) and [`IndentBuffer`](https://github.com/Eyenseo/Macro/blob/master/src/cad/macro/IndentBuffer.cpp) might be useful for quite a few things.


### Known bugs
 
 * [`Interpreter::SmartRef`](https://github.com/Eyenseo/Macro/blob/master/src/cad/macro/interpreter/Interpreter.cpp#L65)
    is very very bad! The `std::reference_wrapper` will point to a deleted object with MSVC after move/copy 
    ([Interpreter::ValueProducer](https://github.com/Eyenseo/Macro/blob/master/src/cad/macro/interpreter/Interpreter.cpp#L426)),
    Linux apparently uses the same memory so this bug didn't manifest. A `std::unique_ptr` with custom deleter is a better solution.  
 * In the [`ast::loop::For`](https://github.com/Eyenseo/Macro/blob/master/src/cad/macro/interpreter/Interpreter.cpp#L528) interpretation the return value from the scope is overwritten by the value from the header.
 * [`parser::Tokens`](https://github.com/Eyenseo/Macro/blob/master/src/cad/macro/parser/Parser.cpp#L41) That ref shouldn't be there as it will point to a deleted object - see [Line 2424](https://github.com/Eyenseo/Macro/blob/master/src/cad/macro/parser/Parser.cpp#L2424).
