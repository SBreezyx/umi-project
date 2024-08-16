#include "llvm/MCA/Context.h"

#include <catch2/catch_test_macros.hpp>

#include <llvm/IR/Module.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Attributes.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/Threading.h>
#include <llvm/Support/ThreadPool.h>

using namespace std;


template <typename T, integral Int = long>
class vec {
public:
  constexpr vec() = default;
  explicit vec(Int n) : sz_{n}, elems_{static_cast<T*>(malloc(sizeof(T) * n))} {}

  vec(vec &&other) noexcept {
      *this = std::move(other);
  }

  auto operator=(vec &&other) noexcept -> vec& {
    if (this != &other) {
      sz_ = exchange(other.sz_, 0);
      elems_ = exchange(other.elems_, nullptr);
    }

    return *this;
  }

  ~vec() {
    free(elems_);
  }

  auto operator[](Int i) -> T& {
    return elems_[i];
  }

  auto begin() -> T* {
    return elems_;
  }

  auto end() -> T* {
    return elems_ + sz_;
  }

private:
  Int sz_;
  T *elems_;
};


/// \note need to an in-order (or some kind of order??) walk of the logical AST.
/// store the traversal order as an index look-aside list
/// Is it necessarily important to aggressively maintain the original ordering
/// of the program into LLVM IR?
/// - It seems even Clang with various optimisation levels produces sometimes
/// radically different IR, implying that it does some analysis before emission.
/// Within this analysis, it likely is doing some form of re-ordering.
/// Since Clang is hardened and reliable, it means that it is *OK* to do reordering.
/// Thus, we should follow suit.

/// \note logical AST holds the order of the program
/// importing from _any_ module natural means that a specific module's
/// checks cannot be completed until the importee's check are.
/// in this chain of checks, there needs to be an ability to insert extra checks.
/// the scope of how many and what kind is yet to be determined.
/// this is not the whole story -- the AST is NOT equivalent to a CFG.
/// there will need to be a transform that takes the depth-first-generated AST
/// and converts it to a control flow graph replete with short-circuit evaluation...

/// \note by the time LLVM IR emission begins, imported names _need_ to already be
/// flattened down to their actual symbol name (incl. with mangling), and these need to be stored.
/// probably, storing by type is sufficient -- the code emitter woud be reading const data.

/// \note At the most global level, LLVM operates on modules.
/// it transforms the bitcode/LLVM IR repr _of a single module_ to obj code.
/// this means everything is scoped to a module.
/// which also means we _have_ to scope everything to a module.
/// but which also means we need to keep global information about every module,
/// just sorted by module.

/// all types should have been checked and logged by the time codegen rolls around

/// tentative naming
enum codegen_kinds {
  EXPORT_GLOBAL,
  IMPORT_GLOBAL,
  IMPORT_FN,
  FUN_DECL,
  FUN_DEF,
  CMP_STMT,
  // by this point, mutability would have been checked so we don't care about const
  VAR_DEF,
  FUN_CALL_EXPR,
  EXPR,
  LIT_EXPR,
  IDENT_EXPR,
  BIN_GT_EXPR,
  BIN_LT_EXPR,
  TRI_IF_EXPR,
  RET_STMT,
  IF_STMT,
};

enum module_ids : int {
  STD_ID,
  MAIN_ID,
  FIB_ID,
  N_MODULES,
};

static auto make_one_context_per_source_file(const int n_sources) -> vec<llvm::LLVMContext> {
  auto contexts = vec<llvm::LLVMContext>(n_sources);

  for (auto i = 0; i < n_sources; ++i) {
    new (contexts.begin() + i) llvm::LLVMContext();
  }

  return contexts;
}

static auto make_one_module_per_context(
  const int n_modules,
  const char * const * modules_names,
  llvm::LLVMContext contexts[]
) -> vec<llvm::Module> {
  auto modules = vec<llvm::Module>(n_modules);

  for (auto i = 0; i < n_modules; ++i) {
    new (modules.begin() + i) llvm::Module(modules_names[i], contexts[i]);
  }

  return modules;
}


struct fun_thing {
  int from_module;
  int fun_id;
};

static auto functions = vector {
  0
};


TEST_CASE("swag") {
  const auto source_file_names = vector {
    "std.umi",
    "main.umi",
    "fib.umi",
  };

  auto contexts = make_one_context_per_source_file(N_MODULES);
  auto modules = make_one_module_per_context(
    N_MODULES,
    source_file_names.data(),
    contexts.begin()
  );

  /// \note store export symbols by type.
  /*const auto imported_ints = array{
    pair{"_UN3std12EXIT_SUCCESSE", int32_t},
    pair{"_UN3std12EXIT_FAILUREE", int32_t},
  };

  /// checking this function came from the module fib would already be done as part of type checking.
  /// hence, this is simply a flat string.
  /// since umi supports using functions before they're declared (so long as they are declared further in the file),
  /// this list should already be finalized before emitting code.
  /// this would avoid the case where we try to look up functions in the llvm::Module, only for
  const auto imported_funs = array{
    tuple{
      "_UN3fib3bif3fibEii", llvm::FunctionType::get(int32_t, {int32_t}, false)
    }
  };

  const auto fun_decls = array {
    tuple{int32_t, "main", vector<llvm::Type*>{}},
  };

  const auto var_defs = array{
    tuple{int32_t, "num"},
  };*/

  const auto traversal_order = vector{
    vector {
      tuple{EXPORT_GLOBAL, 0},
    },
    vector {
      tuple{IMPORT_GLOBAL, 0},
      tuple{IMPORT_GLOBAL, 1},
      tuple{IMPORT_FN, 0},
      tuple{FUN_DECL, 0},
      tuple{CMP_STMT, 0},
      tuple{LIT_EXPR, 0},
      tuple{FUN_CALL_EXPR, 0},
      tuple{VAR_DEF, 0},

      // this needs to be re-evaluated when I do more LLVM-IR if-treatments
      tuple{TRI_IF_EXPR, 0},
      tuple{IDENT_EXPR, 0},
      tuple{LIT_EXPR, 0},
      tuple{BIN_GT_EXPR, 0},
      tuple{IDENT_EXPR, 1},
      tuple{IDENT_EXPR, 2},
      tuple{RET_STMT, 0},
    },
    vector {
      tuple{FUN_DECL, 0},
      tuple{CMP_STMT, 0},
      tuple{BIN_LT_EXPR, 0},
      tuple{IF_STMT, 0},

    },
  };

  llvm::StructType::create(
    contexts[MAIN_ID],
    {llvm::IntegerType::get(contexts[MAIN_ID], 32)},
    "swag"
    );

  modules[MAIN_ID].print(llvm::outs(), nullptr, true, true);

}
