#pragma once
#include <memory>
#include "MidiMonoIn.h"

std::unique_ptr<MidiIn_MonoInterpreter> createMidiIn();
