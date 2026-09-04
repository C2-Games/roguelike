#include <exception>
#include <iostream>

#include "db/generate_database.h"

int main(int argc, char** argv)
{
  if (argc != 4)
  {
    std::cerr
        << "usage: generate_db <schema.sql path> <assets_dir> <db_path>\n";
    return 1;
  }

  try
  {
    db::generateDatabase(argv[1], argv[2], argv[3]);
  }
  catch (const std::exception& e)
  {
    std::cerr << "generate_db: " << e.what() << "\n";
    return 1;
  }

  return 0;
}
