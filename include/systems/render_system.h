#pragma once

#include "NonCopyable.h"

class RenderSystem : public NonCopyable {
public:
	virtual void render() = 0;
};