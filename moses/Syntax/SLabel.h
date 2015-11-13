#pragma once

#include "moses/ScoreComponentCollection.h"
#include "moses/TargetPhrase.h"

namespace Moses
{
namespace Syntax
{

struct SLabel {
  float inputWeight;
  float score;
  ScoreComponentCollection scoreBreakdown;
  const TargetPhrase *translation;
};

}  // Syntax
}  // Moses
