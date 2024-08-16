#include "umi/id.h"
#include <umi/lexer.h>
#include "umi/parser.h"
#include "codegen.hpp"

#include <algorithm/algo.h>
#include <unicode/utf8.h>

#include <algorithm>
#include <array>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <map>
#include <memory>
#include <string>
#include <thread>
#include <vector>


//===----------------------------------------------------------------------===//
// Top-Level parsing and JIT Driver
//===----------------------------------------------------------------------===//

namespace umi {
  id_generator<id_type> id_gen;

  /// BinopPrecedence - This holds the precedence for each binary operator that is defined.
  std::map<token_kind, int> binop_precedence;
}

// Install standard binary operators.
// 1 is lowest precedence.
auto init_binop_precedence(
    std::map<token_kind, int> &mp = umi::binop_precedence) -> void {
  using
  enum token_kind;

  mp.insert({token_kind::left_angle, 10});
  mp.insert({token_kind::plus, 20});
  mp.insert({token_kind::minus, 20});
//  mp.insert({token_kind::a, 40});
}

//static void InitializeModule() {
//  // Open a new context and module.
//  TheContext = std::make_unique<LLVMContext>();
//  TheModule = std::make_unique<Module>("my cool jit", *TheContext);
//
//  // Create a new builder for the module.
//  Builder = std::make_unique<IRBuilder<>>(*TheContext);
//}
//
//static void HandleDefinition() {
//  if (auto FnAST = ParseDefinition()) {
//    if (auto *FnIR = FnAST->codegen()) {
//      fprintf(stderr, "Read function definition:");
//      FnIR->print(errs());
//      fprintf(stderr, "\n");
//    }
//  } else {
//    // Skip token for error recovery.
//    getNextToken();
//  }
//}
//
//static void HandleExtern() {
//  if (auto ProtoAST = ParseExtern()) {
//    if (auto *FnIR = ProtoAST->codegen()) {
//      fprintf(stderr, "Read extern: ");
//      FnIR->print(errs());
//      fprintf(stderr, "\n");
//    }
//  } else {
//    // Skip token for error recovery.
//    getNextToken();
//  }
//}
//
//static void HandleTopLevelExpression() {
//  // Evaluate a top-level expression into an anonymous function.
//  if (auto FnAST = ParseTopLevelExpr()) {
//    if (auto *FnIR = FnAST->codegen()) {
//      fprintf(stderr, "Read top-level expression:");
//      FnIR->print(errs());
//      fprintf(stderr, "\n");
//
//      // Remove the anonymous expression.
//      FnIR->eraseFromParent();
//    }
//  } else {
//    // Skip token for error recovery.
//    getNextToken();
//  }
//}

auto lexically_analyse(lexed &l, const std::vector<utf8::string> &sources) {
  // properly initialize l
  for (auto i = 0; i < sources.size(); ++i) {
    l.sources[i] = {};
    l.kinds[i] = {};
  }

  // spawn worker threads
  auto workers = std::vector<std::thread>{};
  for (auto i = 0; i < std::thread::hardware_concurrency(); ++i) {
    workers.emplace_back(

    );
  }

  // process

  // gather and finish
  for (auto &thr : workers) {
    thr.join();
  }
}

//===----------------------------------------------------------------------===//
// Main driver code.
//===----------------------------------------------------------------------===//
int main() {
  init_binop_precedence();

  auto m1 = std::ifstream{"main.umi"};
  auto m2 = std::ifstream{"fib.umi"};

  auto sources = nonstd::map(std::array{
      &m1, &m2
  }, [](std::ifstream *file) {
    using iter = std::istream_iterator<char>;
    auto s = std::string{iter(*file), iter{}};
    return utf8::string{s.data(), static_cast<ssize_t>(s.size())};
  });

  auto lex = lexed{};

  lexically_analyse(lex, sources);

  // Make the module, which holds all the code.
//  InitializeModule();

  // Run the main "interpreter loop" now.
//  MainLoop();

  // Print out all the generated code.
  TheModule->print(errs(), nullptr);

  return 0;
}