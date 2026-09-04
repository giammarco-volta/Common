#pragma once
#include <memory>
#include "MidiMonoIn.h"

std::unique_ptr<MidiIn_MonoInterpreter> createMidiIn(const MidiIn_MonoInterpreter::Configuration& configuration);
