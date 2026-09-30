#pragma once
#include "what_changed/model.hpp"
namespace wc { Diff compare(const Snapshot&, const Snapshot&, bool include_transient=false); }
