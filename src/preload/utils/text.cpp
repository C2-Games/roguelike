#include "preload/utils/text.h"

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace preload
{

std::string trim(std::string s)
{
  auto notSpace = [](unsigned char c) { return !std::isspace(c); };
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), notSpace));
  s.erase(std::find_if(s.rbegin(), s.rend(), notSpace).base(), s.end());
  return s;
}

// the room grids are authored in Unicode box-drawing glyphs, so a 175-column
// row can run to ~500 bytes; callers must iterate codepoints, not bytes.
// hand-rolled rather than using mbrtowc because preload runs before
// ui_manager.cpp calls setlocale.
std::vector<char32_t> decodeUtf8(const std::string& line,
                                 const std::filesystem::path& path, int row)
{
  auto fail = [&](unsigned char offending, const std::string& reason) {
    std::ostringstream oss;
    oss << "Malformed UTF-8 in row " << row << " of " << path.string() << " ("
        << reason << "): byte 0x" << std::hex << std::uppercase << std::setw(2)
        << std::setfill('0') << static_cast<unsigned int>(offending);
    throw std::runtime_error(oss.str());
  };

  auto byteAt = [&](std::size_t index) {
    return static_cast<unsigned char>(line[index]);
  };

  std::vector<char32_t> codepoints;
  const std::size_t size = line.size();
  for (std::size_t i = 0; i < size;)
  {
    const unsigned char lead = byteAt(i);
    char32_t codepoint = 0;
    std::size_t continuationBytes = 0;
    if (lead < 0x80)
    {
      codepoint = lead;
    }
    else if ((lead & 0xE0) == 0xC0)
    {
      codepoint = static_cast<char32_t>(lead & 0x1F);
      continuationBytes = 1;
    }
    else if ((lead & 0xF0) == 0xE0)
    {
      codepoint = static_cast<char32_t>(lead & 0x0F);
      continuationBytes = 2;
    }
    else if ((lead & 0xF8) == 0xF0)
    {
      codepoint = static_cast<char32_t>(lead & 0x07);
      continuationBytes = 3;
    }
    else
    {
      fail(lead, "invalid lead byte");
    }

    if (i + continuationBytes >= size)
    {
      fail(lead, "truncated multi-byte sequence");
    }
    for (std::size_t k = 1; k <= continuationBytes; ++k)
    {
      const unsigned char continuation = byteAt(i + k);
      if ((continuation & 0xC0) != 0x80)
      {
        fail(continuation, "expected continuation byte");
      }
      codepoint =
          static_cast<char32_t>((codepoint << 6) | (continuation & 0x3F));
    }

    // reject a codepoint encoded in more bytes than its minimum: the
    // per-length floors are the well-known UTF-8 bounds 0x80 / 0x800 /
    // 0x10000.
    if ((continuationBytes == 1 && codepoint < 0x80) ||
        (continuationBytes == 2 && codepoint < 0x800) ||
        (continuationBytes == 3 && codepoint < 0x10000))
    {
      fail(lead, "overlong encoding");
    }
    // reject values past the last Unicode codepoint (0x10FFFF) and the
    // UTF-16 surrogate block (0xD800-0xDFFF), which is never valid UTF-8.
    if (codepoint > 0x10FFFF || (codepoint >= 0xD800 && codepoint <= 0xDFFF))
    {
      fail(lead, "codepoint out of range");
    }

    codepoints.push_back(codepoint);
    i += continuationBytes + 1;
  }
  return codepoints;
}

}  // namespace preload
