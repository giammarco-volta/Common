#pragma once
#include <memory>
#include "IMidiOut.h"

std::unique_ptr<IMidiOut> createMidiOut();
