#include <Catch/catch.hpp>

#include "cad/macro/ast/Scope.h"
#include "cad/macro/ast/logic/Condition.h"

using namespace cad::macro::parser;
using namespace cad::macro::ast;
using namespace cad::macro::ast::executable;
using namespace cad::macro::ast::logic;

TEST_CASE("AST Comparison") {
  SECTION("Empty") {
    AST a({0, 0, ""});
    AST b({0, 0, ""});
    REQUIRE(a == b);
  }
  SECTION("Line different") {
    AST a({0, 0, ""});
    AST b({1, 0, ""});
    REQUIRE_FALSE(a == b);
  }
  SECTION("Column different") {
    AST a({0, 0, ""});
    AST b({0, 2, ""});
    REQUIRE_FALSE(a == b);
  }
  SECTION("Token different") {
    AST a({0, 0, ""});
    AST b({0, 0, "Foo"});
    REQUIRE_FALSE(a == b);
  }
  SECTION("All different") {
    AST a({0, 0, ""});
    AST b({1, 1, "Foo"});
    REQUIRE_FALSE(a == b);
  }
}

TEST_CASE("Scope Comparison") {
  SECTION("Empty") {
    Scope a({0, 0, ""});
    Scope b({0, 0, ""});
    REQUIRE(a == b);
  }
  SECTION("Scope") {
    Scope a({0, 0, ""});
    a.nodes.emplace_back(Scope({0, 0, ""}));
    Scope b({0, 0, ""});
    b.nodes.emplace_back(Scope({0, 0, ""}));
    REQUIRE(a == b);
  }
  SECTION("Different Scope") {
    Scope a({0, 0, ""});
    a.nodes.emplace_back(Scope({0, 0, ""}));
    Scope b({0, 0, ""});
    b.nodes.emplace_back(Scope({1, 0, ""}));
    REQUIRE_FALSE(a == b);
  }
  SECTION("Scope/No-Scope") {
    Scope a({0, 0, ""});
    a.nodes.emplace_back(Scope({0, 0, ""}));
    Scope b({0, 0, ""});
    REQUIRE_FALSE(a == b);
  }
}

TEST_CASE("Executable Comparison") {
  SECTION("Parameterless") {
    Executable a({0, 0, ""});
    Executable b({0, 0, ""});
    REQUIRE(a == b);
  }
  SECTION("Parameter") {
    Executable a({0, 0, ""});
    a.parameter.emplace_back(Variable({0, 0, ""}));
    Executable b({0, 0, ""});
    b.parameter.emplace_back(Variable({0, 0, ""}));
    REQUIRE(a == b);
  }
  SECTION("Parameter/Parameterless") {
    Executable a({0, 0, ""});
    a.parameter.emplace_back(Variable({0, 0, ""}));
    Executable b({0, 0, ""});
    REQUIRE_FALSE(a == b);
  }
}

TEST_CASE("Function Comparison") {
  SECTION("No-Scope") {
    Function a({0, 0, ""});
    Function b({0, 0, ""});
    REQUIRE(a == b);
  }
  SECTION("Scope") {
    Function a({0, 0, ""});
    a.scope = std::make_unique<Scope>(Token(0, 0, ""));
    Function b({0, 0, ""});
    b.scope = std::make_unique<Scope>(Token(0, 0, ""));
    REQUIRE(a == b);
  }
  SECTION("Scope/No-Scope") {
    Function a({0, 0, ""});
    a.scope = std::make_unique<Scope>(Token(0, 0, ""));
    Function b({0, 0, ""});
    REQUIRE_FALSE(a == b);
  }
}

TEST_CASE("Define Comparison") {
  SECTION("No-Definition") {
    Define a({0, 0, ""});
    Define b({0, 0, ""});
    REQUIRE(a == b);
  }
  SECTION("Definition") {
    Define a({0, 0, ""});
    a.definition.emplace(Function({0, 0, ""}));
    Define b({0, 0, ""});
    b.definition.emplace(Function({0, 0, ""}));
    REQUIRE(a == b);
  }
  SECTION("Definition/No-Definition") {
    Define a({0, 0, ""});
    a.definition.emplace(Function({0, 0, ""}));
    Define b({0, 0, ""});
    REQUIRE_FALSE(a == b);
  }
}

