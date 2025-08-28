#pragma once

#include "sgl.h"
#include "structures.h"
#include "scene.h"
#include "ray_tracing_utils.h"
#include <vector>
#include <memory>
#include <type_traits>
#include <thread>

#define M_PI       3.14159265358979323846   // pi

using std::unique_ptr;
using std::is_enum;
using std::vector;
using std::move;

struct SGLContext {
    int width;
    int height;

	unique_ptr<vector<Pixel>> colorBuffer;
	unique_ptr<vector<float>> depthBuffer;

    sglEElementType currentPrimitiveMode;
    sglEAreaMode currentAreaMode;
    sglEMatrixMode currentMatrixMode;

	Pixel currentColor;
	Pixel clearColor;
	float pointSize;
    float scaleFactor;

	bool insideBegin;
	bool enabledDepthTest;

    unique_ptr<vector<vector<Matrix>>> transformationStack;
	unique_ptr<vector<Vertex>> verticesList;
    unique_ptr<vector<ScreenVertex>> screenVertices;
    
    Matrix viewportMatrix;
    Matrix VPMmatrix;

	Scene scene;
	bool insideBeginScene;

	SGLContext(int width, int height);
};

struct SGLSceneManager {
	int currentContextId;
	vector<unique_ptr<SGLContext>> contexts;

	sglEErrorCode errorCode;

	SGLSceneManager();

	auto& getCurrentContext() {
		return *(contexts[currentContextId]);
	}
};

extern unique_ptr<SGLSceneManager> sceneManager;

//---------------------------------------------------------------------------
// Check utils
//---------------------------------------------------------------------------

void setErrCode(sglEErrorCode c);

/// Checks if operation can be done on actual context.
/**
  Returns true if sceneManager or actual context is not initialized.
  Otherwise return false and sets SGL_INVALID_OPERATION error.
*/
bool contextNotInitialized();

/// Checks if operation is called within a sglBegin() / sglEnd() sequence.
/**
  Returns true if operation was called within said sequence.
  Otherwise return false and sets SGL_INVALID_OPERATION error.
*/
bool calledWithinBeginEnd();

/// Checks if operation is called within a sglBegin() / sglEnd() sequence.
/**
  Returns true if operation was called outside said sequence.
  Otherwise return false and sets SGL_INVALID_OPERATION error.
*/
bool calledOutsideBeginEnd();

/// Checks if operation is called within a sglBeginScene() / sglEndScene() sequence.
/**
  Returns true if operation was called within said sequence.
  Otherwise return false and sets SGL_INVALID_OPERATION error.
*/
bool calledWithinBeginSceneEndScene();

/// Checks if operation is called within a sglBeginScene() / sglEndScene() sequence.
/**
  Returns true if operation was called outside said sequence.
  Otherwise return false and sets SGL_INVALID_OPERATION error.
*/
bool calledOutsideBeginSceneEndScene();

/// Checks if enum has an accepted value.
/**
  Returns true if enum does not have an acepted value.
  Otherwise return false and sets SGL_INVALID_ENUM error.

  @param value [in] the value to be checked
*/
template <typename EnumType>
bool isInvalidEnumValue(EnumType value) {
	// implementation of function has to be done in header file
	// because compiler can't link function using template in multiple files together
	if (!is_enum<EnumType>::value) {
		setErrCode(SGL_INVALID_ENUM);
		return true;
	}
	return false;
}

void recalculateVPMMatrix();

