#pragma once

#include "ICloser.hpp"
#include "IReader.hpp"

class IReadCloser : public ICloser, public IReader {};