TEST_CASE("Return Comparison") {
  SECTION("No-Definition") {
    Return a({0, 0, ""});
    Return b({0, 0, ""});
    REQUIRE(a == b);
  }
  SECTION("Definition") {
    Return a({0, 0, ""});
    a.output.emplace(Variable({0, 0, ""}));
    Return b({0, 0, ""});
    b.output.emplace(Variable({0, 0, ""}));
    REQUIRE(a == b);
  }
  SECTION("Different Definition") {
    Return a({0, 0, ""});
    a.output.emplace(Variable({0, 0, ""}));
    Return b({0, 0, ""});
    b.output.emplace(Executable({0, 0, ""}));
    REQUIRE_FALSE(a == b);
  }
  SECTION("Definition/No-Definition") {
    Return a({0, 0, ""});
    a.output.emplace(Variable({0, 0, ""}));
    Return b({0, 0, ""});
    REQUIRE_FALSE(a == b);
  }
}
TEST_CASE("Operator Comparison") {
  SECTION("Unary") {
    SECTION("No-Operand") {
      UnaryOperator a({0, 0, ""});
      UnaryOperator b({0, 0, ""});
      REQUIRE(a == b);
    }
    // Operand
    SECTION("Operand") {
      UnaryOperator a({0, 0, ""});
      a.operand = std::make_unique<Operand>();
      a.operand->operand = Variable({0, 0, ""});
      UnaryOperator b({0, 0, ""});
      b.operand = std::make_unique<Operand>();
      b.operand->operand = Variable({0, 0, ""});
      REQUIRE(a == b);
    }
    SECTION("Different Operand") {
      UnaryOperator a({0, 0, ""});
      a.operand = std::make_unique<Operand>();
      a.operand->operand = Variable({0, 0, ""});
      UnaryOperator b({0, 0, ""});
      b.operand = std::make_unique<Operand>();
      b.operand->operand = Executable({0, 0, ""});
      REQUIRE_FALSE(a == b);
    }
    SECTION("Operand/No-Operand") {
      UnaryOperator a({0, 0, ""});
      a.operand = std::make_unique<Operand>();
      a.operand->operand = Variable({0, 0, ""});
      UnaryOperator b({0, 0, ""});
      REQUIRE_FALSE(a == b);
    }
    // Operation
    SECTION("Operation") {
      UnaryOperator a({0, 0, ""});
      a.operation = UnaryOperation::NOT;
      UnaryOperator b({0, 0, ""});
      b.operation = UnaryOperation::NOT;
      REQUIRE(a == b);
    }
    SECTION("Operation/No-Operation") {
      UnaryOperator a({0, 0, ""});
      a.operation = UnaryOperation::NOT;
      UnaryOperator b({0, 0, ""});
      REQUIRE_FALSE(a == b);
    }
  }
  SECTION("Binary") {
    SECTION("No-Operand") {
      BinaryOperator a({0, 0, ""});
      BinaryOperator b({0, 0, ""});
      REQUIRE(a == b);
    }
    // Left
    SECTION("LeftOperand") {
      BinaryOperator a({0, 0, ""});
      a.left_operand = std::make_unique<Operand>();
      a.left_operand->operand = Variable({0, 0, ""});
      BinaryOperator b({0, 0, ""});
      b.left_operand = std::make_unique<Operand>();
      b.left_operand->operand = Variable({0, 0, ""});
      REQUIRE(a == b);
    }
    SECTION("Different LeftOperand") {
      BinaryOperator a({0, 0, ""});
      a.left_operand = std::make_unique<Operand>();
      a.left_operand->operand = Variable({0, 0, ""});
      BinaryOperator b({0, 0, ""});
      b.left_operand = std::make_unique<Operand>();
      b.left_operand->operand = Executable({0, 0, ""});
      REQUIRE_FALSE(a == b);
    }
    SECTION("LeftOperand/No-LeftOperand") {
      BinaryOperator a({0, 0, ""});
      a.left_operand = std::make_unique<Operand>();
      a.left_operand->operand = Variable({0, 0, ""});
      BinaryOperator b({0, 0, ""});
      REQUIRE_FALSE(a == b);
    }
    // Right
    SECTION("RightOperand") {
      BinaryOperator a({0, 0, ""});
      a.right_operand = std::make_unique<Operand>();
      a.right_operand->operand = Variable({0, 0, ""});
      BinaryOperator b({0, 0, ""});
      b.right_operand = std::make_unique<Operand>();
      b.right_operand->operand = Variable({0, 0, ""});
      REQUIRE(a == b);
    }
    SECTION("Different RightOperand") {
      BinaryOperator a({0, 0, ""});
      a.right_operand = std::make_unique<Operand>();
      a.right_operand->operand = Variable({0, 0, ""});
      BinaryOperator b({0, 0, ""});
      b.right_operand = std::make_unique<Operand>();
      b.right_operand->operand = Executable({0, 0, ""});
      REQUIRE_FALSE(a == b);
    }
    SECTION("RightOperand/No-RightOperand") {
      BinaryOperator a({0, 0, ""});
      a.right_operand = std::make_unique<Operand>();
      a.right_operand->operand = Variable({0, 0, ""});
      BinaryOperator b({0, 0, ""});
      REQUIRE_FALSE(a == b);
    }
    // Both
    SECTION("Right-/LeftOperand") {
      BinaryOperator a({0, 0, ""});
      a.left_operand = std::make_unique<Operand>();
      a.left_operand->operand = Variable({0, 0, ""});
      a.right_operand = std::make_unique<Operand>();
      a.right_operand->operand = Variable({0, 0, ""});
      BinaryOperator b({0, 0, ""});
      b.left_operand = std::make_unique<Operand>();
      b.left_operand->operand = Variable({0, 0, ""});
      b.right_operand = std::make_unique<Operand>();
      b.right_operand->operand = Variable({0, 0, ""});
      REQUIRE(a == b);
    }
    SECTION("No Right-/LeftOperand") {
      BinaryOperator a({0, 0, ""});
      a.left_operand = std::make_unique<Operand>();
      a.left_operand->operand = Variable({0, 0, ""});
      BinaryOperator b({0, 0, ""});
      b.left_operand = std::make_unique<Operand>();
      b.left_operand->operand = Variable({0, 0, ""});
      b.right_operand = std::make_unique<Operand>();
      b.right_operand->operand = Variable({0, 0, ""});
      REQUIRE_FALSE(a == b);
    }
    SECTION("Right-/ No-LeftOperand") {
      BinaryOperator a({0, 0, ""});
      a.right_operand = std::make_unique<Operand>();
      a.right_operand->operand = Variable({0, 0, ""});
      BinaryOperator b({0, 0, ""});
      b.left_operand = std::make_unique<Operand>();
      b.left_operand->operand = Variable({0, 0, ""});
      b.right_operand = std::make_unique<Operand>();
      b.right_operand->operand = Variable({0, 0, ""});
      REQUIRE_FALSE(a == b);
    }
  }
}

