#pragma once

#include <pex/PexAsmWriter.h>
#include <pex/PexString.h>
#include <pex/PexUserFlags.h>
#include <pex/PexValue.h>
#include <pex/PexWriter.h>

namespace caprica { namespace pex {

struct PexFile;

struct PexStructMember final
{
  PexString name{ };
  PexString typeName{ };
  PexUserFlags userFlags{ };
  PexValue defaultValue{ };
  bool isConst{ false };
  PexString documentationString{ };

  PexStructMember() = default;
  ~PexStructMember() = default;

  void write(PexWriter& wtr) const;
  void writeAsm(const PexFile* file, PexAsmWriter& wtr) const;
};

}}
