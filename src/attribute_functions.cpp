#ifndef __cplusplus
#error This file must be compiled as C++
#endif
#include "context.h"

//---------------------------------------------------------------------------
// Attribute functions
//---------------------------------------------------------------------------

void sglClearColor(float r, float g, float b, float alpha) {
	if (contextNotInitialized() || calledWithinBeginEnd()) {
		return;
	}
    sceneManager->getCurrentContext().clearColor = Pixel(r, g, b);
}

void sglColor3f(float r, float g, float b) {
	if (contextNotInitialized() || calledWithinBeginEnd()) {
		return;
	}
    sceneManager->getCurrentContext().currentColor = Pixel(r, g, b);
}

void sglAreaMode(sglEAreaMode mode) {
	if (contextNotInitialized() || 
        calledWithinBeginEnd()  || 
        isInvalidEnumValue(mode)) {
		return;
	}
    sceneManager->getCurrentContext().currentAreaMode = mode;
}

void sglPointSize(float size) {
	if (contextNotInitialized() || calledWithinBeginEnd()) {
		return;
	}

	if (size <= 0) {
		setErrCode(SGL_INVALID_VALUE);
		return;
	}

    sceneManager->getCurrentContext().pointSize = size;
}

void sglEnable(sglEEnableFlags cap) {
	if (contextNotInitialized() || calledWithinBeginEnd()) {
		return;
	}

	if (cap != SGL_DEPTH_TEST) {
		setErrCode(SGL_INVALID_ENUM);
		return;
	}
	else {
        sceneManager->getCurrentContext().enabledDepthTest = true;
	}
}

void sglDisable(sglEEnableFlags cap) {
	if (contextNotInitialized() || calledWithinBeginEnd()) {
		return;
	}

	if (cap != SGL_DEPTH_TEST) {
		setErrCode(SGL_INVALID_ENUM);
		return;
	}
	else {
        sceneManager->getCurrentContext().enabledDepthTest = false;
	}
}