TEST_CASE("Condition") {
  SECTION("No-Definition") {
    Condition a({0, 0, ""});
    Condition b({0, 0, ""});
    REQUIRE(a == b);
  }
  SECTION("Definition") {
    Condition a({0, 0, ""});
    a.condition = UnaryOperator({0, 0, ""});
    Condition b({0, 0, ""});
    b.condition = UnaryOperator({0, 0, ""});
    REQUIRE(a == b);
  }
  SECTION("Different Definition") {
    Condition a({0, 0, ""});
    a.condition = UnaryOperator({0, 0, ""});
    Condition b({0, 0, ""});
    b.condition = BinaryOperator({0, 0, ""});
    REQUIRE_FALSE(a == b);
  }
  SECTION("Definition/No-Definition") {
    Condition a({0, 0, ""});
    a.condition = UnaryOperator({0, 0, ""});
    Condition b({0, 0, ""});
    REQUIRE_FALSE(a == b);
  }
}

TEST_CASE("If") {
  SECTION("No-Scope") {
    If a({0, 0, ""});
    If b({0, 0, ""});
    REQUIRE(a == b);
  }
  // true
  SECTION("TrueScope") {
    If a({0, 0, ""});
    a.true_scope = std::make_unique<Scope>(Token(0, 0, ""));
    If b({0, 0, ""});
    b.true_scope = std::make_unique<Scope>(Token(0, 0, ""));
    REQUIRE(a == b);
  }
  SECTION("TrueScope/No-TrueScope") {
    If a({0, 0, ""});
    a.true_scope = std::make_unique<Scope>(Token(0, 0, ""));
    If b({0, 0, ""});
    REQUIRE_FALSE(a == b);
  }
  // False
  SECTION("FalseScope") {
    If a({0, 0, ""});
    a.false_scope = std::make_unique<Scope>(Token(0, 0, ""));
    If b({0, 0, ""});
    b.false_scope = std::make_unique<Scope>(Token(0, 0, ""));
    REQUIRE(a == b);
  }
  SECTION("FalseScope/No-FalseScope") {
    If a({0, 0, ""});
    a.false_scope = std::make_unique<Scope>(Token(0, 0, ""));
    If b({0, 0, ""});
    REQUIRE_FALSE(a == b);
  }
  // Both
  SECTION("Right-/TrueScope") {
    If a({0, 0, ""});
    a.true_scope = std::make_unique<Scope>(Token(0, 0, ""));
    a.false_scope = std::make_unique<Scope>(Token(0, 0, ""));
    If b({0, 0, ""});
    b.true_scope = std::make_unique<Scope>(Token(0, 0, ""));
    b.false_scope = std::make_unique<Scope>(Token(0, 0, ""));
    REQUIRE(a == b);
  }
  SECTION("No Right-/TrueScope") {
    If a({0, 0, ""});
    a.true_scope = std::make_unique<Scope>(Token(0, 0, ""));
    If b({0, 0, ""});
    b.true_scope = std::make_unique<Scope>(Token(0, 0, ""));
    b.false_scope = std::make_unique<Scope>(Token(0, 0, ""));
    REQUIRE_FALSE(a == b);
  }
  SECTION("Right-/ No-TrueScope") {
    If a({0, 0, ""});
    a.false_scope = std::make_unique<Scope>(Token(0, 0, ""));
    If b({0, 0, ""});
    b.true_scope = std::make_unique<Scope>(Token(0, 0, ""));
    b.false_scope = std::make_unique<Scope>(Token(0, 0, ""));
    REQUIRE_FALSE(a == b);
  }
}
