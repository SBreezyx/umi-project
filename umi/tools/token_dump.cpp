#include <umi/parse/lexer.h>

#include <fstream>
#include <iostream>

using namespace std;

auto main(int argc, char **argv) -> int {
  printf("\13\13");
  return 0;


  if (argc < 2) {
    printf("Usage: <%s> <file_to_dump> [-name]", argv[0]);
    return EXIT_FAILURE;
  }

  const auto print_name = argc == 3;

  const auto sources = ({
      auto file = fstream{argv[1]};
      using iter = istream_iterator<char>;

      auto s = string{iter{file >> noskipws}, iter{}};
      auto ss = umi::string {s.data(), (umi::ssize_t )s.size()};
      vector<umi::string>{ss};
  });

  cout << "{\n\t";
  const auto res = umi::tokenize(sources);
  for (const auto tok : res.tokens[0]) {
    if (print_name) {
      cout << umi::tokenc2str(tok);
    } else {
      cout << tok;
    }
    cout << ", \n\t";
  }
  cout << "\b}" << endl;

  cout << "{\n\t";
  for (const auto &[k, v] : res.common_strings) {
    cout << "{" << k << ", " << v << "},\n\t";
  }
  cout << "\b}";

  return EXIT_SUCCESS;
}